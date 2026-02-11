#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

#define PAGE_SIZE       (4 * 1024 * 1024)
#define PAGE_SHIFT      22

typedef struct {
    uint32_t size;
    uint32_t addr_low;
    uint32_t addr_high;
    uint32_t len_low;
    uint32_t len_high;
    uint32_t type;
} __attribute__((packed)) multiboot_memory_map_t;

_Static_assert(sizeof(multiboot_memory_map_t) == 24, "C18: multiboot_memory_map_t must be 24 bytes");

typedef struct {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    uint32_t syms[4];
    uint32_t mmap_length;
    uint32_t mmap_addr;
} __attribute__((packed)) multiboot_info_t;

_Static_assert(sizeof(multiboot_info_t) == 52, "C18: multiboot_info_t must be 52 bytes");

extern uint32_t pmm_frame_count;
extern uint32_t pmm_used_frames;
extern uint8_t* pmm_bitmap;

void pmm_init(multiboot_info_t* mbd);
void* pmm_alloc_frame(void);
void pmm_free_frame(void* addr);
uint32_t pmm_get_free_count(void);
void pmm_test(void);

#endif
