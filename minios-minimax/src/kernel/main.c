#include <stdint.h>
#include "kernel.h"
#include "minios.h"
#include "memory/memory.h"
#include "cpu/gdt.h"
#include "cpu/idt.h"
#include "cpu/tss.h"
#include "cpu/interrupts.h"
#include "memory/vmm.h"
#include "process/process.h"
#include "syscall/syscall.h"
#include "debug.h"

#include "programs.h"

extern process_table_t process_table;
extern volatile int all_processes_exited;

void process_exit_return(void);

void kernel_main(multiboot_info_t* mbd) {
    volatile uint16_t* vga = (volatile uint16_t*)VGA_MEMORY;
    const char* message = "MinOS Loaded";
    uint8_t color = 0x0A;

    DEBUG_INFO("Kernel starting...");

    pmm_init(mbd);
    pmm_test();
    DEBUG_INFO("PMM initialized");
    DEBUG_INFO("Free frames: %u", pmm_get_free_count());

    gdt_init();
    DEBUG_INFO("GDT initialized");

    idt_init();
    DEBUG_INFO("IDT initialized");

    pit_init();

    vmm_init();

    for (int i = 0; message[i] != '\0'; i++) {
        vga[i] = (color << 8) | message[i];
    }

    DEBUG_INFO("MinOS Loaded");

    tss_init();
    process_init();

    /* ---- Create processes ----
     * Order must match alphabetical order used by build-programs.rb
     * so that each binary's link address matches its process index:
     *   index 0 = hello     -> 0x40000000
     *   index 1 = selfcheck -> 0x40400000
     */

    DEBUG_INFO("[BOOT] Creating hello process...");
    pcb_t* p0 = process_create("hello", 0);
    if (!p0) {
        DEBUG_ERROR("[BOOT] FAILED: Could not create hello process");
        while (1) __asm__ volatile ("hlt");
    }
    if (process_load(p0, hello_bin, hello_bin_size) != 0) {
        DEBUG_ERROR("[BOOT] FAILED: Could not load hello program");
        while (1) __asm__ volatile ("hlt");
    }

    DEBUG_INFO("[BOOT] Creating selfcheck process...");
    pcb_t* p1 = process_create("selfcheck", 0);
    if (!p1) {
        DEBUG_ERROR("[BOOT] FAILED: Could not create selfcheck process");
        while (1) __asm__ volatile ("hlt");
    }
    if (process_load(p1, selfcheck_bin, selfcheck_bin_size) != 0) {
        DEBUG_ERROR("[BOOT] FAILED: Could not load selfcheck program");
        while (1) __asm__ volatile ("hlt");
    }

    DEBUG_INFO("[BOOT] %u processes created, enabling interrupts...", process_table.count);

    /* Enable interrupts and enter idle loop.
     * The scheduler will pick the first READY process on the first
     * timer tick (current_process == NULL at that point). */
    __asm__ volatile ("sti");

    while (1) {
        if (all_processes_exited) {
            break;
        }
        __asm__ volatile ("hlt");
    }

    /* All processes have exited — run the self-check report */
    process_exit_return();
}

void process_exit_return(void) {
    uint32_t total_runs = 0;
    uint32_t exited_count = 0;

    for (uint32_t i = 0; i < process_table.count; i++) {
        pcb_t* p = &process_table.processes[i];
        DEBUG_INFO("[SELFCHECK] Process %s (PID %u): state=%u run_count=%u",
                   p->name, p->id, p->state, p->run_count);
        if (p->state == PROC_EXITED) {
            exited_count++;
        }
        total_runs += p->run_count;
    }

    DEBUG_INFO("[SELFCHECK] Test completed");
    DEBUG_INFO("[SELFCHECK] Exited: %u/%u, total run_count: %u",
               exited_count, process_table.count, total_runs);

    if (exited_count == process_table.count && total_runs >= 2) {
        DEBUG_INFO("[SELFCHECK] PASSED: Scheduler working correctly");
    } else if (exited_count != process_table.count) {
        DEBUG_ERROR("[SELFCHECK] FAILED: Not all processes exited (%u/%u)",
                    exited_count, process_table.count);
    } else {
        DEBUG_ERROR("[SELFCHECK] FAILED: Expected >= 2 context switches, got %u", total_runs);
    }

    while (1) __asm__ volatile ("hlt");
}
