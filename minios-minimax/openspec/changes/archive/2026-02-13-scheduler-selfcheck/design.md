# Scheduler Self-Check: Test Process Spawn (Proposal B)

## Overview

Create a special "test process" that runs at OS startup to verify the preemptive scheduler is functioning correctly. The test process runs in user mode, confirms timer interrupts fire, and validates context switching works.

## Architecture

### Components

1. **Test Process Code** (`programs/src/sched_test.c`)
   - Simple user program that counts ticks in a busy loop
   - Exits with success after confirming preemption occurred

2. **Self-Check Module** (`src/kernel/process/scheduler_test.c`)
   - Orchestrates test process creation and verification
   - Validates scheduler state transitions
   - Reports results via serial output

3. **Integration** (`src/kernel/main.c`)
   - Calls `scheduler_selfcheck()` after scheduler initialization
   - Continues boot only if tests pass

### Test Flow

```
kmain()
  ├─ process_init()
  ├─ pit_init()          // Timer must be ready before tests
  ├─ scheduler_selfcheck()
  │   ├─ Create test process
  │   ├─ Start test process
  │   ├─ Wait for exit (with timeout)
  │   ├─ Verify run_count >= threshold
  │   └─ Report results
  └─ Continue boot...
```

## Implementation Details

### Test Process (User Mode)

```c
// programs/src/sched_test.c
#include <stdint.h>

extern uint32_t get_tick_count(void);
extern void sys_exit(int code);

void main(void) {
    uint32_t start_tick = get_tick_count();

    // Busy loop - timer should preempt us
    for (volatile int i = 0; i < 1000000; i++) {
        if (i % 100000 == 0) {
            // This provides output during test
        }
    }

    uint32_t end_tick = get_tick_count();
    uint32_t elapsed = end_tick - start_tick;

    // Success: timer fired at least 5 times (50ms @ 100Hz)
    if (elapsed >= 5) {
        sys_exit(0);  // Success code
    } else {
        sys_exit(1);  // Failure - no preemption detected
    }
}
```

### Self-Check Module (Kernel Mode)

```c
// src/kernel/process/scheduler_test.c
#include "process.h"
#include "../debug.h"
#include "../cpu/interrupts.h"

#define TEST_PROCESS_NAME "sched_test"
#define SCHED_TEST_TIMEOUT 5000  // 5 seconds max
#define MIN_EXPECTED_TICKS 5     // At least 50ms of preemption

static pcb_t* test_pcb = NULL;
static volatile int test_exit_code = -1;
static volatile uint32_t test_start_time = 0;

void scheduler_selfcheck(void) {
    DEBUG_SCHED("[SCHED-TEST] Starting scheduler self-check...");

    // 1. Verify scheduler infrastructure exists
    if (process_table.count > 0) {
        DEBUG_ERROR("[SCHED-TEST] FAILED: Process table already has processes");
        return;
    }

    // 2. Create test process
    test_pcb = process_create(TEST_PROCESS_NAME, TEST_PROGRAM_ADDR);
    if (test_pcb == NULL) {
        DEBUG_ERROR("[SCHED-TEST] FAILED: Could not create test process");
        return;
    }

    // 3. Load test binary
    uint8_t* test_binary = get_sched_test_binary(&test_binary_size);
    if (test_binary == NULL || test_binary_size == 0) {
        DEBUG_ERROR("[SCHED-TEST] FAILED: Test binary not found");
        return;
    }

    if (process_load(test_pcb, test_binary, test_binary_size) != 0) {
        DEBUG_ERROR("[SCHED-TEST] FAILED: Could not load test binary");
        return;
    }

    // 4. Start test process
    test_start_time = pit_get_ticks();
    test_exit_code = -1;

    DEBUG_SCHED("[SCHED-TEST] Starting test process...");
    process_start(test_pcb);

    // Note: process_start() doesn't return - it calls enter_user_mode()
    // The test process will exit via syscall, and scheduler will be called
    // We need to verify the process completed

    // After test process exits, we're back in kernel context
    uint32_t test_end_time = pit_get_ticks();
    uint32_t elapsed = test_end_time - test_start_time;

    // 5. Verify results
    if (test_exit_code == 0) {
        DEBUG_SCHED("[SCHED-TEST] PASSED: Test process exited successfully");
        DEBUG_SCHED("[SCHED-TEST] Elapsed ticks: %u (expected >= %u)", elapsed, MIN_EXPECTED_TICKS);
    } else if (test_exit_code == 1) {
        DEBUG_ERROR("[SCHED-TEST] FAILED: No preemption detected");
    } else {
        DEBUG_ERROR("[SCHED-TEST] FAILED: Unknown exit code %d", test_exit_code);
    }

    DEBUG_SCHED("[SCHED-TEST] Test process run_count: %u", test_pcb->run_count);
}
```

### Integration Points

#### 1. PCB Modifications
No changes needed - existing PCB tracks `run_count` and state.

#### 2. Timer Tick Counter
```c
// src/kernel/cpu/interrupts.c
static volatile uint32_t pit_ticks = 0;

void timer_handler(void) {
    pit_ticks++;
    // ... existing handler code ...
}

uint32_t pit_get_ticks(void) {
    return pit_ticks;
}
```

#### 3. Syscall Handler Extension
```c
// src/kernel/syscall/syscall.c

// Track test process exit
extern volatile int test_exit_code;

int syscall_handler(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx) {
    switch (eax) {
        case SYSCALL_EXIT:
            {
                pcb_t* pcb = process_get_current();
                if (pcb) {
                    // Check if this is our test process
                    if (test_pcb != NULL && pcb->id == test_pcb->id) {
                        test_exit_code = (int)ebx;
                    }
                    // ... normal exit handling ...
                }
            }
            // ... fall through to normal exit ...
    }
}
```

## Test Process Binary Embedding

The test program must be embedded like other user programs:

```c
// src/kernel/programs.h
extern const uint8_t sched_test_bin[];
extern const uint32_t sched_test_bin_size;

static inline const uint8_t* get_sched_test_binary(uint32_t* size) {
    if (size) *size = sched_test_bin_size;
    return sched_test_bin;
}
```

## Success Criteria

The self-check passes if:
1. Test process is created successfully
2. Test process loads and starts
3. Timer interrupts fire during execution (run_count >= 1)
4. Test process exits via syscall
5. Exit code is 0 (preemption detected)

## Failure Modes

| Failure | Detection | Action |
|---------|-----------|--------|
| Can't create process | process_create() returns NULL | Log error, continue boot |
| Can't load binary | process_load() returns -1 | Log error, continue boot |
| No timer ticks | run_count == 0 after exit | Log error, continue boot |
| Timeout | Process runs too long | Hard timeout after 5s |
| Double fault/panic | Exception during test | Kernel panic |

## Security Considerations

- Test process runs with user privileges (CPL=3)
- No special access to kernel memory
- Limited execution time (timeout protection)
- Cannot modify scheduler state

## Performance Impact

- Boot time: +50-100ms (test duration)
- Memory: ~1KB for test binary
- Runtime: No impact after boot
