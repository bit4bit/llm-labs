# project-initialization Specification

## Purpose
Define the project structure, build system, and boot requirements for MiniOS. The kernel is compiled for x86_64 architecture and boots via GRUB Multiboot.

## Requirements
### Requirement: Project Directory Structure
The project SHALL maintain the following directory structure:
```
├── kernel/
│   ├── entry.asm       # Multiboot header, entry point, long mode switch
│   ├── main.c          # Kernel main, initialization
│   ├── vga.c           # VGA text mode driver
│   └── vga.h           # VGA driver header
├── iso/
│   └── boot/
│       └── grub/
│           └── grub.cfg  # GRUB boot menu
├── linker.ld           # Linker script for kernel placement
└── Makefile            # Build configuration
```

#### Scenario: Directory structure exists
- **WHEN** project is cloned
- **THEN** kernel/, iso/boot/grub/ directories exist with required files

### Requirement: Multiboot Entry Point
The kernel SHALL have an entry point in assembly that:
- Defines the Multiboot header within the first 8192 bytes
- Uses magic value `0x1BADB002` and flags `0x00010003`
- Calculates correct checksum `-(magic + flags) % 2^32`
- Exports the `_start` symbol as kernel entry point
- Switches the processor to 64-bit long mode before calling kernel_main

#### Scenario: Multiboot header valid
- **WHEN** kernel is assembled
- **THEN** Multiboot header is within first 8192 bytes of ELF
- **AND** header contains correct magic and checksum

#### Scenario: Entry point switches to long mode
- **WHEN** bootloader jumps to `_start`
- **THEN** the entry point SHALL enable PAE
- **AND** set up identity-mapped page tables
- **AND** enable long mode via IA-32e mode
- **AND** load a 64-bit GDT
- **AND** jump to `kernel_main` in 64-bit mode

### Requirement: Kernel Main Function
The kernel SHALL define a `kernel_main` function that:
- Accepts multiboot magic in EDI register (64-bit calling convention)
- Accepts pointer to multiboot_info_t in RSI register
- Initializes VGA text mode for output
- Halts with infinite loop after initialization

#### Scenario: Kernel main receives boot info
- **WHEN** kernel_main is called by entry point in long mode
- **THEN** RDI contains valid multiboot magic
- **AND** RSI points to valid multiboot_info structure

### Requirement: Linker Script Configuration
The kernel SHALL use a linker script that:
- Places .text section at virtual address `0x100000` (1MB)
- Aligns sections to 4KB page boundary
- Produces ELF64 executable format

#### Scenario: Linker script defines load address
- **WHEN** kernel is linked
- **THEN** entry point is at address `0x100000`
- **AND** kernel is valid ELF64 format for x86-64

### Requirement: Build System
The project SHALL provide a Makefile with targets for:
- `make` - Build kernel.elf from source
- `make iso` - Create bootable ISO with GRUB
- `make run` - Build ISO and run in QEMU (x86_64)
- `make debug` - Build and run QEMU with GDB stub (x86_64)
- `make deps` - Verify required dependencies
- `make clean` - Remove build artifacts

#### Scenario: Build produces 64-bit kernel ELF
- **WHEN** `make` is executed
- **THEN** kernel.elf is created in project root
- **AND** ELF header indicates ELF64 format for x86-64 architecture

#### Scenario: ISO creation succeeds
- **WHEN** `make iso` is executed
- **THEN** minios.iso is created in project root
- **AND** ISO contains GRUB boot menu
- **AND** kernel.elf is accessible at /boot/minios.elf

#### Scenario: Run target launches QEMU x86_64
- **WHEN** `make run` is executed
- **THEN** ISO is built if not current
- **AND** QEMU launches with minios.iso as CD-ROM using qemu-system-x86_64
- **AND** QEMU boots from CD-ROM device

#### Scenario: Debug target enables GDB for 64-bit kernel
- **WHEN** `make debug` is executed
- **THEN** QEMU starts with GDB stub on port 1234 using qemu-system-x86_64
- **AND** QEMU waits for GDB connection before executing
- **AND** Kernel symbols are available for debugging

#### Scenario: Deps target verifies dependencies
- **WHEN** `make deps` is executed
- **THEN** all required tools are verified (nasm, gcc, ld, grub-mkrescue, xorriso)
- **AND** missing tools report installation instructions
- **AND** exit code is 0 when all dependencies satisfied

#### Scenario: Clean target removes artifacts
- **WHEN** `make clean` is executed
- **THEN** kernel.elf is removed from project root
- **AND** minios.iso is removed from project root
- **AND** object files (*.o) in kernel/ are removed

### Requirement: GRUB Boot Configuration
The system SHALL provide a GRUB configuration file that:
- Defines a default boot menu entry for MiniOS
- Uses `multiboot` command to load kernel.elf
- Boots the kernel immediately (timeout=0)

#### Scenario: GRUB config loads kernel
- **WHEN** GRUB loads the ISO
- **THEN** menu entry "minios" appears
- **AND** selecting it loads /boot/minios.elf via multiboot

### Requirement: x86_64 Long Mode Support
The kernel SHALL properly support x86_64 long mode:

#### Scenario: PAE is enabled
- **WHEN** kernel entry point executes
- **THEN** CR4.PAE bit (bit 5) SHALL be set

#### Scenario: Page tables are configured
- **WHEN** kernel entry point initializes paging
- **THEN** PML4, PDP, and PD tables SHALL be set up
- **AND** identity mapping SHALL be enabled for low memory

#### Scenario: Long mode is activated
- **WHEN** EFER.LME bit (bit 8) is set
- **AND** CR0.PG bit (bit 31) is set
- **THEN** the processor enters 64-bit long mode
- **AND** CS descriptor with 64-bit flag is loaded

#### Scenario: 64-bit GDT is loaded
- **WHEN** switching to long mode
- **THEN** a 64-bit GDT SHALL be loaded with:
- **AND** code segment with S=1, DPL=0, P=1, L=1, D=0
- **AND** data segment with S=1, DPL=0, P=1
