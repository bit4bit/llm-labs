AS = nasm
CC = gcc
LD = ld
OBJCOPY = objcopy

ASFLAGS = -f elf64
CFLAGS = -m64 -ffreestanding -fno-pie -O2 -Wall -Wextra -mno-red-zone
LDFLAGS = -T linker.ld -melf_x86_64

KERNEL_OBJS = kernel/entry.o kernel/main.o kernel/vga.o kernel/multiboot2.o

.PHONY: all iso run debug deps clean

all: kernel.elf

kernel.elf: $(KERNEL_OBJS) linker.ld
	$(LD) $(LDFLAGS) -o $@ $(KERNEL_OBJS)

kernel/entry.o: kernel/entry.asm
	$(AS) $(ASFLAGS) -o $@ $<

kernel/main.o: kernel/main.c kernel/vga.h
	$(CC) $(CFLAGS) -c -o $@ $<

kernel/vga.o: kernel/vga.c kernel/vga.h
	$(CC) $(CFLAGS) -c -o $@ $<

kernel/multiboot2.o: kernel/multiboot2.c kernel/multiboot2.h
	$(CC) $(CFLAGS) -c -o $@ $<

iso: kernel.elf
	rm -rf iso/boot
	mkdir -p iso/boot/grub
	cp kernel.elf iso/boot/minios.elf
	# Make sure grub.cfg exists, copy template if missing
	if [ ! -f iso/boot/grub/grub.cfg ]; then \
		echo 'set timeout=5' > iso/boot/grub/grub.cfg; \
		echo 'set default=0' >> iso/boot/grub/grub.cfg; \
		echo 'menuentry "NASM Multiboot2 Kernel" {' >> iso/boot/grub/grub.cfg; \
		echo '    multiboot2 /boot/minios.elf' >> iso/boot/grub/grub.cfg; \
		echo '    boot' >> iso/boot/grub/grub.cfg; \
		echo '}' >> iso/boot/grub/grub.cfg; \
	fi
	grub-mkrescue -o minios.iso iso || \
	xorriso -as mkisofs -o minios.iso -b boot/grub/stage2_eltorito -no-emul-boot -boot-load-size 4 -boot-info-table iso

run: iso
	qemu-system-x86_64 -cdrom minios.iso -boot d

debug: kernel.elf
	@echo "Starting QEMU in debug mode..."
	qemu-system-x86_64 -s -S -kernel kernel.elf &
	qemu_pid=$$!
	sleep 1
	gdb -ex "target remote localhost:1234" -ex "break kernel_main" -ex "continue" kernel.elf || echo "GDB ready on port 1234"
	kill $$qemu_pid 2>/dev/null || true

deps:
	@echo "Checking dependencies..."
	@for cmd in nasm gcc ld objcopy grub-mkrescue xorriso; do \
		if ! command -v $$cmd >/dev/null 2>&1; then \
			echo "ERROR: $$cmd not found. Install with:"; \
			echo "  sudo apt install nasm gcc binutils grub-common xorriso genisoimage"; \
			exit 1; \
		fi; \
	done
	@echo "All dependencies are installed."

clean:
	rm -f kernel.elf minios.iso
	rm -f kernel/*.o
	rm -rf iso/boot
