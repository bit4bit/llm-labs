## ADDED Requirements

### Requirement: Process Control Block structure
The kernel SHALL maintain a Process Control Block (PCB) for each process containing essential state information.

#### Scenario: PCB allocation
- **WHEN** a new process is created
- **THEN** a PCB is allocated from the free list
- **THEN** PCB is initialized with zeroed registers, READY state, and unique ID

#### Scenario: PCB fields
The PCB SHALL contain at minimum:
- Process ID (unique 32-bit identifier)
- CPU registers (esp, ebp, eip, cr3)
- Process state (READY, RUNNING, BLOCKED)
- Pointer to kernel stack
- Pointer to next PCB in scheduler queue

### Requirement: Process states
Processes SHALL have three states: READY, RUNNING, BLOCKED.

#### Scenario: New process created
- **WHEN** `process_create(entry_point, stack_size)` is called
- **THEN** process state is set to READY
- **THEN** process is added to the scheduler's ready queue

#### Scenario: Process becomes running
- **WHEN** scheduler selects a READY process
- **THEN** process state changes to RUNNING
- **AND** current_process is updated

#### Scenario: Process blocks on sleep
- **WHEN** running process calls `sleep()`
- **THEN** process state changes to BLOCKED
- **AND** process is removed from ready queue
- **AND** scheduler selects next READY process

#### Scenario: Process exits
- **WHEN** running process calls `exit()` or returns from entry point
- **THEN** process state changes to TERMINATED
- **AND** PCB is returned to free list
- **AND** scheduler selects next READY process

### Requirement: Maximum process count
The system SHALL support a configurable maximum number of processes.

#### Scenario: Process limit reached
- **WHEN** creating a new process and all PCBs are in use
- **THEN** creation function returns error (-1)
- **AND** no memory leak occurs

### Requirement: Kernel stack per process
Each process SHALL have its own kernel stack for syscall handling.

#### Scenario: Kernel stack allocation
- **WHEN** process is created
- **THEN** a kernel stack of specified size is allocated
- **AND** stack pointer is initialized to top of stack
- **AND** stack memory is freed when process terminates
