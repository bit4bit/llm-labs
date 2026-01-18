#ifndef MULTIBOOT2_H
#define MULTIBOOT2_H

#include <stdint.h>

#define MULTIBOOT2_BOOTLOADER_MAGIC 0x36D76289

#define MULTIBOOT2_TAG_TYPE_END             0
#define MULTIBOOT2_TAG_TYPE_BOOT_CMDLINE    1
#define MULTIBOOT2_TAG_TYPE_BOOT_LOADER_NAME 2
#define MULTIBOOT2_TAG_TYPE_MODULE          3
#define MULTIBOOT2_TAG_TYPE_BASIC_MEMINFO   4
#define MULTIBOOT2_TAG_TYPE_BOOTDEV         5
#define MULTIBOOT2_TAG_TYPE_MMAP            6
#define MULTIBOOT2_TAG_TYPE_VBE_INFO        7
#define MULTIBOOT2_TAG_TYPE_FRAMEBUFFER     8
#define MULTIBOOT2_TAG_TYPE_ELF_SECTIONS    9
#define MULTIBOOT2_TAG_TYPE_APM_TABLE       10
#define MULTIBOOT2_TAG_TYPE_EFI32           11
#define MULTIBOOT2_TAG_TYPE_EFI64           12
#define MULTIBOOT2_TAG_TYPE_SMBIOS          13
#define MULTIBOOT2_TAG_TYPE_ACPI_OLD        14
#define MULTIBOOT2_TAG_TYPE_ACPI_NEW        15
#define MULTIBOOT2_TAG_TYPE_NETWORK         16
#define MULTIBOOT2_TAG_TYPE_EFI_MMAP        17
#define MULTIBOOT2_TAG_TYPE_EFI_BS          18
#define MULTIBOOT2_TAG_TYPE_CPU_INFO        19
#define MULTIBOOT2_TAG_TYPE_CPUSPEED        20
#define MULTIBOOT2_TAG_TYPE_IRQ             21
#define MULTIBOOT2_TAG_TYPE_PROCESS_APIC    22
#define MULTIBOOT2_TAG_TYPE_IO_APIC         23

#define MULTIBOOT2_MEMORY_AVAILABLE         1
#define MULTIBOOT2_MEMORY_RESERVED          2

typedef struct {
    uint32_t type;
    uint32_t size;
} multiboot2_tag_t;

typedef struct {
    uint32_t type;
    uint32_t size;
    char cmdline[0];
} multiboot2_tag_cmdline_t;

typedef struct {
    uint32_t type;
    uint32_t size;
    char name[0];
} multiboot2_tag_loader_name_t;

typedef struct {
    uint32_t type;
    uint32_t size;
    uint32_t mod_start;
    uint32_t mod_end;
    char cmdline[0];
} multiboot2_tag_module_t;

typedef struct {
    uint32_t type;
    uint32_t size;
    uint32_t mem_lower;
    uint32_t mem_upper;
} multiboot2_tag_meminfo_t;

typedef struct {
    uint32_t type;
    uint32_t size;
    uint32_t biosdev;
    uint32_t slice;
} multiboot2_tag_bootdev_t;

typedef struct {
    uint64_t addr;
    uint64_t len;
    uint32_t type;
    uint32_t reserved;
} multiboot2_mmap_entry_t;

typedef struct {
    uint32_t type;
    uint32_t size;
    uint32_t entry_size;
    uint32_t entry_version;
    multiboot2_mmap_entry_t entries[0];
} multiboot2_tag_mmap_t;

typedef struct {
    uint32_t type;
    uint32_t size;
    uint64_t addr;
    uint64_t row;
    uint64_t pitch;
    uint16_t width;
    uint16_t height;
    uint16_t bpp;
    uint8_t memory_model;
    uint8_t red_mask_size;
    uint8_t red_mask_shift;
    uint8_t green_mask_size;
    uint8_t green_mask_shift;
    uint8_t blue_mask_size;
    uint8_t blue_mask_shift;
} multiboot2_tag_framebuffer_t;

typedef struct {
    uint32_t type;
    uint32_t size;
    uint64_t rsdp_addr;
} multiboot2_tag_rsdp_t;

typedef struct {
    uint32_t type;
    uint32_t size;
    uint64_t efi_system_table_addr;
} multiboot2_tag_efi64_t;

typedef struct {
    uint32_t total_memory;
    uint32_t free_memory;
} multiboot2_memory_info_t;

void multiboot2_parse_memory(multiboot2_memory_info_t *info, void *mb_info);
void multiboot2_parse_mmap(multiboot2_tag_mmap_t *mmap_result, void *mb_info);
void *multiboot2_get_tag(void *mb_info, uint32_t type);
uint32_t multiboot2_get_mmap_entry_count(void *mb_info);
multiboot2_mmap_entry_t *multiboot2_get_mmap_entry(void *mb_info, uint32_t index);
const char *multiboot2_get_cmdline(void *mb_info);
uint32_t multiboot2_get_bootdev(void *mb_info, uint32_t *slice);

#endif
