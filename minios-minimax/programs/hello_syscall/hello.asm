; Hello World program using sys_write syscall
; This program demonstrates the write syscall to print to VGA display
; Loaded at 0x40000000

[BITS 32]

_start:
    ; sys_write(1, msg, len)
    ; eax = 3 (syscall number for write)
    ; ebx = 1 (stdout)
    ; ecx = pointer to message
    ; edx = length of message

    mov eax, 3              ; syscall number: write
    mov ebx, 1              ; file descriptor: stdout

    ; Calculate message address relative to program base
    ; Program is at 0x40000000, message is at offset 35 (0x23)
    mov ecx, 0x40000023     ; pointer to message (absolute address)
    mov edx, 20             ; length of message (20 bytes)
    int 0x80                ; invoke syscall

    ; sys_exit(0)
    ; eax = 1 (syscall number for exit)
    ; ebx = 0 (exit code)

    mov eax, 1              ; syscall number: exit
    mov ebx, 0              ; exit code: success
    int 0x80                ; invoke syscall

    ; Should never reach here
    hlt

; Message data (starts at byte 35)
msg:     db 'Hello from syscall!', 0x0A
