#include "vga.h"
#include "multiboot2.h"

#define MULTIBOOT2_BOOTLOADER_MAGIC 0x36D76289

void kernel_main(unsigned int magic, void *mb_info) {
    vga_clear();

    if (magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
        const char *err = "ERROR: Not Multiboot2!";
        unsigned char colors[20];
        for (int i = 0; i < 20; i++) {
            colors[i] = VGA_COLOR_RED;
        }
        vga_write(err, colors, 18);
        while (1) {
            __asm__ volatile ("hlt");
        }
    }

    const char *msg = "Multiboot2 OK!";
    unsigned char colors[14] = {
        VGA_COLOR_LIGHT_GREEN,
        VGA_COLOR_LIGHT_GREEN,
        VGA_COLOR_LIGHT_GREEN,
        VGA_COLOR_LIGHT_GREEN,
        VGA_COLOR_LIGHT_GREEN,
        VGA_COLOR_LIGHT_GREEN,
        VGA_COLOR_LIGHT_GREEN,
        VGA_COLOR_LIGHT_GREEN,
        VGA_COLOR_LIGHT_GREEN,
        VGA_COLOR_LIGHT_GREEN,
        VGA_COLOR_LIGHT_GREEN,
        VGA_COLOR_LIGHT_GREEN,
        VGA_COLOR_LIGHT_GREEN,
        VGA_COLOR_LIGHT_GREEN
    };

    vga_write(msg, colors, 14);

    while (1) {
        __asm__ volatile ("hlt");
    }
}
