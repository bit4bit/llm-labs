CC = gcc
LD = ld
OBJCOPY = objcopy

# Compiler flags for 32-bit freestanding programs
CFLAGS = -m32 -ffreestanding -nostdlib -fno-pie -fno-stack-protector \
         -fno-builtin -nostdinc -O2 -Wall -Wextra -I./lib

# Linker flags
LDFLAGS = -m elf_i386 -T user.ld --no-relax

# Programs to build
PROGRAMS = hello/hello.bin

.PHONY: all clean programs generated help

# Default target
all: programs

# Build all programs
programs: $(PROGRAMS)

# Generate C arrays from binaries
generated: generated/hello_bin.c

# Help target
help:
	@echo "User Programs Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  make              - Build all programs (default)"
	@echo "  make programs     - Build all .bin files"
	@echo "  make generated    - Generate C arrays from binaries"
	@echo "  make clean        - Remove all build artifacts"
	@echo ""
	@echo "Programs:"
	@echo "  hello/hello.bin   - Hello world program"
	@echo ""

# Create generated directory
generated/:
	mkdir -p generated

# Compile C source to object file
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Link object file to binary
%.bin: %.o
	$(LD) $(LDFLAGS) -o $@ $<

# Generate C array from binary
generated/hello_bin.c: hello/hello.bin | generated/
	../tools/bin2c.py $< hello_bin > $@

clean:
	rm -rf $(PROGRAMS) */*.o generated/
	@echo "Cleaned all build artifacts"

# Specific program rules
hello/hello.bin: hello/hello.o

# Show what will be built
list:
	@echo "Programs to build:"
	@for prog in $(PROGRAMS); do echo "  $$prog"; done
	@echo ""
	@echo "Generated arrays:"
	@for gen in $(GENERATED); do echo "  $$gen"; done
