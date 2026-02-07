.text
.global syscall_entry

syscall_entry:
    # Save all registers first
    pushal
    push %ds
    push %es
    push %fs
    push %gs

    # Set up kernel data segments
    mov $0x10, %edi
    mov %edi, %ds
    mov %edi, %es

    # Push syscall arguments onto stack (right-to-left for cdecl)
    # Stack layout after pushal + 4 segment pushes:
    # ESP+0:  gs (just pushed)
    # ESP+4:  fs
    # ESP+8:  es
    # ESP+12: ds
    # ESP+16: EDI (pushal starts here)
    # ESP+20: ESI
    # ESP+24: EBP
    # ESP+28: ESP (original)
    # ESP+32: EBX
    # ESP+36: EDX
    # ESP+40: ECX
    # ESP+44: EAX

    # Get saved registers from pushal area (add 16 for segment pushes)
    mov 44(%esp), %eax  # Original EAX
    mov 32(%esp), %ebx  # Original EBX
    mov 40(%esp), %ecx  # Original ECX
    mov 36(%esp), %edx  # Original EDX

    # Push in reverse order for cdecl calling convention
    push %edx           # arg4: edx
    push %ecx           # arg3: ecx
    push %ebx           # arg2: ebx
    push %eax           # arg1: eax (syscall number)

    # Call the C syscall handler
    call syscall_handler

    # Clean up arguments (4 * 4 = 16 bytes)
    add $16, %esp

    # Store return value back to saved EAX on stack
    # After cleaning args, EAX is at offset 44 again
    mov %eax, 44(%esp)

    # Restore segment registers
    pop %gs
    pop %fs
    pop %es
    pop %ds

    # Restore general purpose registers
    popal

    # Return to user mode
    iret
