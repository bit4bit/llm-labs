#include <stddef.h>
#include "vga.h"

static volatile vga_cell_t *vga = (volatile vga_cell_t *)VGA_MEMORY;
static size_t vga_cursor = 0;

void vga_putchar(char c, unsigned char color) {
    if (c == '\n') {
        vga_cursor = (vga_cursor / 80 + 1) * 80;
        return;
    }
    vga[vga_cursor].c = c;
    vga[vga_cursor].attr = color;
    vga_cursor++;
}

void vga_write(const char *str, const unsigned char *colors, size_t len) {
    for (size_t i = 0; i < len && str[i] != '\0'; i++) {
        vga_putchar(str[i], colors[i]);
    }
}

void vga_clear(void) {
    for (size_t i = 0; i < 80 * 25; i++) {
        vga[i].c = ' ';
        vga[i].attr = VGA_COLOR_BLACK;
    }
    vga_cursor = 0;
}
