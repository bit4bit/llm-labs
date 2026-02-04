# MiniOS Development Plan

## Phase 1: Environment & Build System
1. Create project structure
2. Set up Makefile with targets:
   - `make kernel` (builds kernel binary)
   - `make iso` (creates bootable ISO)
   - `make run` (launches QEMU)
   - `make clean`

## Phase 2: Bootloader Implementation
1. Create `boot.asm` implementing Multiboot specification
2. Set up initial stack and basic GDT
3. Enable A20 gate
4. Switch to 32-bit protected mode
5. Enter 64-bit long mode with proper paging
6. Hand control to kernel entry point

## Phase 3: Kernel Foundation
1. Create `kernel_entry.asm` (assembly entry point)
2. Implement `_start` function that:
   - Sets up higher-half kernel environment
   - Initializes essential registers
   - Calls C kernel main function
3. Write `kernel.c` with:
   - Basic VGA text output functions
   - Simple memory management for kernel
   - Initial page frame allocator

## Phase 4: ELF64 Loader
1. Parse ELF64 headers in `elf_loader.c`
2. Load program segments to correct virtual addresses
3. Handle program entry point resolution
4. Implement relocation processing if needed

## Phase 5: In-Memory Filesystem
1. Design simple linear filesystem structure
2. Pre-load hello world program into kernel binary
3. Create functions to locate and retrieve program data
4. Implement basic file descriptor system

## Phase 6: System Call Interface
1. Set up IDT for syscall interrupts
2. Implement basic syscalls:
   - `sys_write()` for output
   - `sys_exit()` for program termination
3. Create syscall dispatch mechanism

## Phase 7: Hello World Program
1. Write `hello.c` using only implemented syscalls
2. Create minimal CRT (`crt0.c`) for program startup
3. Link with custom linker script
4. Embed program in filesystem or load directly

## Phase 8: Integration Testing
1. Verify bootloader loads kernel correctly
2. Confirm kernel initializes properly
3. Test ELF64 program loading and execution
4. Validate "Hello, World!" output
5. Debug any issues in QEMU environment

## Program Loading Mechanism

1. **Static Embedding Approach**:
   - The hello world program is compiled separately as an ELF64 binary
   - During kernel build process, this binary is converted to an object file
   - Kernel links with this object file, embedding the program data directly
   - Example commands:
     ```bash
     # Convert program binary to object file
     ld -r -b binary hello_program -o hello.o
     
     # Link with kernel
     ld -T linker.ld kernel.o hello.o -o kernel.bin
     ```

2. **Filesystem-Based Approach**:
   - Kernel implements in-memory filesystem with predefined entries
   - Hello world program stored as fixed data array in kernel source
   - Kernel creates virtual file "/bin/hello" pointing to this data
   - At boot, kernel looks up this file and loads it

3. **Loading Process**:
   ```c
   // 1. Locate program file ("/bin/hello")
   vfs_node_t* prog = vfs_open("/bin/hello");
   
   // 2. Read ELF headers
   elf_header_t* hdr = read_elf_headers(prog);
   
   // 3. Load program segments
   load_elf_segments(hdr, prog);
   
   // 4. Execute program
   jump_to_program(hdr->entry_point);
   ```

4. **Execution Environment**:
   - Set up program stack with argc/argv
   - Initialize heap if needed
   - Configure syscalls for program access
   - Handle program exit gracefully