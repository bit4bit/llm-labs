## ADDED Requirements

### Requirement: GDT Initialization

The kernel SHALL initialize a Global Descriptor Table with proper segment descriptors.

#### Scenario: Initialize GDT
- **WHEN** `gdt_init()` is called
- **THEN** the GDT SHALL contain 5 entries:
  - Index 0: Null descriptor (unused)
  - Index 1: Kernel code segment (selector 0x08)
  - Index 2: Kernel data segment (selector 0x10)
  - Index 3: User code segment (selector 0x18, DPL=3)
  - Index 4: User data segment (selector 0x20, DPL=3)
- **AND** the GDT SHALL be loaded using the `lgdt` instruction
- **AND** all data segment registers (DS, ES, FS, GS, SS) SHALL be set to 0x10

### Requirement: Kernel Code Segment

The kernel code segment SHALL have:
- Base address: 0x0
- Limit: 0xFFFFF (4GB)
- Present bit: 1
- Descriptor privilege level: 0 (kernel)
- Type: code/execute (0x9A)
- Granularity: 4KB (0x0C)

### Requirement: Kernel Data Segment

The kernel data segment SHALL have:
- Base address: 0x0
- Limit: 0xFFFFF (4GB)
- Present bit: 1
- Descriptor privilege level: 0 (kernel)
- Type: data/read-write (0x92)
- Granularity: 4KB (0x0C)

### Requirement: User Segments

User-mode segments SHALL have:
- Descriptor privilege level: 3 (user)
- Same base and limit as kernel segments
- User code type: 0xFA (execute-only/conforming)
- User data type: 0xF2 (read-write)
