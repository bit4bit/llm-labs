#include <stdint.h>
#include "kernel.h"
#include "minios.h"
#include "memory/memory.h"
#include "cpu/gdt.h"
#include "cpu/idt.h"
#include "cpu/tss.h"
#include "memory/paging.h"
#include "process/process.h"
#include "syscall/syscall.h"
#include "debug.h"

#include "programs.h"

void kernel_main(multiboot_info_t* mbd) {
    volatile uint16_t* vga = (volatile uint16_t*)VGA_MEMORY;
    const char* message = "MinOS Loaded";
    uint8_t color = 0x07;

    DEBUG_INFO("Kernel starting...");

    pmm_init(mbd);
    pmm_test();
    DEBUG_INFO("PMM initialized");
    DEBUG_INFO("Free frames: %u", pmm_get_free_count());

    gdt_init();
    DEBUG_INFO("GDT initialized");

    idt_init();
    DEBUG_INFO("IDT initialized");

    paging_init();
    paging_test();
    paging_extended_test();

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
    
    pcb_t* hello = process_create("hello", USER_PROGRAM_BASE);
    if (hello == NULL) {
        DEBUG_ERROR("Could not create hello process");
        while (1) __asm__ volatile ("hlt");
    }

    if (process_load(hello, hello_bin, hello_bin_size) != 0) {
        DEBUG_ERROR("Could not load hello program");
        while (1) __asm__ volatile ("hlt");
    }

    DEBUG_INFO("About to call process_start...");
    process_start(hello);

    DEBUG_ERROR("process_start returned unexpectedly");
    while (1) {
        __asm__ volatile ("hlt");
    }
}
