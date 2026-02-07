#include <stdint.h>
#include "../kernel.h"
#include "../debug.h"

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
        "add $4, %esp\n"
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
    DEBUG_EXCEPT("DIVIDE ERROR");
    while (1) __asm__ volatile ("hlt");
}

void gp_fault_handler(uint32_t* regs) {
    uint32_t error_code = regs[8];
    uint32_t eip = regs[9];
    uint32_t cs = regs[10];
    uint32_t eflags = regs[11];
    
    DEBUG_EXCEPT("GENERAL PROTECTION FAULT");
    DEBUG_EXCEPT("  Error code: 0x%X", error_code);
    DEBUG_EXCEPT("  EIP: 0x%X", eip);
    DEBUG_EXCEPT("  CS: 0x%X", cs);
    DEBUG_EXCEPT("  EFLAGS: 0x%X", eflags);
    DEBUG_EXCEPT("  EAX: 0x%X", regs[7]);
    DEBUG_EXCEPT("  EBX: 0x%X", regs[4]);
    DEBUG_EXCEPT("  ESP: 0x%X", regs[3]);
    
    if (error_code & 0x1) {
        DEBUG_EXCEPT("  External event");
    }
    uint32_t selector = (error_code >> 3) & 0x1FFF;
    if (selector != 0) {
        DEBUG_EXCEPT("  Selector: 0x%X", selector << 3);
    }
    
    while (1) __asm__ volatile ("hlt");
}

void page_fault_handler(void) {
    uint32_t cr2;
    __asm__ volatile ("movl %%cr2, %0" : "=r"(cr2));

    DEBUG_EXCEPT("PAGE FAULT at 0x%X", cr2);

    while (1) __asm__ volatile ("hlt");
}
