.text
.global syscall_entry

syscall_entry:
    push %ds
    push %es
    push %fs
    push %gs
    pushal

    mov $0x10, %eax
    mov %eax, %ds
    mov %eax, %es

    call syscall_handler

    popal
    pop %gs
    pop %fs
    pop %es
    pop %ds

    iret
