#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>
#include <stddef.h>

#define NULL ((void*)0)

void serial_putchar(char c);
void serial_print(const char* str);
void serial_print_uint(uint32_t val);

/* VGA driver functions */
void vga_putchar(char c);
void vga_write(const char* str, size_t len);
void vga_clear(void);

#endif
