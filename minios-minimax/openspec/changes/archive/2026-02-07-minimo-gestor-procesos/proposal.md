## Why

El kernel actualmente está en loop infinito (`hlt`) tras inicializar. Se necesita un gestor de procesos mínimo para ejecutar código de usuario separado del kernel. Este MVP establece las abstracciones base (PCB, tabla de procesos) que el scheduler usará después.

## What Changes

- **Nueva estructura PCB** (`process.h`): `pid`, `state`, `entry`, `eip` para guardar contexto
- **Tabla de procesos** (`process.h`): Array de PCBs con `next_pid` y `running`
- **Funciones de proceso** (`process.c`):
  - `process_create(name, addr)`: Crea PCB y lo agrega a tabla
  - `process_start(pcb)`: Cambia a modo usuario y salta a dirección del proceso
  - `process_exit()`: Syscall que guarda eip y vuelve al kernel
- **Modificar `main.c`**: Crear y ejecutar proceso hello tras inicialización
- **Agregar `hello.c`**: Programa minimal que prints "hola mundo" y hace `sys_exit()`
- **Kernel halt**: Tras exit de hello, kernel entra en halt loop

## Capabilities

### New Capabilities
- `process-management`: Gestión mínima de procesos con PCB, tabla y ciclo de vida
- `hello-execution`: Hello world como proceso de prueba demonstre IPC kernel-usuario

### Modified Capabilities
_(ninguna)_

## Impact

- **Nuevo directorio**: `src/kernel/process/` con `process.h` y `process.c`
- **Modificado**: `src/kernel/main.c` para invocar hello
- **Nuevo**: `programs/hello/hello.c` embebido en kernel
- **Dependencias existentes**: GDT, IDT, paging (ya implementados en kernel)
