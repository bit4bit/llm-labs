#include <stdint.h>
#include "../kernel.h"

extern void serial_print_hex(uint32_t val);

void divide_error_handler(void);
void gp_fault_handler(uint32_t* regs);
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
        "push %esp\n"
        "call gp_fault_handler\n"
        "add $4, %esp\n"
        "popal\n"
        "add $4, %esp\n"  // Skip error code
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

void gp_fault_handler(uint32_t* regs) {
    uint32_t error_code = regs[8];  // Error code is pushed after registers
    uint32_t eip = regs[9];
    uint32_t cs = regs[10];
    uint32_t eflags = regs[11];
    
    serial_print("GENERAL PROTECTION FAULT\n");
    serial_print("  Error code: 0x");
    serial_print_hex(error_code);
    serial_print("\n");
    serial_print("  EIP: 0x");
    serial_print_hex(eip);
    serial_print("\n");
    serial_print("  CS: 0x");
    serial_print_hex(cs);
    serial_print("\n");
    serial_print("  EFLAGS: 0x");
    serial_print_hex(eflags);
    serial_print("\n");
    serial_print("  EAX: 0x");
    serial_print_hex(regs[7]);
    serial_print("\n");
    serial_print("  EBX: 0x");
    serial_print_hex(regs[4]);
    serial_print("\n");
    serial_print("  ESP: 0x");
    serial_print_hex(regs[3]);
    serial_print("\n");
    
    if (error_code & 0x1) {
        serial_print("  External event\n");
    }
    uint32_t selector = (error_code >> 3) & 0x1FFF;
    if (selector != 0) {
        serial_print("  Selector: 0x");
        serial_print_hex(selector << 3);
        serial_print("\n");
    }
    
    while (1) __asm__ volatile ("hlt");
}

void page_fault_handler(void) {
    uint32_t cr2;
    __asm__ volatile ("movl %%cr2, %0" : "=r"(cr2));

    serial_print("PAGE FAULT at 0x");
    serial_print_hex(cr2);
    serial_print("\n");

    while (1) __asm__ volatile ("hlt");
}
