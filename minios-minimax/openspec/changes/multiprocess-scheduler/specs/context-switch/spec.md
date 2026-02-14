## ADDED Requirements

### Requirement: Context switch saves process state via stack pointer
The kernel SHALL save a process's complete CPU state by storing the kernel stack pointer (ESP) in the process's PCB. The kernel stack already contains the full interrupt frame (segment registers, general-purpose registers via pushal, and the CPU's iret frame) so no individual register copying is needed.

#### Scenario: ESP saved on preemption
- **WHEN** the timer interrupt fires while process A is running
- **AND** the scheduler selects a different process B
- **THEN** the kernel SHALL store the current ESP value into process A's `kernel_esp` field in its PCB

#### Scenario: Saved ESP points to complete state
- **WHEN** ESP is saved to the PCB
- **THEN** the kernel stack at that ESP SHALL contain (from top to bottom): GS, FS, ES, DS, EDI, ESI, EBP, ESP_ignored, EBX, EDX, ECX, EAX, EIP, CS, EFLAGS, user_ESP, user_SS

### Requirement: Context switch restores process state via stack pointer
The kernel SHALL restore a process's complete CPU state by loading the saved kernel stack pointer (ESP) from the process's PCB. The subsequent `pop segments + popal + iret` restores all registers and returns to user mode.

#### Scenario: ESP restored on switch-in
- **WHEN** the scheduler selects process B to run
- **THEN** the kernel SHALL load process B's `kernel_esp` from its PCB into the CPU's ESP register

#### Scenario: Registers restored from stack
- **WHEN** ESP is restored and the interrupt return sequence executes (pop gs/fs/es/ds, popal, iret)
- **THEN** all general-purpose registers, segment registers, EFLAGS, EIP, and user stack pointer SHALL be restored to the values process B had when it was last preempted

### Requirement: scheduler_switch performs the stack swap
The kernel SHALL implement `scheduler_switch` as an assembly function that atomically swaps kernel stacks between two processes.

#### Scenario: scheduler_switch saves old and loads new ESP
- **WHEN** `scheduler_switch(prev, next)` is called
- **THEN** the function SHALL store ESP into `prev->kernel_esp`
- **AND** load ESP from `next->kernel_esp`

#### Scenario: scheduler_switch updates TSS
- **WHEN** `scheduler_switch(prev, next)` is called
- **THEN** the function SHALL set `TSS.esp0` to `next->kernel_stack_top` so the CPU uses the correct kernel stack on the next privilege transition

#### Scenario: scheduler_switch returns to restored context
- **WHEN** `scheduler_switch` loads the new ESP and returns
- **THEN** execution SHALL continue at the point where the `next` process was previously suspended (inside `timer_handler_asm`), which will execute `pop segments + popal + iret` to return to user mode

### Requirement: New processes start via fake interrupt frame
The kernel SHALL prepare new processes with a synthetic interrupt frame on their kernel stack so they can be started through the same `scheduler_switch` mechanism as resumed processes.

#### Scenario: Fake frame has correct iret fields
- **WHEN** a new process is created
- **THEN** its kernel stack SHALL contain an iret frame with: SS=0x23, ESP=process's user stack address, EFLAGS=0x202 (IF=1), CS=0x1B, EIP=process's entry point

#### Scenario: Fake frame has zeroed general registers
- **WHEN** a new process is created
- **THEN** its kernel stack SHALL contain a pushal frame with all general-purpose registers set to 0

#### Scenario: Fake frame has user segment registers
- **WHEN** a new process is created
- **THEN** its kernel stack SHALL contain segment register values DS=ES=FS=GS=0x23

#### Scenario: kernel_esp points to top of fake frame
- **WHEN** a new process is created
- **THEN** the PCB's `kernel_esp` field SHALL point to the lowest address of the fake frame (the GS value), so that `pop gs + pop fs + pop es + pop ds + popal + iret` restores the entire frame

#### Scenario: No special case for first run
- **WHEN** the scheduler selects a never-run process
- **THEN** the scheduler SHALL use the same `scheduler_switch` code path as for a previously-suspended process
- **AND** the `enter_user_mode` function SHALL NOT be called by the scheduler

### Requirement: Same process does not switch
The kernel SHALL skip the context switch when the scheduler selects the same process that is currently running.

#### Scenario: No switch when only one ready process
- **WHEN** the scheduler determines the next process is the same as the current process
- **THEN** `scheduler_switch` SHALL NOT be called
- **AND** the timer handler SHALL return normally via `pop + popal + iret`
