# Kernel Development Guide

## Project Overview

MiniOS Minimax is a 32-bit educational operating system for Intel x86 hardware. It provides a minimal viable OS with boot via multiboot, complete paging support, process management, and user-space programs.

## Language Requirements

### C Code
- **Standard**: C18 (ISO/IEC 9899:2018)
- **Compiler**: GCC with `-std=c18 -m32`
- **Flags**: `-fno-pie -no-pie -ffreestanding -O2 -Wall -Wextra`

### Assembly Code
- **Assembler**: GNU Assembler (AS)
- **Syntax**: AT&T syntax (default for GNU as)
- **Target**: 32-bit x86 (`--32` flag)
- **Extensions**: `.S` for preprocessed assembly, `.s` for raw assembly

## Hardware Target

- **Architecture**: 32-bit Intel x86
- **Mode**: Protected mode with paging (handled by GRUB/multiboot)
- **Boot**: Multiboot compliant (GRUB loads kernel in protected mode, no need for manual mode transition)
- **QEMU**: `qemu-system-i386`

## Directory Structure

```
src/kernel/
├── boot/              # Bootstrapping (multiboot, GDT setup)
├── cpu/               # CPU management (GDT, IDT, interrupts, TSS)
├── memory/            # Memory management (PMM, VMM, paging)
├── process/           # Process management and scheduling
├── syscall/           # System call implementation
├── drivers/           # Device drivers
├── main.c             # Kernel entry point
├── kernel.h           # Main kernel header
├── minios.h           # Core definitions
├── serial.c/h         # Serial output
└── programs.c/h       # User program embedding
```

## Build System

### Root Makefile Targets
```bash
make              # Build everything (default)
make all          # Build kernel and programs
make kernel.bin   # Build kernel only
make programs     # Build user programs
make programs-generated  # Generate C arrays from binaries
make clean        # Clean all artifacts
```

### Kernel Compilation
```bash
# 32-bit C compilation (C18 standard)
gcc -std=c18 -m32 -fno-pie -no-pie -ffreestanding -O2 -Wall -Wextra -c -o <output> <source>.c

# 32-bit assembly
as --32 -o <output> <source>.S
```

## Key Subsystems

### Memory Management
- **PMM** (Physical Memory Manager): Allocates physical frames
- **VMM** (Virtual Memory Manager): Manages page tables
- **Paging**: 4KB pages with page fault handling

### Process Management
- **PCB**: Process Control Block structure
- **Scheduler**: Round-robin with context switching
- **Trampoline**: Assembly for mode transitions (kernel ↔ user)

### System Calls
- **Dispatcher**: `syscall.c` routes syscalls
- **Handlers**: `write()`, `exit()`, and others
- **User Interface**: `int 0x80` interface with `syscall_asm.S`

### CPU Management
- **GDT**: Global Descriptor Table for segmentation
- **IDT**: Interrupt Descriptor Table
- **TSS**: Task State Segment for privilege transitions
- **Interrupts**: Interrupt handling and registration

## Coding Conventions

### C Code (C18)
- Use `uint8_t`, `uint32_t` from `<stdint.h>`
- No standard library (freestanding)
- Explicit error handling
- Kernel panic on critical failures

### Assembly Code
- AT&T syntax: `movl $val, %eax`
- Comments with `#` or `/* */`
- Prefix registers with `%` (e.g., `%eax`)
- Prefix immediates with `$` (e.g., `$0x10`)

### Header Files
- Include guards: `#ifndef HEADER_H / #define HEADER_H`
- Minimal includes to avoid circular dependencies
- Static inline for performance-critical functions

## Memory Layout

```
0x00000000 - 0xFFFFFFFF  # 4GB Virtual Address Space

Kernel Space:
  0xC0000000+            # Kernel code and data

User Space:
  0x40000000             # User program base address
  0x40010000+            # Additional programs
```

## Testing

Run QEMU tests after any kernel changes:
```bash
make qemu-simple
```

Verify:
1. Build completes without errors
2. All self-checks pass
3. No kernel panics
4. System boots and operates correctly

## References

- [OSDev Wiki](https://wiki.osdev.org/)
- [Intel Software Developer Manual](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html)
- [Multiboot Spec](https://www.gnu.org/software/grub/manual/multiboot/)
- [System V ABI i386](https://www.sco.com/developers/devspecs/abi386-4.pdf)
