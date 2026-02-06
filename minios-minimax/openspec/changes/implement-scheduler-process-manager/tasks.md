## 1. Timer Interrupt Driver (PIT)

- [ ] 1.1 Create `src/kernel/drivers/timer/pit.c` with PIT register definitions
- [ ] 1.2 Implement `pit_init(uint32_t frequency)` function
- [ ] 1.3 Implement `pit_irq_handler()` interrupt handler
- [ ] 1.4 Implement `pit_set_tick_callback(void (*callback)())` function
- [ ] 1.5 Create `src/kernel/drivers/timer/pit.h` header file
- [ ] 1.6 Add timer initialization call in `main.c`

## 2. Process Management - Data Structures

- [ ] 2.1 Create `src/kernel/process/process.h` with PCB structure definition
- [ ] 2.2 Define process states enum (READY, RUNNING, BLOCKED, TERMINATED)
- [ ] 2.3 Implement PCB allocation/deallocation functions
- [ ] 2.4 Implement `process_create(void* entry_point, uint32_t stack_size)`
- [ ] 2.5 Implement `process_exit()` function
- [ ] 2.6 Implement `process_set_state(uint32_t pid, uint8_t state)` function

## 3. Scheduler - Core Implementation

- [ ] 3.1 Create `src/kernel/process/scheduler.h` with scheduler interface
- [ ] 3.2 Implement `scheduler_init()` function
- [ ] 3.3 Implement `scheduler_add_process(pcb_t* pcb)` function
- [ ] 3.4 Implement `scheduler_remove_process(uint32_t pid)` function
- [ ] 3.5 Implement `scheduler_tick()` callback for timer interrupts
- [ ] 3.6 Implement `scheduler_schedule()` function

## 4. Context Switching

- [ ] 4.1 Create `src/kernel/process/context_switch.S` assembly file
- [ ] 4.2 Implement `context_save()` to save registers to PCB
- [ ] 4.3 Implement `context_restore()` to restore registers from PCB
- [ ] 4.4 Implement `context_switch(pcb_t* from, pcb_t* to)` function
- [ ] 4.5 Implement `switch_to_first_process(pcb_t* pcb)` for initial switch

## 5. Program Symbol Table

- [ ] 5.1 Create `src/kernel/process/programs.c` with symbol table
- [ ] 5.2 Define `struct program { const char* name; uint32_t base; uint32_t size; }`
- [ ] 5.3 Add entries for hello (0x40000000) and shell (0x40010000)
- [ ] 5.4 Implement `find_program(const char* name)` lookup function
- [ ] 5.5 Add programs.c to main Makefile

## 6. Flat Binary Loader

- [ ] 6.1 Create `src/kernel/process/binary.c` file
- [ ] 6.2 Implement `load_flat_binary(const char* name)` function
- [ ] 6.3 Implement `read_sector(uint32_t lba, void* buffer)` for storage access
- [ ] 6.4 Implement `copy_to_memory(uint32_t dest, uint32_t src, uint32_t size)`
- [ ] 6.5 Add binary.c to main Makefile

## 7. Memory Setup for User Processes

- [ ] 7.1 Update `src/kernel/memory/paging.c` to support user page mappings
- [ ] 7.2 Implement `setup_user_address_space(pcb_t* pcb, uint32_t base_addr)`
- [ ] 7.3 Configure page directory for user process (user bit set in page tables)
- [ ] 7.4 Implement kernel stack allocation per process
- [ ] 7.5 Map fixed program addresses in page tables

## 8. Scheduler Integration

- [ ] 8.1 Integrate scheduler tick callback with PIT timer
- [ ] 8.2 Update `scheduler_schedule()` to call `context_switch()`
- [ ] 8.3 Implement `sleep(uint32_t seconds)` syscall wrapper
- [ ] 8.4 Implement sleep queue and wakeup mechanism
- [ ] 8.5 Update timer interrupt handler to handle wakeups

## 9. System Call Integration

- [ ] 9.1 Add `exit` syscall (syscall 1)
- [ ] 9.2 Add `sleep` syscall (syscall 2)
- [ ] 9.3 Update syscall dispatch table
- [ ] 9.4 Update user library (`src/user/lib.c`) with syscall wrappers

## 10. Build System - Flat Binary Compilation

- [ ] 10.1 Update `programs/hello/Makefile` to compile as flat binary
- [ ] 10.2 Add `-nostdlib -Wl,-Ttext=0x40000000,-e=_start` flags to hello
- [ ] 10.3 Update `programs/shell/Makefile` to compile as flat binary
- [ ] 10.4 Add `-nostdlib -Wl,-Ttext=0x40010000,-e=_start` flags to shell
- [ ] 10.5 Compile programs to `.bin` extension instead of `.o`

## 11. Kernel Main Integration

- [ ] 11.1 Initialize PIT timer with 100 Hz frequency in `main.c`
- [ ] 11.2 Initialize scheduler in `main.c`
- [ ] 11.3 Load hello.bin to 0x40000000 using flat binary loader
- [ ] 11.4 Load shell.bin to 0x40010000 using flat binary loader
- [ ] 11.5 Create processes for hello and shell with entry points
- [ ] 11.6 Start first process with `switch_to_first_process()`

## 12. Build System Updates

- [ ] 12.1 Update main `Makefile` to build new timer driver
- [ ] 12.2 Update main `Makefile` to build new process modules (scheduler, binary, programs)
- [ ] 12.3 Add hello.bin and shell.bin to ISO image
- [ ] 12.4 Verify all programs compile to flat binary format

## 13. Testing and Validation

- [ ] 13.1 Build kernel with no errors
- [ ] 13.2 Test PIT timer fires at correct frequency
- [ ] 13.3 Verify context switch occurs between processes
- [ ] 13.4 Verify hello.bin loads at 0x40000000 and runs correctly
- [ ] 13.5 Verify shell.bin loads at 0x40010000 and runs correctly
- [ ] 13.6 Test process exit and cleanup
- [ ] 13.7 Test sleep functionality if implemented
