## Context
MiniOS currently uses Multiboot 1.09 specification for boot protocol. Multiboot 2 is the successor specification that provides better support for modern 64-bit systems and more extensible boot information. The goal is to migrate to Multiboot 2 while maintaining compatibility with GRUB2 bootloaders.

Constraints:
- Must maintain backward compatibility with existing GRUB installations
- Must work with both 32-bit and 64-bit kernels
- Should not break existing boot workflow
- Must fit Multiboot2 header within first 8192 bytes of kernel

## Goals / Non-Goals

### Goals:
- Implement Multiboot2 header in kernel entry point
- Parse Multiboot2 boot information tags correctly
- Support memory map, boot device, and command line tags
- Support EFI system table and RSDP tags for ACPI
- Boot successfully with GRUB2 using `multiboot2` directive
- Maintain VGA text output functionality after boot

### Non-Goals:
- Custom multiboot2 bootloader implementation
- Full UEFI boot support (keep BIOS/GRUB)
- SMP boot protocol support
- Advanced ACPI table parsing beyond RSDP location
- 32-bit kernel Multiboot2 support (focus on 64-bit)

## Decisions

### Decision: Multiboot2 header format
- **Magic**: `0xE85250D6` (Multiboot2)
- **Architecture**: `0` for i386, `4` for x86_64
- **Header length**: Total length of Multiboot2 header
- **Checksum**: `-(magic + architecture + header_length) % 2^32`
- **Requirements**: Must be within first 8192 bytes, aligned to 8-byte boundary

### Decision: Boot information tag parsing
- **Choice**: Iterative tag parsing using start tag and end tag markers
- **Tags to support**: `BT_INFO_MEMORY`, `BT_INFO_BOOTDEV`, `BT_INFO_CMDLINE`, `BT_INFO_MODS`, `BT_INFO_MMAP`, `BT_INFO_ELF_SYMS`, `BT_INFO_PTM`, `BT_INFO_EFI32`, `BT_INFO_EFI64`, `BT_INFO_BOOT_LOADER_NAME`, `BT_INFO_RSDP`
- **Rationale**: Standard approach, well-documented in Multiboot2 spec

### Decision: GRUB configuration
- **Choice**: Use `multiboot2` directive in grub.cfg
- **Rationale**: Explicitly uses Multiboot2 protocol
- **Alternative**: Use `multiboot` which auto-detects, but explicit is clearer

### Decision: 64-bit focused
- **Choice**: Target x86_64 architecture for Multiboot2 support
- **Rationale**: Modern systems are 64-bit, Multiboot2 designed for 64-bit

## Risks / Trade-offs

| Risk | Impact | Mitigation |
|------|--------|------------|
| GRUB compatibility | Low | GRUB2 supports both Multiboot1 and Multiboot2 |
| Header alignment | Medium | Ensure 8-byte alignment in linker script |
| Tag parsing errors | Medium | Add validation for tag size and type |
| Migration complexity | Low | Can maintain Multiboot1 as fallback |

## Migration Plan

1. **Phase 1**: Update entry.asm with Multiboot2 header
2. **Phase 2**: Implement boot info tag parsing in C
3. **Phase 3**: Update grub.cfg to use multiboot2 directive
4. **Phase 4**: Test boot with GRUB2 in QEMU
5. **Phase 5**: Verify functionality (VGA output, memory info)

Rollback: If Multiboot2 causes issues, grub.cfg can revert to `multiboot` directive (GRUB auto-detects).

## Open Questions
- [ ] Should we support both Multiboot1 and Multiboot2 simultaneously?
- [ ] What is the minimum GRUB2 version required for Multiboot2?
- [ ] Should we remove Multiboot1 header after successful Multiboot2 migration?

## References
- Multiboot2 Specification: https://www.gnu.org/software/grub/manual/multiboot2/
- OSDev Wiki: Multiboot2 (https://wiki.osdev.org/Multiboot_2)
- GRUB Manual: Multiboot2 (https://www.gnu.org/software/grub/manual/grub/grub.html#multiboot2)
