## ADDED Requirements

### Requirement: Each process has a dedicated kernel stack
The kernel SHALL allocate a separate 4KB kernel stack for each process. Kernel stacks SHALL be statically allocated as an array in BSS, aligned to 4KB.

#### Scenario: Kernel stack assigned at creation
- **WHEN** `process_create()` is called for process at index `i`
- **THEN** the PCB's `kernel_stack_top` field SHALL be set to the top address of `kernel_stacks[i]` (i.e., `&kernel_stacks[i][4096]`)

#### Scenario: Kernel stacks are distinct
- **WHEN** two processes exist at indices 0 and 1
- **THEN** their `kernel_stack_top` values SHALL differ by exactly 4096 bytes

#### Scenario: TSS uses per-process kernel stack
- **WHEN** the scheduler switches to process `i`
- **THEN** `TSS.esp0` SHALL be set to process `i`'s `kernel_stack_top`

### Requirement: Each process has a dedicated user stack region
The kernel SHALL map a separate 4MB physical frame for each process's user stack at a distinct virtual address via a dedicated PDE.

#### Scenario: User stack PDE assignment
- **WHEN** process at index `i` is created
- **THEN** the kernel SHALL map PDE `767 - i` with a freshly allocated 4MB physical frame and user-mode permissions (Present + R/W + User + PS)

#### Scenario: User stack virtual addresses
- **WHEN** process at index 0 is created
- **THEN** its user stack region SHALL be at virtual address 0xBFC00000 (PDE 767)
- **WHEN** process at index 1 is created
- **THEN** its user stack region SHALL be at virtual address 0xBF800000 (PDE 766)
- **WHEN** process at index 2 is created
- **THEN** its user stack region SHALL be at virtual address 0xBF400000 (PDE 765)
- **WHEN** process at index 3 is created
- **THEN** its user stack region SHALL be at virtual address 0xBF000000 (PDE 764)

#### Scenario: User stack initial pointer
- **WHEN** process at index `i` is created
- **THEN** the PCB's `user_stack` field SHALL be set to the top of its stack region minus a 4KB guard (i.e., `stack_region_base + 4MB - 4KB`)

### Requirement: Each process has a dedicated code region
The kernel SHALL map a separate 4MB physical frame for each process's code at a distinct virtual address via a dedicated PDE.

#### Scenario: Code PDE assignment
- **WHEN** process at index `i` is created
- **THEN** the kernel SHALL map PDE `256 + i` with a freshly allocated 4MB physical frame and user-mode permissions (Present + R/W + User + PS)

#### Scenario: Code virtual addresses
- **WHEN** process at index 0 is created
- **THEN** its code region SHALL be at virtual address 0x40000000 (PDE 256)
- **WHEN** process at index 1 is created
- **THEN** its code region SHALL be at virtual address 0x40400000 (PDE 257)
- **WHEN** process at index 2 is created
- **THEN** its code region SHALL be at virtual address 0x40800000 (PDE 258)
- **WHEN** process at index 3 is created
- **THEN** its code region SHALL be at virtual address 0x40C00000 (PDE 259)

#### Scenario: Binary loaded to correct address
- **WHEN** `process_load(pcb, binary, size)` is called
- **THEN** the binary SHALL be copied to the virtual address corresponding to the process's PDE (i.e., `pcb->entry`)

### Requirement: Programs are compiled for their target address
Each user program SHALL be linked at the virtual address corresponding to its process slot so that absolute addresses in the binary are correct.

#### Scenario: Linker script accepts base address parameter
- **WHEN** a program is linked with `--defsym=_user_base=0x40400000`
- **THEN** the linker script SHALL use `0x40400000` as the base address for the `.text` section

#### Scenario: Default base address is preserved
- **WHEN** a program is linked without `--defsym=_user_base`
- **THEN** the linker script SHALL use `0x40000000` as the base address (backward compatible)

#### Scenario: Build system assigns addresses per program
- **WHEN** user programs are built
- **THEN** the build system SHALL assign each program a unique base address from the set {0x40000000, 0x40400000, 0x40800000, 0x40C00000}
- **AND** pass the address to the linker via `--defsym=_user_base=<address>`

### Requirement: TLB is flushed after PDE changes
The kernel SHALL flush the TLB after modifying page directory entries to ensure the CPU uses the updated mappings.

#### Scenario: TLB flush after mapping creation
- **WHEN** a new PDE is written for a process's code or stack region
- **THEN** the kernel SHALL reload CR3 to flush the TLB before any access to the newly mapped region
