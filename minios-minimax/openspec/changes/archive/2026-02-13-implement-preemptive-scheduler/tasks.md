## 1. Extend PCB Structure

- [x] 1.1 Add register context fields (esp, ebp, eax, ecx, edx, ebx, esi, edi) to pcb_t in process.h
- [x] 1.2 Add segment register fields (ds, es, fs, gs) to pcb_t in process.h
- [x] 1.3 Update process state enum to include PROC_READY (0), PROC_RUNNING (1), PROC_EXITED (2)
- [x] 1.4 Update Static_assert in process.h for new PCB size

**Regression Test (after completing phase 1):**
```bash
make clean && make all && timeout -f 7 make qemu-simple 2>&1 | tee phase1.log
```
Verify: Build succeeds, [SELFCHECK] passes, no kernel panics.

## 2. Add PIT Initialization

- [x] 2.1 Create pit_init() function in interrupts.c
- [x] 2.2 Configure PIT channel 0 with divisor 11932 (~100Hz)
- [x] 2.3 Add pit_init() call in kernel_main after idt_init()

**Regression Test (after completing phase 2):**
```bash
make clean && make all && timeout -f 7 make qemu-simple 2>&1 | tee phase2.log
```
Verify: [PIT] initialized message appears, system boots normally.

## 3. Register Timer Interrupt Handler

- [x] 3.1 Declare timer_handler function in interrupts.h
- [x] 3.2 Register IDT[32] to point to timer_handler in idt_init()
- [x] 3.3 Create timer_handler stub in interrupts.c that prints "TICK"

**Regression Test (after completing phase 3):**
```bash
make clean && make all && timeout -f 7 make qemu-simple 2>&1 | tee phase3.log
```
Verify: "TICK:0 TICK:1 TICK:2..." appears in output (timer firing).

## 4. Implement Timer Interrupt Handler

- [ ] 4.1 Add context save assembly (pushal, push segment registers)
- [ ] 4.2 Add call to scheduler function
- [ ] 4.3 Add context restore assembly (pop segment registers, popal)
- [ ] 4.4 Add iret at end

**Regression Test (after completing phase 4):**
```bash
make clean && make all && timeout -f 7 make qemu-simple 2>&1 | tee phase4.log
```
Verify: TICK continues, no triple faults, processes still complete.

## 5. Implement Scheduler Function

- [x] 5.1 Create scheduler() function in process.c
- [x] 5.2 Implement circular scan for next READY process
- [x] 5.3 Handle idle case (no READY processes, halt)
- [x] 5.4 Update current_process pointer
- [x] 5.5 Restore register context from PCB

**Regression Test (after completing phase 5):**
```bash
make clean && make all && timeout -f 7 make qemu-simple 2>&1 | tee phase5.log
```
Verify: Scheduler runs, TICK continues, no crashes.

## 6. Connect Context Save/Restore

- [x] 6.1 Save current ESP to current_process->esp in timer handler
- [x] 6.2 Save all registers to current_process PCB fields
- [x] 6.3 Set current_process->state to PROC_READY
- [x] 6.4 Restore registers from new process PCB
- [x] 6.5 Set new process state to PROC_RUNNING
- [x] 6.6 Update TSS esp0 for new process kernel stack

**Regression Test (after completing phase 6):**
```bash
make clean && make all && timeout -f 7 make qemu-simple 2>&1 | tee phase6.log
```
Verify: Multiple processes run, output interleaved between selfcheck and hello.

## 7. Create Test Program (seqprint)

- [x] 7.1 Create programs/src/seqprint.c with PID-aware printing
- [x] 7.2 Add syscall for getting current PID
- [x] 7.3 Program prints "PID:i" for i=1..5
- [x] 7.4 Program calls syscall_exit(0) after completion

**Regression Test (after completing phase 7):**
```bash
make clean && make all && timeout -f 7 make qemu-simple 2>&1 | tee phase7.log
```
Verify: seqprint.c builds, appears in programs.h.

## 8. Build and Test

- [x] 8.1 Run `make clean && make all`
- [x] 8.2 Run `make qemu-simple` and observe timer ticks
- [x] 8.3 Verify seqprint output shows interleaved PID:number pairs
- [x] 8.4 Verify all 3 processes complete (15 pairs printed)

## 9. Debug and Fix

- [x] 9.1 If triple fault: check segment selectors in context restore
- [x] 9.2 If no preemption: verify PIT initialization and IDT[32] registration
- [x] 9.3 If context corruption: verify all registers are saved/restored
- [x] 9.4 If incomplete output: verify scheduler loops through all processes

**Debug Commands:**
```bash
# Verbose CPU debug
make clean && make all && timeout -f 7 make qemu-debug 2>&1 | tail -100

# Interrupt debug only
make clean && make all && timeout -f 7 make qemu-int 2>&1 | tail -100
```
