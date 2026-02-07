# C Programs Setup - Complete Guide

This document describes the complete setup for writing user programs in C instead of assembly for MiniOS Minimax.

## Overview

Previously, user programs were written in assembly (.asm) and manually converted to byte arrays embedded in the kernel. Now, programs can be written in C, compiled automatically, and converted to kernel-embeddable format.

## Files Created

### 1. User Library (`programs/lib/`)

#### `syscall.h` - System Call Interface
- Defines syscall numbers (SYS_EXIT, SYS_WRITE)
- Provides inline syscall wrappers (`exit()`, `write()`)
- Uses inline assembly to invoke `int 0x80`
- No separate `.c` file needed - all inline for minimal overhead

#### `stdint.h` - Standard Integer Types
- Defines fixed-width types (uint8_t, uint32_t, etc.)
- Required because we compile with `-nostdinc`
- Provides NULL definition
- 32-bit x86 ABI compliant

### 2. Build System

#### `programs/user.ld` - Linker Script
- Sets output format to flat binary
- Places program at 0x40000000 (USER_PROGRAM_BASE)
- Entry point: `_start`
- Sections: .text, .rodata, .data, .bss
- Discards debug/metadata sections

#### `programs/Makefile` - Build Automation
- Compiles C programs to 32-bit freestanding binaries
- Links with custom linker script
- Generates C arrays from binaries
- Targets:
  - `make programs` - Build all .bin files
  - `make generated` - Generate C arrays
  - `make clean` - Remove artifacts

#### Root `Makefile` Updates
- Added `programs` target to build user programs
- Added `programs-generated` target
- Added `programs-clean` target
- Modified `all` target to depend on `programs`
- Integrated program build into main workflow

### 3. Tools

#### `tools/bin2c.sh` - Binary to C Array Converter
- Converts flat binary to C byte array
- Usage: `./tools/bin2c.sh input.bin array_name > output.c`
- Generates `uint8_t array[]` and `uint32_t array_size`
- Portable (works on Linux and macOS)

#### `tools/test-programs.sh` - Test Suite
- Validates entire build pipeline
- 22 automated tests covering:
  - File existence checks
  - Build process verification
  - Binary content validation
  - Tool functionality tests
  - Syntax checking
- Run with: `./tools/test-programs.sh`

### 4. Example Programs

#### `programs/hello/hello.c` - Hello World
- Demonstrates basic C program structure
- Shows syscall usage
- Calculates string length manually
- Proper exit handling
- 78 bytes compiled size

### 5. Documentation

#### `programs/README.md` - User Guide
- Complete guide for writing C programs
- API documentation for syscalls
- Build process explanation
- Troubleshooting tips
- Multiple examples

## Program Structure

### Minimal C Program Template

```c
#include "../lib/syscall.h"

void _start(void) {
    const char* msg = "Hello!\n";
    
    // Calculate length
    uint32_t len = 0;
    while (msg[len]) len++;
    
    // Write to stdout
    write(1, msg, len);
    
    // Exit
    exit(0);
}
```

### Key Requirements

1. **Entry Point**: Must be `_start`, NOT `main()`
2. **No Standard Library**: Freestanding environment
3. **Manual Length Calculation**: No strlen() (unless you add it)
4. **Explicit Exit**: Must call `exit()` to terminate cleanly
5. **Syscalls Only**: All I/O through syscalls

## Build Pipeline

```
hello.c
   ↓ [gcc -m32 -ffreestanding]
hello.o
   ↓ [ld with user.ld]
hello.bin (flat binary at 0x40000000)
   ↓ [bin2c.sh]
hello_bin.c (C array)
   ↓ [included in kernel]
kernel.bin
```

## Compiler Flags Explained

- `-m32`: Generate 32-bit x86 code
- `-ffreestanding`: No hosted environment (no libc)
- `-nostdlib`: Don't link standard library
- `-fno-pie`: No position-independent executable
- `-fno-stack-protector`: No stack canary checks
- `-fno-builtin`: Don't assume builtin functions exist
- `-nostdinc`: Don't use system include paths
- `-O2`: Optimize for size and speed
- `-I./lib`: Include our custom headers

## Available Syscalls

### exit(int code)
Terminate the process.
```c
exit(0);  // Success
exit(1);  // Error
```

### write(int fd, const char* buf, uint32_t len)
Write data to file descriptor.
```c
write(1, "Hello\n", 6);  // stdout
```

Syscall numbers match Linux convention:
- SYS_EXIT = 1
- SYS_WRITE = 3

## Integration with Kernel

### Current Method: Embedded Arrays

The kernel currently embeds programs as byte arrays:

```c
// In src/kernel/main.c
#include "../../programs/generated/hello_bin.c"

// Copy to user space
uint8_t* dest = (uint8_t*)USER_PROGRAM_BASE;
for (uint32_t i = 0; i < hello_bin_size; i++) {
    dest[i] = hello_bin[i];
}

// Create process
pcb_t* hello = process_create("hello", USER_PROGRAM_BASE);
process_start(hello);
```

### Alternative: External Declaration

Instead of including, you can declare externally:

```c
extern uint8_t hello_bin[];
extern uint32_t hello_bin_size;
```

And link the generated .c file with kernel objects.

## Quick Start Commands

```bash
# Build all programs
make programs

# Generate C arrays for kernel
make programs-generated

# View generated array
cat programs/generated/hello_bin.c

# Test everything
./tools/test-programs.sh

# Build kernel with programs
make all

# Run in QEMU
make qemu-simple
```

## Memory Layout

```
User Space:
0x40000000  ← Program loaded here (hello.bin)
            ← Stack (grows down from kernel allocation)

Kernel allocates:
- Page directory entry for user space
- Page table mapping 0x40000000
- Physical frames for program code/data
- Stack pages for user mode
```

## Adding a New Program

1. Create directory: `mkdir programs/myapp`
2. Write code: `programs/myapp/myapp.c`
3. Update `programs/Makefile`:
   ```makefile
   PROGRAMS = hello/hello.bin myapp/myapp.bin
   
   generated/myapp_bin.c: myapp/myapp.bin | generated/
       ../tools/bin2c.sh $< myapp_bin > $@
   
   generated: generated/hello_bin.c generated/myapp_bin.c
   ```
4. Build: `make programs`
5. Include in kernel: `#include "../../programs/generated/myapp_bin.c"`

## Debugging

### View disassembly:
```bash
objdump -D -b binary -m i386 programs/hello/hello.bin
```

### View hex dump:
```bash
hexdump -C programs/hello/hello.bin
```

### Check for syscalls:
```bash
hexdump -C programs/hello/hello.bin | grep "cd 80"
```

### Verify strings:
```bash
strings programs/hello/hello.bin
```

## Size Comparison

**Assembly (old method)**:
- Manual byte encoding
- ~35 bytes for simple hello world

**C (new method)**:
- hello.c: 78 bytes
- Includes compiler optimizations
- More maintainable
- Easier to extend

Trade-off: Slightly larger, but much more productive.

## Future Enhancements

### Phase 1: String Library (Optional)
Add `programs/lib/string.c` with:
- strlen()
- strcmp()
- strcpy()
- memcpy()

### Phase 2: More Syscalls
- read() - keyboard input
- open() - file operations
- sleep() - timing
- fork() - process creation

### Phase 3: Dynamic Loading
Instead of embedding:
- Store programs in disk sectors
- Implement simple loader
- Load on-demand

### Phase 4: Filesystem
- Minimal filesystem (TAR format)
- Bundle programs in ISO
- Runtime program loading

## Common Issues

### Program crashes immediately
- Check that syscall numbers match kernel
- Verify you're calling exit() at end
- Ensure pointers are valid

### "undefined reference to main"
- Use `_start` not `main`
- Check linker script entry point

### Compilation fails
- Verify gcc supports -m32
- Check that all headers are found
- Run test suite: `./tools/test-programs.sh`

### Binary too large
- Check optimization flags (-O2)
- Review code for unnecessary data
- Use `size` command to analyze

## Testing

The test suite validates:
- ✓ All required files exist
- ✓ Programs compile successfully
- ✓ Binaries are valid
- ✓ Syscall instructions present
- ✓ Message strings included
- ✓ Tools work correctly
- ✓ Headers compile without errors

Run: `./tools/test-programs.sh`

All 22 tests should pass.

## Summary

This setup provides:
- ✅ Easy C program development
- ✅ Automated build pipeline
- ✅ Minimal dependencies (just GCC)
- ✅ Comprehensive testing
- ✅ Good documentation
- ✅ Extensible architecture

You can now write user programs in C instead of manually encoding assembly!