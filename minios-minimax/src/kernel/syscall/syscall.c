#include "syscall.h"
#include "../process/process.h"
#include "../kernel.h"
#include "../debug.h"

#define SYSCALL_EXIT 1
#define SYSCALL_WRITE 3

#define USER_PROGRAM_BASE 0x40000000
#define USER_PROGRAM_SIZE 0x00100000

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
        case SYSCALL_EXIT:
            DEBUG_SYSCALL("exit called with code %u", ebx);

            {
                pcb_t* pcb = process_get_current();
                process_mark_exited(pcb);
                if (pcb) {
                    DEBUG_SYSCALL("Process %s marked as exited with code %d", pcb->name, ebx);
                }
            }

            DEBUG_SYSCALL("Returning to kernel...");
            return SYSCALL_EXIT;

        case SYSCALL_WRITE:
            DEBUG_SYSCALL("write called");
            result = sys_write((int)ebx, (const char*)ecx, (size_t)edx);
            DEBUG_SYSCALL("write returned %d", result);
            break;
            
        default:
            DEBUG_SYSCALL("unknown syscall number %u", eax);
            result = -1;
            break;
    }
    
    return result;
}
