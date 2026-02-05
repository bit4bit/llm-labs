#include "idt.h"
#include "gdt.h"
#include "constants.h"
#include "../kernel.h"

struct idt_entry idt_entries[IDT_ENTRIES];
struct idt_ptr idt_ptr;

extern void handle_divide_error(void);
extern void handle_debug_exception(void);
extern void handle_nmi(void);
extern void handle_breakpoint(void);
extern void handle_overflow(void);
extern void handle_bounds_check(void);
extern void handle_invalid_opcode(void);
extern void handle_device_not_available(void);
extern void handle_double_fault(void);
extern void handle_coprocessor_segment_overrun(void);
extern void handle_invalid_tss(void);
extern void handle_segment_not_present(void);
extern void handle_stack_fault(void);
extern void handle_general_protection_fault(void);
extern void handle_page_fault(void);
extern void handle_reserved(void);
extern void handle_coprocessor_error(void);

void idt_set_gate(uint8_t num, uint32_t handler) {
    uint32_t addr = handler;

    idt_entries[num].offset_low = (addr & 0xFFFF);
    idt_entries[num].selector = KERNEL_CODE_SELECTOR;
    idt_entries[num].zero = 0;
    idt_entries[num].flags = IDT_DESC_PRESENT | IDT_DESC_TYPE_INTERRUPT;
    idt_entries[num].offset_high = (addr >> 16) & 0xFFFF;
}

void idt_init(void) {
    serial_print("IDT: Initializing...\n");

    idt_ptr.limit = sizeof(idt_entries) - 1;
    idt_ptr.base = (uint32_t)idt_entries;

    for (int i = 0; i < IDT_ENTRIES; i++) {
        idt_set_gate(i, (uint32_t)handle_general_protection_fault);
    }

    idt_set_gate(0, (uint32_t)handle_divide_error);
    idt_set_gate(13, (uint32_t)handle_general_protection_fault);
    idt_set_gate(14, (uint32_t)handle_page_fault);

    __asm__ volatile ("lidt %0" : : "m"(idt_ptr));

    serial_print("IDT: Loaded\n");
}
