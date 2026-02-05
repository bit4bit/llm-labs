#include <stdint.h>
#include "../kernel.h"

void divide_error_handler(void);
void gp_fault_handler(void);
void page_fault_handler(void);

__attribute__((naked))
void handle_divide_error(void) {
    __asm__ volatile (
        "pushal\n"
        "call divide_error_handler\n"
        "popal\n"
        "iret\n"
    );
}

__attribute__((naked))
void handle_general_protection_fault(void) {
    __asm__ volatile (
        "pushal\n"
        "call gp_fault_handler\n"
        "popal\n"
        "iret\n"
    );
}

__attribute__((naked))
void handle_page_fault(void) {
    __asm__ volatile (
        "pushal\n"
        "call page_fault_handler\n"
        "popal\n"
        "iret\n"
    );
}

void divide_error_handler(void) {
    serial_print("DIVIDE ERROR\n");
    while (1) __asm__ volatile ("hlt");
}

void gp_fault_handler(void) {
    serial_print("GENERAL PROTECTION FAULT\n");
    while (1) __asm__ volatile ("hlt");
}

void page_fault_handler(void) {
    uint32_t cr2;
    __asm__ volatile ("movl %%cr2, %0" : "=r"(cr2));

    serial_print("PAGE FAULT at 0x");
    serial_print_uint(cr2);
    serial_print("\n");

    while (1) __asm__ volatile ("hlt");
}
