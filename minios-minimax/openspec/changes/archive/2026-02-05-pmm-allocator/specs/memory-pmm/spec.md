## ADDED Requirements

### Requirement: Physical Frame Allocation

The kernel SHALL provide a function `pmm_alloc_frame()` that returns a pointer to a newly allocated 4KB physical memory frame.

#### Scenario: Allocate a single frame
- **WHEN** `pmm_alloc_frame()` is called
- **AND** there is at least one free frame available
- **THEN** the function SHALL return a physical address aligned to 4096 bytes
- **AND** mark the corresponding frame as used in the PMM bitmap
- **AND** the returned address SHALL NOT be used by any other allocation

#### Scenario: Out of memory
- **WHEN** `pmm_alloc_frame()` is called
- **AND** all frames are marked as used
- **THEN** the kernel SHALL halt with a panic message

### Requirement: Physical Frame Deallocation

The kernel SHALL provide a function `pmm_free_frame(void* addr)` that releases a previously allocated frame back to the free pool.

#### Scenario: Free a frame
- **WHEN** `pmm_free_frame(phys_addr)` is called
- **AND** `phys_addr` is a valid allocated frame address
- **THEN** the function SHALL mark the corresponding frame as free
- **AND** the frame SHALL be available for future allocations

### Requirement: PMM Initialization

The kernel SHALL provide a function `pmm_init(multiboot_info_t* mbd)` that initializes the physical memory manager.

#### Scenario: Initialize from multiboot info
- **WHEN** `pmm_init()` is called with a valid multiboot info structure
- **THEN** the PMM SHALL parse the multiboot memory map
- **AND** mark all type-1 (usable) regions as initially free
- **AND** mark all type-2+ (reserved) regions as used
- **AND** initialize the internal bitmap to track frame usage
- **AND** reserve the first 640KB (0x00000000 - 0x0009FFFF)
- **AND** reserve video memory (0x000A0000 - 0x000FFFFF)

### Requirement: Memory Region Enumeration

The PMM SHALL enumerate usable memory regions from the multiboot memory map.

#### Scenario: Parse memory map
- **WHEN** the multiboot memory map is available
- **THEN** the PMM SHALL read entries from `mbd->mmap_addr`
- **AND** for each entry with `type == 1`, add all frames to the free pool
- **AND** for each entry with `type != 1`, mark frames as reserved

### Requirement: Frame Count Query

The kernel SHALL provide a function `pmm_get_free_count()` that returns the number of currently available free frames.

#### Scenario: Query free frames
- **WHEN** `pmm_get_free_count()` is called
- **THEN** the function SHALL return the count of frames marked as free
