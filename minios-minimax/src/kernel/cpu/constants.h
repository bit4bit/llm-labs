#ifndef CPU_CONSTANTS_H
#define CPU_CONSTANTS_H

/* GDT Descriptor flags - bits 0-7 of access byte */
#define GDT_DESC_PRESENT       0x80    /* Entry is present in memory */
#define GDT_DESC_DPL0          0x00    /* Ring 0 (kernel) privilege */
#define GDT_DESC_DPL3          0x60    /* Ring 3 (user) privilege */
#define GDT_DESC_TYPE_CODE     0x9A    /* Code segment: execute/read, accessed */
#define GDT_DESC_TYPE_DATA     0x92    /* Data segment: read/write, accessed */
#define GDT_DESC_GRANULARITY   0xC0    /* 4KB granularity, 32-bit segment */

/* GDT selectors - index shifted left by 3 bits */
#define KERNEL_CODE_SELECTOR   0x08    /* GDT entry 1: kernel code */
#define KERNEL_DATA_SELECTOR   0x10    /* GDT entry 2: kernel data */
#define USER_CODE_SELECTOR     0x18    /* GDT entry 3: user code */
#define USER_DATA_SELECTOR     0x20    /* GDT entry 4: user data */

/* GDT limit for 4GB address space (with 4KB granularity) */
#define GDT_LIMIT_4GB          0xFFFFF  /* 20-bit limit = 4GB */

/* IDT Descriptor flags - stored in access byte */
#define IDT_DESC_PRESENT       0x80    /* Entry is present in memory */
#define IDT_DESC_DPL0          0x00    /* Ring 0 only */
#define IDT_DESC_DPL3          0x60    /* Ring 3 allowed (for syscalls) */
#define IDT_DESC_TYPE_INTERRUPT 0x0E   /* 32-bit interrupt gate */
#define IDT_DESC_TYPE_TRAP     0x0F   /* 32-bit trap gate */
#define IDT_DESC_TYPE_INT_DPL3  (IDT_DESC_PRESENT | IDT_DESC_DPL3 | IDT_DESC_TYPE_INTERRUPT)  /* User-callable interrupt gate (0xEE) */

#endif
