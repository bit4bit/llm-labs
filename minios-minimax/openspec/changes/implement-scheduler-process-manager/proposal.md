## Why

MiniOS currently lacks process management and scheduling capabilities. The kernel can load ELF binaries (hello.c, shell.c) but cannot switch between processes or allocate fixed memory regions. This change implements a simple scheduler and process manager to enable basic multitasking with context switching.

## What Changes

- Implement timer interrupt driver (PIT) for scheduler time-slicing
- Implement process manager with process control blocks (PCB), states, and lifecycle management
- Implement scheduler with round-robin context switching between processes
- Assign fixed memory regions to user programs via linker script configuration
- Integrate scheduler with the existing ELF loader and memory management

## Capabilities

### New Capabilities

- `timer-interrupt`: Simple PIT (Programmable Interval Timer) driver for generating timer interrupts
- `process-management`: Process creation, termination, state transitions (ready, running, blocked), and PCB structure
- `scheduler`: Round-robin scheduling with timer-based context switching using the PIT interrupt
- `fixed-memory-regions`: General mechanism for assigning fixed virtual memory addresses to user programs. Each program is linked to a specific base address, enabling predictable memory layout across programs

### Modified Capabilities

None

## Impact

- **Affected Files:**
  - `src/kernel/drivers/timer/pit.c` - PIT timer interrupt driver
  - `src/kernel/process/process.c` - Process management implementation
  - `src/kernel/process/scheduler.c` - Scheduler implementation
  - `src/kernel/process/binary.c` - Flat binary loader
  - `src/kernel/kernel.h` - Add program symbol table declarations
- **New Files:**
  - `src/kernel/process/programs.c` - Program symbol table with base addresses
- **User Programs:** All programs can be assigned fixed memory regions via linker script configuration
- **Build System:** Makefiles in `programs/` will support base address configuration
- **Dependencies:** Timer interrupt driver (PIT), existing paging system
