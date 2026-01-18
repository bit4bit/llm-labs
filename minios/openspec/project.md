# Project Context

## Purpose
Build a minimal x86 operating system from scratch following the **Multiboot Specification** (Multiboot 1.09) to:
- Boot via Multiboot-compliant bootloaders (GRUB)
- Kernel loaded at aligned memory boundary
- Receive boot info structure (memory map, boot device, etc.)
- Understand kernel initialization and boot protocols

Goals:
- Implement Multiboot header in kernel
- Boot successfully with GRUB
- Basic VGA text mode output
- Memory management (GDT, paging)
- Simple keyboard input handling

## Tech Stack
- **Assembly**: NASM (for entry point, GDT, low-level routines)
- **Kernel**: C with GCC (freestanding, `-m32`, `-ffreestanding`, `-mno-red-zone`)
- **Linker**: GNU ld with custom linker script
- **Bootloader**: GRUB (Multiboot compliant)
- **Emulation/Testing**: QEMU (`qemu-system-x86` with `-kernel` or via GRUB)
- **Debugging**: GDB (with QEMU GDB stub)
- **Build**: GNU Make

## Project Conventions

### Code Style
- Use LF line endings (Unix convention)
- Assembly: Intel syntax (NASM), lowercase mnemonics
- C: C11 standard, K&R style acceptable for bare metal
- Indent with tabs, not spaces
- Max line width: 120 characters

### Architecture Patterns
```
├── kernel/
│   ├── entry.asm       # Multiboot header, entry point
│   ├── main.c          # Kernel main, initialization
│   ├── gdt.c           # Global Descriptor Table
│   ├── idt.c           # Interrupt Descriptor Table
│   └── vga.c           # VGA text mode output
├── bootloader/         # Optional: custom multiboot bootloader (if needed)
├── linker.ld           # Linker script for kernel placement
├── Makefile            # Build configuration
└── iso/                # ISO with GRUB for QEMU
```

- **Multiboot header**: Must be within first 8192 bytes, aligned on 4-byte boundary
- **Kernel format**: ELF32 executable
- **Load address**: Must be aligned to page size (4096 bytes) for Multiboot
- **Entry point**: `_start` in `entry.asm`, exports `multiboot_header_t`

### Testing Strategy
- **Integration testing**: 
  - Test with GRUB via ISO image
  - Command: `qemu-system-x86 -cdrom minios.iso -boot d`
  - Or with `-kernel` flag (simulates direct boot): `qemu-system-x86 -kernel minios.elf`
- **Automated testing**: `make run` builds ISO and runs in QEMU
- **Debugging**:
  - `make debug` starts QEMU with GDB stub on port 1234
  - Connect: `gdb -ex "target remote localhost:1234" vmlinux`
  - Set breakpoints at kernel symbols: `break kernel_main`

### Git Workflow
- **Branching**: Feature branches (e.g., `feat/gdt`, `feat/paging`)
- **Commits**: Conventional commits (`feat:`, `fix:`, `docs:`)
- **Main branch**: `main` (production-ready, boots with GRUB)
- **Tags**: Tag bootable versions (e.g., `v0.1-multiboot`, `v0.2-gdt`)

## Domain Context

### Multiboot Specification (Multiboot 1.09)
- **Header requirements**:
  - Magic: `0x1BADB002`
  - Flags: `0x00010003` (align memory + provide memory info)
  - Checksum: `-(magic + flags) % 2^32`
  - Header must be within first 8192 bytes of kernel
  - Header must be aligned on 4-byte boundary

- **Boot info structure** (`multiboot_info_t`):
  - Address of memory map (`mmap_*`)
  - Boot device (`boot_device`)
  - Command line (`cmdline`)
  - Symbols info (for debugging)

- **Boot flow with GRUB**:
  1. BIOS loads GRUB from MBR
  2. GRUB reads kernel ELF from disk
  3. GRUB places kernel at aligned address
  4. GRUB sets up initial registers:
     - `EAX`: Multiboot magic (`0x2BADB002`)
     - `EBX`: Pointer to `multiboot_info_t`
  5. Jump to kernel entry

### Memory Layout (Multiboot)
```
0x00000000 - 0x000FFFFF    # Lower 1MB (identity mapped initially)
0x00100000 -               # Kernel loaded here (page-aligned)
Stack: Grows downward from high address

Multiboot info structure provided by bootloader:
- mmap_addr: pointer to memory map
- mem_*: lower/upper memory values
```

### VGA Text Mode
- Base address: `0xB8000` (color VGA)
- Format: 2 bytes per character (character byte + attribute byte)
- Standard mode: 80x25 characters

### Key Multiboot Constants
```asm
MULTIBOOT_HEADER_MAGIC     equ 0x1BADB002
MULTIBOOT_HEADER_FLAGS     equ 0x00010003
MULTIBOOT_HEADER_CHECKSUM  equ -0x1BADB003  ; -(MAGIC + FLAGS)
MULTIBOOT_BOOTLOADER_MAGIC equ 0x2BADB002
```

### Useful Resources
- Multiboot Spec: https://www.gnu.org/software/grub/manual/multiboot/
- OSDev Wiki: https://wiki.osdev.org/Multiboot
- Intel Software Developer's Manual

## Important Constraints
- No standard library (libc unavailable)
- Must fit Multiboot header within first 8192 bytes
- Kernel must be valid ELF32
- No dynamic memory until heap is set up
- Manual page table setup for paging support
- Use `-fno-pie` and `-mno-red-zone` for compatibility

## External Dependencies
- **GRUB**: Bootloader (apt: `grub-pc`, `grub-common`)
- **NASM**: Assembler (apt: `nasm`)
- **GCC multilib**: 32-bit support (apt: `gcc-multilib`)
- **GDB**: Debugger (apt: `gdb`)
- **GNU ld**: Linker (part of `binutils`)
- **xorriso**: ISO creation (apt: `xorriso`)
- **mtools**: For GRUB installation (apt: `mtools`)

## Build Commands
```bash
make           # Build kernel.elf
make iso       # Create bootable ISO with GRUB
make run       # Run ISO in QEMU
make debug     # Run QEMU with GDB stub (port 1234)
make deps      # Verify all dependencies are installed
make clean     # Remove build artifacts
```

## Dependency Check Target
The `make deps` target verifies that all required dependencies are installed:
```makefile
deps:
	@echo "Checking dependencies..."
	@for cmd in nasm gcc ld grub-mkrescue xorriso; do \
		if ! command -v $$cmd >/dev/null 2>&1; then \
			echo "ERROR: $$cmd not found. Install with:"; \
			echo "  sudo apt install nasm gcc binutils grub-common xorriso"; \
			exit 1; \
		fi; \
	done
	@echo "All dependencies are installed."
```
- Checks for: `nasm`, `gcc`, `ld`, `grub-mkrescue` (or `xorriso`)
- Shows installation command if any dependency is missing
- Exits with error code 1 if verification fails

## GRUB Configuration (iso/boot/grub/grub.cfg)
```
set default=0
set timeout=0

menuentry "minios" {
    multiboot /boot/minios.elf
    boot
}
```
