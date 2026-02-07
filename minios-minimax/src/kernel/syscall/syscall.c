#include "syscall.h"
#include "../process/process.h"
#include "../kernel.h"

#define SYSCALL_EXIT 1
#define SYSCALL_WRITE 3

#define USER_PROGRAM_BASE 0x40000000
#define USER_PROGRAM_SIZE 0x00100000  // 1MB

extern void syscall_return_to_kernel(void);
extern void serial_print_hex(uint32_t val);

/* Validate that a user pointer is within valid user space */
static int validate_user_pointer(const void* ptr, size_t len) {
    uint32_t addr = (uint32_t)ptr;
    
    // Check for NULL
    if (ptr == NULL) {
        return 0;
    }
    
    // Check for overflow
    if (addr + len < addr) {
        return 0;
    }
    
    // Check that pointer is in user space range
    if (addr < USER_PROGRAM_BASE || addr >= (USER_PROGRAM_BASE + USER_PROGRAM_SIZE)) {
        return 0;
    }
    
    // Check that the entire buffer is in user space
    if ((addr + len) > (USER_PROGRAM_BASE + USER_PROGRAM_SIZE)) {
        return 0;
    }
    
    return 1;
}

/* sys_write - Write to a file descriptor
 * fd: file descriptor (1=stdout, 2=stderr)
 * buf: buffer to write from
 * count: number of bytes to write
 * Returns: number of bytes written, or -1 on error
 */
int sys_write(int fd, const char* buf, size_t count) {
    // Validate file descriptor (only stdout and stderr for now)
    if (fd != 1 && fd != 2) {
        serial_print("sys_write: invalid fd ");
        serial_print_uint(fd);
        serial_print("\n");
        return -1;
    }
    
    // Validate buffer pointer
    if (!validate_user_pointer(buf, count)) {
        serial_print("sys_write: invalid buffer pointer 0x");
        serial_print_hex((uint32_t)buf);
        serial_print(" with count ");
        serial_print_uint(count);
        serial_print("\n");
        return -1;
    }
    
    // Handle zero-length write
    if (count == 0) {
        return 0;
    }
    
    serial_print("sys_write: fd=");
    serial_print_uint(fd);
    serial_print(" buf=0x");
    serial_print_hex((uint32_t)buf);
    serial_print(" count=");
    serial_print_uint(count);
    serial_print("\n");
    
    // Write to VGA display
    vga_write(buf, count);
    
    // Also write to serial for debugging
    serial_print("sys_write: output=\"");
    for (size_t i = 0; i < count; i++) {
        char c = buf[i];
        if (c >= 32 && c < 127) {
            serial_putchar(c);
        } else if (c == '\n') {
            serial_putchar(c);
        } else {
            serial_print("\\x");
            serial_print_hex(c);
        }
    }
    serial_print("\"\n");
    
    return (int)count;
}

int syscall_handler(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx) {
    int32_t result = 0;

    serial_print("Syscall: ");
    serial_print_uint(eax);
    serial_print(" (ebx=");
    serial_print_uint(ebx);
    serial_print(", ecx=0x");
    serial_print_hex(ecx);
    serial_print(", edx=");
    serial_print_uint(edx);
    serial_print(")\n");

    switch (eax) {
        case SYSCALL_WRITE:
            serial_print("Syscall: write called\n");
            result = sys_write((int)ebx, (const char*)ecx, (size_t)edx);
            serial_print("Syscall: write returned ");
            serial_print_uint(result);
            serial_print("\n");
            break;
            
        case SYSCALL_EXIT:
            serial_print("Syscall: exit called with code ");
            serial_print_uint(ebx);
            serial_print("\n");
            
            // Show VGA memory content to verify output was written
            serial_print("VGA Memory (first line as ASCII):\n  \"");
            volatile uint16_t* vga = (volatile uint16_t*)0xB8000;
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
            
            serial_print("VGA Memory (bytes 0-21 as hex):\n  ");
            for (int i = 0; i < 11; i++) {
                uint16_t val = vga[i];
                uint8_t ch = val & 0xFF;
                serial_print("0x");
                serial_print_hex(ch);
                serial_print(" ");
            }
            serial_print("\n");
            
            serial_print("Halting...\n");
            while (1) {
                __asm__ volatile ("hlt");
            }
            break;
            
        default:
            serial_print("Syscall: unknown syscall number ");
            serial_print_uint(eax);
            serial_print("\n");
            result = -1;
            break;
    }
    
    // Return value will be stored in EAX by assembly wrapper
    return result;
}