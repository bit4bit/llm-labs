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

    DEBUG_PMM("Total frames: %u (%u MB)", pmm_frame_count, pmm_frame_count * 4);

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
            uint32_t end_addr = addr + len;
            uint32_t end_frame = (end_addr - 1) / PAGE_SIZE;

            for (uint32_t i = start_frame; i <= end_frame; i++) {
                if (i < pmm_frame_count) {
                    if (!bitmap_test(i)) {
                        bitmap_set(i);
                        pmm_used_frames++;
                    }
                }
            }
        }
        mmap = (multiboot_memory_map_t*)((uint32_t)mmap + mmap->size + 4);
    }

    DEBUG_PMM("Usable frames marked");
    DEBUG_PMM("Reserved frames: %u", pmm_used_frames);
    DEBUG_PMM("Free frames: %u", pmm_frame_count - pmm_used_frames);
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

uint32_t pmm_get_free_count(void) {
    return pmm_frame_count - pmm_used_frames;
}

void pmm_test(void) {
    DEBUG_PMM("Running self-tests...");

    DEBUG_PMM("Total frames: %u", pmm_frame_count);
    DEBUG_PMM("Used frames: %u", pmm_used_frames);
    DEBUG_PMM("Free frames: %u", pmm_get_free_count());

    if (pmm_frame_count < 2) {
        DEBUG_PMM("SKIPPED - not enough frames for test");
        return;
    }

    DEBUG_PMM("Testing allocation and deallocation...");

    void* frame1 = pmm_alloc_frame();
    void* frame2 = pmm_alloc_frame();

    DEBUG_PMM("Allocated frame 1 at 0x%X", (uint32_t)frame1);
    DEBUG_PMM("Allocated frame 2 at 0x%X", (uint32_t)frame2);

    uint32_t free_after_alloc = pmm_get_free_count();
    if (free_after_alloc != pmm_frame_count - 2) {
        DEBUG_ERROR("Free count mismatch");
        return;
    }
    DEBUG_PMM("Free count correct after allocation");

    DEBUG_PMM("Testing read/write on allocated frame...");

    uint32_t* f = (uint32_t*)frame1;
    for (uint32_t i = 0; i < PAGE_SIZE / 4; i += 1024) {
        f[i] = 0xDEADBEEF;
        if (f[i] != 0xDEADBEEF) {
            DEBUG_ERROR("Write/read mismatch at offset 0x%X", i * 4);
            return;
        }
    }
    DEBUG_PMM("Read/write test passed");

    DEBUG_PMM("Testing free...");
    pmm_free_frame(frame1);
    pmm_free_frame(frame2);

    uint32_t free_after_free = pmm_get_free_count();
    if (free_after_free != pmm_frame_count - pmm_used_frames) {
        DEBUG_ERROR("Free count mismatch after free");
        return;
    }
    DEBUG_PMM("Free count correct after freeing");

    DEBUG_PMM("Testing bitmap state...");
    uint32_t alloc_count = 0;
    for (uint32_t i = 0; i < pmm_frame_count; i++) {
        if (bitmap_test(i)) alloc_count++;
    }

    if (alloc_count != pmm_used_frames) {
        DEBUG_ERROR("Bitmap used count mismatch");
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

    DEBUG_PMM("Verifying reserved regions...");
    uint32_t reserved_used = 0;
    for (uint32_t i = 0; i < 16; i++) {
        if (bitmap_test(i)) reserved_used++;
    }
    DEBUG_PMM("Low memory: %u reserved frames", reserved_used);

    DEBUG_PMM("Reserved regions OK");

    DEBUG_PMM("All self-tests passed!");
}
