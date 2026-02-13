#include <stdarg.h>
#include <stdint.h>
#include "debug.h"
#include "../serial.h"

#ifdef ENABLE_DEBUG_PRINT
static void debug_putc(char c) {
    serial_putchar(c);
}

static void debug_puts(const char *str) {
    while (*str) {
        debug_putc(*str++);
    }
}

static void debug_put_hex(uint32_t num) {
    const char hex_chars[] = "0123456789ABCDEF";
    int i;
    int started = 0;
    
    if (num == 0) {
        debug_putc('0');
        return;
    }
    
    for (i = 28; i >= 0; i -= 4) {
        uint8_t nibble = (num >> i) & 0xF;
        if (nibble != 0 || started || i == 0) {
            started = 1;
            debug_putc(hex_chars[nibble]);
        }
    }
}

static void debug_put_uint(uint32_t num) {
    char buffer[12];
    int i = 0;
    int j;
    
    if (num == 0) {
        debug_putc('0');
        return;
    }
    
    while (num > 0) {
        buffer[i++] = '0' + (num % 10);
        num /= 10;
    }
    
    for (j = i - 1; j >= 0; j--) {
        debug_putc(buffer[j]);
    }
}

static void debug_vprintf(const char *fmt, va_list args) {
    const char *p;
    for (p = fmt; *p; p++) {
        if (*p == '%' && *(p + 1)) {
            p++;
            switch (*p) {
                case 's': {
                    char *str = va_arg(args, char*);
                    debug_puts(str ? str : "(null)");
                    break;
                }
                case 'd':
                case 'u': {
                    uint32_t num = va_arg(args, uint32_t);
                    debug_put_uint(num);
                    break;
                }
                case 'x':
                case 'X': {
                    uint32_t num = va_arg(args, uint32_t);
                    debug_put_hex(num);
                    break;
                }
                case 'c': {
                    debug_putc((char)va_arg(args, int));
                    break;
                }
                case '%':
                    debug_putc('%');
                    break;
                default:
                    debug_putc('%');
                    debug_putc(*p);
                    break;
            }
        } else {
            debug_putc(*p);
        }
    }
}

void debug_printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    debug_vprintf(fmt, args);
    va_end(args);
}
#else
void debug_printf(const char *fmt, ...) {
    // Debug printing disabled
    (void)fmt; // Prevent unused parameter warning
}
#endif