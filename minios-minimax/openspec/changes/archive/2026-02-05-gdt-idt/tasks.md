## GDT + IDT Implementation Tasks

### 1. Create GDT

- [x] 1.1 Create `src/kernel/cpu/gdt.h` with struct definitions
- [x] 1.2 Create `src/kernel/cpu/gdt.c`
- [x] 1.3 Implement `gdt_set_gate()` function
- [x] 1.4 Implement `gdt_init()` with 5 entries
- [x] 1.5 Update segment registers after loading GDT

### 2. Create IDT

- [x] 2.1 Create `src/kernel/cpu/idt.h` with struct definitions
- [x] 2.2 Create `src/kernel/cpu/idt.c`
- [x] 2.3 Implement `idt_set_gate()` function
- [x] 2.4 Implement `idt_init()` with 256 entries
- [x] 2.5 Load IDT with `lidt` instruction

### 3. Create Exception Handlers

- [x] 3.1 Create `src/kernel/cpu/interrupts.c`
- [x] 3.2 Implement assembly stubs with naked functions
- [x] 3.3 Implement C handlers for divide, GP, page fault
- [x] 3.4 Print error message on page fault

### 4. Update main.c

- [x] 4.1 Include gdt.h and idt.h
- [x] 4.2 Call `gdt_init()` after `pmm_init()`
- [x] 4.3 Call `idt_init()` after `gdt_init()`
- [x] 4.4 Enable interrupts with `sti`

### 5. Update Makefile

- [x] 5.1 Add cpu/gdt.o to KERNEL_OBJS
- [x] 5.2 Add cpu/idt.o to KERNEL_OBJS
- [x] 5.3 Add cpu/interrupts.o to KERNEL_OBJS

### 6. Build and Test

- [x] 6.1 Run `make clean && make`
- [x] 6.2 Create ISO with `make iso`
- [ ] 6.3 Run `make qemu` and verify GDT/IDT messages
- [ ] 6.4 Test page fault by triggering invalid access

## Files Created/Modified

| File | Action |
|------|--------|
| src/kernel/cpu/gdt.h | Created |
| src/kernel/cpu/gdt.c | Created |
| src/kernel/cpu/idt.h | Created |
| src/kernel/cpu/idt.c | Created |
| src/kernel/cpu/interrupts.c | Created |
| src/kernel/main.c | Modified |
| Makefile | Modified |
