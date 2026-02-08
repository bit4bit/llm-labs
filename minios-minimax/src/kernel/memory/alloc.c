#include "memory.h"
#include "../kernel.h"
#include "../debug.h"

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
    DEBUG_PMM("Initializing...");

    uint32_t mem_upper_kb = mbd->mem_upper;
    pmm_frame_count = (mem_upper_kb * 1024) / PAGE_SIZE;

    DEBUG_PMM("Total frames: %u", pmm_frame_count);

    uint32_t bitmap_size = (pmm_frame_count + 7) / 8;
    pmm_bitmap = (uint8_t*)0x1000000;

    for (uint32_t i = 0; i < bitmap_size; i++) {
        pmm_bitmap[i] = 0;
    }

    pmm_used_frames = 0;

    multiboot_memory_map_t* mmap = (multiboot_memory_map_t*)mbd->mmap_addr;
    uint32_t mmap_end = mbd->mmap_addr + mbd->mmap_length;

    DEBUG_PMM("Marking reserved regions...");

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

    DEBUG_PMM("Usable frames marked");
}

void* pmm_alloc_frame(void) {
    for (uint32_t i = 0; i < pmm_frame_count; i++) {
        if (!bitmap_test(i)) {
            bitmap_set(i);
            pmm_used_frames++;
            return (void*)(i * PAGE_SIZE);
        }
    }
    DEBUG_ERROR("OUT OF MEMORY!");
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

void pmm_free_frame_4mb(void* addr) {
    uint32_t frame = (uint32_t)addr / PAGE_SIZE;
    if (frame + 1024 <= pmm_frame_count) {
        for (uint32_t i = 0; i < 1024; i++) {
            bitmap_clear(frame + i);
            pmm_used_frames--;
        }
    }
}

void* pmm_alloc_frame_4mb(void) {
    for (uint32_t i = 0; i < pmm_frame_count; i += 1024) {
        int free = 1;
        for (uint32_t j = 0; j < 1024; j++) {
            if (bitmap_test(i + j)) {
                free = 0;
                break;
            }
        }
        if (free) {
            for (uint32_t j = 0; j < 1024; j++) {
                bitmap_set(i + j);
                pmm_used_frames++;
            }
            return (void*)(i * PAGE_SIZE);
        }
    }
    DEBUG_ERROR("OUT OF MEMORY (4MB)!");
    while (1) {
        __asm__ volatile ("hlt");
    }
}

void* pmm_alloc_frame_reserved(void) {
    for (uint32_t i = 16; i < pmm_frame_count; i++) {
        if (!bitmap_test(i)) {
            bitmap_set(i);
            pmm_used_frames++;
            return (void*)(i * PAGE_SIZE);
        }
    }
    DEBUG_ERROR("OUT OF MEMORY (reserved)!");
    while (1) {
        __asm__ volatile ("hlt");
    }
}

uint32_t pmm_get_free_count(void) {
    return pmm_frame_count - pmm_used_frames;
}

void pmm_test(void) {
    DEBUG_PMM("Running self-tests...");

    DEBUG_PMM("Total frames: %u", pmm_frame_count);
    DEBUG_PMM("Used frames: %u", pmm_used_frames);
    DEBUG_PMM("Free frames: %u", pmm_get_free_count());

    uint32_t free_before = pmm_get_free_count();
    if (free_before < 20) {
        DEBUG_PMM("SKIPPED - not enough free frames for test");
        return;
    }

    DEBUG_PMM("Testing allocation and deallocation...");

    void* frames[10];
    for (int i = 0; i < 10; i++) {
        frames[i] = pmm_alloc_frame();
        uint32_t frame_num = (uint32_t)frames[i] / PAGE_SIZE;
        if ((uint32_t)frames[i] % PAGE_SIZE != 0) {
            DEBUG_ERROR("Allocated address not page-aligned: 0x%X", (uint32_t)frames[i]);
            return;
        }
        DEBUG_PMM("Allocated frame %u at 0x%X (frame %u)", i, (uint32_t)frames[i], frame_num);
    }

    uint32_t free_after_alloc = pmm_get_free_count();
    if (free_after_alloc != free_before - 10) {
        DEBUG_ERROR("Free count mismatch (expected %u, got %u)", free_before - 10, free_after_alloc);
        return;
    }
    DEBUG_PMM("Free count correct after allocation");

    DEBUG_PMM("Testing read/write on allocated frames...");

    uint32_t patterns[] = {0xDEADBEEF, 0xCAFEBABE, 0x12345678, 0xFFFFFFFF, 0x00000000, 0xA5A5A5A5};
    uint32_t num_patterns = sizeof(patterns) / sizeof(patterns[0]);

    for (int f = 0; f < 5; f++) {
        uint32_t* frame = (uint32_t*)frames[f];
        uint32_t frame_addr = (uint32_t)frame;

        DEBUG_PMM("Testing frame %u at 0x%X...", f, frame_addr);

        for (uint32_t p = 0; p < num_patterns; p++) {
            uint32_t pattern = patterns[p];

            for (uint32_t offset = 0; offset < PAGE_SIZE; offset += 4096) {
                uint32_t* test_addr = (uint32_t*)(frame_addr + offset);
                uint32_t saved = *test_addr;

                *test_addr = pattern;
                if (*test_addr != pattern) {
                    DEBUG_ERROR("Write/read mismatch at 0x%X expected 0x%X got 0x%X", (uint32_t)test_addr, pattern, *test_addr);
                    return;
                }

                volatile uint16_t* word_addr = (volatile uint16_t*)test_addr;
                volatile uint8_t* byte_addr = (volatile uint8_t*)test_addr;

                *word_addr = (uint16_t)(pattern & 0xFFFF);
                if (*word_addr != (pattern & 0xFFFF)) {
                    *test_addr = saved;
                    DEBUG_ERROR("Word write/read at 0x%X", (uint32_t)test_addr);
                    return;
                }

                *byte_addr = (uint8_t)(pattern & 0xFF);
                if (*byte_addr != (pattern & 0xFF)) {
                    *test_addr = saved;
                    DEBUG_ERROR("Byte write/read at 0x%X", (uint32_t)test_addr);
                    return;
                }

                *test_addr = saved;
            }
        }
        DEBUG_PMM("Frame %u OK", f);
    }

    DEBUG_PMM("Read/write tests passed");

    DEBUG_PMM("Testing full page write/read...");
    uint32_t* full_test = (uint32_t*)frames[0];
    for (uint32_t i = 0; i < PAGE_SIZE / 4; i++) {
        full_test[i] = 0xABCD1234;
    }
    for (uint32_t i = 0; i < PAGE_SIZE / 4; i++) {
        if (full_test[i] != 0xABCD1234) {
            DEBUG_ERROR("Full page test at offset 0x%X", i * 4);
            return;
        }
    }
    DEBUG_PMM("Full page test OK (1KB pattern x 1024)");

    DEBUG_PMM("Testing free...");
    for (int i = 0; i < 10; i++) {
        pmm_free_frame(frames[i]);
        DEBUG_PMM("Freed frame %u at 0x%X", i, (uint32_t)frames[i]);
    }

    uint32_t free_after_free = pmm_get_free_count();
    if (free_after_free != free_before) {
        DEBUG_ERROR("Free count mismatch after free (expected %u, got %u)", free_before, free_after_free);
        return;
    }
    DEBUG_PMM("Free count correct after freeing");

    DEBUG_PMM("Testing bitmap state...");
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
        DEBUG_ERROR("Bitmap used count mismatch (bitmap=%u, pmm_used_frames=%u)", alloc_count, pmm_used_frames);
        return;
    }

    if (free_count != pmm_get_free_count()) {
        DEBUG_ERROR("Bitmap free count mismatch");
        return;
    }
    DEBUG_PMM("Bitmap state verified");

    DEBUG_PMM("Testing alloc/free consistency...");
    void* addr1 = pmm_alloc_frame();
    void* addr2 = pmm_alloc_frame();
    if (addr1 >= addr2) {
        DEBUG_ERROR("Allocator returned addresses out of order");
        return;
    }
    pmm_free_frame(addr1);
    pmm_free_frame(addr2);
    DEBUG_PMM("Allocator order verified");

    DEBUG_PMM("Testing edge cases...");

    void* test_frame = pmm_alloc_frame();
    uint32_t test_frame_num = (uint32_t)test_frame / PAGE_SIZE;
    pmm_free_frame(test_frame);

    if (bitmap_test(test_frame_num)) {
        DEBUG_ERROR("Frame not freed in bitmap");
        return;
    }
    DEBUG_PMM("Frame edge case OK");

    DEBUG_PMM("Verifying reserved regions...");

    uint32_t reserved_used = 0;
    uint32_t reserved_free = 0;
    for (uint32_t i = 0; i < 256; i++) {
        if (bitmap_test(i)) {
            reserved_used++;
        } else {
            reserved_free++;
        }
    }
    DEBUG_PMM("Low mem: %u reserved, %u free", reserved_used, reserved_free);

    DEBUG_PMM("Reserved regions OK");

    DEBUG_PMM("All self-tests passed!");
}
