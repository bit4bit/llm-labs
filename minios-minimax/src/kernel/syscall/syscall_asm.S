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

    # Get syscall number (eax) from original registers (saved in pushal area)
    mov 44(%esp), %eax  # Original EAX = syscall number
    
    # Check if this is the exit syscall BEFORE calling the handler
    cmpl $1, %eax       # SYSCALL_EXIT = 1
    je syscall_exit_to_kernel

    # For non-exit syscalls, continue with normal handler call
    # Get remaining syscall arguments from original registers
    mov 32(%esp), %ebx  # Original EBX
    mov 40(%esp), %ecx  # Original ECX
    mov 36(%esp), %edx  # Original EDX

    # Push arguments in reverse order for cdecl calling convention
    push %edx           # arg4: edx
    push %ecx           # arg3: ecx
    push %ebx           # arg2: ebx
    push %eax           # arg1: eax (syscall number)

    # Call the C syscall handler
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

syscall_exit_to_kernel:
    # For exit syscall, we don't call syscall_handler at all
    # Instead we extract the exit code and go directly to kernel mode
    # Get exit code from original EBX register
    mov 32(%esp), %ebx  # Original EBX = exit code
    
    # Disable interrupts
    cli

    # Pop saved segment registers
    pop %gs
    pop %fs
    pop %es
    pop %ds
    
    # Pop general purpose registers except EAX (save exit code for later)
    pop %edi
    pop %esi
    pop %ebp
    pop %edx            # Note ESP is skipped but we're about to do iret anyway
    pop %ebx
    pop %ecx
    # eax is already in ebx (exit code)
    
    # Build IRET frame to kernel mode:
    # SS (0x10), ESP (kernel), EFLAGS, CS (0x08), EIP (process_exit_return)
    push $0x10              # SS (kernel data segment)
    push $0xBFFFF000        # ESP (kernel stack)
    pushf                   # EFLAGS
    pop %eax
    and $0xFFFFFDFF, %eax   # Clear IF bit (disable interrupts in kernel mode)
    push %eax               # EFLAGS with IF cleared
    push $0x08              # CS (kernel code segment)
    push $process_exit_return  # EIP (where to go in kernel)
    
    # Return to kernel mode
    iret