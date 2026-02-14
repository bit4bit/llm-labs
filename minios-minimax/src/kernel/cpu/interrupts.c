#include <stdint.h>
#include "../kernel.h"
#include "../debug.h"
#include "idt.h"
#include "../process/process.h"

#define PIT_PORT 0x40
#define PIT_CMD  0x43
#define PIT_FREQ 1193182
#define PIC_MASTER_CMD  0x20
#define PIC_MASTER_DATA 0x21
#define PIC_SLAVE_CMD   0xA0
#define PIC_SLAVE_DATA  0xA1
#define PIC_EOI         0x20

static volatile uint32_t pit_ticks = 0;

void divide_error_handler(void);
void gp_fault_handler(uint32_t* regs);
void page_fault_handler(void);
void timer_handler_asm(void);
extern void scheduler(void);

void pit_init(void) {
    DEBUG_PIT("Initializing PIT at 100Hz...");

    outb(0x20, 0x11);
    outb(0x21, 0x20);
    outb(0x21, 0x04);
    outb(0x21, 0x01);
    outb(0x21, 0x00);

    outb(0xA0, 0x11);
    outb(0xA1, 0x28);
    outb(0xA1, 0x02);
    outb(0xA1, 0x01);
    outb(0xA1, 0x00);

    outb(0x21, 0xFC);
    outb(0xA1, 0xFF);

    DEBUG_PIT("PIC remapped to vectors 0x20-0x27, 0x28-0x2F");

    uint32_t divisor = PIT_FREQ / 100;

    outb(PIT_CMD, 0x36);
    outb(PIT_PORT, divisor & 0xFF);
    outb(PIT_PORT, (divisor >> 8) & 0xFF);

    DEBUG_PIT("PIT initialized with divisor %u", divisor);

    uint8_t mask = inb(PIC_MASTER_DATA);
    outb(PIC_MASTER_DATA, mask & 0xFE);
    DEBUG_PIT("Unmasked PIC IRQ0 (timer)");
}

__attribute__((naked))
void timer_handler_asm(void) {
    __asm__ volatile (
        "pushal\n"
        "push %ds\n" "push %es\n" "push %fs\n" "push %gs\n"
        "mov $0x10, %eax\n"
        "mov %eax, %ds\n"
        "mov %eax, %es\n"
        "call timer_handler\n"
        "pop %gs\n" "pop %fs\n" "pop %es\n" "pop %ds\n"
        "popal\n"
        "iret\n"
    );
}

void timer_handler(void) {
    DEBUG_TIMER("TICK");

    outb(PIC_MASTER_CMD, PIC_EOI);
    outb(PIC_SLAVE_CMD, PIC_EOI);

    pit_ticks++;

    pcb_t* pcb = process_get_current();
    if (pcb != (void*)0) {
        pcb->run_count++;
    }

    scheduler();
}

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

uint32_t pit_get_ticks(void) {
    return pit_ticks;
}