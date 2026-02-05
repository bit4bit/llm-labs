.globl _start
.global kernel_main

.section .bss
.align 16
stack:
    .skip 4096

.section .text
_start:
    mov $stack, %esp
    mov %ebx, %eax
    push %eax
    call kernel_main
hang:
    hlt
    jmp hang
