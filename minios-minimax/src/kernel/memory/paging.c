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

    // Set PDE 0-63: 256MB identity mapping (4MB each)
    for (int i = 0; i < 64; i++) {
        page_dir[i] = (i * 0x400000) | 0x83;
    }

    serial_print("Paging: PDE 0 set\n");

    serial_print("Paging: Enabling...\n");

    // Enable paging
    enable_paging_dir();

    serial_print("Paging: Enabled!\n");
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
