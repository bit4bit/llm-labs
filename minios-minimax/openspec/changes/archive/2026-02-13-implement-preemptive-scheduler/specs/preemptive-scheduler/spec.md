## ADDED Requirements

### Requirement: PIT initialization
The kernel SHALL configure the Programmable Interval Timer to fire at approximately 100Hz (10ms timeslice).

#### Scenario: PIT configured correctly
- **WHEN** the kernel initializes
- **THEN** PIT channel 0 is configured with divisor 11932 (1193182Hz / 11932 ≈ 100Hz)

#### Scenario: Timer interrupt enabled
- **WHEN** PIT initialization completes
- **THEN** IRQ0 timer interrupts are unmasked and will fire at configured frequency

### Requirement: Timer interrupt handler
The kernel SHALL register an interrupt handler at IDT vector 32 that responds to timer interrupts.

#### Scenario: Handler registered
- **WHEN** IDT is initialized
- **THEN** entry 32 points to the timer interrupt handler function

#### Scenario: Handler executes on timer tick
- **WHEN** PIT fires IRQ0
- **THEN** the timer interrupt handler executes and increments a tick counter

#### Scenario: Handler preserves all registers
- **WHEN** timer interrupt occurs
- **THEN** all general-purpose registers (eax, ecx, edx, ebx, esi, edi, ebp) are pushed to the stack
- **AND** segment registers (ds, es, fs, gs) are pushed to the stack

### Requirement: Extended process control block
Each process SHALL store complete CPU state in its PCB for context switching.

#### Scenario: PCB contains register context
- **WHEN** a process is created
- **THEN** its PCB SHALL contain fields for: esp, ebp, eax, ecx, edx, ebx, esi, edi, ds, es, fs, gs

#### Scenario: PCB state transitions
- **WHEN** a process is created
- **THEN** its state is set to PROC_READY
- **WHEN** process starts executing
- **THEN** its state is set to PROC_RUNNING
- **WHEN** process calls exit
- **THEN** its state is set to PROC_EXITED

### Requirement: Context save on timer interrupt
On each timer interrupt, the kernel SHALL save the current process's CPU state to its PCB.

#### Scenario: Context saved before scheduling
- **WHEN** timer interrupt occurs while a process is running
- **THEN** the kernel SHALL save the current ESP to the running process's PCB
- **AND** the kernel SHALL save all registers (eax, ecx, edx, ebx, esi, edi, ebp) to the running process's PCB
- **AND** the kernel SHALL save segment registers (ds, es, fs, gs) to the running process's PCB

#### Scenario: Current process marked ready
- **WHEN** context is saved
- **THEN** the running process's state is set to PROC_READY

### Requirement: Round-robin scheduler
The kernel SHALL implement a round-robin scheduler that selects the next READY process.

#### Scenario: Scheduler finds next ready process
- **WHEN** scheduler is invoked
- **THEN** it SHALL scan the process table circularly from the current position
- **AND** select the first process with state PROC_READY
- **AND** if no READY process exists, the kernel SHALL halt at idle loop

#### Scenario: Scheduler updates current process
- **WHEN** a new process is selected
- **THEN** the selected process's state is set to PROC_RUNNING
- **AND** current_process pointer is updated to the selected process
- **AND** the process's register context is restored from its PCB

### Requirement: Context restore and switch
The kernel SHALL restore the next process's state and return to it via iret.

#### Scenario: Context restored before iret
- **WHEN** scheduler selects a process
- **THEN** the kernel SHALL restore all registers (eax, ecx, edx, ebx, esi, edi, ebp) from the selected process's PCB
- **AND** the kernel SHALL restore segment registers (ds, es, fs, gs) from the selected process's PCB
- **AND** the kernel SHALL restore ESP from the selected process's PCB
- **AND** the kernel SHALL return to user mode via iret instruction

#### Scenario: TSS kernel stack set
- **WHEN** switching to a new process
- **THEN** the TSS esp0 field SHALL be set to the new process's kernel stack

### Requirement: Preemptive multitasking test
The system SHALL include a test program that verifies preemption is working correctly.

#### Scenario: Sequential number test
- **WHEN** three instances of seqprint run concurrently
- **THEN** each process SHALL print its PID followed by numbers 1 through 5
- **AND** output SHALL show interleaved PID:number pairs
- **AND** output SHALL demonstrate alternating execution (not all of one PID then all of another)

#### Scenario: All processes complete
- **WHEN** three seqprint instances run
- **THEN** all three processes SHALL complete execution
- **AND** all 15 PID:number pairs SHALL be printed
