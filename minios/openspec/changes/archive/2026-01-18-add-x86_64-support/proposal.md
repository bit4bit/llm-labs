# Change: Add x86_64 architecture support

## Why
The kernel is currently compiled for 32-bit i386 architecture. Running on modern x86_64 machines requires either:
1. Running a 32-bit kernel on 64-bit hardware (works but wastes memory addressing capabilities)
2. Compiling the kernel for 64-bit to take advantage of extended registers, larger address space, and improved performance

This change migrates the kernel to x86_64 while maintaining Multiboot compatibility.

## What Changes
- Update Makefile to use 64-bit compilation flags (`-m64`, remove `-m32`)
- Modify linker script to output ELF64 format
- Update entry point assembly to switch to long mode (64-bit)
- Add GDT setup for 64-bit mode (different segment descriptor format)
- Update kernel code to be compatible with 64-bit pointers and calling conventions
- Modify Makefile run/debug targets for 64-bit QEMU invocation
- Update GRUB configuration if needed for 64-bit boot

## Impact
- Affected specs: `project-initialization` (modified capability)
- Affected code:
  - `Makefile` (modified - compiler flags, QEMU targets)
  - `linker.ld` (modified - output format, entry point)
  - `kernel/entry.asm` (modified - long mode setup)
  - `kernel/main.c` (potentially modified - 64-bit compatibility)
  - `openspec/specs/project-initialization/spec.md` (modified requirements)
