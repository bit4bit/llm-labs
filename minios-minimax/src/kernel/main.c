#include <stdint.h>

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

void kernel_main(void) {
    volatile uint16_t* vga = (volatile uint16_t*)VGA_MEMORY;
    const char* message = "MinOS Loaded";
    uint8_t color = 0x07;

    serial_print("Kernel starting...\n");

    for (int i = 0; message[i] != '\0'; i++) {
        vga[i] = (color << 8) | message[i];
    }

    serial_print("MinOS Loaded\n");

    while (1) {
        __asm__ volatile ("hlt");
    }
}
