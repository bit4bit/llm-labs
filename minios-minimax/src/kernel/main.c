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

    void* test_frame = pmm_alloc_frame();
    DEBUG_INFO("Allocated test frame at: 0x%X", (uint32_t)test_frame);

    pmm_free_frame(test_frame);
    DEBUG_INFO("Freed test frame");

    DEBUG_INFO("Free frames after: %u", pmm_get_free_count());

    tss_init();
    process_init();

    DEBUG_INFO("Testing memory at user program base...");
    volatile uint32_t* test_ptr = (volatile uint32_t*)USER_PROGRAM_BASE;

    DEBUG_INFO("Test: Reading initial value...");
    uint32_t initial_val = *test_ptr;
    DEBUG_INFO("Test: Initial value = 0x%X", initial_val);

    DEBUG_INFO("Test: Writing 0xDEADBEEF...");
    *test_ptr = 0xDEADBEEF;

    DEBUG_INFO("Test: Reading back...");
    uint32_t read_val = *test_ptr;
    DEBUG_INFO("Test: Read value = 0x%X", read_val);

    if (read_val == 0xDEADBEEF) {
        DEBUG_INFO("Test: Write/read OK!");
    } else {
        DEBUG_ERROR("Test: FAILED - memory not working!");
    }

    DEBUG_INFO("[SELFCHECK] Starting scheduler self-check...");

    pcb_t* p1 = process_create("selfcheck", USER_PROGRAM_BASE);
    if (!p1) {
        DEBUG_ERROR("[SELFCHECK] FAILED: Could not create process");
        while (1) __asm__ volatile ("hlt");
    }

    if (process_load(p1, selfcheck_bin, selfcheck_bin_size) != 0) {
        DEBUG_ERROR("[SELFCHECK] FAILED: Could not load program");
        while (1) __asm__ volatile ("hlt");
    }

    uint32_t run_count_before = p1->run_count;
    (void)run_count_before;

    DEBUG_INFO("[SELFCHECK] Running test (PID %u)...", p1->id);
    process_start(p1);

    // Should never reach here if process_start works correctly
    while (1) __asm__ volatile ("hlt");
}

void process_exit_return(void) {
    uint32_t total_runs = 0;
    uint32_t exited_count = 0;

    for (uint32_t i = 0; i < process_table.count; i++) {
        pcb_t* p = &process_table.processes[i];
        if (p->state == PROC_EXITED) {
            exited_count++;
        }
        total_runs += p->run_count;
    }

    DEBUG_INFO("[SELFCHECK] Test completed");
    DEBUG_INFO("[SELFCHECK] Run count: %u", total_runs);

    if (total_runs >= 2) {
        DEBUG_INFO("[SELFCHECK] PASSED: Scheduler working correctly");
    } else {
        DEBUG_ERROR("[SELFCHECK] FAILED: Expected >= 2 context switches, got %u", total_runs);
    }

    while (1) __asm__ volatile ("hlt");
}