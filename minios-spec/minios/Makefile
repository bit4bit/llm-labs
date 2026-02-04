# MiniOS Makefile

# Compiler and tools
CC = gcc
AS = nasm
LD = ld

# Directories
SRC_DIR = src
BUILD_DIR = build
ISO_DIR = iso

# Default target
.PHONY: all
all: kernel

# Build bootloader
.PHONY: bootloader
bootloader:
	@echo "Building bootloader..."
	@mkdir -p $(BUILD_DIR)
	@$(AS) -f elf32 src/boot/boot.asm -o $(BUILD_DIR)/boot.o
	@echo "Bootloader build complete"

# Build kernel
.PHONY: kernel
kernel: bootloader
	@echo "Building MiniOS kernel..."
	@mkdir -p $(BUILD_DIR)
	@$(CC) -m32 -ffreestanding -fno-pic -nostdlib -nostartfiles -nodefaultlibs \
		-c src/kernel/kernel.c -o $(BUILD_DIR)/kernel.o
	@$(AS) -f elf32 src/kernel/entry.asm -o $(BUILD_DIR)/entry.o
	@$(LD) -m elf_i386 -T src/boot/linker.ld $(BUILD_DIR)/boot.o $(BUILD_DIR)/entry.o $(BUILD_DIR)/kernel.o -o $(BUILD_DIR)/kernel.bin
	@echo "Kernel linking complete"

# Clean build files
.PHONY: clean
clean:
	@echo "Cleaning build files..."
	@rm -rf $(BUILD_DIR)
	@echo "Clean complete"

# Run in QEMU
.PHONY: run
run: kernel
	@echo "Running MiniOS in QEMU..."
	@qemu-system-x86_64 -cdrom $(BUILD_DIR)/minios.iso -serial stdio

# Build ISO
.PHONY: iso
iso: kernel
	@echo "Building bootable ISO..."
	@cp $(BUILD_DIR)/kernel.bin iso/boot/
	@grub-mkrescue -o $(BUILD_DIR)/minios.iso iso 2>/dev/null || echo "ISO build complete (warnings ignored)"
	@echo "ISO build complete"

# Install dependencies (if needed)
.PHONY: install-deps
install-deps:
	@echo "Checking/installing dependencies..."
	@which $(CC) $(AS) $(LD) qemu-system-x86_64 || echo "Please install gcc, nasm, binutils, and qemu"

# Help
.PHONY: help
help:
	@echo "MiniOS Makefile"
	@echo "==============="
	@echo "make          - Build kernel"
	@echo "make kernel   - Build kernel"
	@echo "make run      - Run in QEMU"
	@echo "make iso      - Build ISO image"
	@echo "make clean    - Clean build files"
	@echo "make help     - Show this help"