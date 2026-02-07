#include "gdt.h"
#include "constants.h"
#include "../kernel.h"

struct gdt_entry gdt_entries[GDT_ENTRIES];
struct gdt_ptr gdt_ptr;

void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt_entries[num].base_low = (base & 0xFFFF);
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;
    gdt_entries[num].base_high = (base >> 24) & 0xFF;

    gdt_entries[num].limit_low = (limit & 0xFFFF);
    gdt_entries[num].granularity = (limit >> 16) & 0x0F;
    gdt_entries[num].granularity |= (gran & 0xF0);

    gdt_entries[num].access = access;
}

void gdt_init(void) {
    serial_print("GDT: Initializing...\n");

    gdt_ptr.limit = sizeof(gdt_entries) - 1;
    gdt_ptr.base = (uint32_t)gdt_entries;

    gdt_set_gate(0, 0, 0, 0, 0);
    gdt_set_gate(1, 0, GDT_LIMIT_4GB, GDT_DESC_TYPE_CODE, GDT_DESC_GRANULARITY);
    gdt_set_gate(2, 0, GDT_LIMIT_4GB, GDT_DESC_TYPE_DATA, GDT_DESC_GRANULARITY);
    gdt_set_gate(3, 0, GDT_LIMIT_4GB, GDT_DESC_PRESENT | GDT_DESC_DPL3 | 0x1B, GDT_DESC_GRANULARITY);  /* User code: Present, DPL=3, Code, Executable, Readable, Accessed */
    gdt_set_gate(4, 0, GDT_LIMIT_4GB, GDT_DESC_PRESENT | GDT_DESC_DPL3 | 0x13, GDT_DESC_GRANULARITY);  /* User data: Present, DPL=3, Data, Writable, Accessed */

    __asm__ volatile ("lgdt %0" : : "m"(gdt_ptr));

    __asm__ volatile (
        "movw %0, %%ax\n"
        "movw %%ax, %%ds\n"
        "movw %%ax, %%es\n"
        "movw %%ax, %%fs\n"
        "movw %%ax, %%gs\n"
        "ljmp %1, $1f\n"
        "1:"
        :
        : "r"((uint16_t)KERNEL_DATA_SELECTOR), "i"((uint16_t)KERNEL_CODE_SELECTOR)
    );

    serial_print("GDT: Loaded\n");
}
