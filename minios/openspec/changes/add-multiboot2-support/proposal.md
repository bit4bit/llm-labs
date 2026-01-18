# Change: Add Multiboot 2 specification support

## Why
The current kernel uses Multiboot 1.09 specification which was designed primarily for 32-bit systems. Multiboot 2 offers:
- Better support for 64-bit kernels with standardized boot protocols
- Improved memory map information formats
- Better EFI system table handling
- More extensible boot information structure
- Modern bootloader compatibility (GRUB 2 supports both, but increasingly favors Multiboot 2)

## What Changes
- Replace Multiboot 1 header with Multiboot 2 header (different magic, flags, checksum format)
- Update entry.asm to parse Multiboot 2 boot information structure
- Modify kernel boot info parsing to handle Multiboot 2 tag format
- Update linker script and Makefile if needed for Multiboot 2 compliance
- Support both 32-bit and 64-bit boot information structures
- Add RSDP (ACPI) tag parsing support
- Update GRUB configuration to use `multiboot2` directive

## Impact
- Affected specs: `project-initialization` (modified capability)
- Affected code:
  - `kernel/entry.asm` (modified - Multiboot header and boot info parsing)
  - `kernel/main.c` (modified - boot info handling)
  - `iso/boot/grub/grub.cfg` (modified - multiboot2 directive)
  - `openspec/specs/project-initialization/spec.md` (modified requirements)
