#include <stdint.h>

void serial_putchar(char c);
void serial_print(const char* str);

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
