AS = nasm
CC = gcc
LD = ld

ASFLAGS = -f elf32
CFLAGS = -m32 -ffreestanding -O2 -Wall -Wextra -nostdlib
LDFLAGS = -T linker.ld -m elf_i386

TARGET = boot.img
ISO = minios.iso

.PHONY: all clean run iso qemu

all: $(TARGET)

kernel.bin: kernel_asm.o kernel_c.o
	$(LD) $(LDFLAGS) -o kernel.bin kernel_asm.o kernel_c.o

kernel_asm.o: kernel.asm
	$(AS) $(ASFLAGS) kernel.asm -o kernel_asm.o

kernel_c.o: kernel.c
	$(CC) $(CFLAGS) -c kernel.c -o kernel_c.o

$(TARGET): kernel.bin
	cp kernel.bin $(TARGET)

run: $(TARGET)
	qemu-system-i386 -kernel $(TARGET)

qemu: run

iso: isodir/boot/grub/grub.cfg $(ISO)

isodir/boot/grub/grub.cfg: kernel.bin
	mkdir -p isodir/boot/grub
	echo 'multiboot /boot/kernel.bin' > isodir/boot/grub/grub.cfg
	echo 'boot' >> isodir/boot/grub/grub.cfg
	cp kernel.bin isodir/boot/

$(ISO): isodir/boot/grub/grub.cfg kernel.bin
	grub-mkrescue -o $(ISO) isodir 2>/dev/null || echo "Install grub-common for ISO generation"

clean:
	rm -f kernel_asm.o kernel_c.o kernel.bin $(TARGET) $(ISO)
	rm -rf isodir
