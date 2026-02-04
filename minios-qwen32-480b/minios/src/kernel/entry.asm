; MiniOS Kernel Entry Point
; Handles transition to 64-bit mode

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
    
    ; Check if we should enable 64-bit mode
    ; For now, we'll implement this as a proof of concept
    ; In a real implementation, this would be controlled by a configuration option
    
    ; Set up 64-bit mode (simplified)
    call setup_64bit_mode
    
    ; Far jump to 64-bit code segment
    jmp 0x08:long_mode_start

setup_64bit_mode:
    ; This is a simplified implementation for demonstration
    ; In a full implementation, we would:
    ; 1. Set up proper page tables
    ; 2. Enable PAE
    ; 3. Enable long mode
    ; 4. Enable paging
    
    ; For now, we'll just return and stay in 32-bit mode
    ret

bits 64
long_mode_start:
    ; Load 64-bit GDT
    lgdt [gdt64.pointer]
    
    ; Setup segment selectors
    mov ax, 0x10
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Set up stack in higher memory area
    mov rsp, 0x200000  ; 2MB stack space
    
    ; Call kernel main function (would need to be 64-bit compatible)
    ; For now, we'll just halt
    hlt

section .data
align 16
gdt64:
    dq 0 ; Null descriptor
.code: equ $ - gdt64
    dw 0xFFFF    ; Limit
    dw 0         ; Base (low)
    db 0         ; Base (middle)
    db 10011010b ; Access byte (code, present, ring 0)
    db 10101111b ; Flags + Limit (high 4 bits)
    db 0         ; Base (high)
.data: equ $ - gdt64
    dw 0xFFFF    ; Limit
    dw 0         ; Base (low)
    db 0         ; Base (middle)
    db 10010010b ; Access byte (data, present, ring 0)
    db 10101111b ; Flags + Limit (high 4 bits)
    db 0         ; Base (high)
.pointer:
    dw $ - gdt64 - 1
    dq gdt64

section .bss
align 16
stack_bottom:
    resb 16384          ; 16KiB stack
stack_top: