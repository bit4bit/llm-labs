#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>
#include <stddef.h>

// VGA text buffer
extern volatile uint16_t* vga_buffer;
extern uint8_t cursor_x;
extern uint8_t cursor_y;
extern uint8_t color;

// Terminal functions
void terminal_init();
void terminal_putchar(char c);
void terminal_write(const char* data, size_t size);
void terminal_writestring(const char* data);

// Utility functions
size_t strlen(const char* str);

#endif