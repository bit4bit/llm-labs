CC = gcc
AS = as
LD = ld
OBJCOPY = objcopy

CFLAGS = -m32 -fno-pie -no-pie -ffreestanding -O2 -Wall -Wextra
ASFLAGS = --32
LDFLAGS = -T link.ld -nostdlib -m elf_i386

KERNEL_OBJS = src/kernel/boot/multiboot.o src/kernel/boot/boot.o src/kernel/serial.o src/kernel/main.o src/kernel/programs.o src/kernel/minios-c.o src/kernel/memory/alloc.o src/kernel/memory/vmm.o src/kernel/memory/page_dir.o src/kernel/memory/enable_paging.o src/kernel/cpu/gdt.o src/kernel/cpu/idt.o src/kernel/cpu/interrupts.o src/kernel/cpu/tss.o src/kernel/syscall/syscall.o src/kernel/syscall/syscall_asm.o src/kernel/process/process.o src/kernel/process/trampoline.o

.PHONY: all iso qemu qemu-test qemu-simple qemu-debug qemu-int qemu-vga help clean programs programs-clean programs-generated

# Default target
all: programs programs-generated kernel.bin

# Help target - show available commands
help:
	@echo "MinOS Minimax - Available Make Targets:"
	@echo ""
	@echo "  make              - Build programs and kernel.bin (default)"
	@echo "  make all          - Same as 'make'"
	@echo "  make programs     - Build user programs only"
	@echo "  make clean        - Remove all build artifacts"
	@echo ""
	@echo "QEMU Execution:"
	@echo "  make qemu         - Run kernel from ISO with GRUB"
	@echo "  make qemu-simple  - Run kernel directly (clean output)"
	@echo "  make qemu-test    - Run with CPU debugging (int + cpu_reset)"
	@echo "  make qemu-debug   - Run with full CPU debug (saves to qemu.log)"
	@echo "  make qemu-int     - Run with interrupt debugging only"
	@echo "  make qemu-vga     - Run with graphical display (see VGA output)"
	@echo ""
	@echo "Build artifacts:"
	@echo "  make iso          - Create bootable ISO image"
	@echo ""
	@echo "User Programs:"
	@echo "  make programs          - Compile C programs to binaries"
	@echo "  make programs-generated - Generate C arrays from binaries"
	@echo "  make programs-clean    - Clean program build artifacts"
	@echo ""
	@echo "Expected output: Program prints 'Hello from C program!' to VGA memory"
	@echo ""

# Build user programs
programs:
	$(MAKE) -C programs

# Generate C arrays from program binaries
programs-generated:
	$(MAKE) -C programs generated

# Clean program build artifacts
programs-clean:
	$(MAKE) -C programs clean

kernel.bin: $(KERNEL_OBJS) link.ld
	$(LD) $(LDFLAGS) -o $@ $(KERNEL_OBJS)

src/kernel/boot/multiboot.o: src/kernel/boot/multiboot.S
	$(AS) $(ASFLAGS) -o $@ $<

src/kernel/boot/boot.o: src/kernel/boot/boot.s
	$(AS) $(ASFLAGS) -o $@ $<

src/kernel/serial.o: src/kernel/serial.c src/kernel/serial.h src/kernel/minios.h
	$(CC) $(CFLAGS) -c -o $@ $<

src/kernel/main.o: src/kernel/main.c src/kernel/programs.h
	$(CC) $(CFLAGS) -c -o $@ $<

src/kernel/programs.o: src/kernel/programs.c src/kernel/programs.h
	$(CC) $(CFLAGS) -c -o $@ $<

src/kernel/minios-c.o: src/kernel/minios-c.c src/kernel/minios-c.h
	$(CC) $(CFLAGS) -c -o $@ $<

src/kernel/memory/alloc.o: src/kernel/memory/pmm.c src/kernel/memory/memory.h
	$(CC) $(CFLAGS) -c -o $@ $<

src/kernel/memory/vmm.o: src/kernel/memory/vmm.c
	$(CC) $(CFLAGS) -c -o $@ $<

src/kernel/memory/page_dir.o: src/kernel/memory/page_dir.S
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

# Run kernel with serial output and CPU debugging (interrupts + CPU resets)
qemu-test: kernel.bin
	qemu-system-i386 -kernel kernel.bin -serial stdio -display none -d int,cpu_reset

# Run kernel with simple serial output (no debug info)
qemu-simple: kernel.bin
	qemu-system-i386 -kernel kernel.bin -serial stdio -display none

# Run kernel with full CPU state debugging (verbose)
qemu-debug: kernel.bin
	qemu-system-i386 -kernel kernel.bin -serial stdio -display none -d int,cpu_reset,cpu -D qemu.log

# Run kernel with interrupt debugging only
qemu-int: kernel.bin
	qemu-system-i386 -kernel kernel.bin -serial stdio -display none -d int

# Run kernel with graphical display (to see VGA output)
qemu-vga: kernel.bin
	qemu-system-i386 -kernel kernel.bin -serial stdio

clean: programs-clean
	rm -f kernel.bin minios.iso $(KERNEL_OBJS) qemu.log
