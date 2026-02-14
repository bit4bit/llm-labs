## MODIFIED Requirements

### Requirement: Context save on timer interrupt
On each timer interrupt, the kernel SHALL save the current process's CPU state by storing the kernel stack pointer in the PCB. The kernel stack already contains the complete interrupt frame (segment registers pushed by the handler, general-purpose registers via pushal, and the CPU's automatic iret frame), so individual register copying is not needed.

#### Scenario: Context saved before scheduling
- **WHEN** timer interrupt occurs while a process is running
- **THEN** the kernel SHALL store the current kernel ESP into the running process's `kernel_esp` field in its PCB

#### Scenario: Current process marked ready
- **WHEN** context is saved
- **THEN** the running process's state is set to PROC_READY

### Requirement: Extended process control block
Each process SHALL store its kernel stack pointer and kernel stack top address in its PCB for context switching. Individual register fields are not needed because the full CPU state lives on the kernel stack.

#### Scenario: PCB contains kernel stack context
- **WHEN** a process is created
- **THEN** its PCB SHALL contain fields for: `kernel_esp` (saved stack pointer) and `kernel_stack_top` (top of the allocated kernel stack)

#### Scenario: PCB state transitions
- **WHEN** a process is created
- **THEN** its state is set to PROC_READY
- **WHEN** process starts executing
- **THEN** its state is set to PROC_RUNNING
- **WHEN** process calls exit
- **THEN** its state is set to PROC_EXITED

### Requirement: Context restore and switch
The kernel SHALL restore the next process's state by loading its saved kernel stack pointer and executing the interrupt return sequence (pop segment registers, popal, iret).

#### Scenario: Context restored before iret
- **WHEN** scheduler selects a different process
- **THEN** the kernel SHALL call `scheduler_switch(prev, next)` which loads ESP from `next->kernel_esp`
- **AND** the subsequent pop gs/fs/es/ds, popal, iret in `timer_handler_asm` SHALL restore all registers and return to user mode

#### Scenario: TSS kernel stack set
- **WHEN** switching to a new process
- **THEN** the TSS esp0 field SHALL be set to the new process's `kernel_stack_top`

### Requirement: Round-robin scheduler
The kernel SHALL implement a round-robin scheduler that selects the next READY process.

#### Scenario: Scheduler finds next ready process
- **WHEN** scheduler is invoked
- **THEN** it SHALL scan the process table circularly from the current position
- **AND** select the first process with state PROC_READY

#### Scenario: Scheduler updates current process
- **WHEN** a new process is selected
- **THEN** the selected process's state is set to PROC_RUNNING
- **AND** current_process pointer is updated to the selected process

#### Scenario: All processes exited
- **WHEN** scheduler is invoked and all processes have state PROC_EXITED
- **THEN** the kernel SHALL set `all_processes_exited = 1` and return without switching

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
- **THEN** all general-purpose registers (eax, ecx, edx, ebx, esi, edi, ebp) SHALL be pushed to the kernel stack via pushal
- **AND** segment registers (ds, es, fs, gs) SHALL be pushed to the kernel stack
- **AND** the handler SHALL NOT copy registers individually to the PCB
