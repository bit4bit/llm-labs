## MODIFIED Requirements

### Requirement: Process has lifecycle states
The system SHALL represent each process with one of three states: READY, RUNNING, or EXITED.

#### Scenario: New process is in READY state
- **WHEN** `process_create(name, entry_addr)` is called
- **THEN** the PCB state SHALL be set to `PROC_READY`

#### Scenario: Process transitions to RUNNING when scheduled
- **WHEN** the scheduler selects a process to run
- **THEN** the PCB state SHALL be set to `PROC_RUNNING`

#### Scenario: Process transitions to EXITED on sys_exit
- **WHEN** a running process executes `sys_exit()`
- **THEN** the PCB state SHALL be set to `PROC_EXITED`

### Requirement: Process creation initializes PCB
The system SHALL create a PCB with process name, entry address, kernel stack, user stack, and a prepared fake interrupt frame for first scheduling.

#### Scenario: PCB contains process name
- **WHEN** `process_create("hello", 0x40000000)` is called
- **THEN** the PCB name field SHALL contain the string "hello"

#### Scenario: PCB contains entry address
- **WHEN** `process_create("hello", 0x40000000)` is called
- **THEN** the PCB entry field SHALL equal 0x40000000

#### Scenario: PCB has kernel stack assigned
- **WHEN** `process_create()` is called for process at index `i`
- **THEN** the PCB `kernel_stack_top` field SHALL point to the top of `kernel_stacks[i]`
- **AND** the PCB `kernel_esp` field SHALL point to the bottom of the fake interrupt frame prepared on that kernel stack

#### Scenario: PCB has user stack assigned
- **WHEN** `process_create()` is called for process at index `i`
- **THEN** the PCB `user_stack` field SHALL be set to the initial user stack pointer for that process's stack region

#### Scenario: Fake interrupt frame prepared
- **WHEN** `process_create()` is called
- **THEN** the process's kernel stack SHALL contain a synthetic interrupt frame with: SS=0x23, ESP=user_stack, EFLAGS=0x202, CS=0x1B, EIP=entry_addr, zeroed general-purpose registers, and segment registers DS=ES=FS=GS=0x23

### Requirement: Process start transitions to user mode
The system SHALL start the first process by invoking the scheduler, which uses the fake interrupt frame to transition to user mode. The `process_start` function SHALL set `current_process` and invoke the scheduler mechanism.

#### Scenario: First process starts via scheduler
- **WHEN** `process_start(pcb)` is called
- **THEN** the system SHALL set `current_process` to `pcb`
- **AND** invoke the context restore mechanism that pops the fake interrupt frame and executes `iret` to enter user mode at `pcb->entry`

#### Scenario: User mode CS selector is used
- **WHEN** the first process starts
- **THEN** the CPU SHALL load code segment selector 0x1B (CPL=3)

#### Scenario: Stack pointer is set to user stack
- **WHEN** the first process starts
- **THEN** the CPU ESP SHALL point to the process's `user_stack` address

### Requirement: Syscall exit returns control to kernel
The system SHALL handle exit syscall by marking the process as exited and invoking the scheduler to switch to the next process.

#### Scenario: Exit syscall number is 1
- **WHEN** a user process executes `int 0x80` with eax=1
- **THEN** the syscall handler SHALL invoke `sys_exit()`

#### Scenario: Exit code is passed in ebx
- **WHEN** a user process executes `int 0x80` with eax=1 and ebx=0
- **THEN** the syscall handler SHALL save 0 as the exit code

#### Scenario: Exited process is not rescheduled
- **WHEN** a process calls `sys_exit()`
- **THEN** its state SHALL be set to `PROC_EXITED`
- **AND** the scheduler SHALL select the next PROC_READY process
- **AND** the exited process SHALL NOT be selected again

## REMOVED Requirements

### Requirement: Process start transitions to user mode
**Reason**: The original requirement specified `process_start` directly calling `enter_user_mode()` and that control returns to the instruction following `process_start` after exit. With the fake interrupt frame approach, `process_start` uses the scheduler mechanism and the exit path goes through the scheduler rather than returning to the caller.
**Migration**: Use `process_start(pcb)` which now sets up `current_process` and triggers the scheduler-based context restore. Exit returns control via the scheduler, not to the `process_start` call site.
