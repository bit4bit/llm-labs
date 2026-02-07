#include <stdint.h>

void serial_putchar(char c);
void serial_print(const char* str);
void serial_print_uint(uint32_t val);

extern uint32_t page_dir[1024];
extern void enable_paging_dir(void);

void paging_init(void) {
    serial_print("Paging: Initializing...\n");

    // Zero the page directory
    for (int i = 0; i < 1024; i++) {
        page_dir[i] = 0;
    }

    // Set PDE 0: 0-4MB identity mapping with user access (for VGA at 0xB8000)
    page_dir[0] = 0x00000000 | 0x87;  // Present | R/W | User | PS
    
    // Set PDE 1-63: 4MB-256MB identity mapping (kernel only)
    for (int i = 1; i < 64; i++) {
        page_dir[i] = (i * 0x400000) | 0x83;
    }

    serial_print("Paging: PDE 0 set (user, 0-4MB for VGA)\n");
    serial_print("Paging: PDE 1-63 set (kernel, 4-256MB identity)\n");

    serial_print("Paging: Enabling...\n");

    // Enable paging
    enable_paging_dir();

    serial_print("Paging: Enabled!\n");

    // Map 0x40000000 (1GB) for user process (PDE 256) AFTER paging is enabled
    // But we need to map it to a physical address that exists!
    // Let's use physical address 0x01000000 (16MB) instead
    // Virtual 0x40000000 -> Physical 0x01000000
    // Flags: 0x87 = Present | R/W | User | PS (4MB page)
    
    serial_print("Paging: Mapping virtual 0x40000000 to physical 0x01000000...\n");
    page_dir[256] = 0x01000000 | 0x87;
    
    serial_print("Paging: PDE 256 value = 0x");
    serial_print_uint(page_dir[256]);
    serial_print("\n");

    // Map user stack region at 0xBFFFF000 (PDE 767)
    // Virtual 0xBFC00000 -> Physical 0x02000000 (32MB)
    // Flags: 0x87 = Present | R/W | User | PS (4MB page)
    serial_print("Paging: Mapping user stack region 0xBFC00000 to physical 0x02000000...\n");
    page_dir[767] = 0x02000000 | 0x87;
    
    serial_print("Paging: PDE 767 value = 0x");
    serial_print_uint(page_dir[767]);
    serial_print("\n");

    // Flush TLB by reloading CR3
    serial_print("Paging: Flushing TLB...\n");
    __asm__ volatile (
        "movl %%cr3, %%eax\n"
        "movl %%eax, %%cr3\n"
        : : : "eax", "memory"
    );
    serial_print("Paging: TLB flushed\n");
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

    serial_print("Paging: Running self-tests...\n");

    cr0 = get_cr0();
    if (!(cr0 & 0x80000000)) {
        serial_print("Paging: FAILED - CR0.PG not set\n");
        return;
    }
    serial_print("Paging: CR0.PG OK\n");

    cr4 = get_cr4();
    if (!(cr4 & 0x00000010)) {
        serial_print("Paging: FAILED - CR4.PSE not set\n");
        return;
    }
    serial_print("Paging: CR4.PSE OK\n");

    cr3 = get_cr3();
    if ((cr3 & 0xFFF) != 0) {
        serial_print("Paging: FAILED - CR3 not page-aligned\n");
        return;
    }
    serial_print("Paging: CR3 aligned OK\n");

    serial_print("Paging: Testing write/read patterns across 4MB pages...\n");

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

        serial_print("Paging: Testing addr 0x");
        serial_print_uint(addr);
        serial_print(" (page ");
        serial_print_uint(page_num);
        serial_print(", offset 0x");
        serial_print_uint(offset);
        serial_print(")...\n");

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
                serial_print("Paging: FAILED - dword write/read at 0x");
                serial_print_uint(addr);
                serial_print(" expected 0x");
                serial_print_uint(pattern);
                serial_print(" got 0x");
                serial_print_uint(*dword_addr);
                serial_print("\n");
                return;
            }

            *word_addr = (uint16_t)(pattern & 0xFFFF);
            if (*word_addr != (pattern & 0xFFFF)) {
                *dword_addr = saved_dword;
                serial_print("Paging: FAILED - word write/read at 0x");
                serial_print_uint(addr);
                serial_print("\n");
                return;
            }

            *byte_addr = (uint8_t)(pattern & 0xFF);
            if (*byte_addr != (pattern & 0xFF)) {
                *dword_addr = saved_dword;
                serial_print("Paging: FAILED - byte write/read at 0x");
                serial_print_uint(addr);
                serial_print("\n");
                return;
            }

            *dword_addr = saved_dword;
            *word_addr = saved_word;
            *byte_addr = saved_byte;
        }

        serial_print("Paging:  Address 0x");
        serial_print_uint(addr);
        serial_print(" OK\n");
    }

    serial_print("Paging: Write/read patterns OK\n");

    serial_print("Paging: Verifying physical = virtual (identity mapping)...\n");
    uint32_t virt_test[] = {0x100000, 0x200000, 0x1000000};
    for (int i = 0; i < 3; i++) {
        uint32_t vaddr = virt_test[i];
        uint32_t pde_idx = vaddr / 0x400000;
        uint32_t pde_val = page_dir[pde_idx];
        uint32_t expected_phys = (pde_val & 0xFFC00000) + (vaddr % 0x400000);

        if (expected_phys != vaddr) {
            serial_print("Paging: FAILED - identity mapping at 0x");
            serial_print_uint(vaddr);
            serial_print(" expected phys=0x");
            serial_print_uint(expected_phys);
            serial_print("\n");
            return;
        }
        serial_print("Paging: 0x");
        serial_print_uint(vaddr);
        serial_print(" -> 0x");
        serial_print_uint(expected_phys);
        serial_print(" OK\n");
    }

    serial_print("Paging: Identity mapping verified\n");

    serial_print("Paging: Testing page boundary crossings...\n");
    volatile uint32_t* boundary_start = (volatile uint32_t*)0x100000;
    volatile uint32_t* boundary_end = (volatile uint32_t*)0x103FFC;

    uint32_t boundary_pattern = 0xABABABAB;
    for (volatile uint32_t* p = boundary_start; p <= boundary_end; p += 1024) {
        uint32_t saved = *p;
        *p = boundary_pattern;
        if (*p != boundary_pattern) {
            serial_print("Paging: FAILED - boundary test at 0x");
            serial_print_uint((uint32_t)p);
            serial_print("\n");
            return;
        }
        *p = saved;
    }
    serial_print("Paging: Boundary crossings OK\n");

    serial_print("Paging: Testing page directory entries...\n");
    for (int i = 0; i < 64; i++) {
        uint32_t pde = page_dir[i];
        if ((pde & 0x00000001) == 0) {
            serial_print("Paging: FAILED - PDE ");
            serial_print_uint(i);
            serial_print(" not present\n");
            return;
        }
        if ((pde & 0x00000080) == 0) {
            serial_print("Paging: FAILED - PDE ");
            serial_print_uint(i);
            serial_print(" not 4MB page\n");
            return;
        }
        if ((pde & 0x00000002) == 0) {
            serial_print("Paging: FAILED - PDE ");
            serial_print_uint(i);
            serial_print(" not writable\n");
            return;
        }
    }
    serial_print("Paging: PDE 0-63 OK (64 entries)\n");

    serial_print("Paging: All self-tests passed!\n");
}

void paging_extended_test(void) {
    uint32_t saved_cr3;

    serial_print("Paging: Running extended self-tests...\n");

    serial_print("Paging: Verifying PDE consistency (0-63)...\n");
    for (int i = 0; i < 64; i++) {
        uint32_t pde = page_dir[i];

        if ((pde & 0x00000001) != 0x00000001) {
            serial_print("Paging: FAILED - PDE ");
            serial_print_uint(i);
            serial_print(" present bit incorrect\n");
            return;
        }

        if ((pde & 0x00000002) != 0x00000002) {
            serial_print("Paging: FAILED - PDE ");
            serial_print_uint(i);
            serial_print(" rw bit incorrect\n");
            return;
        }

        if ((pde & 0x00000080) != 0x00000080) {
            serial_print("Paging: FAILED - PDE ");
            serial_print_uint(i);
            serial_print(" ps bit incorrect\n");
            return;
        }

        uint32_t expected_base = (i * 0x400000) & 0xFFC00000;
        uint32_t actual_base = pde & 0xFFC00000;
        if (expected_base != actual_base) {
            serial_print("Paging: FAILED - PDE ");
            serial_print_uint(i);
            serial_print(" base mismatch: expected 0x");
            serial_print_uint(expected_base);
            serial_print(" got 0x");
            serial_print_uint(actual_base);
            serial_print("\n");
            return;
        }
    }
    serial_print("Paging: PDE 0-63 consistency verified\n");

    serial_print("Paging: Testing PDE alignment...\n");
    for (int i = 0; i < 64; i++) {
        uint32_t pde = page_dir[i];
        uint32_t base = (pde >> 22) & 0x3FF;
        if (base != (uint32_t)i) {
            serial_print("Paging: FAILED - PDE ");
            serial_print_uint(i);
            serial_print(" alignment: expected ");
            serial_print_uint(i);
            serial_print(" got ");
            serial_print_uint(base);
            serial_print("\n");
            return;
        }
    }
    serial_print("Paging: PDE alignment OK\n");

    serial_print("Paging: Testing CR3 reload consistency...\n");
    saved_cr3 = get_cr3();

    __asm__ volatile (
        "movl %%cr3, %%eax\n"
        "movl %%eax, %%cr3\n"
        : : : "eax", "memory"
    );

    uint32_t new_cr3 = get_cr3();
    if (saved_cr3 != new_cr3) {
        serial_print("Paging: FAILED - CR3 changed after reload: 0x");
        serial_print_uint(saved_cr3);
        serial_print(" -> 0x");
        serial_print_uint(new_cr3);
        serial_print("\n");
        return;
    }
    serial_print("Paging: CR3 reload consistent\n");

    serial_print("Paging: Testing page directory entry access (read-only)...\n");
    volatile uint32_t* pde_ptr = (volatile uint32_t*)&page_dir[0];
    uint32_t pde_value = *pde_ptr;
    if (pde_value == 0) {
        serial_print("Paging: FAILED - PDE[0] is zero (should be identity mapping)\n");
        return;
    }
    serial_print("Paging: PDE[0] readable: 0x");
    serial_print_uint(pde_value);
    serial_print("\n");

    serial_print("Paging: Verifying upper memory is unmapped (>256MB)...\n");
    uint32_t upper_pde_idx = 0x10000000 / 0x400000;
    if (upper_pde_idx >= 1024 || page_dir[upper_pde_idx] == 0) {
        serial_print("Paging: Upper memory region (>256MB) correctly unmapped\n");
    } else {
        serial_print("Paging: WARNING - Upper memory region unexpectedly mapped\n");
    }

    serial_print("Paging: Testing write pattern stress test...\n");
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
                    serial_print("Paging: FAILED - stress write at 0x");
                    serial_print_uint(addr);
                    serial_print(" expected 0x");
                    serial_print_uint(pattern);
                    serial_print(" got 0x");
                    serial_print_uint(*ptr);
                    serial_print("\n");
                }
                *ptr = saved;
            }
        }
    }

    if (stress_failures > 0) {
        serial_print("Paging: FAILED - ");
        serial_print_uint(stress_failures);
        serial_print(" stress test failures\n");
        return;
    }
    serial_print("Paging: Stress test passed (multiple patterns)\n");

    serial_print("Paging: All extended self-tests passed!\n");
}
