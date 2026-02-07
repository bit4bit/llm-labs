#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>
#include <stddef.h>

#define COM1_PORT 0x3F8

void serial_putchar(char c);
void serial_print(const char* str);
void serial_print_uint(uint32_t val);
void serial_print_hex(uint32_t val);
void vga_putchar(char c);
void vga_write(const char* str, size_t len);
void vga_clear(void);
void debug_print(const char* fmt, ...);

#endif
