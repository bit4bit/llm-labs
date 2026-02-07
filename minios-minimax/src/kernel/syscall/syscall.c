#include "syscall.h"
#include "../process/process.h"
#include "../kernel.h"

#define SYSCALL_EXIT 1
#define SYSCALL_WRITE 3

extern void syscall_return_to_kernel(void);
extern void serial_print_hex(uint32_t val);

void syscall_handler(void) {
    uint32_t eax, ebx;

    __asm__ volatile (
        "mov %%eax, %0\n"
        "mov %%ebx, %1"
        : "=r"(eax), "=r"(ebx)
    );

    serial_print("Syscall: ");
    serial_print_uint(eax);
    serial_print("\n");

    switch (eax) {
        case SYSCALL_EXIT:
            serial_print("Syscall: exit called\n");
            
            // Show VGA memory content to verify "hola mundo" was written
            serial_print("VGA Memory (first line as ASCII):\n  \"");
            volatile uint16_t* vga = (volatile uint16_t*)0xB8000;
            for (int i = 0; i < 40; i++) {
                uint16_t val = vga[i];
                uint8_t ch = val & 0xFF;
                if (ch >= 32 && ch < 127) {
                    serial_putchar(ch);
                } else if (ch == 0) {
                    serial_print(" ");
                } else {
                    serial_print("?");
                }
            }
            serial_print("\"\n");
            
            serial_print("VGA Memory (bytes 0-21 as hex):\n  ");
            for (int i = 0; i < 11; i++) {
                uint16_t val = vga[i];
                uint8_t ch = val & 0xFF;
                serial_print("0x");
                serial_print_hex(ch);
                serial_print(" ");
            }
            serial_print("\n");
            
            serial_print("Halting...\n");
            while (1) {
                __asm__ volatile ("hlt");
            }
            break;
        case SYSCALL_WRITE:
            serial_print("Syscall: write (ignored in MVP)\n");
            break;
        default:
            serial_print("Syscall: unknown\n");
            break;
    }
}
