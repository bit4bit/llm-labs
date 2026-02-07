#include <stdint.h>
#include "kernel.h"
#include "minios.h"
#include "memory/memory.h"
#include "cpu/gdt.h"
#include "cpu/idt.h"
#include "cpu/tss.h"
#include "memory/paging.h"
#include "process/process.h"
#include "syscall/syscall.h"

// Include user programs registry
#include "programs.h"

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
    
    // Skip leading zeros
    i = 0;
    while (i < 8 && buf[i] == '0') {
        i++;
    }
    
    // Print remaining digits
    while (i < 8) {
        serial_putchar(buf[i++]);
    }
}

/* VGA driver state */
static uint16_t cursor_x = 0;  // Column (0-79)
static uint16_t cursor_y = 0;  // Row (0-24)
static volatile uint16_t* vga_buffer = (volatile uint16_t*)0xB8000;
static const uint8_t vga_color = 0x07;  // Light gray on black
static const uint16_t VGA_WIDTH = 80;
static const uint16_t VGA_HEIGHT = 25;

/* VGA driver functions */
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
    // Move all lines up by one
    for (uint16_t y = 0; y < VGA_HEIGHT - 1; y++) {
        for (uint16_t x = 0; x < VGA_WIDTH; x++) {
            const uint16_t src_index = (y + 1) * VGA_WIDTH + x;
            const uint16_t dst_index = y * VGA_WIDTH + x;
            vga_buffer[dst_index] = vga_buffer[src_index];
        }
    }
    
    // Clear the last line
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
    
    // Regular character
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

void kernel_main(multiboot_info_t* mbd) {
    volatile uint16_t* vga = (volatile uint16_t*)VGA_MEMORY;
    const char* message = "MinOS Loaded";
    uint8_t color = 0x07;

    serial_print("Kernel starting...\n");

    pmm_init(mbd);
    pmm_test();
    serial_print("PMM initialized\n");

    serial_print("Free frames: ");
    serial_print_uint(pmm_get_free_count());
    serial_print("\n");

    gdt_init();
    serial_print("GDT initialized\n");

    idt_init();
    serial_print("IDT initialized\n");

    paging_init();
    paging_test();
    paging_extended_test();

    for (int i = 0; message[i] != '\0'; i++) {
        vga[i] = (color << 8) | message[i];
    }

    serial_print("MinOS Loaded\n");

    void* test_frame = pmm_alloc_frame();
    serial_print("Allocated test frame at: 0x");
    serial_print_hex((uint32_t)test_frame);
    serial_print("\n");

    pmm_free_frame(test_frame);
    serial_print("Freed test frame\n");

    serial_print("Free frames after: ");
    serial_print_uint(pmm_get_free_count());
    serial_print("\n");

    /* Process management initialization */
    tss_init();
    process_init();

    /* Test write/read at USER_PROGRAM_BASE before copying */
    serial_print("Testing memory at user program base...\n");
    volatile uint32_t* test_ptr = (volatile uint32_t*)USER_PROGRAM_BASE;
    
    serial_print("Test: Reading initial value...\n");
    uint32_t initial_val = *test_ptr;
    serial_print("Test: Initial value = 0x");
    serial_print_hex(initial_val);
    serial_print("\n");
    
    serial_print("Test: Writing 0xDEADBEEF...\n");
    *test_ptr = 0xDEADBEEF;
    
    serial_print("Test: Reading back...\n");
    uint32_t read_val = *test_ptr;
    serial_print("Test: Read value = 0x");
    serial_print_hex(read_val);
    serial_print("\n");
    
    if (read_val == 0xDEADBEEF) {
        serial_print("Test: Write/read OK!\n");
    } else {
        serial_print("Test: FAILED - memory not working!\n");
    }
    
    /* Create hello process */
    pcb_t* hello = process_create("hello", USER_PROGRAM_BASE);
    if (hello == NULL) {
        serial_print("Error: Could not create hello process\n");
        while (1) __asm__ volatile ("hlt");
    }

    /* Load C program binary into user space */
    if (process_load(hello, hello_bin, hello_bin_size) != 0) {
        serial_print("Error: Could not load hello program\n");
        while (1) __asm__ volatile ("hlt");
    }

    /* Start hello process in user mode */
    serial_print("About to call process_start...\n");
    process_start(hello);

    /* This should never be reached if process_start works correctly */
    serial_print("Error: process_start returned unexpectedly\n");
    while (1) {
        __asm__ volatile ("hlt");
    }
}
