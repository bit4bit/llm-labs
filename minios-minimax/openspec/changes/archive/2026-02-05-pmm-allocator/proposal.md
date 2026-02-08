## Why

The kernel currently cannot allocate dynamic memory. To implement paging, kernel heap, processes, and virtually any advanced feature, we need a physical memory allocator that:
- Tracks which physical frames are free/used
- Provides alloc/free primitives
- Parses the multiboot memory map for available regions

## What Changes

- Create `src/kernel/memory/memory.h`: Constants and type definitions
- Create `src/kernel/memory/pmm.c`: PMM implementation with bitmap
- Update `src/kernel/main.c`: Initialize PMM during boot
- Update `src/kernel/kernel.h`: Add PMM function declarations

## Capabilities

### New Capabilities
- `memory-pmm`: Physical Memory Manager for allocating/freeing 4KB frames

## Impact

- Required for all subsequent memory features (paging, kmalloc, user processes)
- Minimal footprint: ~120 lines of code
- No external dependencies beyond multiboot info
