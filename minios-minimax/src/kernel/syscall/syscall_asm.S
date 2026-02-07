.text
.global syscall_entry

syscall_entry:
    # Save all segment registers and general purpose registers
    push %ds
    push %es
    push %fs
    push %gs
    pushal

    # Set up kernel data segments (use ECX to avoid corrupting EAX)
    mov $0x10, %ecx
    mov %ecx, %ds
    mov %ecx, %es

    # Call the C syscall handler
    # EAX and EBX are preserved on the stack from pushal
    call syscall_handler

    # Restore all registers
    popal
    pop %gs
    pop %fs
    pop %es
    pop %ds

    # Return to user mode
    iret
