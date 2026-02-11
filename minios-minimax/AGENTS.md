# AGENTS.md

This file provides guidelines for agents working with the MiniOS Minimax project.


## Program-Specific Documentation

For detailed documentation on user programs, see:
- [programs/AGENTS.md](programs/AGENTS.md) - Complete guide for creating and compiling C user programs

## Quick Reference

### Build Commands
```bash
make programs              # Build user programs
make programs-generated    # Generate C arrays for kernel
make all                   # Build everything
```

## Testing Requirements

Every code change must be tested before considering the work complete. This ensures the stability and reliability of the MiniOS Minimax system.

### When to Run Tests
- After making any code changes to the kernel
- After modifying or adding user programs
- After updating build configurations
- Before finalizing any implementation

### How to Run Tests
Run the QEMU-based test suite using:
```bash
make qemu-simple
```

### What to Verify
After running `make qemu-simple`, verify that:
1. The build completes successfully without errors
2. All self-checks pass in the system
3. No assertion failures or kernel panics occur
4. The system boots and operates correctly

If any self-checks fail, investigate and fix the underlying issue before considering the changes complete.