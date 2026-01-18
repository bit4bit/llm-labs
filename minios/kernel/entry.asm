; entry.asm - Minimal x86_64 Multiboot2 kernel entry
; Assemble: nasm -f elf64 entry.asm -o entry.o

; ------------------------------
; Multiboot2 Header
; ------------------------------
MULTIBOOT2_HEADER_MAGIC     equ 0xE85250D6
MULTIBOOT2_ARCH_X86_64      equ 3          ; x86_64
MULTIBOOT2_HEADER_LENGTH    equ 12
MULTIBOOT2_CHECKSUM      equ -(MULTIBOOT2_HEADER_MAGIC + MULTIBOOT2_ARCH_X86_64 + MULTIBOOT2_HEADER_LENGTH)

section .multiboot2_header
align 8
multiboot2_header:
    dd MULTIBOOT2_HEADER_MAGIC
    dd MULTIBOOT2_ARCH_X86_64
    dd MULTIBOOT2_HEADER_LENGTH
    dd MULTIBOOT2_CHECKSUM

; ------------------------------
; Code Entry Point
; ------------------------------
section .text
global _start
extern kernel_main

_start:
    ; Multiboot2 registers: eax = magic, ebx = info ptr
    mov rdi, rax        ; optional: pass magic
    mov rsi, rbx        ; optional: pass multiboot info pointer

    call enable_pae
    call setup_page_tables
    call enable_long_mode
    call enable_paging
    call load_gdt64

    ; Far jump to 64-bit code
    mov ax, 0x08         ; code64 selector
    push rax
    lea rax, [rel long_mode_start]
    push rax
    retfq                ; jump to 64-bit mode

hang:
    hlt
    jmp hang

; ------------------------------
; 64-bit long mode code
; ------------------------------
section .text.long_mode
align 16
long_mode_start:
    xor ax, ax
    mov ss, ax
    mov rsp, 0x200000       ; stack in 2MB

    mov rax, 0xDEADBEEF
    mov rbx, 0xCAFEBABE
    mov rcx, 0xBADB00B
    mov rdx, 0xFEEDFACE

    sti                      ; enable interrupts

    call kernel_main          ; jump to C kernel

    jmp hang                  ; hang after return

; ------------------------------
; Paging and Long Mode Setup
; ------------------------------
enable_pae:
    mov rax, cr4
    or rax, 1 << 5
    mov cr4, rax
    ret

setup_page_tables:
    ; Simple identity map (1:1 mapping)
    lea rax, [rel pml4]
    mov cr3, rax
    ret

enable_long_mode:
    mov ecx, 0xC0000080      ; IA32_EFER MSR
    rdmsr
    or rax, 1 << 8           ; set LME
    wrmsr
    ret

enable_paging:
    mov rax, cr0
    or rax, 1 << 31          ; enable paging
    mov cr0, rax
    ret

load_gdt64:
    lgdt [rel gdt64_pointer]
    ret

; ------------------------------
; GDT (64-bit)
; ------------------------------
section .rodata
align 8
gdt64:
    dq 0                      ; NULL descriptor
    dq 0x00AF9A000000FFFF      ; code segment (64-bit)
    dq 0x00AF92000000FFFF      ; data segment (64-bit)
gdt64_end:

gdt64_pointer:
    dw gdt64_end - gdt64 - 1   ; limit
    dq gdt64                    ; base

; ------------------------------
; Page Tables (identity mapping first 2MB)
; ------------------------------
section .bss
align 4096
pml4:
    resb 4096
pdpt:
    resb 4096
pd:
    resb 4096
