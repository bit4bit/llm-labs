#include "multiboot2.h"
#include <stddef.h>

void *multiboot2_get_tag(void *mb_info, uint32_t type) {
    multiboot2_tag_t *tag = (multiboot2_tag_t *)((uint64_t)mb_info + 8);

    while (tag->type != MULTIBOOT2_TAG_TYPE_END) {
        if (tag->type == type) {
            return tag;
        }
        tag = (multiboot2_tag_t *)((uint64_t)tag + ((tag->size + 7) & ~7));
    }

    return NULL;
}

void multiboot2_parse_memory(multiboot2_memory_info_t *info, void *mb_info) {
    multiboot2_tag_meminfo_t *mem_tag = multiboot2_get_tag(mb_info, MULTIBOOT2_TAG_TYPE_BASIC_MEMINFO);

    if (mem_tag) {
        info->total_memory = mem_tag->mem_lower + mem_tag->mem_upper;
        info->free_memory = mem_tag->mem_lower;
    } else {
        info->total_memory = 0;
        info->free_memory = 0;
    }
}

void multiboot2_parse_mmap(void *mb_info) {
    multiboot2_tag_mmap_t *mmap_tag = multiboot2_get_tag(mb_info, MULTIBOOT2_TAG_TYPE_MMAP);

    if (!mmap_tag) {
        return;
    }
}
