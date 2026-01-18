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
- Uses magic value appropriate for the target specification version
- Calculates correct checksum for the specification version
- Exports the `_start` symbol as kernel entry point
- Switches the processor to 64-bit long mode before calling kernel_main

#### Scenario: Multiboot1 header (deprecated)
- **WHEN** the kernel is compiled with Multiboot1 support
- **THEN** the header SHALL contain magic `0x1BADB002`
- **AND** flags SHALL be `0x00010003` for memory info
- **AND** checksum SHALL be `-(magic + flags) % 2^32`
- **AND** the header SHALL be aligned to 4-byte boundary

#### Scenario: Multiboot2 header
- **WHEN** the kernel is compiled with Multiboot2 support
- **THEN** the header SHALL contain magic `0xE85250D6`
- **AND** architecture field SHALL be `4` for x86_64
- **AND** header length SHALL include all required fields
- **AND** checksum SHALL be `-(magic + architecture + header_length) % 2^32`
- **AND** the header SHALL be aligned to 8-byte boundary
- **AND** the header SHALL be within first 8192 bytes of kernel image

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
- Uses appropriate Multiboot directive to load kernel.elf
- Boots the kernel immediately (timeout=0)

#### Scenario: Multiboot1 boot with GRUB
- **WHEN** GRUB loads the kernel with `multiboot` directive
- **THEN** GRUB SHALL detect Multiboot1 header automatically
- **AND** pass Multiboot1 magic and boot info

#### Scenario: Multiboot2 boot with GRUB
- **WHEN** GRUB loads the kernel with `multiboot2` directive
- **THEN** GRUB SHALL use Multiboot2 protocol
- **AND** pass Multiboot2 magic and boot info
- **AND** the kernel SHALL be a 64-bit ELF executable

#### Scenario: GRUB config loads kernel
- **WHEN** GRUB loads the ISO
- **THEN** menu entry "minios" appears
- **AND** selecting it loads /boot/minios.elf via appropriate multiboot directive

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

### Requirement: Boot Magic Verification
The system SHALL verify the boot magic passed by the bootloader to confirm Multiboot compliance.

#### Scenario: Multiboot1 magic verification
- **WHEN** the kernel receives `0x2BADB002` in EAX
- **THEN** the kernel SHALL use Multiboot1 boot info parsing
- **AND** the boot info pointer in EBX SHALL be cast to `multiboot_info_t*`

#### Scenario: Multiboot2 magic verification
- **WHEN** the kernel receives `0x36D76289` in EAX
- **THEN** the kernel SHALL use Multiboot2 boot info parsing
- **AND** the boot info pointer in EBX SHALL be cast to `multiboot2_boot_info_t*`

### Requirement: Boot Information Parsing
The system SHALL parse boot information tags provided by the bootloader to obtain memory map, boot device, and command line.

#### Scenario: Multiboot1 info structure
- **WHEN** Multiboot1 magic is verified
- **THEN** the kernel SHALL read memory map from `mmap_*` fields
- **AND** boot device from `boot_device` field
- **AND** command line from `cmdline` field

#### Scenario: Multiboot2 tag iteration
- **WHEN** Multiboot2 magic is verified
- **THEN** the kernel SHALL iterate through boot info tags
- **AND** parse memory map from `BT_INFO_MMAP` tag
- **AND** parse boot device from `BT_INFO_BOOTDEV` tag
- **AND** parse command line from `BT_INFO_CMDLINE` tag
- **AND** terminate iteration at `BT_END` tag

### Requirement: Memory Information
The system SHALL provide memory information obtained from the bootloader.

#### Scenario: Multiboot1 memory info
- **WHEN** Multiboot1 boot info is available
- **THEN** lower memory SHALL be available from `mem_lower` (typically 640 KB)
- **AND** upper memory SHALL be available from `mem_upper`

#### Scenario: Multiboot2 memory map
- **WHEN** Multiboot2 boot info is available
- **THEN** memory map tag SHALL contain entry size and entry version
- **AND** each entry SHALL contain base address, length, and type
- **AND** type 1 SHALL indicate available memory
- **AND** type 2 SHALL indicate reserved memory

### Requirement: Boot Device Information
The system SHALL identify the boot device from which the kernel was loaded.

#### Scenario: Multiboot1 boot device
- **WHEN** Multiboot1 boot info is available
- **THEN** boot device SHALL be encoded in `boot_device` as BIOS drive number
- **AND** partition information SHALL be available for BIOS partitioning

#### Scenario: Multiboot2 boot device
- **WHEN** Multiboot2 boot info is available
- **THEN** boot device tag SHALL contain BIOS drive number
- **AND** partition sequence SHALL identify the boot partition

