## 1. PCB and Kernel Stacks

- [x] 1.1 Update `pcb_t` in `process.h`: remove individual register fields (`eip`, `esp`, `ebp`, `eax`-`edi`, `ds`-`gs`), add `kernel_esp`, `kernel_stack_top`, `user_stack`. Update `_Static_assert` for new size.
- [x] 1.2 Declare static kernel stacks array in `process.c`: `static char kernel_stacks[MAX_PROCESSES][4096] __attribute__((aligned(4096)))`.
- [x] 1.3 Update `process_create()` to assign `kernel_stack_top` from the stacks array based on process index.

## 2. Per-Process Memory Regions

- [x] 2.1 Move user code/stack PDE allocation from `vmm_init()` to `process_create()`. Each process gets its own PDE for code (`256 + i`) and stack (`767 - i`) with a freshly allocated 4MB frame via `pmm_alloc_frame()`. Flush TLB after PDE writes.
- [x] 2.2 Add helper function or macros in `minios.h` to compute code virtual address (`0x40000000 + i * 0x400000`) and stack virtual address for process index `i`.
- [x] 2.3 Update `process_create()` to set `entry` to the process-specific code virtual address and `user_stack` to the process-specific stack top (stack region base + 4MB - 4KB).
- [x] 2.4 Verify `process_load()` still works correctly (copies binary to `pcb->entry` which now varies per process).

## 3. Fake Interrupt Frame

- [x] 3.1 Implement fake interrupt frame setup in `process_create()`: push SS, ESP, EFLAGS, CS, EIP (iret frame), then EAX-EDI (pushal order), then DS, ES, FS, GS onto the process's kernel stack. Set `kernel_esp` to the resulting stack pointer.

## 4. Context Switch (Assembly)

- [x] 4.1 Implement `scheduler_switch(prev, next)` in `trampoline.S`: save ESP into `prev->kernel_esp` (at correct struct offset), load ESP from `next->kernel_esp`, update `TSS.esp0` to `next->kernel_stack_top`, `ret`.
- [x] 4.2 Define PCB field offsets as constants (either in a shared header or as `.equ` in the assembly file) so `scheduler_switch` can access `kernel_esp` and `kernel_stack_top` at the correct offsets.

## 5. Timer Handler Simplification

- [x] 5.1 Remove the register-by-register saving in `timer_handler()` (`interrupts.c` lines 85-133). The handler should only do: EOI, increment `pit_ticks`, increment `run_count`, call `scheduler()`.
- [x] 5.2 Verify `timer_handler_asm` already does `pushal` + segment push before `call timer_handler` and `popal` + segment pop + `iret` after. This is the correct frame that `scheduler_switch` relies on.

## 6. Scheduler Update

- [x] 6.1 Update `scheduler()` in `process.c`: remove the `enter_user_mode()` branch and the `eip` check. When `next != prev`, call `scheduler_switch(prev, next)`. When `next == prev`, return without switching.
- [x] 6.2 Update `process_start()`: instead of calling `enter_user_mode()` directly, set `current_process`, mark as RUNNING, set TSS, then restore the fake frame from the kernel stack (load `kernel_esp` and execute `pop segments + popal + iret`). This can reuse a new assembly helper or be inlined.
- [x] 6.3 Update `process_exit_return()` in `main.c` to handle multiple processes: instead of halting immediately, check if all processes have exited and report results.

## 7. Build System

- [x] 7.1 Parametrize `programs/user.ld`: change `. = 0x40000000` to `. = DEFINED(_user_base) ? _user_base : 0x40000000`.
- [x] 7.2 Update `tools/build-programs.rb` to assign each program a unique base address (0x40000000, 0x40400000, etc.) and pass `--defsym=_user_base=<addr>` to the linker. Add address mapping (e.g., based on sorted program discovery order).
- [x] 7.3 Update `src/kernel/programs.h` and `src/kernel/programs.c` if new test programs are added.

## 8. Kernel Main and Integration

- [x] 8.1 Update `kernel_main()` in `main.c`: create multiple processes (e.g., selfcheck + hello), load each with its binary, then call `process_start()` on the first one.
- [x] 8.2 Update `vmm_init()` to remove the single user code/stack PDE allocation (now done per-process in `process_create()`).

## 9. Testing

- [x] 9.1 Build with `make clean && make all` and verify no compilation errors.
- [x] 9.2 Run `make qemu-simple` and verify selfcheck still passes (scheduler self-check with run_count >= 2).
- [ ] 9.3 Verify serial output shows interleaved execution of multiple processes (both hello and selfcheck produce output concurrently).
- [ ] 9.4 Verify all processes complete (no hangs, no page faults, no GPFs).
