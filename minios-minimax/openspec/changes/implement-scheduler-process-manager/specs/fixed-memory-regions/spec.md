## ADDED Requirements

### Requirement: Program symbol table
The kernel SHALL maintain a symbol table mapping program names to their fixed memory addresses.

#### Scenario: Symbol table access
- **WHEN** `find_program("hello")` is called
- **THEN** function returns `{base: 0x40000000, size: 4096}`
- **AND** for unknown program, returns NULL

#### Scenario: Symbol table structure
The symbol table SHALL contain:
- Program name (string identifier)
- Base address (fixed load address)
- Size (memory size to allocate/load)

### Requirement: Flat binary loading
The kernel SHALL load programs as flat binaries directly to their fixed addresses.

#### Scenario: Load hello program
- **WHEN** `load_flat_binary("hello")` is called
- **THEN** kernel reads binary data from storage
- **THEN** data is copied to address 0x40000000
- **AND** page tables are configured for that address range
- **AND** program entry point is set to base address

#### Scenario: Load shell program
- **WHEN** `load_flat_binary("shell")` is called
- **THEN** kernel reads binary data from storage
- **THEN** data is copied to address 0x40010000
- **AND** page tables are configured for that address range
- **AND** program entry point is set to base address

### Requirement: Fixed address assignment
Each user program SHALL be linked to a specific fixed address at compile time.

#### Scenario: Hello linked at 0x40000000
- **WHEN** hello.c is compiled with `-Wl,-Ttext=0x40000000`
- **THEN** code and data sections are placed at 0x40000000
- **AND** binary contains absolute addresses at linked locations

#### Scenario: Shell linked at 0x40010000
- **WHEN** shell.c is compiled with `-Wl,-Ttext=0x40010000`
- **THEN** code and data sections are placed at 0x40010000
- **AND** binary contains absolute addresses at linked locations

### Requirement: Memory regions non-overlapping
Programs with fixed addresses SHALL have non-overlapping memory regions.

#### Scenario: Address space layout
- **WHEN** hello (0x40000000-0x40000FFF) and shell (0x40010000-0x40011FFF) are loaded
- **THEN** their address ranges do not overlap
- **AND** each program can use its fixed addresses independently

### Requirement: Kernel page table setup for user programs
The kernel SHALL configure page tables to map fixed user addresses.

#### Scenario: Page mapping for program
- **WHEN** a program is loaded at address 0x40000000
- **THEN** kernel creates/updates page directory entries
- **AND** user pages are marked as present, read/write, user-mode
- **AND** kernel addresses are not accessible from user mode
