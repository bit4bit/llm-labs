# Change: Setup MiniOS Project Initial Structure

## Why
The project needs a complete initial setup with source code, build system, and linker configuration to begin implementing the x86 operating system following the Multiboot Specification.

## What Changes
- Create project directory structure (kernel/, iso/boot/grub/)
- Add entry.asm with Multiboot header and entry point
- Add main.c with basic kernel initialization
- Add linker.ld for ELF32 kernel placement
- Add Makefile with build, iso, run, and debug targets
- Add iso/boot/grub/grub.cfg for GRUB boot menu

## Impact
- Affected specs: project-initialization
- Affected code: New files in kernel/, root directory, and iso/
