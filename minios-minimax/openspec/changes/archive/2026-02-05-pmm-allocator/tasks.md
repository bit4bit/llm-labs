## PMM Implementation Tasks

### 1. Create memory.h constants and structs

- [x] 1.1 Define PAGE_SIZE (4096), PAGE_SHIFT (12)
- [x] 1.2 Define MAX_FRAMES based on memory limits
- [x] 1.3 Define multiboot_memory_map_t struct (packed)
- [x] 1.4 Declare extern variables: bitmap, frame_count, used_frames

### 2. Create alloc.c PMM implementation

- [x] 2.1 Implement bitmap_test(index) - return bit at index
- [x] 2.2 Implement bitmap_set(index) - set bit to 1
- [x] 2.3 Implement bitmap_clear(index) - set bit to 0
- [x] 2.4 Implement pmm_init(mbd) - parse mmap, init bitmap
- [x] 2.5 Implement pmm_alloc_frame() - linear search for free frame
- [x] 2.6 Implement pmm_free_frame(addr) - clear corresponding bit
- [x] 2.7 Implement pmm_get_free_count() - for debugging

### 3. Update kernel.h

- [x] 3.1 Add serial_putchar, serial_print declarations
- [x] 3.2 Add NULL definition

### 4. Update main.c

- [x] 4.1 Include memory.h
- [x] 4.2 Call pmm_init(&mbd) before using memory
- [x] 4.3 Test allocation: alloc and immediately free
- [x] 4.4 Print free frame count to serial
- [x] 4.5 Add serial_print_uint function

### 5. Update Makefile

- [x] 5.1 Add memory/memory.o to KERNEL_OBJS
- [x] 5.2 Add memory/alloc.o to KERNEL_OBJS

### 6. Build and Test

- [x] 6.1 Run `make clean && make`
- [ ] 6.2 Run `make qemu` and verify serial output
- [ ] 6.3 Verify "MinOS Loaded" still appears
- [ ] 6.4 Verify PMM initialization messages

## Files Created/Modified

| File | Action |
|------|--------|
| src/kernel/memory/memory.h | Created |
| src/kernel/memory/alloc.c | Created |
| src/kernel/kernel.h | Created |
| src/kernel/main.c | Modified |
| src/kernel/boot/boot.s | Modified |
| Makefile | Modified |
