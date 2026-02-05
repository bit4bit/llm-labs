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
    pmm_bitmap = (uint8_t*)0x100000;

    for (uint32_t i = 0; i < bitmap_size; i++) {
        pmm_bitmap[i] = 0;
    }

    pmm_used_frames = 0;

    multiboot_memory_map_t* mmap = (multiboot_memory_map_t*)mbd->mmap_addr;
    while ((uint32_t)mmap < mbd->mmap_addr + mbd->mmap_length) {
        if (mmap->type == 1) {
            uint32_t addr = mmap->addr_low;
            uint32_t len = mmap->len_low;

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
