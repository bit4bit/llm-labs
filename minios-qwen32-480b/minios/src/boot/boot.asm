; MiniOS Bootloader
; Compatible with GRUB Multiboot specification

MBOOT_PAGE_ALIGN    equ 1<<0
MBOOT_MEM_INFO      equ 1<<1
MBOOT_GRAPHICS      equ 1<<2
MBOOT_FLAGS         equ MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO
MBOOT_CHECKSUM      equ -(0x1BADB002 + MBOOT_FLAGS)

section .multiboot
align 4
    dd 0x1BADB002       ; Multiboot magic number
    dd MBOOT_FLAGS      ; Flags
    dd MBOOT_CHECKSUM   ; Checksum

section .bss
align 16
stack_bottom:
    resb 16384          ; 16KiB stack
stack_top:

section .text
global _start
_start:
    ; Disable interrupts
    cli
    
    ; Set up stack
    mov esp, stack_top
    
    ; Ensure data segment registers are set
    mov ebp, 0
    mov edi, 0
    mov esi, 0
    mov edx, 0
    mov ecx, 0
    mov eax, 0
    
    ; Call kernel entry point
    extern kernel_entry
    call kernel_entry
    
    ; Halt if kernel returns
.hang:
    hlt
    jmp .hang

; Global Descriptor Table (GDT) will be set up in kernel
; Paging will be set up in kernel