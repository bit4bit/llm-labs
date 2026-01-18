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

void multiboot2_parse_mmap(multiboot2_tag_mmap_t *mmap_result, void *mb_info) {
    multiboot2_tag_mmap_t *mmap_tag = multiboot2_get_tag(mb_info, MULTIBOOT2_TAG_TYPE_MMAP);

    if (mmap_tag) {
        mmap_result->type = mmap_tag->type;
        mmap_result->size = mmap_tag->size;
        mmap_result->entry_size = mmap_tag->entry_size;
        mmap_result->entry_version = mmap_tag->entry_version;
    } else {
        mmap_result->type = 0;
        mmap_result->size = 0;
        mmap_result->entry_size = 0;
        mmap_result->entry_version = 0;
    }
}

uint32_t multiboot2_get_mmap_entry_count(void *mb_info) {
    multiboot2_tag_mmap_t *mmap_tag = multiboot2_get_tag(mb_info, MULTIBOOT2_TAG_TYPE_MMAP);
    
    if (!mmap_tag || mmap_tag->entry_size == 0) {
        return 0;
    }
    
    uint32_t data_size = mmap_tag->size - sizeof(multiboot2_tag_mmap_t);
    return data_size / mmap_tag->entry_size;
}

multiboot2_mmap_entry_t *multiboot2_get_mmap_entry(void *mb_info, uint32_t index) {
    multiboot2_tag_mmap_t *mmap_tag = multiboot2_get_tag(mb_info, MULTIBOOT2_TAG_TYPE_MMAP);
    
    if (!mmap_tag) {
        return NULL;
    }
    
    uint32_t entry_count = multiboot2_get_mmap_entry_count(mb_info);
    if (index >= entry_count) {
        return NULL;
    }
    
    return (multiboot2_mmap_entry_t *)((uint64_t)mmap_tag->entries + (index * mmap_tag->entry_size));
}

const char *multiboot2_get_cmdline(void *mb_info) {
    multiboot2_tag_cmdline_t *cmdline_tag = multiboot2_get_tag(mb_info, MULTIBOOT2_TAG_TYPE_BOOT_CMDLINE);
    
    if (cmdline_tag) {
        return cmdline_tag->cmdline;
    }
    return NULL;
}

uint32_t multiboot2_get_bootdev(void *mb_info, uint32_t *slice) {
    multiboot2_tag_bootdev_t *bootdev_tag = multiboot2_get_tag(mb_info, MULTIBOOT2_TAG_TYPE_BOOTDEV);
    
    if (bootdev_tag) {
        if (slice) {
            *slice = bootdev_tag->slice;
        }
        return bootdev_tag->biosdev;
    }
    if (slice) {
        *slice = 0;
    }
    return 0;
}
