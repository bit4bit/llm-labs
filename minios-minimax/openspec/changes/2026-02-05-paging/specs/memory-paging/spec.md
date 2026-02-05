## ADDED Requirements

### Requirement: Paging Initialization

The kernel SHALL initialize paging with identity mapping for the kernel.

#### Scenario: Initialize paging
- **WHEN** `paging_init()` is called
- **THEN** the kernel SHALL allocate a page directory from PMM
- **AND** the kernel SHALL allocate a page table for the kernel
- **AND** the page directory SHALL have entries that map virtual addresses to physical addresses
- **AND** the kernel SHALL enable paging by loading CR3 and setting CR0.PG

### Requirement: Kernel Identity Mapping

The kernel SHALL map the first 4MB of physical memory to the kernel virtual space.

#### Scenario: Kernel identity mapping
- **WHEN** paging is enabled
- **THEN** virtual address 0xC0000000 SHALL map to physical address 0x00000000
- **AND** the mapping SHALL use 4MB pages (PDE.PS = 1)
- **AND** kernel code/data SHALL remain accessible after enabling paging

### Requirement: CR3 Register Setup

The kernel SHALL load the page directory address into CR3.

#### Scenario: Load page directory
- **WHEN** `enable_paging()` is called
- **THEN** the physical address of the page directory SHALL be written to CR3
- **AND** the PCID feature SHALL be disabled (bit 63 of CR3 = 0)

### Requirement: CR0.PG Enable

The kernel SHALL enable paging by setting the PG bit in CR0.

#### Scenario: Enable paging
- **WHEN** the PG bit in CR0 is set
- **THEN** the CPU SHALL use the page directory for address translation
- **AND** the kernel SHALL continue executing at the next instruction

### Requirement: Page Fault During Kernel

If a page fault occurs while in kernel mode, the kernel SHALL display diagnostic information.

#### Scenario: Kernel page fault
- **WHEN** a page fault occurs (interrupt 14)
- **AND** the kernel was executing
- **THEN** the handler SHALL print "KERNEL PAGE FAULT"
- **AND** print the faulting virtual address (CR2)
- **AND** print the error code
- **AND** halt the system
