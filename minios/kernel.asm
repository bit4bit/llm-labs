section .text
global _start

align 8
multiboot_header:
    dd 0x1BADB002
    dd 0
    dd -(0x1BADB002)

_start:
    mov esp, stack_top
    extern kernel_main
    call kernel_main

hang:
    jmp hang

section .bss
align 16
stack_bottom:
    resb 16384
stack_top:
