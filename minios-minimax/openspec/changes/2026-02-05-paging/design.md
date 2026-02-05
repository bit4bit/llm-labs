## Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                    PAGING ARCHITECTURE                           │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  PHYSICAL MEMORY                                                │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │  Page Directory (1 page = 4KB = 1024 entries)          │   │
│  │  Page Tables (N pages, each 4KB = 1024 entries)         │   │
│  │  Kernel Data                                             │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                 │
│  VIRTUAL ADDRESS SPACE                                          │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │  0x00000000 - 0xBFFFFFFF  (User space - 3GB)            │   │
│  │  0xC0000000 - 0xFFFFFFFF  (Kernel space - 1GB)          │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                 │
│  IDENTITY MAPPING (early)                                       │
│  └─ Kernel at 0xC0000000 maps to physical 0x00000000+       │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

## Page Table Structure

```
┌─────────────────────────────────────────────────────────────────┐
│                    PAGE DIRECTORY ENTRY (PDE)                    │
├─────────────────────────────────────────────────────────────────┤
│  Bit 0: Present (P)      - Page is in memory                   │
│  Bit 1: Read/Write (R/W) - 0=read-only, 1=read-write          │
│  Bit 2: User/Supervisor   - 0=kernel, 1=user                   │
│  Bit 3: PWT               - Write-through caching               │
│  Bit 4: PCD               - Cache disable                      │
│  Bit 5: Accessed (A)      - CPU set when accessed             │
│  Bit 6: Reserved          - Must be 0                          │
│  Bit 7: Page Size (PS)    - 0=4KB, 1=4MB                      │
│  Bits 12-31: Page Table Base Address                            │
└─────────────────────────────────────────────────────────────────┘
```

## Implementation Strategy

### 1. Early Identity Mapping
- Create page directory with first entry pointing to first page table
- Map 0x00000000 - 0xC0000000 (identity, until kernel space)
- Use 4MB pages for kernel (PS bit = 1)

### 2. Enable Paging
```c
// Load page directory
__asm__ volatile ("mov %0, %%cr3" : : "r"(page_directory));

// Enable paging (CR0.PG) and protected mode (CR0.PE)
uint32_t cr0;
__asm__ volatile ("mov %%cr0, %0" : "=r"(cr0));
cr0 |= 0x80000000;  // Set PG bit
__asm__ volatile ("mov %0, %%cr0" : : "r"(cr0));
```

### 3. Page Fault Handling
- Page fault handler already exists (from IDT)
- Need to extend to handle page table allocation on demand

## Constants
```c
#define PAGE_SIZE           4096
#define PAGE_ALIGN          0x1000
#define PAGE_SHIFT          12

#define PDE_PRESENT         0x01
#define PDE_RW              0x02
#define PDE_USER            0x04
#define PDE_PS              0x80    // 4MB page

#define KERNEL_VIRTUAL_BASE 0xC0000000
```

## Non-Goals

- Demand paging (allocate pages on fault)
- Copy-on-write
- Page swap to disk
- Advanced MMU features (PAT, NX bit)
