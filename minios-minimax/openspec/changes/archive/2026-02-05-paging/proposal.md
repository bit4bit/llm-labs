## Why

The kernel currently uses physical addressing directly. To implement virtual memory, memory protection, and user processes, we need paging enabled. Paging provides:
- Virtual address spaces (kernel vs user)
- Memory protection (prevent invalid accesses)
- Demand paging (load pages on demand)
- Memory isolation between processes

## What Changes

- Create `src/kernel/memory/paging.h`: Page table structs, constants
- Create `src/kernel/memory/paging.c`: Page directory/table management
- Update `src/kernel/main.c`: Enable paging after GDT/IDT
- Update `Makefile`: Add paging.o to build

## Capabilities

### New Capabilities
- `memory-paging`: Virtual memory with 4MB pages for kernel

## Impact

- Required for: user mode, process isolation, kmalloc
- Kernel moves to high virtual addresses (0xC0000000+)
- All physical memory becomes accessible via virtual mapping
- Enables future features: user processes, memory protection
