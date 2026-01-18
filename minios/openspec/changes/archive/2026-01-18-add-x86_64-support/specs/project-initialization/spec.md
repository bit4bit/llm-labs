## MODIFIED Requirements

### Requirement: Build System Configuration
The system SHALL compile the kernel using GCC with appropriate architecture flags.

#### Scenario: 32-bit build
- **WHEN** `make` is executed with default CFLAGS including `-m32`
- **THEN** the kernel SHALL be compiled as a 32-bit ELF executable

#### Scenario: 64-bit build
- **WHEN** `make` is executed with updated CFLAGS including `-m64`
- **THEN** the kernel SHALL be compiled as a 64-bit ELF executable
- **AND** the output format SHALL be `elf64-x86-64`
- **AND** the linker script SHALL specify 64-bit output format

### Requirement: Kernel Entry Point
The system SHALL provide a kernel entry point that initializes the processor to the target architecture mode.

#### Scenario: 32-bit entry
- **WHEN** the kernel is compiled for i386
- **THEN** the entry point SHALL set up a 32-bit GDT
- **AND** jump directly to `kernel_main` in protected mode

#### Scenario: 64-bit entry
- **WHEN** the kernel is compiled for x86_64
- **THEN** the entry point SHALL switch to long mode
- **AND** set up a 64-bit GDT with appropriate descriptors
- **AND** configure the stack pointer for 64-bit mode
- **AND** jump to `kernel_main` in 64-bit long mode

### Requirement: Boot Compatibility
The system SHALL boot via GRUB on both 32-bit and 64-bit x86 hardware.

#### Scenario: 32-bit boot
- **WHEN** a 32-bit kernel is loaded by GRUB
- **THEN** the Multiboot header SHALL be within the first 8192 bytes
- **AND** the kernel SHALL receive the Multiboot magic in EAX
- **AND** the kernel SHALL receive the boot info pointer in EBX

#### Scenario: 64-bit boot
- **WHEN** a 64-bit kernel is loaded by GRUB on x86_64 hardware
- **THEN** the Multiboot header SHALL be within the first 8192 bytes
- **AND** the kernel SHALL receive the Multiboot magic in EAX
- **AND** the kernel SHALL receive the boot info pointer in EBX
- **AND** the kernel SHALL be in 64-bit long mode after initialization

### Requirement: Kernel Execution
The system SHALL execute the kernel main function after processor initialization.

#### Scenario: 32-bit execution
- **WHEN** the kernel is in protected mode
- **THEN** `kernel_main(unsigned int magic, void *mb_info)` SHALL be called
- **AND** the function SHALL run using 32-bit calling conventions

#### Scenario: 64-bit execution
- **WHEN** the kernel is in long mode
- **THEN** `kernel_main(unsigned int magic, void *mb_info)` SHALL be called
- **AND** the function SHALL run using 64-bit calling conventions
- **AND** all pointers SHALL be 64-bit addresses
