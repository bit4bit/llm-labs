## Paging Implementation Tasks

### 1. Create paging.h

- [ ] 1.1 Define PAGE_SIZE (4096), PAGE_SHIFT (12)
- [ ] 1.2 Define PDE flags: PRESENT, RW, USER, PS
- [ ] 1.3 Define struct page_directory_entry
- [ ] 1.4 Define struct page_table_entry
- [ ] 1.5 Declare paging_init(), enable_paging()

### 2. Create paging.c

- [ ] 2.1 Implement create_page_directory()
- [ ] 2.2 Implement setup_identity_mapping()
- [ ] 2.3 Implement paging_enable()
- [ ] 2.4 Allocate page directory from PMM
- [ ] 2.5 Allocate kernel page table from PMM

### 3. Update main.c

- [ ] 3.1 Include paging.h
- [ ] 3.2 Call paging_init() after GDT/IDT
- [ ] 3.3 Print paging status to serial

### 4. Update Makefile

- [ ] 4.1 Add memory/paging.o to KERNEL_OBJS

### 5. Build and Test

- [ ] 5.1 Run `make clean && make`
- [ ] 5.2 Verify paging messages in serial output
- [ ] 5.3 Verify "MinOS Loaded" still appears
- [ ] 5.4 Test page fault by accessing unmapped memory

## Files Created/Modified

| File | Action |
|------|--------|
| src/kernel/memory/paging.h | Created |
| src/kernel/memory/paging.c | Created |
| src/kernel/main.c | Modified |
| Makefile | Modified |
