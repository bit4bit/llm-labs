## ADDED Requirements

### Requirement: Process has unique identifier
The system SHALL assign each process a unique positive integer identifier (PID) starting from 1.

#### Scenario: First process gets PID 1
- **WHEN** `process_create("hello", 0x40000000)` is called
- **THEN** the returned PCB SHALL have `pid` equal to 1

#### Scenario: Second process gets PID 2
- **WHEN** `process_create("shell", 0x40010000)` is called after first process
- **THEN** the returned PCB SHALL have `pid` equal to 2

### Requirement: Process has lifecycle states
The system SHALL represent each process with one of two states: RUNNING or EXITED.

#### Scenario: New process is in RUNNING state
- **WHEN** `process_create("hello", addr)` is called
- **THEN** the PCB state SHALL be set to `PROC_RUNNING`

#### Scenario: Process transitions to EXITED on sys_exit
- **WHEN** a running process executes `sys_exit()`
- **THEN** the PCB state SHALL be set to `PROC_EXITED`

#### Scenario: Process eip is saved on exit
- **WHEN** a running process executes `sys_exit()`
- **THEN** the PCB `eip` field SHALL contain the instruction pointer where the process exited

### Requirement: Process table tracks all processes
The system SHALL maintain a process table with capacity for at least 4 processes.

#### Scenario: Table starts empty
- **WHEN** `process_init()` is called during kernel boot
- **THEN** the process table count SHALL be 0

#### Scenario: Table increments on process creation
- **WHEN** `process_create()` is called
- **THEN** the process table count SHALL increase by 1

#### Scenario: Table prevents exceeding maximum
- **WHEN** `process_create()` is called with table at capacity (4)
- **THEN** the function SHALL return NULL

### Requirement: Process creation initializes PCB
The system SHALL create a PCB with process name, entry address, and initialized context.

#### Scenario: PCB contains process name
- **WHEN** `process_create("hello", 0x40000000)` is called
- **THEN** the PCB name field SHALL contain the string "hello"

#### Scenario: PCB contains entry address
- **WHEN** `process_create("hello", 0x40000000)` is called
- **THEN** the PCB entry field SHALL equal 0x40000000

#### Scenario: PCB eip is initially 0
- **WHEN** `process_create("hello", addr)` is called
- **THEN** the PCB eip field SHALL be initialized to 0

### Requirement: Process start transitions to user mode
The system SHALL switch from kernel mode (ring 0) to user mode (ring 3) and begin executing at the process entry address.

#### Scenario: User mode CS selector is used
- **WHEN** `process_start(pcb)` is called
- **THEN** the CPU SHALL load code segment selector with CPL=3

#### Scenario: Stack pointer is set to user stack
- **WHEN** `process_start(pcb)` is called
- **THEN** the CPU ESP SHALL point to a valid user stack address

#### Scenario: Execution begins at entry address
- **WHEN** `process_start(pcb)` is called
- **THEN** the CPU EIP SHALL be set to `pcb->entry`

### Requirement: Syscall exit returns control to kernel
The system SHALL handle exit syscall by saving process state and returning to kernel context.

#### Scenario: Exit syscall number is 1
- **WHEN** a user process executes `int 0x80` with eax=1
- **THEN** the syscall handler SHALL invoke `sys_exit()`

#### Scenario: Exit code is passed in ebx
- **WHEN** a user process executes `int 0x80` with eax=1 and ebx=0
- **THEN** the syscall handler SHALL save 0 as the exit code

#### Scenario: Process eip is captured
- **WHEN** `sys_exit()` is called
- **THEN** the PCB eip field SHALL be set to the saved EIP from the interrupt

#### Scenario: Control returns to kernel after exit
- **WHEN** `sys_exit()` completes
- **THEN** the CPU SHALL resume execution in kernel mode at the instruction following the call to `process_start()`
