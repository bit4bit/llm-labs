#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include "minios.h"
#include "serial.h"

#ifndef NULL
#define NULL ((void*)0)
#endif

static inline uint8_t inb(uint16_t port) {
    uint8_t val;
    __asm__ volatile ("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

void serial_putchar(char c) {
    while (!(inb(COM1_PORT + 5) & 0x20));
    outb(COM1_PORT, c);
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

void serial_print_hex(uint32_t val) {
    const char hex_chars[] = "0123456789ABCDEF";
    char buf[9];
    int i = 7;
    
    buf[8] = '\0';
    
    if (val == 0) {
        serial_putchar('0');
        return;
    }
    
    while (i >= 0) {
        buf[i] = hex_chars[val & 0xF];
        val >>= 4;
        i--;
    }
    
    i = 0;
    while (i < 8 && buf[i] == '0') {
        i++;
    }
    
    while (i < 8) {
        serial_putchar(buf[i++]);
    }
}

static uint16_t cursor_x = 0;
static uint16_t cursor_y = 0;
static volatile uint16_t* vga_buffer = (volatile uint16_t*)0xB8000;
static const uint8_t vga_color = 0x07;
static const uint16_t VGA_WIDTH = 80;
static const uint16_t VGA_HEIGHT = 25;

void vga_clear(void) {
    for (uint16_t y = 0; y < VGA_HEIGHT; y++) {
        for (uint16_t x = 0; x < VGA_WIDTH; x++) {
            const uint16_t index = y * VGA_WIDTH + x;
            vga_buffer[index] = (vga_color << 8) | ' ';
        }
    }
    cursor_x = 0;
    cursor_y = 0;
}

static void vga_scroll(void) {
    for (uint16_t y = 0; y < VGA_HEIGHT - 1; y++) {
        for (uint16_t x = 0; x < VGA_WIDTH; x++) {
            const uint16_t src_index = (y + 1) * VGA_WIDTH + x;
            const uint16_t dst_index = y * VGA_WIDTH + x;
            vga_buffer[dst_index] = vga_buffer[src_index];
        }
    }
    
    for (uint16_t x = 0; x < VGA_WIDTH; x++) {
        const uint16_t index = (VGA_HEIGHT - 1) * VGA_WIDTH + x;
        vga_buffer[index] = (vga_color << 8) | ' ';
    }
    
    cursor_y = VGA_HEIGHT - 1;
}

void vga_putchar(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
        if (cursor_y >= VGA_HEIGHT) {
            vga_scroll();
        }
        return;
    }
    
    if (c == '\r') {
        cursor_x = 0;
        return;
    }
    
    if (c == '\t') {
        cursor_x = (cursor_x + 4) & ~(4 - 1);
        if (cursor_x >= VGA_WIDTH) {
            cursor_x = 0;
            cursor_y++;
            if (cursor_y >= VGA_HEIGHT) {
                vga_scroll();
            }
        }
        return;
    }
    
    const uint16_t index = cursor_y * VGA_WIDTH + cursor_x;
    vga_buffer[index] = (vga_color << 8) | c;
    
    cursor_x++;
    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        cursor_y++;
        if (cursor_y >= VGA_HEIGHT) {
            vga_scroll();
        }
    }
}

void vga_write(const char* str, size_t len) {
    for (size_t i = 0; i < len; i++) {
        vga_putchar(str[i]);
    }
}

void debug_print(const char* fmt, ...) {
    const char* p = fmt;
    uint32_t val;
    char* str;
    char c;
    void* ptr;

    va_list args;
    va_start(args, fmt);

    while (*p) {
        if (*p == '%' && *(p + 1)) {
            p++;
            switch (*p) {
                case 'c':
                    c = va_arg(args, int);
                    serial_putchar(c);
                    break;
                case 's':
                    str = va_arg(args, char*);
                    if (str == NULL) str = "(null)";
                    serial_print(str);
                    break;
                case 'u':
                    val = va_arg(args, uint32_t);
                    serial_print_uint(val);
                    break;
                case 'd':
                    val = va_arg(args, int);
                    if ((int32_t)val < 0) {
                        serial_putchar('-');
                        val = (uint32_t)(-(int32_t)val);
                    }
                    serial_print_uint(val);
                    break;
                case 'X':
                case 'x':
                    val = va_arg(args, uint32_t);
                    serial_print_hex(val);
                    break;
                case 'p':
                    ptr = va_arg(args, void*);
                    serial_putchar('0');
                    serial_putchar('x');
                    val = (uint32_t)ptr;
                    serial_print_hex(val);
                    break;
                case '%':
                    serial_putchar('%');
                    break;
                default:
                    serial_putchar('%');
                    serial_putchar(*p);
                    break;
            }
        } else {
            serial_putchar(*p);
        }
        p++;
    }

    va_end(args);
}
