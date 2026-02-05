#ifndef IDT_H
#define IDT_H

#include <stdint.h>

#define IDT_ENTRIES 256

struct idt_entry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t  zero;
    uint8_t  flags;
    uint16_t offset_high;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

extern struct idt_ptr idt_ptr;
extern struct idt_entry idt_entries[IDT_ENTRIES];

void idt_init(void);
void idt_set_gate(uint8_t num, uint32_t handler);

#endif
