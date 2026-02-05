## 1. Build System Setup

- [ ] 1.1 Create `Makefile` with CC=i686-linux-gnu-gcc, AS=i686-linux-gnu-as
- [ ] 1.2 Add targets: build, iso, qemu, clean
- [ ] 1.3 Add CFLAGS: -m32 -fno-pie -no-pie -ffreestanding -O2
- [ ] 1.4 Create `link.ld` with ENTRY(_start), SECTIONS for .text at 0x100000

## 2. Multiboot Header

- [ ] 2.1 Create `src/kernel/boot/multiboot.S` with header magic 0x1BADB002
- [ ] 2.2 Add flags 0x00010000 (align modules on page boundary)
- [ ] 2.3 Include checksum for valid header

## 3. Bootstrap Assembly

- [ ] 3.1 Create `src/kernel/boot/boot.s` with _start label
- [ ] 3.2 Declare global _start for linker
- [ ] 3.3 Create minimal stack (declare 4096 bytes stack space)
- [ ] 3.4 Call kernel_main

## 4. Kernel Main

- [ ] 4.1 Create `src/kernel/main.c` with kernel_main()
- [ ] 4.2 Define VGA buffer address: (volatile uint16_t*) 0xB8000
- [ ] 4.3 Write "MinOS Loaded" character by character
- [ ] 4.4 Use attribute 0x07 for gray on black
- [ ] 4.5 Infinite loop with hlt after printing

## 5. Directory Structure

- [ ] 5.1 Create `src/kernel/boot/` directory
- [ ] 5.2 Create `src/kernel/` directory
- [ ] 5.3 Create `iso/boot/grub/` directory

## 6. ISO Creation

- [ ] 6.1 Create `iso/boot/grub/grub.cfg` with multiboot /kernel.bin
- [ ] 6.2 Add `make iso` target using grub-mkrescue or xorriso
- [ ] 6.3 Verify ISO is bootable with `grub-file --is-x86-multiboot iso/boot/grub/i386-pc/boot.img`

## 7. Testing

- [ ] 7.1 Run `make` to compile kernel.bin
- [ ] 7.2 Run `make iso` to create ISO
- [ ] 7.3 Run `make qemu` to test in QEMU
- [ ] 7.4 Verify "MinOS Loaded" appears on screen
