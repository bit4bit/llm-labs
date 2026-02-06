## ADDED Requirements

### Requirement: Round-robin scheduling
The scheduler SHALL implement round-robin scheduling among all READY processes.

#### Scenario: Scheduler initializes
- **WHEN** `scheduler_init()` is called
- **THEN** ready queue is initialized as empty circular linked list
- **AND** current_process is set to NULL
- **AND** time slice is configured (default 100 ticks)

#### Scenario: Process added to ready queue
- **WHEN** `scheduler_add_process(pcb)` is called
- **THEN** PCB is appended to end of ready queue
- **AND** PCB.next points to head of queue

#### Scenario: Time slice expiration
- **WHEN** timer tick occurs and current process time slice reaches 0
- **THEN** current process state changes from RUNNING to READY
- **AND** current process is moved to end of ready queue
- **AND** scheduler selects next process from head of queue

#### Scenario: Scheduler selects next process
- **WHEN** `scheduler_schedule()` is called or context switch needed
- **THEN** if ready queue not empty, process at head becomes current_process
- **AND** current_process state changes to RUNNING
- **AND** time slice is reset to default

#### Scenario: No ready processes
- **WHEN** scheduler is called and ready queue is empty
- **THEN** idle loop is entered ( halt CPU until interrupt )

### Requirement: Context switching
The scheduler SHALL save and restore process context during switches.

#### Scenario: Context save
- **WHEN** `context_switch_to(new_pcb)` is called
- **THEN** current process registers (esp, ebp) are saved to its PCB
- **AND** new process registers are restored from its PCB
- **AND** stack pointer is switched to new process kernel stack

#### Scenario: First process start
- **WHEN** starting the very first process
- **THEN** no context save is performed for previous process
- **AND** new process state is set to RUNNING
- **AND** CPU jumps to process entry point

### Requirement: Scheduler integration with timer
The scheduler tick callback SHALL trigger scheduling decisions.

#### Scenario: Timer tick increments counter
- **WHEN** timer interrupt fires
- **THEN** scheduler tick callback is invoked
- **AND** current process time slice is decremented
- **AND** if counter reaches 0, context switch is triggered
