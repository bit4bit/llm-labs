## Why

The current MiniOS implementation lacks preemptive multitasking. Processes run sequentially to completion or until they call `exit()`. An infinite loop in a user program hangs the entire system. Adding a timer-driven scheduler enables true concurrent execution, allowing multiple processes to share CPU time fairly.

## What Changes

- **PIT Initialization**: Configure Programmable Interval Timer at ~100Hz (10ms timeslice)
- **Timer Interrupt**: Register IRQ0 → IDT vector 32 for timer ticks
- **Extended PCB**: Add register context (esp, ebp, eax, ecx, edx, ebx, esi, edi, segment registers) for context switching
- **Context Switch**: Assembly functions to save/restore complete process state
- **Round-Robin Scheduler**: Select next READY process and switch to it
- **Test Program**: `seqprint.c` that prints "PID:i" for i=1..5 to verify preemption

## Capabilities

### New Capabilities
- `preemptive-scheduler`: Timer-driven round-robin CPU scheduling enabling true preemptive multitasking

### Modified Capabilities
- (none)

## Impact

- `src/kernel/cpu/interrupts.c`: Add PIT initialization and timer interrupt handler
- `src/kernel/process/process.h`: Extend `pcb_t` with full register context
- `src/kernel/process/process.c`: Add scheduler function and process state management
- `src/kernel/process/trampoline.S`: Add context save/restore assembly
- `programs/src/seqprint.c`: New test program for verification
- `src/kernel/programs.h`: Embed seqprint binary
