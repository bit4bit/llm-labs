#include "syscall.h"
#include "../process/process.h"
#include "../kernel.h"

#define SYSCALL_EXIT 1
#define SYSCALL_WRITE 3

extern void syscall_return_to_kernel(void);

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
            serial_print("Syscall: exit called, halting...\n");
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
