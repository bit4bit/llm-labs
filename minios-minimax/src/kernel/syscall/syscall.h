#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>
#include <stddef.h>
#include "../serial.h"

#define SYSCALL_EXIT 1
#define SYSCALL_WRITE 3

int syscall_handler(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx);
int sys_write(int fd, const char* buf, size_t count);

#endif