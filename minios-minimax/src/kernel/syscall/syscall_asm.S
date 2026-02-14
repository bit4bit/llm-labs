.text
.global syscall_entry

syscall_entry:
    # Save all registers (same frame layout as timer_handler_asm)
    pushal
    push %ds
    push %es
    push %fs
    push %gs

    # Set up kernel data segments
    mov $0x10, %edi
    mov %edi, %ds
    mov %edi, %es

    # Get syscall arguments from saved registers on stack
    # Stack layout after pushes: GS,FS,ES,DS, EDI,ESI,EBP,ESP,EBX,EDX,ECX,EAX
    #   GS  = 0(%esp)
    #   FS  = 4(%esp)
    #   ES  = 8(%esp)
    #   DS  = 12(%esp)
    #   EDI = 16(%esp)  [pushal: first pushed]
    #   ESI = 20(%esp)
    #   EBP = 24(%esp)
    #   ESP = 28(%esp)  [original, ignored by popal]
    #   EBX = 32(%esp)
    #   EDX = 36(%esp)
    #   ECX = 40(%esp)
    #   EAX = 44(%esp)  [syscall number]

    mov 44(%esp), %eax  # Original EAX = syscall number
    mov 32(%esp), %ebx  # Original EBX = arg1
    mov 40(%esp), %ecx  # Original ECX = arg2
    mov 36(%esp), %edx  # Original EDX = arg3

    # Push arguments in reverse order for cdecl calling convention
    push %edx           # arg4: edx
    push %ecx           # arg3: ecx
    push %ebx           # arg2: ebx
    push %eax           # arg1: eax (syscall number)

    # Call the C syscall handler
    # For exit: handler marks process EXITED and calls scheduler(),
    # which does scheduler_switch() to another process and never returns here.
    # For other syscalls: handler returns normally.
    call syscall_handler

    # Clean up arguments (4 * 4 = 16 bytes)
    add $16, %esp

    # Store return value back to saved EAX on stack
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
