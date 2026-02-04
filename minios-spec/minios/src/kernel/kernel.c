/*
 * MiniOS Kernel Main
 */

#include "../include/kernel.h"

// VGA text buffer
volatile uint16_t* vga_buffer = (uint16_t*)0xB8000;
uint8_t cursor_x = 0;
uint8_t cursor_y = 0;
uint8_t color = 0x07; // Light gray on black

// Kernel main function
void kernel_main() {
    terminal_init();
    terminal_writestring("MiniOS Kernel Loaded Successfully!\n");
    
    // Infinite loop to prevent kernel from returning
    for (;;);
}

// Initialize terminal
void terminal_init() {
    // Clear screen
    for (int y = 0; y < 25; y++) {
        for (int x = 0; x < 80; x++) {
            const int index = y * 80 + x;
            vga_buffer[index] = (color << 8) | ' ';
        }
    }
    cursor_x = 0;
    cursor_y = 0;
}

// Put character on terminal
void terminal_putchar(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\r') {
        cursor_x = 0;
    } else {
        const int index = cursor_y * 80 + cursor_x;
        vga_buffer[index] = (color << 8) | c;
        cursor_x++;
    }

    if (cursor_x >= 80) {
        cursor_x = 0;
        cursor_y++;
    }

    if (cursor_y >= 25) {
        // Scroll screen
        for (int y = 0; y < 24; y++) {
            for (int x = 0; x < 80; x++) {
                vga_buffer[y * 80 + x] = vga_buffer[(y + 1) * 80 + x];
            }
        }
        // Clear last line
        for (int x = 0; x < 80; x++) {
            vga_buffer[24 * 80 + x] = (color << 8) | ' ';
        }
        cursor_y = 24;
    }
}

// Write data to terminal
void terminal_write(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++)
        terminal_putchar(data[i]);
}

// Write null-terminated string to terminal
void terminal_writestring(const char* data) {
    terminal_write(data, strlen(data));
}

// Simple strlen implementation
size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len])
        len++;
    return len;
}