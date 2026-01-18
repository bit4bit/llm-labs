AS = nasm
CC = gcc
LD = ld
OBJCOPY = objcopy

ASFLAGS = -f elf32
CFLAGS = -m32 -ffreestanding -fno-pie -O2 -Wall -Wextra
LDFLAGS = -T linker.ld -melf_i386

KERNEL_OBJS = kernel/entry.o kernel/main.o

.PHONY: all iso run debug deps clean

all: kernel.elf

kernel.elf: $(KERNEL_OBJS) linker.ld
	$(LD) $(LDFLAGS) -o $@ $(KERNEL_OBJS)

kernel/entry.o: kernel/entry.asm
	$(AS) $(ASFLAGS) -o $@ $<

kernel/main.o: kernel/main.c
	$(CC) $(CFLAGS) -c -o $@ $<

iso: kernel.elf
	rm -rf iso/boot/stage2_eltorito iso/boot/grub/boot.img
	mkdir -p iso/boot/grub
	cp kernel.elf iso/boot/minios.elf
	cp iso/boot/grub/grub.cfg.template iso/boot/grub/grub.cfg 2>/dev/null || true
	if [ ! -f iso/boot/grub/grub.cfg ]; then \
		echo 'set default=0' > iso/boot/grub/grub.cfg && \
		echo 'set timeout=0' >> iso/boot/grub/grub.cfg && \
		echo '' >> iso/boot/grub/grub.cfg && \
		echo 'menuentry "minios" {' >> iso/boot/grub/grub.cfg && \
		echo '    multiboot /boot/minios.elf' >> iso/boot/grub/grub.cfg && \
		echo '    boot' >> iso/boot/grub/grub.cfg && \
		echo '}' >> iso/boot/grub/grub.cfg; \
	fi
	grub-mkrescue -o minios.iso iso 2>/dev/null || xorriso -as mkisofs -o minios.iso -boot-info-table -boot-load-size 4 -append_partition 2 0xef iso/efiboot.img iso 2>/dev/null || (echo "Trying alternative ISO creation..." && genisoimage -o minios.iso -b boot/grub/stage2_eltorito -no-emul-boot -boot-load-size 4 -boot-info-table -r -J iso)

run: iso
	qemu-system-x86 -cdrom minios.iso -boot d

debug: kernel.elf
	qemu-system-x86 -s -S -kernel kernel.elf &
	qemu_pid=$$!
	sleep 1
	gdb -ex "target remote localhost:1234" -ex "break kernel_main" -ex "continue" vmlinux 2>/dev/null || (echo "GDB connection ready on port 1234"; kill $$qemu_pid 2>/dev/null)

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
	rm -rf iso/boot/stage2_eltorito iso/boot/grub/boot.img iso/boot/minios.elf
