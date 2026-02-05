#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>

#define NULL ((void*)0)

void serial_putchar(char c);
void serial_print(const char* str);
void serial_print_uint(uint32_t val);

#endif
