## Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                    CPU INITIALIZATION ORDER                     │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  kernel_main                                                    │
│      │                                                         │
│      ├─► pmm_init()                                            │
│      │                                                         │
│      ├─► gdt_init()                                           │
│      │    ├─ Create 5 descriptors:                            │
│      │    │   ├─ Null descriptor (index 0)                   │
│      │    │   ├─ Kernel code segment (index 1)              │
│      │    │   ├─ Kernel data segment (index 2)              │
│      │    │   ├─ User code segment (index 3)                │
│      │    │   └─ User data segment (index 4)                │
│      │    └─ Load GDT pointer (lgdt)                         │
│      │                                                         │
│      └─► idt_init()                                           │
│           ├─ Create IDT with 256 entries                      │
│           ├─ Set interrupt gates for exceptions              │
│           └─ Load IDT pointer (lidt)                          │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

## GDT Design

### Entries (5 total)
| Index | Selector | Type | DPL | Present |
|-------|----------|------|-----|---------|
| 0 | 0x00 | null (unused) | 0 | no |
| 1 | 0x08 | code segment | 0 | yes |
| 2 | 0x10 | data segment | 0 | yes |
| 3 | 0x18 | code segment | 3 | yes |
| 4 | 0x20 | data segment | 3 | yes |

### Structs
```c
struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} __attribute__((packed));

struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));
```

## IDT Design

### Entries (256 total)
- 0-31: CPU exceptions (reserved)
- 32-127: Hardware interrupts (PIT, keyboard, etc.)
- 128-255: Software interrupts (syscalls, etc.)

### Exception Handlers (minimal)
| Vector | Name | Handler |
|--------|------|---------|
| 0 | #DE (Divide Error) | handle_divide_error |
| 13 | #GP (General Protection) | handle_gp_fault |
| 14 | #PF (Page Fault) | handle_page_fault |

### Struct
```c
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
```

## Loading Segments

```c
void gdt_init(void) {
    gdt_set_gate(0, 0, 0, 0, 0);              // null
    gdt_set_gate(1, 0, 0xFFFFF, 0x9A, 0x0C);   // kernel code
    gdt_set_gate(2, 0, 0xFFFFF, 0x92, 0x0C);   // kernel data
    gdt_set_gate(3, 0, 0xFFFFF, 0xFA, 0x0C);   // user code
    gdt_set_gate(4, 0, 0xFFFFF, 0xF2, 0x0C);   // user data

    gdt_ptr.limit = sizeof(gdt_entries) - 1;
    gdt_ptr.base = (uint32_t)gdt_entries;

    __asm__ volatile ("lgdt %0" : : "m"(gdt_ptr));

    __asm__ volatile (
        "movw $0x10, %ax\n"
        "movw %ax, %ds\n"
        "movw %ax, %es\n"
        "movw %ax, %fs\n"
        "movw %ax, %gs\n"
        "ljmp $0x08, $1f\n"
        "1:"
    );
}
```

## Exception Handling

```c
void page_fault_handler(void) {
    serial_print("PAGE FAULT!\n");
    serial_print("Accessing invalid memory\n");
    while (1) __asm__ volatile ("hlt");
}
```

## Non-Goals

- User mode execution
- Hardware interrupts (PIT, keyboard)
- Syscall handling
- APIC or advanced interrupt controllers
