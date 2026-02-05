## Why

The kernel currently lacks proper segment descriptors (GDT) and interrupt vectors (IDT). These are essential for:
- Proper protected mode operation
- Catching and handling CPU exceptions (especially page fault)
- Enabling hardware interrupts (PIT, keyboard)
- Foundation for paging and user mode

## What Changes

- Create `src/kernel/cpu/gdt.h`: GDT entry and pointer structs
- Create `src/kernel/cpu/gdt.c`: GDT initialization, segment selectors
- Create `src/kernel/cpu/idt.h`: IDT entry and pointer structs
- Create `src/kernel/cpu/idt.c`: IDT initialization, interrupt gates
- Create `src/kernel/cpu/interrupts.c`: Exception handlers
- Update `src/kernel/main.c`: Initialize GDT/IDT after PMM
- Update `Makefile`: Add new object files

## Capabilities

### New Capabilities
- `cpu-gdt`: Global Descriptor Table for segmentation
- `cpu-idt`: Interrupt Descriptor Table for exception/interrupt handling

## Impact

- Required for: paging, exceptions, hardware interrupts
- ~200 lines of code
- Enables meaningful error messages on kernel crashes
