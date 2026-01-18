## 1. Multiboot2 Header Implementation
- [ ] 1.1 Update kernel/entry.asm with Multiboot2 header (magic 0xE85250D6)
- [ ] 1.2 Set correct architecture field (4 for x86_64)
- [ ] 1.3 Calculate and set Multiboot2 checksum
- [ ] 1.4 Ensure 8-byte alignment of Multiboot2 header
- [ ] 1.5 Verify header is within first 8192 bytes of kernel

## 2. Boot Information Parsing
- [ ] 2.1 Define Multiboot2 tag structures in multiboot2.h
- [ ] 2.2 Implement boot info tag iteration loop
- [ ] 2.3 Implement memory map tag parsing
- [ ] 2.4 Implement boot device tag parsing
- [ ] 2.5 Implement command line tag parsing
- [ ] 2.6 Add EFI system table tag support (optional)
- [ ] 2.7 Add RSDP tag parsing for ACPI (optional)

## 3. Kernel Integration
- [ ] 3.1 Update kernel/main.c to use Multiboot2 boot info
- [ ] 3.2 Replace Multiboot1 magic check with Multiboot2 (0x36D76289)
- [ ] 3.3 Update memory info display to use Multiboot2 format
- [ ] 3.4 Test VGA output after Multiboot2 boot

## 4. GRUB Configuration
- [ ] 4.1 Update iso/boot/grub/grub.cfg to use `multiboot2` directive
- [ ] 4.2 Test ISO creation with GRUB2

## 5. Testing
- [ ] 5.1 Build kernel with `make`
- [ ] 5.2 Create ISO with `make iso`
- [ ] 5.3 Test boot in QEMU with `make run`
- [ ] 5.4 Verify memory map is correctly parsed
- [ ] 5.5 Verify boot device info is available
- [ ] 5.6 Test with real hardware if possible
