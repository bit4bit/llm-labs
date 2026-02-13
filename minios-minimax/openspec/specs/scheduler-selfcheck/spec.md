# Scheduler Self-Check Specification

## Purpose

This specification defines the requirements for verifying that the MiniOS preemptive scheduler is working correctly at boot time through automated self-check mechanisms.

## Functional Requirements

### Requirement: Timer Interrupt Verification
- **Requirement**: Timer interrupts must fire at configured frequency (~100Hz)
- **Verification**: Test process runs for minimum 50ms, accumulating >=5 timer ticks
- **Pass Condition**: `run_count >= 5` or elapsed ticks >= 5

### Requirement: Context Switching Verification
- **Requirement**: Scheduler must save and restore process state correctly
- **Verification**: Test process completes successfully after being preempted
- **Pass Condition**: Process exits with code 0 (success)

### Requirement: Round-Robin Scheduling
- **Requirement**: Scheduler must select next READY process
- **Verification**: Test process runs, yields, other processes can run
- **Pass Condition**: Multiple processes complete in round-robin order

### Requirement: Exit Handling
- **Requirement**: Process exit must transition state and call scheduler
- **Verification**: Test process exits cleanly, scheduler continues
- **Pass Condition**: Exit syscall returns to kernel, no memory leaks

### Requirement: Timeout Protection
- **Requirement**: System must not hang if scheduler fails
- **Verification**: Hard timeout limits test duration
- **Pass Condition**: Test completes within 5 seconds or times out

## Non-Functional Requirements

### Requirement: Performance
- Test duration: <= 500ms
- Memory overhead: <= 2KB
- No impact on runtime performance

### Requirement: Reliability
- Must detect scheduler failures with 100% accuracy
- False positive rate: < 1%
- Deterministic results across runs

### Requirement: Usability
- Clear pass/fail output via serial
- Detailed diagnostics on failure
- No user interaction required

## Test Process Specification

### Requirement: Test Program Implementation
The system SHALL include a self-check test program that validates scheduler functionality.

#### Scenario: Self-check program runs at boot
- **WHEN** kernel initializes
- **THEN** self-check program SHALL be loaded and executed
- **AND** program SHALL run for sufficient time to accumulate timer interrupts

#### Scenario: Self-check program exits with validation
- **WHEN** self-check program completes
- **THEN** it SHALL exit with code 0 if successful (>=5 context switches)
- **OR** exit with code 1 if unsuccessful (<5 context switches)

### Exit Codes
- `0`: Success - preemption detected
- `1`: Failure - no preemption detected
- Other: Error condition

### API Dependencies
- `get_tick_count()`: Returns PIT tick counter (kernel helper)
- `sys_exit(int code)`: Standard exit syscall

## Interface Specification

### Kernel API
```c
// scheduler_test.c
void scheduler_selfcheck(void);
```

### Expected Output
```
[SCHED-TEST] Starting scheduler self-check...
[SCHED-TEST] Creating test process...
[SCHED-TEST] Starting test process...
[SCHED-TEST] Test process exited with code 0
[SCHED-TEST] Elapsed ticks: 7
[SCHED-TEST] PASSED: Scheduler working correctly
```

### Error Output
```
[SCHED-TEST] Starting scheduler self-check...
[SCHED-TEST] Creating test process...
[SCHED-TEST] Test process exited with code 1
[SCHED-TEST] FAILED: No preemption detected
```

## Implementation Constraints

### Requirement: No External Dependencies
- Must use only existing kernel infrastructure
- Cannot add new syscalls (reuse SYSCALL_EXIT)
- Cannot modify bootloader

### Requirement: Safe Failure Modes
- On test failure, continue boot (degraded mode)
- Log detailed error information
- Do not panic kernel

### Requirement: Deterministic Behavior
- Same input → same output
- No race conditions
- Reproducible across QEMU runs

## Test Matrix

### Requirement: Comprehensive Test Coverage
The self-check mechanism SHALL handle various failure modes appropriately.

#### Scenario: Normal operation test
- **WHEN** scheduler works normally
- **THEN** test passes with message "PASSED: Scheduler working correctly"

#### Scenario: Timer disabled test
- **WHEN** PIT timer is not firing
- **THEN** test fails with message "FAILED: No preemption detected"

#### Scenario: Scheduler broken test
- **WHEN** context switching is not working
- **THEN** test fails with message "FAILED: No preemption detected"

#### Scenario: Bad PCB test
- **WHEN** process control block is corrupted
- **THEN** kernel panics (pre-existing error handling should apply)

#### Scenario: Timeout test
- **WHEN** system hangs during test
- **THEN** kernel continues boot and logs timeout error