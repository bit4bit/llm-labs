## ADDED Requirements

### Requirement: IDT Initialization

The kernel SHALL initialize an Interrupt Descriptor Table with 256 entries.

#### Scenario: Initialize IDT
- **WHEN** `idt_init()` is called
- **THEN** the IDT SHALL contain 256 entries
- **AND** entries 0-31 SHALL be set for CPU exceptions
- **AND** entries 32-255 SHALL be initially empty (or point to default handler)
- **AND** the IDT SHALL be loaded using the `lidt` instruction

### Requirement: Interrupt Gate Structure

Each IDT entry SHALL be an interrupt gate with:
- Selector: kernel code segment (0x08)
- IST: 0 (no interrupt stack table)
- Type: 0xE (interrupt gate)
- DPL: 0 (kernel only)
- Present: 1
- Offset: address of handler function

### Requirement: Page Fault Handler

The kernel SHALL provide a handler for interrupt vector 14 (Page Fault).

#### Scenario: Page fault occurs
- **WHEN** the CPU raises a page fault (#PF)
- **THEN** the handler SHALL print diagnostic information:
  - The string "PAGE FAULT"
  - The faulting virtual address (from CR2 register)
  - The error code (present/protection/user/supervisor bits)
- **AND** the kernel SHALL halt after displaying the error

### Requirement: Divide Error Handler

The kernel SHALL provide a handler for interrupt vector 0 (Divide Error).

#### Scenario: Divide by zero
- **WHEN** the CPU raises a divide error (#DE)
- **THEN** the handler SHALL print "DIVIDE ERROR"
- **AND** the kernel SHALL halt

### Requirement: General Protection Fault Handler

The kernel SHALL provide a handler for interrupt vector 13 (General Protection Fault).

#### Scenario: General protection fault
- **WHEN** the CPU raises a GP fault (#GP)
- **THEN** the handler SHALL print "GENERAL PROTECTION FAULT"
- **AND** the kernel SHALL halt

### Requirement: Interrupt Enable

After IDT initialization, the kernel SHALL be able to enable interrupts.

#### Scenario: Enable interrupts
- **WHEN** the `sti` instruction is executed
- **THEN** maskable interrupts SHALL be enabled on the CPU
