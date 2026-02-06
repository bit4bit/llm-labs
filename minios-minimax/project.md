# MiniOS - Sistema Operativo Educativo

## Descripción
MiniOS es un sistema operativo educativo de 32 bits, mínimo viable, que implementa las características esenciales para boot via multiboot, un kernel con soporte completo de paging, y un proceso hello world que demuestra las APIs básicas del sistema.

## Objetivos
- Bootloader compatible con GRUB (multiboot)
- Kernel de 32 bits compilado con GCC
- Soporte completo de memoria virtual (paging)
- APIs POSIX básicas para syscalls
- Sistema de archivos plano (solo /)
- Proceso hello world funcional
- Scheduler round-robin con context switching
- Memoria fija para programas (hello.c, shell.c) via binarios planos

## Estructura del Proyecto

```
minios/
├── Makefile                 # Build system
├── link.ld                  # Linker script para el kernel
├── README.md
├── docs/
│   └── arquitectura.md      # Documentación técnica
├── src/
│   ├── kernel/
│   │   ├── boot/
│   │   │   ├── multiboot.S  # Entry point multiboot
│   │   │   └── boot.s       # Bootstrap del bootloader
│   │   ├── cpu/
│   │   │   ├── gdt.c        # Global Descriptor Table
│   │   │   └── idt.c        # Interrupt Descriptor Table
│   │   ├── memory/
│   │   │   ├── paging.c     # Implementación de paging
│   │   │   └── alloc.c      # Physical Memory Manager
│   │   ├── drivers/
│   │   │   ├── timer/
│   │   │   │   └── pit.c    # PIT timer interrupt driver
│   │   │   ├── video/
│   │   │   │   ├── vga.c    # Controlador VGA
│   │   │   │   └── video.h
│   │   │   ├── keyboard/
│   │   │   │   ├── keyboard.c  # Controlador de teclado
│   │   │   │   └── keyboard.h
│   │   │   └── driver.h     # Interfaz común de drivers
│   │   ├── syscall/
│   │   │   ├── syscall.c    # Manejo de syscalls
│   │   │   └── syscalls.c   # Definición de syscalls POSIX
│   │   ├── process/
│   │   │   ├── process.c    # Gestión de procesos
│   │   │   ├── scheduler.c # Scheduler round-robin
│   │   │   ├── context_switch.S # Context switching assembly
│   │   │   ├── binary.c     # Flat binary loader
│   │   │   └── programs.c   # Program symbol table (fixed addresses)
│   │   ├── main.c           # Entry point del kernel
│   │   └── kernel.h         # Headers del kernel
│   ├── libc/
│   │   ├── string/
│   │   │   ├── string.c     # Funciones string
│   │   │   └── string.h
│   │   ├── stdio/
│   │   │   ├── printf.c     # Implementación printf
│   │   │   └── stdio.h
│   │   └── ctype.h
│   └── user/
│       └── lib.c            # Biblioteca de usuario
├── programs/                 # Programas de usuario compilados como binarios planos
│   ├── hello/
│   │   ├── hello.c          # Hello world - imprime "hola mundo"
│   │   └── Makefile         # Compila a hello.bin (0x40000000)
│   ├── shell/
│   │   ├── shell.c          # Intérprete de comandos básico
│   │   └── Makefile         # Compila a shell.bin (0x40010000)
│   └── Makefile             # Compila todos los programas
├── include/
│   ├── kernel.h
│   ├── sys/
│   │   ├── syscall.h
│   │   └── types.h
│   └── stdio.h
└── iso/
    ├── boot/
    │   └── grub/
    │       └── stage2_eltorito
    └── kernel.bin
```

## Componentes Principales

### 1. Boot (src/kernel/boot/)
- **multiboot.S**: Header multiboot para que GRUB reconozca el kernel
- **boot.s**: Configuración inicial, transición a modo protegido

### 2. CPU (src/kernel/cpu/)
- **gdt.c**: Configuración de GDT para segmentación
- **idt.c**: Manejo de interrupciones (IDT)

### 3. Memory (src/kernel/memory/)
- **paging.c**: Implementación de page tables, page faults
- **物理内存管理**: Allocator físico

### 4. Timer (src/kernel/drivers/timer/)
- **pit.c**: Driver del Programmable Interval Timer para scheduler

### 5. Process (src/kernel/process/)
- **process.c**: Gestión de procesos con PCB
- **scheduler.c**: Scheduler round-robin con context switching
- **binary.c**: Loader de binarios planos a direcciones fijas
- **programs.c**: Symbol table con direcciones de programas

### 6. Syscall (src/kernel/syscall/)
- **syscall.c**: Dispatcher de syscalls
- **syscalls.c**: Implementación write, exit, sleep

### 7. LibC (src/libc/)
- Implementación mínima de funciones C estándar

### 8. User (src/user/)
- **lib.c**: Biblioteca de usuario para syscalls

## Dependencias

```
src/kernel/boot/multiboot.S  →  src/kernel/main.c
src/kernel/cpu/gdt.c         →  src/kernel/main.c
src/kernel/cpu/idt.c         →  src/kernel/main.c
src/kernel/memory/paging.c   →  src/kernel/main.c
src/kernel/memory/alloc.c    →  src/kernel/memory/paging.c
src/kernel/drivers/video/vga.c →  src/kernel/drivers/driver.h
src/kernel/drivers/keyboard/keyboard.c →  src/kernel/drivers/driver.h
src/kernel/drivers/timer/pit.c →  src/kernel/main.c
src/kernel/process/process.c →  src/kernel/main.c
src/kernel/process/scheduler.c →  src/kernel/process/process.c
src/kernel/process/binary.c  →  src/kernel/process/process.c
src/kernel/process/programs.c →  src/kernel/process/binary.c
src/kernel/syscall/syscall.c →  src/kernel/main.c
src/kernel/syscall/syscalls.c →  src/kernel/syscall/syscall.c
programs/hello/hello.bin      →  kernel (init process) - loads to 0x40000000
programs/shell/shell.bin      →  kernel (init process) - loads to 0x40010000
```

## Formato de Procesos

Los programas de usuario son binarios planos de 32 bits. El kernel incluye un loader de binario plano (`src/kernel/process/binary.c`) que:
- Lee binario desde almacenamiento (sector del disco)
- Copia datos directamente a dirección fija (especificada en `programs.c`)
- Configura page tables para la dirección del programa

### Direcciones Fijas
- **hello**: 0x40000000 (tamaño: 4KB)
- **shell**: 0x40010000 (tamaño: 8KB)

### Compilación
```bash
# hello.c → hello.bin
i686-elf-gcc -nostdlib -Wl,-Ttext=0x40000000,-e=_start -o hello.bin hello.c

# shell.c → shell.bin
i686-elf-gcc -nostdlib -Wl,-Ttext=0x40010000,-e=_start -o shell.bin shell.c
```

### Symbol Table
El kernel mantiene una tabla de programas (`src/kernel/process/programs.c`) con las direcciones fijas:

## Testing Automatizado

```
tests/
├── run_tests.sh           # Script principal de testing
├── test_hello.sh          # Test hello world
├── test_memory.sh         # Test de memoria/paging
├── test_keyboard.sh       # Test de interacción con teclado
├── expected/
│   ├── hello.output       # Output esperado: "hola mundo"
│   └── keyboard.output   # Output esperado de keyboard
├── fixtures/
│   ├── hello.keys         # Keys para test hello (Enter)
│   └── keyboard.keys     # Secuencia de teclas a enviar
└── README.md              # Documentación de tests
```

### Test de Teclado

**Método 1: QEMU Monitor**
```bash
# Enviar keys via monitor
echo "sendkey h" | nc localhost 4444
echo "sendkey e" | nc localhost 4444
```

**Método 2: Archivo de input (console)**
```bash
qemu -serial file:output.txt -readconfig fixtures/keyboard.keys
```

**Script de test:**
```bash
#!/bin/bash
./run_tests.sh setup
echo "hello" | timeout 5 qemu -nographic -serial stdio ...
grep -q "hola mundo" output.txt && echo "PASS" || echo "FAIL"
```

### Comandos de Testing

```bash
make test              # Ejecutar todos los tests
make test-hello        # Test específico hello world
make test-keyboard     # Test de teclado
make test-ci           # Para integración continua
```

## Compilación

```bash
make              # Compilar kernel y user programs
make programs     # Compilar programas como binarios planos
make iso          # Crear imagen ISO
make qemu         # Ejecutar en QEMU
make clean        # Limpiar build
```

### Compilación de Programas
```bash
# Los programas se compilan como binarios planos
programs/hello/hello.bin  → carga en 0x40000000
programs/shell/shell.bin   → carga en 0x40010000
```

## Requisitos de Herramientas
- GCC 32-bit (i686-elf-gcc)
- GNU Binutils
- GRUB (legacy)
- QEMU (para testing)
- make

## Licencia
MIT
