# User Programs

MiniOS Minimax supports user-space programs written in C that are embedded into the kernel binary.

## Creating User Programs

### 1. Create Program Directory

```bash
mkdir programs/src/<program_name>
```

### 2. Write C Source File

Create `programs/src/<program_name>/<program_name>.c`:

```c
#include "../../lib/syscall.h"

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

**Key requirements:**
- Entry point must be `_start`, NOT `main()`
- No standard library available (freestanding environment)
- Must call `exit()` to terminate
- Use syscalls for all I/O

### 3. Available Syscalls (lib/syscall.h)

```c
void exit(int code);                          // Terminate process
int write(int fd, const char* buf, uint32_t len);  // Write to file descriptor
```

## Compiling User Programs

### Build Commands

```bash
make programs              # Compile all C programs to .bin files
make programs-generated    # Generate C arrays from binaries for kernel embedding
make programs-clean        # Remove program build artifacts
```

### Program Structure

```
programs/
├── src/                       # User program sources
│   └── <program_name>/        # Your program
│       ├── <program_name>.c   # Source code
│       ├── <program_name>.o   # Object file (generated)
│       └── <program_name>.bin # Binary executable (generated)
├── lib/                       # User-space library
│   ├── syscall.h              # System call wrappers
│   └── stdint.h               # Standard integer types
├── generated/                 # C arrays for kernel (generated)
│   └── <program_name>_bin.c
├── user.ld                    # Linker script
└── Makefile                  # Build system (delegates to Ruby)
```

### Integration with Kernel

After building, the generated C arrays are included in the kernel via `src/kernel/programs.c`:

```c
#include "../../programs/generated/<program_name>_bin.c"
```

Access programs in kernel code via `src/kernel/programs.h` extern declarations.

### Memory Layout

- **Load Address**: `0x40000000` (USER_PROGRAM_BASE)
- **Mode**: User mode (Ring 3)
- **Stack**: Provided by kernel
- **Architecture**: 32-bit x86

### Quick Build All

```bash
make all  # Builds programs, generates C arrays, and kernel.bin
```

### Using Ruby Build Script

The build system uses Ruby (`tools/build-programs.rb`):

```bash
ruby tools/build-programs.rb --all       # Build programs + generate C arrays
ruby tools/build-programs.rb --clean      # Clean artifacts
ruby tools/build-programs.rb --programs   # Build only .bin files
ruby tools/build-programs.rb --generated  # Generate only C arrays
```
