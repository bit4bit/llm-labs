# MiniOS Project Proposal

## Overview
MiniOS is a minimalist 64-bit operating system designed for educational purposes and experimentation. The primary goal is to create a functional OS that can:
- Boot using GRUB
- Run on x86_64 hardware
- Execute ELF64 binaries
- Follow POSIX standards
- Include a basic in-memory filesystem
- Work with GNU development tools

## Objectives

### Core Requirements
1. **Bootloader Compatibility**: Load using GRUB with Multiboot specification
2. **Hardware Target**: Run exclusively on x86_64 machines
3. **Executable Format**: Support ELF64 file execution
4. **Standards Compliance**: Adhere to POSIX standards for system interfaces
5. **Filesystem**: Implement a basic in-memory filesystem
6. **Program Integration**: Compile programs directly with the OS (no runtime loading)
7. **Toolchain Compatibility**: Work seamlessly with GNU tools (gcc, make, nasm)
8. **Development Workflow**: Include a Makefile for compilation and QEMU execution

### Key Features
- Minimal kernel with essential services
- Static program linking model
- Basic system call interface
- Text-mode VGA output for early debugging
- Simple memory management
- Process execution environment

## Technical Approach

### System Architecture
```
+---------------------+
|   User Programs     | <- Hello World and future apps
+----------+----------+
           |
+----------v----------+
|  System Call Layer  | <- POSIX-compliant interface
+----------+----------+
           |
+----------v----------+
|     Kernel Core     | <- Core OS services
| - Memory Management |
| - Process Scheduler |
| - File System       |
+----------+----------+
           |
+----------v----------+
| Hardware Abstraction| <- Device drivers
+----------+----------+

+---------------------+
|   Bootloader        | <- GRUB Multiboot (separate component)
+---------------------+
```

### Development Phases

#### Phase 1: Foundation
- Project structure and build system
- GRUB-compatible bootloader
- Basic kernel with 64-bit mode setup
- Initial Makefile with QEMU integration

#### Phase 2: Core Services
- VGA text output for debugging
- Simple memory manager
- ELF64 loader implementation
- Basic in-memory filesystem

#### Phase 3: Runtime Environment
- System call interface
- Process execution framework
- Program loading and execution
- Basic POSIX compliance layer

#### Phase 4: First Program
- Hello World application
- Minimal C runtime
- Static linking with kernel
- Demonstration of full execution cycle

### Implementation Details

#### Boot Process
1. PC boots and loads GRUB
2. GRUB loads our multiboot-compliant kernel
3. Bootloader sets up initial environment:
   - Enables A20 line
   - Switches to 32-bit protected mode
   - Sets up basic GDT
   - Switches to 64-bit long mode
4. Control transferred to kernel entry point

#### Kernel Initialization
1. Higher-half kernel setup
2. Basic VGA text driver initialization
3. Memory management subsystem
4. In-memory filesystem initialization
5. System call interface setup
6. First program execution

#### Program Execution Flow
1. Kernel locates program in filesystem
2. ELF64 loader parses program headers
3. Program segments loaded to virtual addresses
4. Execution environment prepared:
   - Stack allocated and initialized
   - Registers set appropriately
5. Control transferred to program entry point
6. Program runs until exit or error

### Development Toolchain
- **Compiler**: GCC (cross-compiler for x86_64)
- **Assembler**: NASM
- **Emulator**: QEMU
- **Build System**: GNU Make
- **Version Control**: Git

### Expected Outcomes
1. Fully bootable OS that runs in QEMU
2. Demonstrable "Hello World" program execution
3. POSIX-compliant system call interface
4. Extensible architecture for future enhancements
5. Comprehensive documentation and build process

## Conclusion
MiniOS represents a focused effort to build a functional, educational operating system that demonstrates core concepts while maintaining practical utility. By adhering to established standards and leveraging proven tools, we aim to create a stable foundation for learning and experimentation in systems programming.