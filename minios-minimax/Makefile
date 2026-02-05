CC = gcc
AS = as
LD = ld
OBJCOPY = objcopy

CFLAGS = -m32 -fno-pie -no-pie -ffreestanding -O2 -Wall -Wextra
ASFLAGS = --32
LDFLAGS = -T link.ld -nostdlib -m elf_i386

KERNEL_OBJS = src/kernel/boot/multiboot.o src/kernel/boot/boot.o src/kernel/main.o

.PHONY: all iso qemu clean

all: kernel.bin

kernel.bin: $(KERNEL_OBJS) link.ld
	$(LD) $(LDFLAGS) -o $@ $(KERNEL_OBJS)

src/kernel/boot/multiboot.o: src/kernel/boot/multiboot.S
	$(AS) $(ASFLAGS) -o $@ $<

src/kernel/boot/boot.o: src/kernel/boot/boot.s
	$(AS) $(ASFLAGS) -o $@ $<

src/kernel/main.o: src/kernel/main.c
	$(CC) $(CFLAGS) -c -o $@ $<

iso: kernel.bin iso/boot/grub/grub.cfg
	mkdir -p iso/boot/grub
	cp kernel.bin iso/boot/
	grub-mkrescue -o minios.iso iso 2>/dev/null || echo "Install grub-mkresuce for ISO generation"

qemu: iso
	qemu-system-i386 -cdrom minios.iso -m 64 -nographic

clean:
	rm -f kernel.bin minios.iso $(KERNEL_OBJS)
