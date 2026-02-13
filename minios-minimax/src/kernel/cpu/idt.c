#include "idt.h"
#include "constants.h"
#include "../kernel.h"

extern void serial_print_hex(uint32_t val);

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
extern void handle_general_protection_fault(void);
extern void syscall_entry(void);
extern void timer_handler_asm(void);

void idt_init(void) {
    serial_print("IDT: Initializing...\n");

    idt_ptr.limit = sizeof(idt_entries) - 1;
    idt_ptr.base = (uint32_t)idt_entries;

    for (int i = 0; i < IDT_ENTRIES; i++) {
        idt_set_gate(i, (uint32_t)default_handler);
    }

    idt_set_gate(13, (uint32_t)handle_general_protection_fault);
    idt_set_gate(14, (uint32_t)handle_page_fault);

    idt_set_gate(32, (uint32_t)timer_handler_asm);

    serial_print("IDT: Configuring syscall gate at 0x80...\n");
    idt_entries[0x80].offset_low = ((uint32_t)syscall_entry) & 0xFFFF;
    idt_entries[0x80].selector = KERNEL_CODE_SELECTOR;
    idt_entries[0x80].zero = 0;
    idt_entries[0x80].flags = IDT_DESC_TYPE_INT_DPL3;  /* 0xEE: Present, DPL=3, Interrupt Gate */
    idt_entries[0x80].offset_high = ((uint32_t)syscall_entry >> 16) & 0xFFFF;

    serial_print("IDT: Verifying entry 0x80...\n");
    serial_print("IDT:   offset_low=0x");
    serial_print_hex(idt_entries[0x80].offset_low);
    serial_print("\n");
    serial_print("IDT:   selector=0x");
    serial_print_hex(idt_entries[0x80].selector);
    serial_print("\n");
    serial_print("IDT:   zero=0x");
    serial_print_hex(idt_entries[0x80].zero);
    serial_print("\n");
    serial_print("IDT:   flags=0x");
    serial_print_hex(idt_entries[0x80].flags);
    serial_print(" (");
    serial_print_uint(idt_entries[0x80].flags);
    serial_print(" dec, expected 0xEE=238)");
    serial_print("\n");
    serial_print("IDT:   offset_high=0x");
    serial_print_hex(idt_entries[0x80].offset_high);
    serial_print("\n");

    __asm__ volatile ("lidt %0" : : "m"(idt_ptr));

    serial_print("IDT: Loaded\n");
}
