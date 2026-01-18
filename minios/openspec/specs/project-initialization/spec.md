# project-initialization Specification

## Purpose
TBD - created by archiving change setup-project-initial. Update Purpose after archive.
## Requirements
### Requirement: Project Directory Structure
The project SHALL maintain the following directory structure:
```
├── kernel/
│   ├── entry.asm       # Multiboot header, entry point
│   └── main.c          # Kernel main, initialization
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

#### Scenario: Multiboot header valid
- **WHEN** kernel is assembled
- **THEN** Multiboot header is within first 8192 bytes of ELF
- **AND** header contains correct magic and checksum

#### Scenario: Entry point jumps to C code
- **WHEN** bootloader jumps to `_start`
- **THEN** control transfers to `kernel_main` in C

### Requirement: Kernel Main Function
The kernel SHALL define a `kernel_main` function that:
- Accepts multiboot magic in EAX register
- Accepts pointer to multiboot_info_t in EBX register
- Initializes VGA text mode for output
- Halts with infinite loop after initialization

#### Scenario: Kernel main receives boot info
- **WHEN** kernel_main is called by entry point
- **THEN** EAX contains `0x2BADB002` (valid multiboot)
- **AND** EBX points to valid multiboot_info structure

### Requirement: Linker Script Configuration
The kernel SHALL use a linker script that:
- Places .text section at virtual address `0x100000` (1MB)
- Aligns sections to 4KB page boundary
- Produces ELF32 executable format

#### Scenario: Linker script defines load address
- **WHEN** kernel is linked
- **THEN** entry point is at address `0x100000`
- **AND** kernel is valid ELF32 format

### Requirement: Build System
The project SHALL provide a Makefile with targets for:
- `make` - Build kernel.elf from source
- `make iso` - Create bootable ISO with GRUB
- `make run` - Build ISO and run in QEMU
- `make debug` - Build and run QEMU with GDB stub
- `make deps` - Verify required dependencies
- `make clean` - Remove build artifacts

#### Scenario: Build produces kernel ELF
- **WHEN** `make` is executed
- **THEN** kernel.elf is created in project root
- **AND** ELF header indicates ELF32 format

#### Scenario: ISO creation succeeds
- **WHEN** `make iso` is executed
- **THEN** minios.iso is created in project root
- **AND** ISO contains GRUB boot menu
- **AND** kernel.elf is accessible at /boot/minios.elf

#### Scenario: Run target launches QEMU
- **WHEN** `make run` is executed
- **THEN** ISO is built if not current
- **AND** QEMU launches with minios.iso as CD-ROM
- **AND** QEMU boots from CD-ROM device

#### Scenario: Debug target enables GDB
- **WHEN** `make debug` is executed
- **THEN** QEMU starts with GDB stub on port 1234
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

