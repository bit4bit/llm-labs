; Hello World program with multiple sys_write calls
; This program demonstrates multiple syscall invocations and newline handling
; Loaded at 0x40000000

[BITS 32]

_start:
    ; Write first message: "Line 1: Hello"
    mov eax, 3              ; syscall number: write
    mov ebx, 1              ; file descriptor: stdout
    mov ecx, 0x40000060     ; pointer to message 1
    mov edx, 14             ; length of message 1
    int 0x80                ; invoke syscall

    ; Write second message: "Line 2: from"
    mov eax, 3              ; syscall number: write
    mov ebx, 1              ; file descriptor: stdout
    mov ecx, 0x4000006E     ; pointer to message 2
    mov edx, 13             ; length of message 2
    int 0x80                ; invoke syscall

    ; Write third message: "Line 3: syscall!"
    mov eax, 3              ; syscall number: write
    mov ebx, 1              ; file descriptor: stdout
    mov ecx, 0x4000007B     ; pointer to message 3
    mov edx, 16             ; length of message 3
    int 0x80                ; invoke syscall

    ; Write fourth message: "Testing multiple writes..."
    mov eax, 3              ; syscall number: write
    mov ebx, 1              ; file descriptor: stdout
    mov ecx, 0x4000008B     ; pointer to message 4
    mov edx, 27             ; length of message 4
    int 0x80                ; invoke syscall

    ; Write fifth message: "All done!"
    mov eax, 3              ; syscall number: write
    mov ebx, 1              ; file descriptor: stdout
    mov ecx, 0x400000A6     ; pointer to message 5
    mov edx, 10             ; length of message 5
    int 0x80                ; invoke syscall

    ; sys_exit(0)
    mov eax, 1              ; syscall number: exit
    mov ebx, 0              ; exit code: success
    int 0x80                ; invoke syscall

    ; Should never reach here
    hlt

; Message data (starts at byte 0x60 = 96)
; Padding to reach offset 0x60
times 96-($-$$) db 0

msg1:     db 'Line 1: Hello', 0x0A     ; offset 0x60
msg2:     db 'Line 2: from', 0x0A      ; offset 0x6E
msg3:     db 'Line 3: syscall!', 0x0A  ; offset 0x7B
msg4:     db 'Testing multiple writes...', 0x0A  ; offset 0x8B
msg5:     db 'All done!', 0x0A         ; offset 0xA6
