#include "tss.h"
#include "gdt.h"
#include "constants.h"
#include "../kernel.h"

#define GDT_DESC_TYPE_TSS 0x09

extern void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);

struct tss_entry tss_entry;

void tss_init(void) {
    serial_print("TSS: Initializing...\n");

    tss_entry.ss0 = KERNEL_DATA_SELECTOR;
    tss_entry.esp0 = 0xBFFFF000;
    tss_entry.cr3 = 0;
    tss_entry.eip = 0;
    tss_entry.eflags = 0;
    tss_entry.eax = 0;
    tss_entry.ecx = 0;
    tss_entry.edx = 0;
    tss_entry.ebx = 0;
    tss_entry.esp = 0;
    tss_entry.ebp = 0;
    tss_entry.esi = 0;
    tss_entry.edi = 0;
    tss_entry.es = 0;
    tss_entry.cs = 0;
    tss_entry.ss = 0;
    tss_entry.ds = 0;
    tss_entry.fs = 0;
    tss_entry.gs = 0;
    tss_entry.ldt_selector = 0;
    tss_entry.trap = 0;
    tss_entry.iomap_base = sizeof(struct tss_entry);

    gdt_set_gate(5, (uint32_t)&tss_entry, sizeof(tss_entry) - 1,
                 GDT_DESC_PRESENT | GDT_DESC_TYPE_TSS, 0);

    __asm__ volatile ("ltr %0" : : "r"((uint16_t)TSS_SELECTOR));

    serial_print("TSS: Loaded\n");
}

void tss_set_stack(uint32_t kstack) {
    tss_entry.esp0 = kstack;
}
