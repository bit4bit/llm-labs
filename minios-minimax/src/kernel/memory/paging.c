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

    // Set PDE 0: 4MB page at 0x0
    page_dir[0] = 0x00000083;

    serial_print("Paging: PDE 0 set\n");

    serial_print("Paging: Enabling...\n");

    // Enable paging
    enable_paging_dir();

    serial_print("Paging: Enabled!\n");
}
