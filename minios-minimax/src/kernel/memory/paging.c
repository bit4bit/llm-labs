#include "paging.h"
#include "memory.h"
#include "../kernel.h"
#include "../cpu/constants.h"

page_directory_entry_t* kernel_directory = NULL;

static void set_page_directory_entry(page_directory_entry_t* entry, uint32_t physical_addr, int user, int size_4mb) {
    entry->present = 1;
    entry->rw = 1;
    entry->user = user ? 1 : 0;
    entry->ps = size_4mb ? 1 : 0;
    entry->base = physical_addr >> 12;
}

void paging_switch(page_directory_entry_t* dir) {
    __asm__ volatile ("mov %0, %%cr3" : : "r"(dir));
}

void enable_paging(void) {
    uint32_t cr0;
    __asm__ volatile ("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    __asm__ volatile ("mov %0, %%cr0" : : "r"(cr0));
}

void paging_init(void) {
    serial_print("Paging: Initializing...\n");

    kernel_directory = (page_directory_entry_t*)pmm_alloc_frame();

    serial_print("Paging: Kernel directory at 0x");
    serial_print_uint((uint32_t)kernel_directory);
    serial_print("\n");

    for (int i = 0; i < 1024; i++) {
        kernel_directory[i].present = 0;
        kernel_directory[i].rw = 0;
        kernel_directory[i].user = 0;
        kernel_directory[i].ps = 0;
        kernel_directory[i].base = 0;
    }

    set_page_directory_entry(&kernel_directory[0], 0, 0, 1);
    set_page_directory_entry(&kernel_directory[1], 0x400000, 0, 1);
    set_page_directory_entry(&kernel_directory[2], 0x800000, 0, 1);
    set_page_directory_entry(&kernel_directory[3], 0xC00000, 0, 1);

    serial_print("Paging: Identity mapping 0MB-12MB\n");

    paging_switch(kernel_directory);
    enable_paging();

    serial_print("Paging: Enabled\n");
}
