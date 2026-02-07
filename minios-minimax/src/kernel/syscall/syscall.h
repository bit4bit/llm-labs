#ifndef SYSCALL_H
#define SYSCALL_H

#define SYSCALL_EXIT 1

void syscall_handler(void);
void sys_exit(int code);

#endif
