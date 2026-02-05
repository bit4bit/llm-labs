## Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                    PMM ARCHITECTURE                              │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  BOOT (GRUB → boot.s → kernel_main)                             │
│       │                                                        │
│       ▼                                                        │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │  pmm_init(multiboot_info_t* mbd)                        │   │
│  │  ├─ Parse mmap (multiboot memory map)                  │   │
│  │  ├─ Calculate total frames from mem_upper               │   │
│  │  ├─ Allocate bitmap in kernel space                     │   │
│  │  └─ Mark reserved regions (kernel, VGA, BIOS)           │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                 │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │  BITMAP OPERATIONS (internal)                           │   │
│  │  ├─ bitmap_set(index)     →  frame[index] = 1          │   │
│  │  ├─ bitmap_clear(index)   →  frame[index] = 0          │   │
│  │  └─ bitmap_test(index)    →  return frame[index]        │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                 │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │  PUBLIC API                                             │   │
│  │  ├─ pmm_alloc_frame()                                   │   │
│  │  │     Find first 0 bit → set to 1                     │   │
│  │  │     Return frame_phys_addr = index * 4096           │   │
│  │  │     Panic if no free frames                          │   │
│  │  │                                                      │   │
│  │  └─ pmm_free_frame(phys_addr)                           │   │
│  │        index = phys_addr / 4096                         │   │
│  │        bitmap_clear(index)                              │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

## Data Structures

### Memory Map Entry (from multiboot)
```c
typedef struct {
    uint32_t size;
    uint32_t addr_low, addr_high;
    uint32_t len_low, len_high;
    uint32_t type;
} __attribute__((packed)) multiboot_memory_map_t;
```

### Constants
```c
#define PAGE_SIZE       4096
#define PAGE_SHIFT      12
#define MAX_FRAMES      (1024 * 1024)  // 4GB max
```

## Memory Regions

```
┌─────────────────────────────────────────────────────────────────┐
│                    RESERVED REGIONS                             │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  Type 1 (usable)     → Mark FREE in bitmap                     │
│  Type 2 (reserved)   → Mark USED in bitmap                     │
│  Type 3 (ACPI)       → Mark USED in bitmap                     │
│  Type 4 (NVS)        → Mark USED in bitmap                     │
│                                                                 │
│  HARDCODED RESERVATIONS:                                        │
│  ├─ 0x00000000 - 0x0009FFFF  (First 640KB - IVT, BDA, etc)    │
│  ├─ 0x000C0000 - 0x000FFFFF  (Video ROM, BIOS)                 │
│  └─ Kernel loaded at ~1MB by GRUB                              │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

## Implementation Strategy

### 1. Bitmap Search (linear scan)
```c
void* pmm_alloc_frame(void) {
    for (uint32_t i = 0; i < bitmap_size; i++) {
        if (!bitmap_test(i)) {
            bitmap_set(i);
            return (void*)(i * PAGE_SIZE);
        }
    }
    panic("Out of memory");
}
```

### 2. Multiboot Memory Map Parsing
```c
multiboot_memory_map_t* mmap = (multiboot_memory_map_t*)mbd->mmap_addr;
while ((uint32_t)mmap < mbd->mmap_addr + mbd->mmap_length) {
    if (mmap->type == 1) {  // Usable
        // Mark frames as free
    }
    mmap = (multiboot_memory_map_t*)((uint32_t)mmap + mmap->size + 4);
}
```

## Non-Goals

- Virtual memory (paging)
- Kernel heap (kmalloc)
- Frame deallocation optimization (buddy system)
- Memory compaction
