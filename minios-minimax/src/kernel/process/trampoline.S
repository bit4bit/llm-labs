.text
.global enter_user_mode
.global scheduler_switch
.global trampoline_to_user

# PCB struct offsets (must match process.h)
.equ PCB_KERNEL_ESP,       44
.equ PCB_KERNEL_STACK_TOP, 48

# enter_user_mode(uint32_t entry_point, uint32_t user_stack)
# Switches from kernel mode (CPL=0) to user mode (CPL=3).
# Used only as a fallback; new processes use fake interrupt frames.

enter_user_mode:
    cli

    mov 4(%esp), %eax           # eax = entry_point
    mov 8(%esp), %ebx           # ebx = user_stack

    mov $0x23, %cx
    mov %cx, %ds
    mov %cx, %es
    mov %cx, %fs
    mov %cx, %gs

    pushl $0x23                 # SS
    pushl %ebx                  # ESP
    pushf
    pop %ecx
    or $0x200, %ecx
    pushl %ecx                  # EFLAGS (IF=1)
    pushl $0x1B                 # CS
    pushl %eax                  # EIP
    iret

# trampoline_to_user
#
# Entry point for newly created processes. scheduler_switch loads the
# new process's kernel_esp and does `ret`, which pops this address.
# We then pop the fake interrupt frame and iret to user mode.
#
# Stack at this point:
#   GS, FS, ES, DS, [pushal frame], [iret frame]

trampoline_to_user:
    pop %gs
    pop %fs
    pop %es
    pop %ds
    popal
    iret

# scheduler_switch(pcb_t* prev, pcb_t* next)
#
# Context switch between two processes by swapping kernel stack pointers.
# prev may be NULL (first switch from kernel idle loop).
#
# For resumed processes, `ret` returns to wherever the process was
# suspended (inside timer_handler call chain, which eventually
# returns to timer_handler_asm's pop+iret).
#
# For new processes, `ret` pops the trampoline_to_user address
# pushed during fake frame setup, which does pop segs + popal + iret.

scheduler_switch:
    mov 4(%esp), %eax           # eax = prev
    mov 8(%esp), %edx           # edx = next

    # If prev != NULL, save current ESP
    test %eax, %eax
    jz .Lload_next
    mov %esp, PCB_KERNEL_ESP(%eax)

.Lload_next:
    # Load next process's kernel stack pointer
    mov PCB_KERNEL_ESP(%edx), %esp

    # Update TSS.esp0 to next->kernel_stack_top
    mov PCB_KERNEL_STACK_TOP(%edx), %eax
    mov %eax, (tss_entry + 4)   # tss_entry.esp0 at offset 4 in struct

    ret
