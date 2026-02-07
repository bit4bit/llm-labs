# User Programs - C Development Guide

This directory contains user-space programs written in C that run on MiniOS.

## Directory Structure

```
programs/
├── lib/                    # User-space library
│   ├── syscall.h          # System call wrappers (inline functions)
│   └── stdint.h           # Standard integer types
├── hello/                  # Hello world program
│   ├── hello.c            # Source code
│   ├── hello.o            # Object file (generated)
│   └── hello.bin          # Binary executable (generated)
├── generated/              # Auto-generated C arrays (generated)
│   └── hello_bin.c        # hello.bin as C array
├── user.ld                 # Linker script for user programs
├── Makefile                # Build system for programs
└── README.md              # This file
```

## Quick Start

### Build all programs:
```bash
make programs
```

### Generate C arrays for kernel embedding:
```bash
make programs-generated
```

### Clean build artifacts:
```bash
make programs-clean
```

## Writing a New Program

### 1. Create program directory
```bash
mkdir programs/myprogram
```

### 2. Write your program (`programs/myprogram/myprogram.c`)

```c
#include "../lib/syscall.h"

void _start(void) {
    const char* msg = "Hello, MiniOS!\n";
    
    // Calculate string length
    uint32_t len = 0;
    while (msg[len]) len++;
    
    // Write to stdout
    write(1, msg, len);
    
    // Exit cleanly
    exit(0);
}
```

**Important notes:**
- Entry point is `_start`, NOT `main()`
- No standard library available (freestanding)
- Must use syscalls for all I/O
- Must call `exit()` to terminate

### 3. Update Makefile

Edit `programs/Makefile` and add your program to the `PROGRAMS` list:

```makefile
PROGRAMS = hello/hello.bin myprogram/myprogram.bin
```

Add a generation rule:

```makefile
generated/myprogram_bin.c: myprogram/myprogram.bin | generated/
	../tools/bin2c.sh $< myprogram_bin > $@
```

Update the `generated` target:

```makefile
generated: generated/hello_bin.c generated/myprogram_bin.c
```

### 4. Build your program

```bash
make programs
```

Your program will be compiled to `programs/myprogram/myprogram.bin`

## Available System Calls

All syscalls are defined in `lib/syscall.h`:

### `void exit(int code)`
Terminate the program with the given exit code.

```c
exit(0);  // Exit successfully
```

### `int write(int fd, const char* buf, uint32_t len)`
Write data to a file descriptor.

```c
const char* msg = "Hello!\n";
write(1, msg, 7);  // Write to stdout (fd=1)
```

## Memory Layout

User programs are loaded at fixed addresses:

- **Base Address**: `0x40000000`
- **Address Space**: User mode (Ring 3)
- **Stack**: Provided by kernel
- **Heap**: Not implemented yet

The linker script (`user.ld`) automatically places your program at the correct address.

## Build Process

### Compilation Flow:

1. **Compile**: `hello.c` → `hello.o`
   - 32-bit freestanding code
   - No standard library
   - Optimizations enabled

2. **Link**: `hello.o` → `hello.bin`
   - Custom linker script (`user.ld`)
   - Output format: flat binary
   - Loaded at 0x40000000

3. **Convert**: `hello.bin` → `hello_bin.c`
   - Binary to C array conversion
   - Embedded in kernel image

### Compiler Flags:

```
-m32                    # 32-bit x86 code
-ffreestanding          # Freestanding environment (no stdlib)
-nostdlib               # Don't link standard library
-fno-pie                # No position-independent executable
-fno-stack-protector    # No stack canary
-fno-builtin            # Don't use builtin functions
-nostdinc               # Don't use standard includes
-O2                     # Optimize
-I./lib                 # Include path for our headers
```

## Inspecting Binaries

### View binary size:
```bash
ls -lh programs/hello/hello.bin
```

### View as hex:
```bash
hexdump -C programs/hello/hello.bin
```

### Disassemble:
```bash
objdump -D -b binary -m i386 programs/hello/hello.bin
```

### View generated C array:
```bash
cat programs/generated/hello_bin.c
```

## Integrating with Kernel

The generated C arrays can be included in the kernel:

```c
// In src/kernel/main.c
#include "../../programs/generated/hello_bin.c"

void kernel_main(multiboot_info_t* mbd) {
    // Copy hello_bin to user space
    uint8_t* dest = (uint8_t*)USER_PROGRAM_BASE;
    for (uint32_t i = 0; i < hello_bin_size; i++) {
        dest[i] = hello_bin[i];
    }
    
    // Create and run process
    pcb_t* hello = process_create("hello", USER_PROGRAM_BASE);
    process_start(hello);
}
```

## Troubleshooting

### "undefined reference to `main`"
- Use `_start` as entry point, not `main`
- Check linker script has `ENTRY(_start)`

### "relocation truncated to fit"
- Your program is too large
- Check the output size and address space

### Program crashes immediately
- Verify syscall numbers match kernel
- Check that you're calling `exit()` at the end
- Ensure string pointers are valid

### Cannot write to stdout
- Make sure you're passing the correct length
- Verify `write()` syscall is implemented in kernel
- Check that VGA driver is initialized

## Adding More Syscalls

To add a new syscall:

1. Define syscall number in `lib/syscall.h`:
```c
#define SYS_READ 4
```

2. Add wrapper function:
```c
static inline int read(int fd, char* buf, uint32_t len) {
    return syscall3(SYS_READ, fd, (int)buf, len);
}
```

3. Implement in kernel (`src/kernel/syscall/syscall.c`)

## Future Enhancements

- [ ] String library (strlen, strcmp, etc.)
- [ ] Dynamic memory allocation (malloc/free)
- [ ] Multiple programs support
- [ ] Load programs from filesystem instead of embedding
- [ ] Command-line argument passing
- [ ] Environment variables
- [ ] More POSIX syscalls (open, read, close, etc.)

## Examples

### Example 1: Count to 10
```c
#include "../lib/syscall.h"

void _start(void) {
    for (int i = 1; i <= 10; i++) {
        char digit = '0' + i;
        write(1, &digit, 1);
        write(1, "\n", 1);
    }
    exit(0);
}
```

### Example 2: Echo a string
```c
#include "../lib/syscall.h"

void _start(void) {
    const char* message = "Echo: This is a test\n";
    
    uint32_t len = 0;
    while (message[len]) len++;
    
    write(1, message, len);
    exit(0);
}
```

## Resources

- [System V ABI i386](https://www.sco.com/developers/devspecs/abi386-4.pdf)
- [Linux System Call Convention](https://chromium.googlesource.com/chromiumos/docs/+/master/constants/syscalls.md)
- [OSDev Wiki - System Calls](https://wiki.osdev.org/System_Calls)

## License

MIT License - See main project README