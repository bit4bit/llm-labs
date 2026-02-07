.text
.global enter_user_mode

# enter_user_mode(uint32_t entry_point, uint32_t user_stack)
# This function switches from kernel mode (CPL=0) to user mode (CPL=3)
# by setting up the stack for an iret and executing it.

enter_user_mode:
    cli                         # Disable interrupts during transition

    mov 4(%esp), %eax           # eax = entry_point (first argument)
    mov 8(%esp), %ebx           # ebx = user_stack (second argument)

    # Set up user data segments (selector 0x20 | RPL=3 = 0x23)
    mov $0x23, %cx
    mov %cx, %ds
    mov %cx, %es
    mov %cx, %fs
    mov %cx, %gs

    # Build iret stack frame for user mode transition
    # Stack layout (bottom to top):
    #   SS (user data selector with RPL=3)
    #   ESP (user stack pointer)
    #   EFLAGS (with IF=1 to enable interrupts in user mode)
    #   CS (user code selector with RPL=3)
    #   EIP (entry point)

    pushl $0x23                 # SS: user data selector (0x20 | RPL=3)
    pushl %ebx                  # ESP: user stack pointer

    # Push EFLAGS without IF flag (keep interrupts disabled)
    pushf
    pop %ecx
    and $0xFFFFFDFF, %ecx       # Clear IF flag (keep interrupts disabled)
    pushl %ecx                  # EFLAGS

    pushl $0x1B                 # CS: user code selector (0x18 | RPL=3)
    pushl %eax                  # EIP: entry point

    # Execute iret to jump to user mode
    # This will:
    # 1. Pop EIP, CS, EFLAGS, ESP, SS
    # 2. Switch to CPL=3 (user mode)
    # 3. Enable interrupts
    # 4. Start executing at entry_point with user_stack
    iret
