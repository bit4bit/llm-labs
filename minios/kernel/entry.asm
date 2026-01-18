MULTIBOOT_HEADER_MAGIC     equ 0x1BADB002
MULTIBOOT_HEADER_FLAGS     equ 0x00010003
MULTIBOOT_HEADER_CHECKSUM  equ -0x1BADB003
MULTIBOOT_BOOTLOADER_MAGIC equ 0x2BADB002

section .multiboot
align 4
    dd MULTIBOOT_HEADER_MAGIC
    dd MULTIBOOT_HEADER_FLAGS
    dd MULTIBOOT_HEADER_CHECKSUM

section .text
global _start
extern kernel_main

_start:
    mov eax, MULTIBOOT_BOOTLOADER_MAGIC
    mov ebx, multiboot_info_ptr
    call kernel_main
    jmp hang

hang:
    hlt
    jmp hang

section .data
align 4
multiboot_info_ptr:
    dd 0
