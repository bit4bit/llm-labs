#include "vga.h"
#include "multiboot2.h"

#define MULTIBOOT2_BOOTLOADER_MAGIC 0x36D76289

static char digit_to_hex(uint8_t d) {
    if (d < 10) return '0' + d;
    return 'A' + (d - 10);
}

static void write_hex(uint64_t val, unsigned char color, int width) {
    char buf[16];
    int i;
    for (i = 0; i < width; i++) {
        buf[width - 1 - i] = digit_to_hex(val & 0xF);
        val >>= 4;
    }
    vga_write(buf, &color, width);
}

static void write_dec(uint32_t val, unsigned char color) {
    char buf[12];
    int i = 0;
    if (val == 0) {
        vga_write("0", &color, 1);
        return;
    }
    while (val > 0) {
        buf[i++] = '0' + (val % 10);
        val /= 10;
    }
    while (i > 0) {
        vga_write(&buf[--i], &color, 1);
    }
}

static void write_string(const char *str, unsigned char color) {
    int len = 0;
    while (str[len]) len++;
    vga_write(str, &color, len);
}

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

    write_string("Multiboot2 OK!", VGA_COLOR_LIGHT_GREEN);
    vga_putchar('\n', VGA_COLOR_LIGHT_GREEN);

    multiboot2_memory_info_t mem_info;
    multiboot2_parse_memory(&mem_info, mb_info);

    write_string("Memory: ", VGA_COLOR_LIGHT_CYAN);
    write_dec(mem_info.free_memory / 1024, VGA_COLOR_LIGHT_CYAN);
    write_string("KB lower, ", VGA_COLOR_LIGHT_CYAN);
    write_dec((mem_info.total_memory - mem_info.free_memory) / 1024, VGA_COLOR_LIGHT_CYAN);
    write_string("KB upper", VGA_COLOR_LIGHT_CYAN);
    vga_putchar('\n', VGA_COLOR_LIGHT_CYAN);

    write_string("Total: ", VGA_COLOR_LIGHT_CYAN);
    write_dec(mem_info.total_memory / 1024, VGA_COLOR_LIGHT_CYAN);
    write_string(" KB", VGA_COLOR_LIGHT_CYAN);
    vga_putchar('\n', VGA_COLOR_LIGHT_CYAN);

    write_string("Magic: 0x", VGA_COLOR_LIGHT_BLUE);
    write_hex(magic, VGA_COLOR_LIGHT_BLUE, 8);
    vga_putchar('\n', VGA_COLOR_LIGHT_BLUE);

    write_string("MB info: 0x", VGA_COLOR_LIGHT_BLUE);
    write_hex((uint64_t)mb_info, VGA_COLOR_LIGHT_BLUE, 16);
    vga_putchar('\n', VGA_COLOR_LIGHT_BLUE);

    multiboot2_tag_mmap_t mmap_info;
    multiboot2_parse_mmap(&mmap_info, mb_info);

    if (mmap_info.type == MULTIBOOT2_TAG_TYPE_MMAP) {
        write_string("MMAP: entry_size=", VGA_COLOR_LIGHT_MAGENTA);
        write_dec(mmap_info.entry_size, VGA_COLOR_LIGHT_MAGENTA);
        write_string(", entries=", VGA_COLOR_LIGHT_MAGENTA);
        uint32_t entry_count = multiboot2_get_mmap_entry_count(mb_info);
        write_dec(entry_count, VGA_COLOR_LIGHT_MAGENTA);
        vga_putchar('\n', VGA_COLOR_LIGHT_MAGENTA);
    }

    while (1) {
        __asm__ volatile ("hlt");
    }
}
