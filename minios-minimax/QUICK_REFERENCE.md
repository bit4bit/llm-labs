# Quick Reference - C Programs in MiniOS

## Common Commands

```bash
# Build everything (programs + kernel)
make all

# Build only user programs
make programs

# Generate C arrays from binaries
make programs-generated

# Run tests
./tools/test-programs.sh

# Run in QEMU
make qemu-simple

# Clean everything
make clean

# After changing C programs, rebuild:
make clean && make all
```

## File Structure

```
programs/
├── lib/
│   ├── syscall.h      # Syscall wrappers (exit, write)
│   └── stdint.h       # Standard types
├── hello/
│   └── hello.c        # Your C program
├── generated/
│   └── hello_bin.c    # Auto-generated array
├── user.ld            # Linker script
└── Makefile           # Build rules
```

## Minimal C Program Template

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

## Available Syscalls

```c
// Exit program
exit(0);              // Success
exit(1);              // Error

// Write to stdout (fd=1)
write(1, buffer, length);
```

## Adding New Program

1. Create `programs/myapp/myapp.c`
2. Add to `programs/Makefile`:
   ```makefile
   PROGRAMS = hello/hello.bin myapp/myapp.bin
   
   generated/myapp_bin.c: myapp/myapp.bin | generated/
       ../tools/bin2c.py $< myapp_bin > $@
   
   generated: ... generated/myapp_bin.c
   ```
3. Build: `make programs && make programs-generated`

## Important Rules

❌ **DON'T**:
- Use `main()` as entry point (use `_start`)
- Include standard library headers
- Assume any libc functions exist
- Forget to call `exit()` at the end

✅ **DO**:
- Use `_start` as entry point
- Calculate string lengths manually
- Use syscalls for all I/O
- Call `exit()` to terminate

## Memory Layout

```
0x40000000  ← Your program loads here
            ← Stack provided by kernel
```

## Troubleshooting

**"undefined reference to main"**
→ Use `_start` not `main`

**Program crashes immediately**
→ Check syscall numbers match kernel
→ Verify you call `exit()` at end

**Build fails**
→ Run `./tools/test-programs.sh`
→ Check `programs/README.md`

**Changed hello.c but no effect**
→ Run `make clean && make all`
→ Incremental builds may miss program changes

## Test Output

When working correctly, you should see:
```
sys_write: output="Hello from C program!
"
Syscall: write returned 22
Syscall: exit called with code 0
```

## Important: Rebuilding After Changes

If you modify a C program (`hello.c`), you may need to force a rebuild:
```bash
make clean && make all
```

The Makefile doesn't explicitly track generated C arrays as dependencies,
so incremental builds might not detect changes. Clean builds always work.

## Documentation

- `programs/README.md` - Detailed user guide
- `C_PROGRAMS_SETUP.md` - Architecture and setup
- `INTEGRATION_COMPLETE.md` - What was implemented

## Example: Count to 3

```c
#include "../lib/syscall.h"

void _start(void) {
    char digits[] = "1\n2\n3\n";
    write(1, digits, 6);
    exit(0);
}
```

## Compiler Flags (Reference)

```
-m32                   32-bit x86
-ffreestanding         No stdlib
-nostdlib              No libc
-fno-pie               No PIE
-fno-stack-protector   No canaries
-O2                    Optimize
```

## Success Indicators

✓ Test suite passes (22/22 tests)
✓ Binary size reasonable (<1KB)
✓ Contains "cd 80" (syscall instruction)
✓ Program outputs expected message
✓ Clean exit (code 0)