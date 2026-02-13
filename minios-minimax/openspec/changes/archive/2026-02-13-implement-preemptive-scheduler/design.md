## Context

MiniOS currently runs processes sequentially. When `kernel_main()` starts a process, it runs to completion or `exit()`. There is no timer interrupt, no scheduler, and no context switching. The system cannot recover from an infinite loop in user space.

## Goals / Non-Goals

**Goals:**
- Implement timer-driven preemptive multitasking
- Enable multiple processes to share CPU time fairly
- Allow system to remain responsive even if one process hangs
- Verify implementation with deterministic test program

**Non-Goals:**
- Priority-based scheduling (round-robin only)
- Process blocking/sleep syscalls
- Inter-process communication (IPC)
- Virtual memory for user processes
- Multiprocessor support (SMP)

## Decisions

### 1. Timer Frequency: 100Hz (10ms timeslice)

**Decision**: Configure PIT channel 0 at 1193182 Hz / 65536 = ~18.2Hz base, use divisor 11932 for ~100Hz.

**Rationale**:
- 10ms is standard for educational OS (long enough to be visible, short enough for responsiveness)
- Simple to calculate: divisor = PIT_FREQ / desired_freq = 1193182 / 100 = 11932
- PIT is universally available, no ACPI or HPET dependency

**Alternative**: 60Hz (16.67ms) - also reasonable, but 100Hz is more standard

### 2. Timer Interrupt Vector: IDT[32]

**Decision**: Use vector 32 for timer interrupt (first available IRQ0 mapping).

**Rationale**:
- Intel reserves vectors 0-31 for exceptions
- syscall is at 0x80 (128)
- Vector 32 is the standard location for PIC IRQ0 remapping

### 3. PCB Structure Extension

**Decision**: Add full register context to `pcb_t`:

```c
typedef struct {
    uint32_t id;
    uint32_t state;
    uint32_t entry;
    uint32_t eip;
    char name[32];
    // New fields for preemption:
    uint32_t esp;
    uint32_t ebp;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esi;
    uint32_t edi;
    uint16_t ds;
    uint16_t es;
    uint16_t fs;
    uint16_t gs;
} pcb_t;
```

**Rationale**:
- Complete state needed for context switch
- TSS handles only ESP0 (kernel stack), not user registers
- Simpler than using TSS for full context

### 4. Process States

**Decision**: Extend states to include READY:

```c
#define PROC_READY    0
#define PROC_RUNNING  1
#define PROC_EXITED   2
```

**Rationale**:
- Simple state machine: READY → RUNNING → EXITED
- No BLOCKED state needed (no blocking syscalls yet)

### 5. Context Switch: Assembly Save/Restore

**Decision**: Write inline assembly in C handler for save, assembly function for restore.

**Save in timer_handler (C)**:
```c
__asm__ volatile (
    "pushal\n"
    "push %ds\n" "push %es\n" "push %fs\n" "push %gs\n"
    "mov %esp, %eax\n"
    "mov %eax, %cr3\n"  // Flush TLB if needed
);
```

**Restore via iret** (after loading new process's ESP from PCB).

### 6. Scheduler Algorithm: Simple Round-Robin

**Decision**: Scan `process_table.processes[]` circularly for next READY process.

**Rationale**:
- MAX_PROCESSES = 4 (fixed array)
- Linear scan is O(n) but n=4 is trivial
- No priority complexity

## Risks / Trade-offs

| Risk | Mitigation |
|------|------------|
| Triple fault on context switch | Use `make qemu-debug` to inspect state; verify segment selectors |
| Timer interrupt not firing | Add debug output in handler; verify PIT initialization |
| Context corruption (wrong register saved) | Use seqprint test to verify interleaved output |
| Lost ticks (IRQ not acknowledged) | PIC already initialized by GRUB; verify IRQ0 mask |

## Migration Plan

**Phase 1**: Add PIT and timer interrupt
- Initialize PIT in `interrupts.c`
- Register IDT[32] handler
- Verify handler fires (debug output)

**Phase 2**: Extend PCB and add context save
- Modify `pcb_t` structure
- Add context save in timer handler
- Verify no crashes

**Phase 3**: Implement scheduler and context restore
- Add `scheduler()` function
- Add context restore and iret to new process
- Connect timer → scheduler → switch

**Phase 4**: Test with seqprint
- Create programs/seqprint.c
- Embed in programs.h
- Verify interleaved output

**Rollback**: If system triple faults, `make clean && git checkout` to restore working state.

## Open Questions

1. **Should timer interrupt be edge-triggered or level-triggered?** PIT uses edge-triggered by default. No change needed.

2. **Should we disable interrupts during context switch?** Yes, `cli` before modifying ESP, `sti` after iret.

3. **What happens if all processes are EXITED?** Fall through to idle loop (hlt).
