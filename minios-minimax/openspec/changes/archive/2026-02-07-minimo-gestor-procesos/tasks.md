## 1. TSS Setup for Ring 3 Transition

- [x] 1.1 Create `src/kernel/cpu/tss.h` with TSS structure definition
- [x] 1.2 Create `src/kernel/cpu/tss.c` with `tss_init()` function
- [x] 1.3 Add TSS entry to GDT (entry 5)
- [x] 1.4 Load TSS with `ltr` instruction
- [x] 1.5 TSS initialized and loaded successfully

## 2. Process Core: Data Structures

- [x] 2.1 Create `src/kernel/process/process.h` with PCB struct
- [x] 2.2 Define process states (PROC_RUNNING, PROC_EXITED)
- [x] 2.3 Define MAX_PROCESSES (4) and process_table_t struct
- [x] 2.4 Declare global `process_table_t` and `current_process`
- [x] 2.5 All function declarations in process.h

## 3. Process Core: Functions

- [x] 3.1 Implement `process_init()` - initializes table
- [x] 3.2 Implement `process_create(name, entry)` - allocates PCB
- [x] 3.3 Implement `process_start(pcb)` - calls enter_user_mode
- [x] 3.4 Implement `process_get_current()` - returns current_process
- [x] 3.5 Bounds checking in `process_create` for MAX_PROCESSES

## 4. Syscall Infrastructure

- [x] 4.1 Create `src/kernel/syscall/syscall.h` with syscall numbers
- [x] 4.2 Implement `syscall_handler()` in syscall.c
- [x] 4.3 Implement `syscall_asm.S` entry point
- [x] 4.4 IDT gate 0x80 configured for syscall (BUG: flags=0x238 instead of 0xEE)

## 5. Hello Program

- [x] 5.1 hello_bin defined in main.c (syscall exit only)
- [x] 5.2 HELLO_ADDR defined as 0x40000000
- [ ] 5.3 Copy hello_bin to 0x40000000 via memcpy (MISSING)

## 6. Kernel Integration

- [x] 6.1 Include process.h, tss.h, syscall.h in main.c
- [x] 6.2 Call tss_init() after GDT init
- [x] 6.3 Call process_init() after TSS init
- [x] 6.4 Call process_create("hello", HELLO_ADDR)
- [ ] 6.5 Copy hello_bin to 0x40000000 via memcpy (MISSING)
- [x] 6.6 Call process_start(hello) to launch hello
- [x] 6.7 kernel_halt_loop() after process_start returns
- [ ] 6.8 Fix paging to map 0x40000000 with U/S=1 (0x40000000 not mapped)

## 7. Build System

- [x] 7.1 Add process.c, tss.c, syscall.c, syscall_asm.S to Makefile
- [x] 7.2 Add gdt_set_gate export to gdt.h
- [x] 7.3 iso/boot/grub directory and grub.cfg created
- [x] 7.4 Makefile iso and qemu targets working

## 8. Testing

**ISSUE: IDT flags corruption**
- Writing flags=0xEE, reading flags=0x238
- Causes: int 0x80 goes to default handler instead of syscall_handler
- Status: INVESTIGATING

**ISSUE: hello_bin not copied**
- memcpy to 0x40000000 is missing from main.c
- Status: NEEDS FIX

**ISSUE: Paging user access**
- 0x40000000 needs U/S=1 bit in PDE
- Current PDEs only cover 0-256MB (0x10000000)
- Status: NEEDS FIX

- [ ] 8.1 Fix IDT entry 0x80 flags (0xEE not 0x238)
- [ ] 8.2 Add memcpy to copy hello_bin to 0x40000000
- [ ] 8.3 Extend PDEs to cover 0x40000000 (256+ entries)
- [ ] 8.4 Add U/S=1 bit to PDE for 0x40000000
- [ ] 8.5 Verify serial output shows "Syscall: exit called"
- [ ] 8.6 Verify hello prints "hola mundo" or "HELLO" to VGA
- [ ] 8.7 Kernel halts after hello exit

## 9. Documentation

- [ ] 9.1 Update project.md with new process architecture
- [ ] 9.2 Document TSS and ring transition flow
- [ ] 9.3 Add syscall interface documentation

---

## Known Bugs (Blocking)

| Bug | Symptom | Root Cause | Status |
|-----|---------|------------|--------|
| IDT flags wrong | Default handler called | Memory corruption or struct issue | Investigating |
| No memcpy hello | hello_bin not at 0x40000000 | Missing code | Needs fix |
| Paging user | Page fault at 0x40000000 | PDEs don't cover address | Needs fix |

## Architecture

```
kernel_main()
    ├─ PMM init
    ├─ GDT init
    ├─ IDT init (with bug in 0x80 entry)
    ├─ Paging init (missing user mapping)
    ├─ TSS init
    ├─ Process init
    ├─ process_create("hello", 0x40000000)
    ├─ memcpy(hello_bin → 0x40000000)  [MISSING]
    └─ process_start(hello)
         └─ enter_user_mode() → iret to ring 3
              └─ hello runs → int 0x80
                   └─ syscall_handler()  [NOT REACHED - bug]
                        └─ sys_exit() → kernel_halt()
```
