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

uint8_t hello_bin[] = {
    // mov eax, 0xB8000 (VGA text memory address)
    0xB8, 0x00, 0x80, 0x0B, 0x00,
    // mov es, eax (can't directly, so we'll skip this for now)
    // Let's write directly to VGA memory at 0xB8000
    // mov edi, 0xB8000 (destination)
    0xBF, 0x00, 0x80, 0x0B, 0x00,
    // mov esi, msg (source - we'll put message right after code)
    0xBE, 0x22, 0x00, 0x00, 0x40,  // 0x40000022 (after this code at byte 34)
    // mov ecx, 22 (11 characters * 2 bytes each)
    0xB9, 0x16, 0x00, 0x00, 0x00,
    // rep movsb (copy string to VGA)
    0xF3, 0xA4,
    // mov eax, 1 (syscall exit)
    0xB8, 0x01, 0x00, 0x00, 0x00,
    // mov ebx, 0 (exit code)
    0xBB, 0x00, 0x00, 0x00, 0x00,
    // int 0x80 (syscall)
    0xCD, 0x80,
    // Message data: "hola mundo" with attributes
    'h', 0x07, 'o', 0x07, 'l', 0x07, 'a', 0x07,
    ' ', 0x07, 'm', 0x07, 'u', 0x07, 'n', 0x07,
    'd', 0x07, 'o', 0x07, '!', 0x07
};

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
    
    /* Copy hello_bin to user space at USER_PROGRAM_BASE */
    serial_print("Copying hello_bin (");
    serial_print_uint(sizeof(hello_bin));
    serial_print(" bytes) to 0x");
    serial_print_hex(USER_PROGRAM_BASE);
    serial_print("...\n");
    
    uint8_t* dest = (uint8_t*)USER_PROGRAM_BASE;
    for (uint32_t i = 0; i < sizeof(hello_bin); i++) {
        dest[i] = hello_bin[i];
    }
    
    serial_print("hello_bin copied successfully\n");
    
    /* Verify first few bytes */
    serial_print("Verifying: first 4 bytes at 0x");
    serial_print_hex(USER_PROGRAM_BASE);
    serial_print(" = 0x");
    serial_print_hex(dest[0]);
    serial_print(" 0x");
    serial_print_hex(dest[1]);
    serial_print(" 0x");
    serial_print_hex(dest[2]);
    serial_print(" 0x");
    serial_print_hex(dest[3]);
    serial_print("\n");

    /* Create hello process */
    pcb_t* hello = process_create("hello", USER_PROGRAM_BASE);
    if (hello == NULL) {
        serial_print("Error: Could not create hello process\n");
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
