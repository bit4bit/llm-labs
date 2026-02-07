.text
.global enter_user_mode

enter_user_mode:
    cli
    mov 4(%esp), %eax
    mov 8(%esp), %ebx

    mov $0x23, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss

    mov %ebx, %esp

    push $0x23
    push %ebx
    pushf
    push $0x1B
    push %eax

    iret
