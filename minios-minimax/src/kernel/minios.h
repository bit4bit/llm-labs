#ifndef MINIOS_H
#define MINIOS_H

#include <stdint.h>

/**
 * minios.h - Centralized Memory Map Configuration
 * 
 * This header defines all memory-related constants for the MiniOS system.
 * All memory addresses, regions, and layout configurations are centralized here
 * to provide a single source of truth and prevent duplication.
 */

/* ============================================================================
 * HARDWARE ADDRESSES
 * ============================================================================ */

/** VGA Text Mode Buffer */
#define VGA_MEMORY              0xB8000

/** Serial Port COM1 Base Address */
#define COM1_PORT               0x3F8

/* ============================================================================
 * PAGE SIZE CONSTANTS
 * ============================================================================ */

/** Standard 4KB Page Size */
#define PAGE_SIZE_4KB           0x1000      /* 4096 bytes */

/** Extended 4MB Page Size (PSE) */
#define PAGE_SIZE_4MB           0x400000    /* 4,194,304 bytes */

/** Kernel Page Size (using 4MB pages) */
#define KERNEL_PAGE_SIZE        PAGE_SIZE_4MB

/* ============================================================================
 * VIRTUAL MEMORY LAYOUT - KERNEL SPACE
 * ============================================================================ */

/** Kernel Virtual Base Address (3GB boundary) */
#define KERNEL_VIRTUAL_BASE     0xC0000000

/** Kernel Identity Mapping Start */
#define KERNEL_IDENTITY_START   0x00000000

/** Kernel Identity Mapping End (256MB) */
#define KERNEL_IDENTITY_END     0x0FFFFFFF

/* ============================================================================
 * VIRTUAL MEMORY LAYOUT - USER SPACE
 * ============================================================================ */

/** User Program Load Address (1GB virtual, base for process 0) */
#define USER_PROGRAM_BASE       0x40000000

/** Per-process code virtual address: 0x40000000 + i * 4MB */
#define USER_CODE_VADDR(i)      (USER_PROGRAM_BASE + (i) * PAGE_SIZE_4MB)

/** Per-process code PDE index: 256 + i */
#define USER_CODE_PDE(i)        (PDE_USER_PROGRAM + (i))

/** Per-process stack PDE index: 767 - i */
#define USER_STACK_PDE(i)       (PDE_USER_STACK - (i))

/** Per-process stack virtual address base */
#define USER_STACK_VADDR(i)     (PDE_INDEX_TO_VADDR(USER_STACK_PDE(i)))

/** Per-process user stack initial pointer (top of 4MB region minus 4KB guard) */
#define USER_STACK_INITIAL(i)   (USER_STACK_VADDR(i) + PAGE_SIZE_4MB - PAGE_SIZE_4KB)

/* ============================================================================
 * PAGE DIRECTORY ENTRY INDICES
 * ============================================================================ */

/** PDE Index for VGA Memory and Low Memory (0-4MB) */
#define PDE_LOW_MEMORY          0

/** PDE Index Range Start for Kernel Identity Mapping (4MB-256MB) */
#define PDE_KERNEL_START        1

/** PDE Index Range End for Kernel Identity Mapping */
#define PDE_KERNEL_END          63

/** PDE Index for User Program (1GB virtual = 0x40000000) */
#define PDE_USER_PROGRAM        256         /* 0x40000000 / 0x400000 */

/** PDE Index for User Stack Region (0xBFC00000) */
#define PDE_USER_STACK          767         /* 0xBFC00000 / 0x400000 */

/** PDE Index for Kernel Virtual Base (3GB = 0xC0000000) */
#define PDE_KERNEL_VIRTUAL      768         /* 0xC0000000 / 0x400000 */

/* ============================================================================
 * PAGE DIRECTORY ENTRY FLAGS
 * ============================================================================ */

/** Present bit */
#define PDE_PRESENT             0x01

/** Read/Write bit */
#define PDE_RW                  0x02

/** User/Supervisor bit (0=kernel, 1=user) */
#define PDE_USER                0x04

/** Page Size bit (0=4KB, 1=4MB with PSE) */
#define PDE_PS                  0x80

/** Common flag combination: Present + R/W + Supervisor + 4MB */
#define PDE_KERNEL_4MB          (PDE_PRESENT | PDE_RW | PDE_PS)  /* 0x83 */

/** Common flag combination: Present + R/W + User + 4MB */
#define PDE_USER_4MB            (PDE_PRESENT | PDE_RW | PDE_USER | PDE_PS)  /* 0x87 */

/* ============================================================================
 * PHYSICAL MEMORY REGIONS
 * ============================================================================ */

/** Physical Memory Region: Low Memory (0-4MB) - VGA, BIOS, etc. */
#define PHYS_LOW_MEMORY_START   0x00000000
#define PHYS_LOW_MEMORY_END     0x003FFFFF

/** Physical Memory Region: Kernel Code/Data (4MB-256MB) */
#define PHYS_KERNEL_START       0x00400000
#define PHYS_KERNEL_END         0x0FFFFFFF

/* (User program and stack physical frames are now allocated dynamically
   per-process via pmm_alloc_frame() in process_create()) */

/* ============================================================================
 * UTILITY MACROS
 * ============================================================================ */

/** Convert virtual address to PDE index */
#define VADDR_TO_PDE_INDEX(vaddr)   ((vaddr) / PAGE_SIZE_4MB)

/** Convert PDE index to virtual address base */
#define PDE_INDEX_TO_VADDR(idx)     ((idx) * PAGE_SIZE_4MB)

/** Align address down to 4MB boundary */
#define ALIGN_4MB_DOWN(addr)        ((addr) & 0xFFC00000)

/** Align address up to 4MB boundary */
#define ALIGN_4MB_UP(addr)          (((addr) + PAGE_SIZE_4MB - 1) & 0xFFC00000)

/** Align address down to 4KB boundary */
#define ALIGN_4KB_DOWN(addr)        ((addr) & 0xFFFFF000)

/** Align address up to 4KB boundary */
#define ALIGN_4KB_UP(addr)          (((addr) + PAGE_SIZE_4KB - 1) & 0xFFFFF000)

/** Check if address is 4MB aligned */
#define IS_4MB_ALIGNED(addr)        (((addr) & 0x3FFFFF) == 0)

/** Check if address is 4KB aligned */
#define IS_4KB_ALIGNED(addr)        (((addr) & 0xFFF) == 0)

_Static_assert(sizeof(char) == 1, "C18: char size invariant");
_Static_assert(PAGE_SIZE_4KB == 4096, "C18: PAGE_SIZE_4KB must be 4096 bytes");
_Static_assert(PAGE_SIZE_4MB == 4194304, "C18: PAGE_SIZE_4MB must be 4194304 bytes");

#endif /* MINIOS_H */