CC = gcc
AS = as
LD = ld
OBJCOPY = objcopy

CFLAGS = -m32 -fno-pie -no-pie -ffreestanding -O2 -Wall -Wextra
ASFLAGS = --32
LDFLAGS = -T link.ld -nostdlib -m elf_i386

KERNEL_OBJS = src/kernel/boot/multiboot.o src/kernel/boot/boot.o src/kernel/main.o src/kernel/memory/alloc.o src/kernel/memory/paging.o src/kernel/memory/page_dir.o src/kernel/memory/enable_paging.o src/kernel/cpu/gdt.o src/kernel/cpu/idt.o src/kernel/cpu/interrupts.o src/kernel/cpu/tss.o src/kernel/syscall/syscall.o src/kernel/syscall/syscall_asm.o src/kernel/process/process.o src/kernel/process/trampoline.o

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

src/kernel/memory/alloc.o: src/kernel/memory/alloc.c src/kernel/memory/memory.h
	$(CC) $(CFLAGS) -c -o $@ $<

src/kernel/memory/paging.o: src/kernel/memory/paging.c
	$(CC) $(CFLAGS) -c -o $@ $<

src/kernel/memory/page_dir.o: src/kernel/memory/page_dir.asm
	$(AS) $(ASFLAGS) -o $@ $<

src/kernel/memory/enable_paging.o: src/kernel/memory/enable_paging.S
	$(AS) $(ASFLAGS) -o $@ $<

src/kernel/cpu/gdt.o: src/kernel/cpu/gdt.c src/kernel/cpu/gdt.h
	$(CC) $(CFLAGS) -c -o $@ $<

src/kernel/cpu/idt.o: src/kernel/cpu/idt.c src/kernel/cpu/idt.h
	$(CC) $(CFLAGS) -c -o $@ $<

src/kernel/cpu/interrupts.o: src/kernel/cpu/interrupts.c
	$(CC) $(CFLAGS) -c -o $@ $<

src/kernel/cpu/tss.o: src/kernel/cpu/tss.c src/kernel/cpu/tss.h
	$(CC) $(CFLAGS) -c -o $@ $<

src/kernel/syscall/syscall.o: src/kernel/syscall/syscall.c src/kernel/syscall/syscall.h
	$(CC) $(CFLAGS) -c -o $@ $<

src/kernel/syscall/syscall_asm.o: src/kernel/syscall/syscall_asm.S
	$(AS) $(ASFLAGS) -o $@ $<

src/kernel/process/process.o: src/kernel/process/process.c src/kernel/process/process.h
	$(CC) $(CFLAGS) -c -o $@ $<

src/kernel/process/trampoline.o: src/kernel/process/trampoline.S
	$(AS) $(ASFLAGS) -o $@ $<

iso: kernel.bin iso/boot/grub/grub.cfg
	mkdir -p iso/boot/grub
	cp kernel.bin iso/boot/
	grub-mkrescue -o minios.iso iso 2>/dev/null || echo "Install grub-mkresuce for ISO generation"

qemu: iso
	qemu-system-i386 -cdrom minios.iso -m 64 -nographic

clean:
	rm -f kernel.bin minios.iso $(KERNEL_OBJS)
