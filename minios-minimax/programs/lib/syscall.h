#ifndef SYSCALL_H
#define SYSCALL_H

#include "stdint.h"

/* Syscall numbers - must match kernel definitions */
#define SYS_EXIT  1
#define SYS_WRITE 3

/* 
 * exit - terminate the current process
 * @code: exit status code
 */
static inline void exit(int code) {
    __asm__ volatile(
        "int $0x80"
        :
        : "a"(SYS_EXIT), "b"(code)
    );
    while(1); /* Should never reach here */
}

/* 
 * write - write data to a file descriptor
 * @fd: file descriptor (1 = stdout)
 * @buf: pointer to data buffer
 * @len: number of bytes to write
 * Returns: number of bytes written, or -1 on error
 */
static inline int write(int fd, const char* buf, uint32_t len) {
    int ret;
    __asm__ volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(SYS_WRITE), "b"(fd), "c"(buf), "d"(len)
    );
    return ret;
}

#endif /* SYSCALL_H */