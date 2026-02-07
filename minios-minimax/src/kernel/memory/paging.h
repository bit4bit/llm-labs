#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

#define PAGE_SIZE           4096
#define PAGE_ALIGN          0x1000
#define PAGE_SHIFT          12

#define PDE_PRESENT         0x01
#define PDE_RW              0x02
#define PDE_USER            0x04
#define PDE_PS              0x80

#define KERNEL_VIRTUAL_BASE 0xC0000000
#define KERNEL_PAGE_SIZE    0x400000

typedef struct {
    uint32_t present    : 1;
    uint32_t rw         : 1;
    uint32_t user       : 1;
    uint32_t pwt        : 1;
    uint32_t pcd        : 1;
    uint32_t accessed   : 1;
    uint32_t reserved   : 1;
    uint32_t ps         : 1;
    uint32_t global     : 1;
    uint32_t available  : 3;
    uint32_t base       : 20;
} __attribute__((packed)) page_directory_entry_t;

typedef struct {
    uint32_t present    : 1;
    uint32_t rw         : 1;
    uint32_t user       : 1;
    uint32_t pwt        : 1;
    uint32_t pcd        : 1;
    uint32_t accessed   : 1;
    uint32_t dirty      : 1;
    uint32_t pat        : 1;
    uint32_t global     : 1;
    uint32_t available  : 3;
    uint32_t base       : 20;
} __attribute__((packed)) page_table_entry_t;

void paging_init(void);
void enable_paging(void);
void paging_switch(page_directory_entry_t* dir);
void paging_test(void);
void paging_extended_test(void);

#endif
