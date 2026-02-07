#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

#define PAGE_SIZE       4096
#define PAGE_SHIFT      12

typedef struct {
    uint32_t size;
    uint32_t addr_low;
    uint32_t addr_high;
    uint32_t len_low;
    uint32_t len_high;
    uint32_t type;
} __attribute__((packed)) multiboot_memory_map_t;

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

extern uint32_t pmm_frame_count;
extern uint32_t pmm_used_frames;
extern uint8_t* pmm_bitmap;

void pmm_init(multiboot_info_t* mbd);
void* pmm_alloc_frame(void);
void pmm_free_frame(void* addr);
uint32_t pmm_get_free_count(void);
void pmm_test(void);

#endif
