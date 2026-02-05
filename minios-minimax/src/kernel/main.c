#include <stdint.h>
#include "memory/memory.h"
#include "cpu/gdt.h"
#include "cpu/idt.h"
#include "memory/paging.h"

#define VGA_MEMORY 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define COM1 0x3F8

static inline uint8_t inb(uint16_t port) {
    uint8_t val;
    __asm__ volatile ("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline void outb_delay(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1; jmp 1f; 1:" : : "a"(val), "Nd"(port));
}

void serial_putchar(char c) {
    while (!(inb(COM1 + 5) & 0x20));
    outb_delay(COM1, c);
}

void serial_print(const char* str) {
    while (*str) {
        if (*str == '\n') serial_putchar('\r');
        serial_putchar(*str++);
    }
}

void serial_print_uint(uint32_t val) {
    char buf[12];
    int i = 0;
    if (val == 0) {
        serial_putchar('0');
        return;
    }
    while (val > 0) {
        buf[i++] = '0' + (val % 10);
        val /= 10;
    }
    while (i--) {
        serial_putchar(buf[i]);
    }
}

void kernel_main(multiboot_info_t* mbd) {
    volatile uint16_t* vga = (volatile uint16_t*)VGA_MEMORY;
    const char* message = "MinOS Loaded";
    uint8_t color = 0x07;

    serial_print("Kernel starting...\n");

    pmm_init(mbd);
    serial_print("PMM initialized\n");

    serial_print("Free frames: ");
    serial_print_uint(pmm_get_free_count());
    serial_print("\n");

    gdt_init();
    idt_init();
    paging_init();

    __asm__ volatile ("sti");

    for (int i = 0; message[i] != '\0'; i++) {
        vga[i] = (color << 8) | message[i];
    }

    serial_print("MinOS Loaded\n");

    void* test_frame = pmm_alloc_frame();
    serial_print("Allocated test frame at: 0x");
    serial_print_uint((uint32_t)test_frame);
    serial_print("\n");

    pmm_free_frame(test_frame);
    serial_print("Freed test frame\n");

    serial_print("Free frames after: ");
    serial_print_uint(pmm_get_free_count());
    serial_print("\n");

    while (1) {
        __asm__ volatile ("hlt");
    }
}
