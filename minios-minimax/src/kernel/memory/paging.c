#include <stdint.h>
#include "../minios.h"
#include "../debug.h"
#include "memory.h"

extern uint32_t page_dir[1024];
extern void enable_paging_dir(void);

void paging_init(void) {
    DEBUG_PAGING("Initializing...");

    for (int i = 0; i < 1024; i++) {
        page_dir[i] = 0;
    }

    DEBUG_PAGING("Setting up kernel identity mapping (4MB pages)...");

    for (int i = PDE_LOW_MEMORY; i <= PDE_KERNEL_END; i++) {
        page_dir[i] = (i * PAGE_SIZE_4MB) | PDE_KERNEL_4MB;
    }

    DEBUG_PAGING("PDE 0 set (user, 0-4MB for VGA)");
    DEBUG_PAGING("PDE 1-63 set (kernel, 4-256MB identity)");

    DEBUG_PAGING("Enabling...");

    enable_paging_dir();

    DEBUG_PAGING("Enabled!");

    DEBUG_PAGING("Allocating user program region...");
    void* user_prog_phys = pmm_alloc_frame_4mb();
    if (!user_prog_phys) {
        DEBUG_ERROR("Failed to allocate user program region");
        return;
    }
    page_dir[PDE_USER_PROGRAM] = ((uint32_t)user_prog_phys) | PDE_USER_4MB;
    DEBUG_PAGING("PDE %u: 0x%X -> 0x%X", PDE_USER_PROGRAM, PDE_INDEX_TO_VADDR(PDE_USER_PROGRAM), (uint32_t)user_prog_phys);

    DEBUG_PAGING("Allocating user stack region...");
    void* user_stack_phys = pmm_alloc_frame_4mb();
    if (!user_stack_phys) {
        DEBUG_ERROR("Failed to allocate user stack region");
        return;
    }
    page_dir[PDE_USER_STACK] = ((uint32_t)user_stack_phys) | PDE_USER_4MB;
    DEBUG_PAGING("PDE %u: 0x%X -> 0x%X", PDE_USER_STACK, PDE_INDEX_TO_VADDR(PDE_USER_STACK), (uint32_t)user_stack_phys);

    DEBUG_PAGING("Flushing TLB...");
    __asm__ volatile (
        "movl %%cr3, %%eax\n"
        "movl %%eax, %%cr3\n"
        : : : "eax", "memory"
    );
    DEBUG_PAGING("TLB flushed");
}

static inline uint32_t get_cr0(void) {
    uint32_t val;
    __asm__ volatile ("mov %%cr0, %0" : "=r"(val));
    return val;
}

static inline uint32_t get_cr3(void) {
    uint32_t val;
    __asm__ volatile ("mov %%cr3, %0" : "=r"(val));
    return val;
}

static inline uint32_t get_cr4(void) {
    uint32_t val;
    __asm__ volatile ("mov %%cr4, %0" : "=r"(val));
    return val;
}

void paging_test(void) {
    uint32_t cr0, cr3, cr4;

    DEBUG_PAGING("Running self-tests...");

    cr0 = get_cr0();
    if (!(cr0 & 0x80000000)) {
        DEBUG_ERROR("FAILED - CR0.PG not set");
        return;
    }
    DEBUG_PAGING("CR0.PG OK");

    cr4 = get_cr4();
    if (!(cr4 & 0x00000010)) {
        DEBUG_ERROR("FAILED - CR4.PSE not set");
        return;
    }
    DEBUG_PAGING("CR4.PSE OK");

    cr3 = get_cr3();
    if ((cr3 & 0xFFF) != 0) {
        DEBUG_ERROR("FAILED - CR3 not page-aligned");
        return;
    }
    DEBUG_PAGING("CR3 aligned OK");

    DEBUG_PAGING("Testing write/read patterns across 4MB pages...");

    uint32_t test_patterns[] = {0xDEADBEEF, 0xCAFEBABE, 0x12345678, 0xFFFFFFFF, 0x00000000, 0xA5A5A5A5};
    uint32_t num_patterns = sizeof(test_patterns) / sizeof(test_patterns[0]);

    uint32_t test_addresses[] = {
        0x100000,
        0x104000,
        0x1FFFFFF,
        0x200000,
        0x400000,
        0x800000,
        0x1000000,
        0x1000000 + 0x1000,
        0x3FFFFF0
    };
    uint32_t num_addresses = sizeof(test_addresses) / sizeof(test_addresses[0]);

    for (uint32_t a = 0; a < num_addresses; a++) {
        uint32_t addr = test_addresses[a];
        uint32_t page_num = addr / 0x400000;
        uint32_t offset = addr % 0x400000;

        DEBUG_PAGING("Testing addr 0x%X (page %u, offset 0x%X)...", addr, page_num, offset);

        volatile uint8_t* byte_addr = (volatile uint8_t*)addr;
        volatile uint16_t* word_addr = (volatile uint16_t*)addr;
        volatile uint32_t* dword_addr = (volatile uint32_t*)addr;

        for (uint32_t p = 0; p < num_patterns; p++) {
            uint32_t pattern = test_patterns[p];

            uint32_t saved_byte = *byte_addr;
            uint32_t saved_word = *word_addr;
            uint32_t saved_dword = *dword_addr;

            *dword_addr = pattern;
            if (*dword_addr != pattern) {
                DEBUG_ERROR("FAILED - dword write/read at 0x%X expected 0x%X got 0x%X", addr, pattern, *dword_addr);
                return;
            }

            *word_addr = (uint16_t)(pattern & 0xFFFF);
            if (*word_addr != (pattern & 0xFFFF)) {
                *dword_addr = saved_dword;
                DEBUG_ERROR("FAILED - word write/read at 0x%X", addr);
                return;
            }

            *byte_addr = (uint8_t)(pattern & 0xFF);
            if (*byte_addr != (pattern & 0xFF)) {
                *dword_addr = saved_dword;
                DEBUG_ERROR("FAILED - byte write/read at 0x%X", addr);
                return;
            }

            *dword_addr = saved_dword;
            *word_addr = saved_word;
            *byte_addr = saved_byte;
        }

        DEBUG_PAGING("Address 0x%X OK", addr);
    }

    DEBUG_PAGING("Write/read patterns OK");

    DEBUG_PAGING("Verifying physical = virtual (identity mapping)...");
    uint32_t virt_test[] = {0x100000, 0x200000, 0x1000000};
    for (int i = 0; i < 3; i++) {
        uint32_t vaddr = virt_test[i];
        uint32_t pde_idx = vaddr / 0x400000;
        uint32_t pde_val = page_dir[pde_idx];
        uint32_t expected_phys = (pde_val & 0xFFC00000) + (vaddr % 0x400000);

        if (expected_phys != vaddr) {
            DEBUG_ERROR("FAILED - identity mapping at 0x%X expected phys=0x%X", vaddr, expected_phys);
            return;
        }
        DEBUG_PAGING("0x%X -> 0x%X OK", vaddr, expected_phys);
    }

    DEBUG_PAGING("Identity mapping verified");

    DEBUG_PAGING("Testing page boundary crossings...");
    volatile uint32_t* boundary_start = (volatile uint32_t*)0x100000;
    volatile uint32_t* boundary_end = (volatile uint32_t*)0x103FFC;

    uint32_t boundary_pattern = 0xABABABAB;
    for (volatile uint32_t* p = boundary_start; p <= boundary_end; p += 1024) {
        uint32_t saved = *p;
        *p = boundary_pattern;
        if (*p != boundary_pattern) {
            DEBUG_ERROR("FAILED - boundary test at 0x%X", (uint32_t)p);
            return;
        }
        *p = saved;
    }
    DEBUG_PAGING("Boundary crossings OK");

    DEBUG_PAGING("Testing page directory entries...");
    for (int i = 0; i < 64; i++) {
        uint32_t pde = page_dir[i];
        if ((pde & 0x00000001) == 0) {
            DEBUG_ERROR("FAILED - PDE %u not present", i);
            return;
        }
        if ((pde & 0x00000080) == 0) {
            DEBUG_ERROR("FAILED - PDE %u not 4MB page", i);
            return;
        }
        if ((pde & 0x00000002) == 0) {
            DEBUG_ERROR("FAILED - PDE %u not writable", i);
            return;
        }
    }
    DEBUG_PAGING("PDE 0-63 OK (64 entries)");

    DEBUG_PAGING("All self-tests passed!");
}

void paging_extended_test(void) {
    uint32_t saved_cr3;

    DEBUG_PAGING("Running extended self-tests...");

    DEBUG_PAGING("Verifying PDE consistency (kernel range)...");
    for (int i = PDE_LOW_MEMORY; i <= PDE_KERNEL_END; i++) {
        uint32_t pde = page_dir[i];

        if ((pde & 0x00000001) != 0x00000001) {
            DEBUG_ERROR("FAILED - PDE %u present bit incorrect", i);
            return;
        }

        if ((pde & 0x00000002) != 0x00000002) {
            DEBUG_ERROR("FAILED - PDE %u rw bit incorrect", i);
            return;
        }

        if ((pde & 0x00000080) != 0x00000080) {
            DEBUG_ERROR("FAILED - PDE %u ps bit incorrect", i);
            return;
        }

        uint32_t expected_base = (i * PAGE_SIZE_4MB) & 0xFFC00000;
        uint32_t actual_base = pde & 0xFFC00000;
        if (expected_base != actual_base) {
            DEBUG_ERROR("FAILED - PDE %u base mismatch expected 0x%X got 0x%X", i, expected_base, actual_base);
            return;
        }
    }
    DEBUG_PAGING("PDE kernel range consistency verified");

    DEBUG_PAGING("Testing PDE alignment...");
    for (int i = PDE_LOW_MEMORY; i <= PDE_KERNEL_END; i++) {
        uint32_t pde = page_dir[i];
        uint32_t base = (pde >> 22) & 0x3FF;
        if (base != (uint32_t)i) {
            DEBUG_ERROR("FAILED - PDE %u alignment expected %u got %u", i, i, base);
            return;
        }
    }
    DEBUG_PAGING("PDE alignment OK");

    DEBUG_PAGING("Testing CR3 reload consistency...");
    saved_cr3 = get_cr3();

    __asm__ volatile (
        "movl %%cr3, %%eax\n"
        "movl %%eax, %%cr3\n"
        : : : "eax", "memory"
    );

    uint32_t new_cr3 = get_cr3();
    if (saved_cr3 != new_cr3) {
        DEBUG_ERROR("FAILED - CR3 changed after reload: 0x%X -> 0x%X", saved_cr3, new_cr3);
        return;
    }
    DEBUG_PAGING("CR3 reload consistent");

    DEBUG_PAGING("Testing page directory entry access (read-only)...");
    volatile uint32_t* pde_ptr = (volatile uint32_t*)&page_dir[0];
    uint32_t pde_value = *pde_ptr;
    if (pde_value == 0) {
        DEBUG_ERROR("FAILED - PDE[0] is zero (should be identity mapping)");
        return;
    }
    DEBUG_PAGING("PDE[0] readable: 0x%X", pde_value);

    DEBUG_PAGING("Verifying upper memory is unmapped (>256MB)...");
    uint32_t upper_pde_idx = 0x10000000 / PAGE_SIZE_4MB;
    if (upper_pde_idx >= 1024 || page_dir[upper_pde_idx] == 0) {
        DEBUG_PAGING("Upper memory region (>256MB) correctly unmapped");
    } else {
        DEBUG_WARN("Upper memory region unexpectedly mapped");
    }

    DEBUG_PAGING("Testing write pattern stress test...");
    uint32_t stress_patterns[] = {
        0xFFFFFFFF, 0x00000000, 0xA5A5A5A5,
        0x5A5A5A5A, 0x12345678, 0x87654321
    };
    int stress_failures = 0;

    for (int p = 0; p < 6; p++) {
        uint32_t pattern = stress_patterns[p];
        for (uint32_t addr = 0x100000; addr < 0x4000000; addr += 0x100000) {
            volatile uint32_t* ptr = (volatile uint32_t*)addr;
            uint32_t saved = *ptr;
            *ptr = pattern;
            if (*ptr != pattern) {
                stress_failures++;
                if (stress_failures <= 5) {
                    DEBUG_ERROR("FAILED - stress write at 0x%X expected 0x%X got 0x%X", addr, pattern, *ptr);
                }
                *ptr = saved;
            }
        }
    }

    if (stress_failures > 0) {
        DEBUG_ERROR("FAILED - %u stress test failures", stress_failures);
        return;
    }
    DEBUG_PAGING("Stress test passed (multiple patterns)");

    DEBUG_PAGING("All extended self-tests passed!");
}
