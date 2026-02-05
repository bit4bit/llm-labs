.globl _start
.global kernel_main

.section .bss
.align 16
stack:
    .skip 4096

.section .text
_start:
    mov $stack, %esp
    call kernel_main
hang:
    hlt
    jmp hang
