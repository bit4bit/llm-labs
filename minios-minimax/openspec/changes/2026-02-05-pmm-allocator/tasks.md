## PMM Implementation Tasks

### 1. Create memory.h constants and structs

- [ ] 1.1 Define PAGE_SIZE (4096), PAGE_SHIFT (12)
- [ ] 1.2 Define MAX_FRAMES based on memory limits
- [ ] 1.3 Define multiboot_memory_map_t struct (packed)
- [ ] 1.4 Declare extern variables: bitmap, frame_count, used_frames

### 2. Create alloc.c PMM implementation

- [ ] 2.1 Implement bitmap_test(index) - return bit at index
- [ ] 2.2 Implement bitmap_set(index) - set bit to 1
- [ ] 2.3 Implement bitmap_clear(index) - set bit to 0
- [ ] 2.4 Implement pmm_init(mbd) - parse mmap, init bitmap
- [ ] 2.5 Implement pmm_alloc_frame() - linear search for free frame
- [ ] 2.6 Implement pmm_free_frame(addr) - clear corresponding bit
- [ ] 2.7 Implement pmm_get_free_count() - for debugging

### 3. Update kernel.h

- [ ] 3.1 Add pmm_init() declaration
- [ ] 3.2 Add pmm_alloc_frame() declaration
- [ ] 3.3 Add pmm_free_frame() declaration

### 4. Update main.c

- [ ] 4.1 Include memory.h
- [ ] 4.2 Call pmm_init(&mbd) before using memory
- [ ] 4.3 Test allocation: alloc and immediately free
- [ ] 4.4 Print free frame count to serial

### 5. Update Makefile

- [ ] 5.1 Add memory/memory.o to KERNEL_OBJS
- [ ] 5.2 Add memory/alloc.o to KERNEL_OBJS

### 6. Build and Test

- [ ] 6.1 Run `make clean && make`
- [ ] 6.2 Run `make qemu` and verify serial output
- [ ] 6.3 Verify "MinOS Loaded" still appears
- [ ] 6.4 Verify PMM initialization messages
