#include "syscall.h"
#include "../process/process.h"
#include "../kernel.h"
#include "../minios.h"
#include "../debug.h"
#include "../cpu/interrupts.h"

#define SYSCALL_EXIT 1
#define SYSCALL_WRITE 3
#define SYSCALL_GETPID 100
#define SYSCALL_GET_TICK_COUNT 101

static int validate_user_pointer(const void* ptr, size_t len) {
    uint32_t addr = (uint32_t)ptr;

    if (ptr == NULL) {
        return 0;
    }

    if (addr + len < addr) {
        return 0;
    }

    /* Reject pointers in kernel space */
    if (addr >= KERNEL_VIRTUAL_BASE || addr + len > KERNEL_VIRTUAL_BASE) {
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

    for (int i = 0; i < count; i++) {
      serial_putchar(buf[i]);
    }

    DEBUG_SYSCALL("output=\"%.*s\"", count, buf);

    return (int)count;
}

extern void scheduler(void);

int syscall_handler(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx) {
    int32_t result = 0;

    switch (eax) {
        case SYSCALL_EXIT:
            {
                pcb_t* pcb = process_get_current();
                if (pcb) {
                    DEBUG_SYSCALL("exit called by %s with code %u", pcb->name, ebx);
                    pcb->state = PROC_EXITED;
                }
                scheduler();
                return 0;
            }

        case SYSCALL_WRITE:
            result = sys_write((int)ebx, (const char*)ecx, (size_t)edx);
            break;

        case SYSCALL_GETPID:
            {
                pcb_t* pcb = process_get_current();
                if (pcb) {
                    result = pcb->id;
                } else {
                    result = 0;
                }
            }
            break;

        case SYSCALL_GET_TICK_COUNT:
            result = pit_get_ticks();
            break;

        default:
            result = -1;
            break;
    }

    return result;
}
