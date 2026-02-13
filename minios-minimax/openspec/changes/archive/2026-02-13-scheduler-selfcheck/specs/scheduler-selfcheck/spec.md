# Scheduler Self-Check Specification

## Functional Requirements

### FR1: Timer Interrupt Verification
- **Requirement**: Timer interrupts must fire at configured frequency (~100Hz)
- **Verification**: Test process runs for minimum 50ms, accumulating >=5 timer ticks
- **Pass Condition**: `run_count >= 5` or elapsed ticks >= 5

### FR2: Context Switching Verification
- **Requirement**: Scheduler must save and restore process state correctly
- **Verification**: Test process completes successfully after being preempted
- **Pass Condition**: Process exits with code 0 (success)

### FR3: Round-Robin Scheduling
- **Requirement**: Scheduler must select next READY process
- **Verification**: Test process runs, yields, other processes can run
- **Pass Condition**: Multiple processes complete in round-robin order

### FR4: Exit Handling
- **Requirement**: Process exit must transition state and call scheduler
- **Verification**: Test process exits cleanly, scheduler continues
- **Pass Condition**: Exit syscall returns to kernel, no memory leaks

### FR5: Timeout Protection
- **Requirement**: System must not hang if scheduler fails
- **Verification**: Hard timeout limits test duration
- **Pass Condition**: Test completes within 5 seconds or times out

## Non-Functional Requirements

### NFR1: Performance
- Test duration: <= 500ms
- Memory overhead: <= 2KB
- No impact on runtime performance

### NFR2: Reliability
- Must detect scheduler failures with 100% accuracy
- False positive rate: < 1%
- Deterministic results across runs

### NFR3: Usability
- Clear pass/fail output via serial
- Detailed diagnostics on failure
- No user interaction required

## Test Process Specification

### Code
```c
// User-mode test program
void main(void) {
    uint32_t start = get_tick_count();

    // Busy loop - timer preempts us
    for (volatile int i = 0; i < 1000000; i++) {
        // Empty body - just burn cycles
    }

    uint32_t elapsed = get_tick_count() - start;

    // Pass: timer fired >= 5 times (50ms @ 100Hz)
    if (elapsed >= 5) {
        sys_exit(0);  // SUCCESS
    } else {
        sys_exit(1);  // FAILURE - no preemption
    }
}
```

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

### IC1: No External Dependencies
- Must use only existing kernel infrastructure
- Cannot add new syscalls (reuse SYSCALL_EXIT)
- Cannot modify bootloader

### IC2: Safe Failure Modes
- On test failure, continue boot (degraded mode)
- Log detailed error information
- Do not panic kernel

### IC3: Deterministic Behavior
- Same input → same output
- No race conditions
- Reproducible across QEMU runs

## Test Matrix

| Condition | Expected Behavior |
|-----------|-------------------|
| Normal operation | Test passes, all processes run |
| Timer disabled | Test fails (0 ticks) |
| Scheduler broken | Test fails (no context switch) |
| Bad PCB | Panic (pre-existing bug) |
| Timeout (5s) | Kernel continues, logs error |
