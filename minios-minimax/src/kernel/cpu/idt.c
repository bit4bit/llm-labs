#include "idt.h"
#include "constants.h"
#include "../kernel.h"

struct idt_entry idt_entries[IDT_ENTRIES];
struct idt_ptr idt_ptr;

static void default_handler(void) {
    __asm__ volatile ("hlt");
}

void idt_set_gate(uint8_t num, uint32_t handler) {
    idt_entries[num].offset_low = (handler & 0xFFFF);
    idt_entries[num].selector = KERNEL_CODE_SELECTOR;
    idt_entries[num].zero = 0;
    idt_entries[num].flags = IDT_DESC_PRESENT | IDT_DESC_TYPE_INTERRUPT;
    idt_entries[num].offset_high = (handler >> 16) & 0xFFFF;
}

extern void handle_page_fault(void);

void idt_init(void) {
    serial_print("IDT: Initializing...\n");

    idt_ptr.limit = sizeof(idt_entries) - 1;
    idt_ptr.base = (uint32_t)idt_entries;

    for (int i = 0; i < IDT_ENTRIES; i++) {
        idt_set_gate(i, (uint32_t)default_handler);
    }

    idt_set_gate(14, (uint32_t)handle_page_fault);

    __asm__ volatile ("lidt %0" : : "m"(idt_ptr));

    serial_print("IDT: Loaded\n");
}
