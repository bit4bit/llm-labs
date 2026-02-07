#include "syscall.h"
#include "../process/process.h"
#include "../kernel.h"
#include "../debug.h"

#define SYSCALL_EXIT 1
#define SYSCALL_WRITE 3

#define USER_PROGRAM_BASE 0x40000000
#define USER_PROGRAM_SIZE 0x00100000

extern void syscall_return_to_kernel(void);

static int validate_user_pointer(const void* ptr, size_t len) {
    uint32_t addr = (uint32_t)ptr;
    
    if (ptr == NULL) {
        return 0;
    }
    
    if (addr + len < addr) {
        return 0;
    }
    
    if (addr < USER_PROGRAM_BASE || addr >= (USER_PROGRAM_BASE + USER_PROGRAM_SIZE)) {
        return 0;
    }
    
    if ((addr + len) > (USER_PROGRAM_BASE + USER_PROGRAM_SIZE)) {
        return 0;
    }
    
    return 1;
}

int sys_write(int fd, const char* buf, size_t count) {
    if (fd != 1 && fd != 2) {
        DEBUG_SYSCALL("invalid fd %d", fd);
        return -1;
    }
    
    if (!validate_user_pointer(buf, count)) {
        DEBUG_SYSCALL("invalid buffer pointer 0x%X with count %u", (uint32_t)buf, count);
        return -1;
    }
    
    if (count == 0) {
        return 0;
    }
    
    DEBUG_SYSCALL("fd=%u buf=0x%X count=%u", fd, (uint32_t)buf, count);

    vga_write(buf, count);
    
    DEBUG_SYSCALL("output=\"%.*s\"", count, buf);
    
    return (int)count;
}

int syscall_handler(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx) {
    int32_t result = 0;

    DEBUG_SYSCALL("%u (ebx=%u, ecx=0x%X, edx=%u)", eax, ebx, ecx, edx);

    switch (eax) {
        case SYSCALL_WRITE:
            DEBUG_SYSCALL("write called");
            result = sys_write((int)ebx, (const char*)ecx, (size_t)edx);
            DEBUG_SYSCALL("write returned %d", result);
            break;
            
        case SYSCALL_EXIT:
            DEBUG_SYSCALL("exit called with code %u", ebx);

            DEBUG_SYSCALL("VGA Memory (first line as ASCII):");
            volatile uint16_t* vga = (volatile uint16_t*)0xB8000;
            serial_print("  \"");
            for (int i = 0; i < 40; i++) {
                uint16_t val = vga[i];
                uint8_t ch = val & 0xFF;
                if (ch >= 32 && ch < 127) {
                    serial_putchar(ch);
                } else if (ch == 0) {
                    serial_print(" ");
                } else {
                    serial_print("?");
                }
            }
            serial_print("\"\n");
            
            DEBUG_SYSCALL("VGA Memory (bytes 0-21 as hex):");
            serial_print("  ");
            for (int i = 0; i < 11; i++) {
                uint16_t val = vga[i];
                uint8_t ch = val & 0xFF;
                serial_print("0x");
                serial_print_hex(ch);
                serial_print(" ");
            }
            serial_print("\n");
            
            DEBUG_SYSCALL("Halting...");
            while (1) {
                __asm__ volatile ("hlt");
            }
            break;
            
        default:
            DEBUG_SYSCALL("unknown syscall number %u", eax);
            result = -1;
            break;
    }
    
    return result;
}
