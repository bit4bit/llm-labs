; MiniOS Kernel Entry Point
; Simple 32-bit entry point

section .text
bits 32

global kernel_entry
extern kernel_main

kernel_entry:
    ; Set up stack
    mov esp, stack_top
    
    ; Zero registers
    xor ebp, ebp
    xor edi, edi
    xor esi, esi
    xor edx, edx
    xor ecx, ecx
    xor ebx, ebx
    xor eax, eax
    
    ; Call kernel main
    call kernel_main
    
    ; Halt if kernel returns
.hang:
    cli
    hlt
    jmp .hang

section .bss
align 16
stack_bottom:
    resb 16384          ; 16KiB stack
stack_top: