# Project Context

## Purpose
A minimal 32-bit operating system kernel written in C and x86 Assembly. Educational project demonstrating OS development basics - booting via Multiboot specification and displaying text to VGA framebuffer. Target: POSIX-compatible API and userland compatibility.

## Tech Stack
- **Assembly**: NASM (x86 32-bit, Intel syntax)
- **C**: GCC with `-m32 -ffreestanding -nostdlib` flags
- **Linker**: GNU ld (ELF32-i386 output)
- **Bootloader**: GRUB (Multiboot compliant)
- **Emulator**: QEMU (qemu-system-i386)
- **Build System**: GNU Make
- **POSIX**: musl libc (for userland compatibility)

## Project Conventions

### Code Style
- C code follows K&R style with 4-space indentation
- Assembly uses lower case with `section` directives for segment organization
- No comments in production code (as per existing files)

### Architecture Patterns
- **Multiboot compliant**: Header at `_start` with magic `0x1BADB002`
- **Higher half kernel**: Linked at `0x100000` (1MB)
- **Stack-based**: 16KB stack growing downward from entry point
- **VGA text mode**: Direct video memory writes to `0xB8000` (80x25, 16-color)
- **No stdlib**: Freestanding environment for kernel code
- **POSIX subsystem**: Implements POSIX APIs for userland compatibility
- **ELF32 executable format**: Standard Unix-like executable format

### Testing Strategy
- Run with `make run` or `make qemu` to test in QEMU
- Build ISO with `make iso` for physical hardware/GRUB testing
- `make clean` removes all build artifacts
- POSIX compliance tested via shell and userland utilities

### Git Workflow
- Simple linear history suitable for small educational project
- Conventional commits: `feat:`, `fix:`, `docs:`, `refactor:`
- Feature branches for major POSIX subsystem implementations

## Domain Context
- x86 protected mode (32-bit)
- Multiboot specification for bootloader interface
- VGA text mode video memory layout: `character + (color << 8)` per cell
- Colors: lower 4 bits = foreground, upper 4 bits = background
- POSIX APIs: fork, exec, open, read, write, close, pipe, signal
- ELF32 executable loading and program headers
- Virtual memory management for user processes

## Important Constraints
- Must boot via GRUB or Multiboot-compliant bootloader
- 32-bit protected mode only
- No standard library in kernel (freestanding)
- POSIX compatibility requires careful syscall interface design
- Limited to 4GB address space (32-bit)

## External Dependencies
- NASM assembler
- GCC cross-compiler (i386-elf target for proper freestanding code)
- GNU binutils (ld, objcopy)
- GRUB (for ISO generation)
- QEMU (for testing)
- musl libc (for building POSIX-compatible userland programs)
