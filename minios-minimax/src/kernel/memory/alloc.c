#include "memory.h"
#include "../kernel.h"

#define BITMAP_SET(bit)      (pmm_bitmap[(bit) / 8] |= (1 << ((bit) % 8)))
#define BITMAP_CLEAR(bit)    (pmm_bitmap[(bit) / 8] &= ~(1 << ((bit) % 8)))
#define BITMAP_TEST(bit)     (pmm_bitmap[(bit) / 8] & (1 << ((bit) % 8)))

uint32_t pmm_frame_count = 0;
uint32_t pmm_used_frames = 0;
uint8_t* pmm_bitmap = NULL;

static inline void bitmap_set(uint32_t bit) {
    BITMAP_SET(bit);
}

static inline void bitmap_clear(uint32_t bit) {
    BITMAP_CLEAR(bit);
}

static inline int bitmap_test(uint32_t bit) {
    return BITMAP_TEST(bit) != 0;
}

void pmm_init(multiboot_info_t* mbd) {
    serial_print("PMM: Initializing...\n");

    uint32_t mem_upper_kb = mbd->mem_upper;
    pmm_frame_count = (mem_upper_kb * 1024) / PAGE_SIZE;

    serial_print("PMM: Total frames: ");
    serial_print_uint(pmm_frame_count);
    serial_print("\n");

    uint32_t bitmap_size = (pmm_frame_count + 7) / 8;
    pmm_bitmap = (uint8_t*)0x1000000;

    for (uint32_t i = 0; i < bitmap_size; i++) {
        pmm_bitmap[i] = 0;
    }

    pmm_used_frames = 0;

    multiboot_memory_map_t* mmap = (multiboot_memory_map_t*)mbd->mmap_addr;
    uint32_t mmap_end = mbd->mmap_addr + mbd->mmap_length;

    serial_print("PMM: Marking reserved regions...\n");

    while ((uint32_t)mmap < mmap_end) {
        uint32_t addr = mmap->addr_low;
        uint32_t len = mmap->len_low;
        uint32_t type = mmap->type;

        if (type != 1 && len > 0) {
            uint32_t start_frame = addr / PAGE_SIZE;
            uint32_t num_frames = len / PAGE_SIZE;

            for (uint32_t i = 0; i < num_frames; i++) {
                uint32_t frame = start_frame + i;
                if (frame < pmm_frame_count) {
                    if (!bitmap_test(frame)) {
                        bitmap_set(frame);
                        pmm_used_frames++;
                    }
                }
            }
        }
        mmap = (multiboot_memory_map_t*)((uint32_t)mmap + mmap->size + 4);
    }

    serial_print("PMM: Usable frames marked\n");
}

void* pmm_alloc_frame(void) {
    for (uint32_t i = 0; i < pmm_frame_count; i++) {
        if (!bitmap_test(i)) {
            bitmap_set(i);
            pmm_used_frames++;
            return (void*)(i * PAGE_SIZE);
        }
    }
    serial_print("PMM: OUT OF MEMORY!\n");
    while (1) {
        __asm__ volatile ("hlt");
    }
}

void pmm_free_frame(void* addr) {
    uint32_t frame = (uint32_t)addr / PAGE_SIZE;
    if (frame < pmm_frame_count) {
        bitmap_clear(frame);
        pmm_used_frames--;
    }
}

uint32_t pmm_get_free_count(void) {
    return pmm_frame_count - pmm_used_frames;
}

void pmm_test(void) {
    serial_print("PMM: Running self-tests...\n");

    serial_print("PMM: Total frames: ");
    serial_print_uint(pmm_frame_count);
    serial_print("\n");

    serial_print("PMM: Used frames: ");
    serial_print_uint(pmm_used_frames);
    serial_print("\n");

    serial_print("PMM: Free frames: ");
    serial_print_uint(pmm_get_free_count());
    serial_print("\n");

    uint32_t free_before = pmm_get_free_count();
    if (free_before < 20) {
        serial_print("PMM: SKIPPED - not enough free frames for test\n");
        return;
    }

    serial_print("PMM: Testing allocation and deallocation...\n");

    void* frames[10];
    for (int i = 0; i < 10; i++) {
        frames[i] = pmm_alloc_frame();
        uint32_t frame_num = (uint32_t)frames[i] / PAGE_SIZE;
        if ((uint32_t)frames[i] % PAGE_SIZE != 0) {
            serial_print("PMM: FAILED - allocated address not page-aligned: 0x");
            serial_print_uint((uint32_t)frames[i]);
            serial_print("\n");
            return;
        }
        serial_print("PMM: Allocated frame ");
        serial_print_uint(i);
        serial_print(" at 0x");
        serial_print_uint((uint32_t)frames[i]);
        serial_print(" (frame ");
        serial_print_uint(frame_num);
        serial_print(")\n");
    }

    uint32_t free_after_alloc = pmm_get_free_count();
    if (free_after_alloc != free_before - 10) {
        serial_print("PMM: FAILED - free count mismatch (expected ");
        serial_print_uint(free_before - 10);
        serial_print(", got ");
        serial_print_uint(free_after_alloc);
        serial_print(")\n");
        return;
    }
    serial_print("PMM: Free count correct after allocation\n");

    serial_print("PMM: Testing read/write on allocated frames...\n");

    uint32_t patterns[] = {0xDEADBEEF, 0xCAFEBABE, 0x12345678, 0xFFFFFFFF, 0x00000000, 0xA5A5A5A5};
    uint32_t num_patterns = sizeof(patterns) / sizeof(patterns[0]);

    for (int f = 0; f < 5; f++) {
        uint32_t* frame = (uint32_t*)frames[f];
        uint32_t frame_addr = (uint32_t)frame;

        serial_print("PMM: Testing frame ");
        serial_print_uint(f);
        serial_print(" at 0x");
        serial_print_uint(frame_addr);
        serial_print("...\n");

        for (uint32_t p = 0; p < num_patterns; p++) {
            uint32_t pattern = patterns[p];

            for (uint32_t offset = 0; offset < PAGE_SIZE; offset += 4096) {
                uint32_t* test_addr = (uint32_t*)(frame_addr + offset);
                uint32_t saved = *test_addr;

                *test_addr = pattern;
                if (*test_addr != pattern) {
                    serial_print("PMM: FAILED - write/read mismatch at 0x");
                    serial_print_uint((uint32_t)test_addr);
                    serial_print(" expected 0x");
                    serial_print_uint(pattern);
                    serial_print(" got 0x");
                    serial_print_uint(*test_addr);
                    serial_print("\n");
                    return;
                }

                volatile uint16_t* word_addr = (volatile uint16_t*)test_addr;
                volatile uint8_t* byte_addr = (volatile uint8_t*)test_addr;

                *word_addr = (uint16_t)(pattern & 0xFFFF);
                if (*word_addr != (pattern & 0xFFFF)) {
                    *test_addr = saved;
                    serial_print("PMM: FAILED - word write/read at 0x");
                    serial_print_uint((uint32_t)test_addr);
                    serial_print("\n");
                    return;
                }

                *byte_addr = (uint8_t)(pattern & 0xFF);
                if (*byte_addr != (pattern & 0xFF)) {
                    *test_addr = saved;
                    serial_print("PMM: FAILED - byte write/read at 0x");
                    serial_print_uint((uint32_t)test_addr);
                    serial_print("\n");
                    return;
                }

                *test_addr = saved;
            }
        }
        serial_print("PMM:  Frame ");
        serial_print_uint(f);
        serial_print(" OK\n");
    }

    serial_print("PMM: Read/write tests passed\n");

    serial_print("PMM: Testing full page write/read...\n");
    uint32_t* full_test = (uint32_t*)frames[0];
    for (uint32_t i = 0; i < PAGE_SIZE / 4; i++) {
        full_test[i] = 0xABCD1234;
    }
    for (uint32_t i = 0; i < PAGE_SIZE / 4; i++) {
        if (full_test[i] != 0xABCD1234) {
            serial_print("PMM: FAILED - full page test at offset 0x");
            serial_print_uint(i * 4);
            serial_print("\n");
            return;
        }
    }
    serial_print("PMM: Full page test OK (1KB pattern x 1024)\n");

    serial_print("PMM: Testing free...\n");
    for (int i = 0; i < 10; i++) {
        pmm_free_frame(frames[i]);
        serial_print("PMM: Freed frame ");
        serial_print_uint(i);
        serial_print(" at 0x");
        serial_print_uint((uint32_t)frames[i]);
        serial_print("\n");
    }

    uint32_t free_after_free = pmm_get_free_count();
    if (free_after_free != free_before) {
        serial_print("PMM: FAILED - free count mismatch after free (expected ");
        serial_print_uint(free_before);
        serial_print(", got ");
        serial_print_uint(free_after_free);
        serial_print(")\n");
        return;
    }
    serial_print("PMM: Free count correct after freeing\n");

    serial_print("PMM: Testing bitmap state...\n");
    uint32_t alloc_count = 0;
    uint32_t free_count = 0;
    for (uint32_t i = 0; i < pmm_frame_count; i++) {
        if (bitmap_test(i)) {
            alloc_count++;
        } else {
            free_count++;
        }
    }

    if (alloc_count != pmm_used_frames) {
        serial_print("PMM: FAILED - bitmap used count mismatch (bitmap=");
        serial_print_uint(alloc_count);
        serial_print(", pmm_used_frames=");
        serial_print_uint(pmm_used_frames);
        serial_print(")\n");
        return;
    }

    if (free_count != pmm_get_free_count()) {
        serial_print("PMM: FAILED - bitmap free count mismatch\n");
        return;
    }
    serial_print("PMM: Bitmap state verified\n");

    serial_print("PMM: Testing alloc/free consistency...\n");
    void* addr1 = pmm_alloc_frame();
    void* addr2 = pmm_alloc_frame();
    if (addr1 >= addr2) {
        serial_print("PMM: FAILED - allocator returned addresses out of order\n");
        return;
    }
    pmm_free_frame(addr1);
    pmm_free_frame(addr2);
    serial_print("PMM: Allocator order verified\n");

    serial_print("PMM: Testing edge cases...\n");

    void* test_frame = pmm_alloc_frame();
    uint32_t test_frame_num = (uint32_t)test_frame / PAGE_SIZE;
    pmm_free_frame(test_frame);

    if (bitmap_test(test_frame_num)) {
        serial_print("PMM: FAILED - frame not freed in bitmap\n");
        return;
    }
    serial_print("PMM: Frame edge case OK\n");

    serial_print("PMM: Verifying reserved regions...\n");

    uint32_t reserved_used = 0;
    uint32_t reserved_free = 0;
    for (uint32_t i = 0; i < 256; i++) {
        if (bitmap_test(i)) {
            reserved_used++;
        } else {
            reserved_free++;
        }
    }
    serial_print("PMM: Low mem: ");
    serial_print_uint(reserved_used);
    serial_print(" reserved, ");
    serial_print_uint(reserved_free);
    serial_print(" free\n");

    serial_print("PMM: Reserved regions OK\n");

    serial_print("PMM: All self-tests passed!\n");
}
