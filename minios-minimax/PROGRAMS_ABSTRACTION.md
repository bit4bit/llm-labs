# Programs Abstraction Layer

## Overview

We've introduced an abstraction layer for embedded user programs using `programs.h` and `programs.c`. This provides a cleaner separation between the kernel code and the generated program binaries.

## Architecture

### Before (Direct Include)

```c
// src/kernel/main.c
#include "../../programs/generated/hello_bin.c"  // Direct coupling

void kernel_main() {
    process_load(hello, hello_bin, hello_bin_size);
}
```

**Problems**:
- ❌ main.c directly depends on generated files
- ❌ Exposes internal details (path to generated/)
- ❌ Hard to manage multiple programs
- ❌ Generated code mixed with kernel code

### After (Abstraction Layer)

```c
// src/kernel/programs.h
extern uint8_t hello_bin[];
extern uint32_t hello_bin_size;

// src/kernel/programs.c
#include "../../programs/generated/hello_bin.c"

// src/kernel/main.c
#include "programs.h"  // Clean interface

void kernel_main() {
    process_load(hello, hello_bin, hello_bin_size);
}
```

**Benefits**:
- ✅ Clean separation of concerns
- ✅ main.c doesn't know about generated files
- ✅ Easy to add new programs
- ✅ Single point of program registration

## File Structure

```
src/kernel/
├── programs.h          # Extern declarations for all programs
├── programs.c          # Includes generated binaries
├── main.c              # Uses programs via programs.h
└── ...

programs/
└── generated/
    └── hello_bin.c     # Generated binary (included by programs.c)
```

## How It Works

### Compilation Flow

```
┌─────────────────────────────────────────────────────────┐
│ Step 1: Generate Program Binaries                      │
└─────────────────────────────────────────────────────────┘

hello.c → hello.bin → hello_bin.c
    Contains: uint8_t hello_bin[] = { ... };


┌─────────────────────────────────────────────────────────┐
│ Step 2: Compile programs.c (Contains Data)             │
└─────────────────────────────────────────────────────────┘

programs.c:
    #include "programs.h"
    #include "../../programs/generated/hello_bin.c"
    
    ↓ [gcc -c]
    
programs.o:
    [.data section]
    - hello_bin[] (78 bytes)
    - hello_bin_size (4 bytes)
    
    Symbols defined (D):
    - 00000020 D hello_bin
    - 00000000 D hello_bin_size


┌─────────────────────────────────────────────────────────┐
│ Step 3: Compile main.c (References Data)               │
└─────────────────────────────────────────────────────────┘

main.c:
    #include "programs.h"  // extern declarations
    
    void kernel_main() {
        process_load(hello, hello_bin, hello_bin_size);
    }
    
    ↓ [gcc -c]
    
main.o:
    [.text section]
    - kernel_main() code
    
    Symbols undefined (U):
    - U hello_bin       ← expects from elsewhere
    - U hello_bin_size  ← expects from elsewhere


┌─────────────────────────────────────────────────────────┐
│ Step 4: Link Everything Together                       │
└─────────────────────────────────────────────────────────┘

programs.o + main.o + boot.o + ... 
    
    ↓ [ld]
    
kernel.bin:
    Linker resolves symbols:
    - main.o's "U hello_bin" → programs.o's "D hello_bin"
    - main.o's "U hello_bin_size" → programs.o's "D hello_bin_size"
```

### Symbol Resolution

Check with `nm`:

```bash
# programs.o defines the data
$ nm src/kernel/programs.o | grep hello
00000020 D hello_bin         # D = defined in .data
00000000 D hello_bin_size

# main.o references the data
$ nm src/kernel/main.o | grep hello
         U hello_bin         # U = undefined, needs linking
         U hello_bin_size

# kernel.bin has resolved addresses
$ nm kernel.bin | grep hello
00104c60 D hello_bin         # Final address in kernel
00104c40 D hello_bin_size
```

## Adding New Programs

### 1. Generate the binary

Create `programs/shell/shell.c` and build it:
```bash
make programs
make programs-generated
```

This creates `programs/generated/shell_bin.c`

### 2. Update programs.h

Add extern declarations:

```c
// src/kernel/programs.h
#ifndef PROGRAMS_H
#define PROGRAMS_H

#include <stdint.h>

/* Hello world program */
extern uint8_t hello_bin[];
extern uint32_t hello_bin_size;

/* Shell program */
extern uint8_t shell_bin[];
extern uint32_t shell_bin_size;

#endif
```

### 3. Update programs.c

Include the generated binary:

```c
// src/kernel/programs.c
#include <stdint.h>
#include "programs.h"

/* Include generated program binaries */
#include "../../programs/generated/hello_bin.c"
#include "../../programs/generated/shell_bin.c"
```

### 4. Use in kernel code

```c
// src/kernel/main.c
#include "programs.h"

void kernel_main() {
    // Load hello
    pcb_t* hello = process_create("hello", USER_PROGRAM_BASE);
    process_load(hello, hello_bin, hello_bin_size);
    
    // Load shell
    pcb_t* shell = process_create("shell", USER_PROGRAM_BASE + 0x10000);
    process_load(shell, shell_bin, shell_bin_size);
}
```

### 5. Rebuild

```bash
make clean && make all
```

## Benefits of This Approach

### 1. Encapsulation
- main.c doesn't know where programs come from
- Generated files are isolated in programs.c
- Clean interface via programs.h

### 2. Maintainability
- All program registrations in one place
- Easy to see what programs are available
- Clear separation between kernel and programs

### 3. Scalability
- Add programs without touching main.c logic
- Just update programs.h and programs.c
- No scattered includes across kernel code

### 4. Type Safety
- Compiler checks extern declarations match definitions
- Linker verifies all symbols are resolved
- Catch errors at build time

### 5. Documentation
- programs.h serves as a registry of available programs
- Self-documenting: see all programs at a glance
- Comments can explain each program's purpose

## Comparison with Alternatives

### Alternative 1: Direct Include Everywhere

```c
// main.c
#include "../../programs/generated/hello_bin.c"

// other_file.c
#include "../../programs/generated/hello_bin.c"  // Duplicate definition!
```

**Problem**: Multiple definitions cause linker errors

### Alternative 2: Include in Header

```c
// programs.h
#include "../../programs/generated/hello_bin.c"

// main.c
#include "programs.h"
```

**Problem**: Still couples header to generated files, not clean

### Alternative 3: Our Approach (Extern + Definition)

```c
// programs.h
extern uint8_t hello_bin[];

// programs.c
#include "../../programs/generated/hello_bin.c"

// main.c
#include "programs.h"
```

**Advantages**: Clean, scalable, follows C conventions

## Build Dependencies

The Makefile ensures correct build order:

```makefile
# Generate programs first
all: programs programs-generated kernel.bin

# programs.o depends on generated files
src/kernel/programs.o: src/kernel/programs.c src/kernel/programs.h
	$(CC) $(CFLAGS) -c -o $@ $<

# main.o depends on programs.h (but not programs.c)
src/kernel/main.o: src/kernel/main.c src/kernel/programs.h
	$(CC) $(CFLAGS) -c -o $@ $<
```

## Testing

Verify the abstraction works:

```bash
# Clean build
make clean && make all

# Check symbols in programs.o
nm src/kernel/programs.o | grep hello
# Should show: D hello_bin, D hello_bin_size

# Check symbols in main.o
nm src/kernel/main.o | grep hello
# Should show: U hello_bin, U hello_bin_size

# Check final kernel
nm kernel.bin | grep hello
# Should show resolved addresses

# Run it
make qemu-simple
# Should output: "Hello from C program!"
```

## Summary

The programs.h/programs.c abstraction provides:

✅ **Clean separation**: Kernel code isolated from generated files  
✅ **Easy extension**: Add programs without modifying kernel logic  
✅ **Type safety**: Compiler and linker verify correctness  
✅ **Maintainability**: Single registry of all programs  
✅ **Standard practice**: Follows C header/implementation convention  

This is a **best practice** for managing embedded resources in C projects.