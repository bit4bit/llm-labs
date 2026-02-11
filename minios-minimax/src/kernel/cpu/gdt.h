#ifndef GDT_H
#define GDT_H

#include <stdint.h>

#define GDT_ENTRIES 6

struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} __attribute__((packed));

_Static_assert(sizeof(struct gdt_entry) == 8, "C18: gdt_entry must be 8 bytes");

struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

_Static_assert(sizeof(struct gdt_ptr) == 6, "C18: gdt_ptr must be 6 bytes");

extern struct gdt_ptr gdt_ptr;
extern struct gdt_entry gdt_entries[GDT_ENTRIES];

void gdt_init(void);
void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);

#endif
