## Why

Currently there's no automated verification that the preemptive scheduler is functioning correctly. While manual testing with seqprint.c shows preemption works, the OS should self-verify critical scheduler invariants at boot time to catch bugs early and provide confidence in the scheduler's correctness.

## What Changes

Add a self-check mechanism that runs at OS startup to verify:
1. **Timer interrupt fires**: PIT generates interrupts at expected frequency
2. **Context switching works**: Register save/restore cycle is correct
3. **Scheduler selects correct process**: Round-robin finds READY processes correctly
4. **Exit handling works**: Process state transitions from RUNNING to EXITED
5. **No deadlocks**: All processes can run and exit

## Capabilities

### New Capabilities
- `scheduler-selfcheck`: Automated verification of scheduler correctness at boot

### Modified Capabilities
- `preemptive-scheduler`: Now includes built-in self-verification

## Impact

- `src/kernel/process/scheduler_test.c`: New self-check module
- `src/kernel/process/process.h`: Add test helper functions
- `src/kernel/main.c`: Call scheduler_selfcheck() after scheduler init
- `src/kernel/debug.h`: Add assertion macros for checks
