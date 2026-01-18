## Context
MiniOS currently targets 32-bit i386 architecture. Modern hardware runs in 64-bit mode (long mode) by default, and GRUB can boot 64-bit ELF kernels. This change migrates the kernel to x86_64 to:
- Take advantage of 64-bit registers and larger address space
- Match modern hardware capabilities
- Improve kernel performance with 64-bit operations

Constraints:
- Must maintain GRUB Multiboot compatibility
- Must boot on standard x86_64 hardware with GRUB
- Should minimize code changes while ensuring correctness

## Goals / Non-Goals

### Goals:
- Compile kernel as 64-bit ELF executable
- Successfully boot via GRUB on x86_64 systems
- Maintain VGA text output functionality
- Enable use of 64-bit registers and addressing
- Support QEMU x86_64 emulation

### Non-Goals:
- Full 64-bit paging implementation (keep identity mapping initially)
- SMP (Symmetric Multiprocessing) support
- ACPI integration
- UEFI boot support (keep BIOS/GRUB)
- 64-bit driver framework (keep simple VGA text mode)

## Decisions

### Decision: Use Multiboot1 vs Multiboot2
- **Choice**: Keep Multiboot1 (current)
- **Rationale**: Multiboot1 is simpler and GRUB supports both 32-bit and 64-bit kernels
- **Risk**: Multiboot1 spec was designed for 32-bit, but works fine for 64-bit boot

### Decision: Long mode entry sequence
- **Choice**: Direct long mode enable sequence
- **Steps**: Disable interrupts → Load GDT64 → Enable PAE → Enable long mode → Paging enable → Far jump to 64-bit code
- **Rationale**: Standard sequence, well-documented, minimal complexity

### Decision: Memory layout
- **Choice**: Keep 1MB load address (0x100000)
- **Rationale**: Simple, avoids low memory conflicts, works with GRUB

## Risks / Trade-offs

| Risk | Impact | Mitigation |
|------|--------|------------|
| GRUB compatibility | Medium | Test with multiple GRUB versions; GRUB2 supports 64-bit Multiboot |
| Assembly complexity | Medium | Use well-tested long mode enable sequence; add comments |
| Testing hardware | Low | Primary testing in QEMU x86_64; secondary on real hardware |

## Migration Plan

1. **Phase 1**: Update build system (Makefile, linker script)
2. **Phase 2**: Rewrite entry.asm for 64-bit long mode entry
3. **Phase 3**: Update kernel code for 64-bit compatibility
4. **Phase 4**: Build and test in QEMU
5. **Phase 5**: Verify on real x86_64 hardware

Rollback: If issues arise, revert Makefile changes to `-m32` and use 32-bit entry point.

## Open Questions
- [ ] Should we use `-fno-pie` for 64-bit? (Yes, for consistent linking)
- [ ] What is the correct GDT64 layout for code and data segments?
- [ ] Do we need a 64-bit IDT for interrupts?

## References
- OSDev Wiki: Long Mode (https://wiki.osdev.org/Long_Mode)
- Intel SDM Volume 3: System Programming Guide
- Multiboot Specification: https://www.gnu.org/software/grub/manual/multiboot/
