#include "tss.h"
#include "gdt.h"
#include "constants.h"
#include "../kernel.h"
#include "../minios.h"

#define GDT_DESC_TYPE_TSS 0x89  /* Available 32-bit TSS */

extern void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);
extern void serial_print_hex(uint32_t val);

struct tss_entry tss_entry;

void tss_init(void) {
    serial_print("TSS: Initializing...\n");

    serial_print("TSS: Setting up TSS entry at 0x");
    serial_print_hex((uint32_t)&tss_entry);
    serial_print("\n");

    tss_entry.ss0 = KERNEL_DATA_SELECTOR;
    /* esp0 will be set per-process by scheduler before any user-to-kernel
     * transition.  Initialise to 0 as a safe sentinel. */
    tss_entry.esp0 = 0;
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

    uint32_t tss_base = (uint32_t)&tss_entry;
    uint32_t tss_limit = sizeof(tss_entry) - 1;
    uint8_t tss_access = GDT_DESC_PRESENT | GDT_DESC_TYPE_TSS;

    serial_print("TSS: Configuring GDT entry 5...\n");
    serial_print("TSS:   base=0x");
    serial_print_hex(tss_base);
    serial_print("\n");
    serial_print("TSS:   limit=0x");
    serial_print_hex(tss_limit);
    serial_print("\n");
    serial_print("TSS:   access=0x");
    serial_print_hex(tss_access);
    serial_print("\n");
    serial_print("TSS:   selector=0x");
    serial_print_hex(TSS_SELECTOR);
    serial_print("\n");

    gdt_set_gate(5, tss_base, tss_limit, tss_access, 0);

    serial_print("TSS: Loading TR with selector 0x");
    serial_print_hex(TSS_SELECTOR);
    serial_print("...\n");

    __asm__ volatile ("ltr %0" : : "r"((uint16_t)TSS_SELECTOR));

    serial_print("TSS: Loaded\n");
}

void tss_set_stack(uint32_t kstack) {
    tss_entry.esp0 = kstack;
}
