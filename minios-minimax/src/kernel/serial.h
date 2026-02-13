#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>
#include <stddef.h>

#define COM1_PORT 0x3F8

static inline uint8_t inb(uint16_t port) {
    uint8_t val;
    __asm__ volatile ("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

void serial_putchar(char c);
void serial_print(const char* str);
void serial_print_uint(uint32_t val);
void serial_print_hex(uint32_t val);
void vga_putchar(char c);
void vga_write(const char* str, size_t len);
void vga_clear(void);
void debug_print(const char* fmt, ...);

#endif
