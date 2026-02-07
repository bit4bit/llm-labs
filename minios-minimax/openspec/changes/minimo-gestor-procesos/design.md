## Context

El kernel actual (`main.c`) tras inicializar PMM, GDT, IDT y paging, entra en un loop infinito con `hlt`. No existe abstracción de proceso: todo código corre en ring 0 como parte del kernel.

Este diseño implementa un gestor de procesos mínimo que permite:
1. Ejecutar código de "usuario" separado del kernel
2. Establecer abstracciones (PCB, tabla) que el scheduler usará después
3. Demostrar syscall de exit

## Goals / Non-Goals

**Goals:**
- Crear estructura PCB mínima con: `pid`, `state`, `entry`, `eip`
- Implementar tabla de procesos con máximo 4 procesos
- Proveer funciones: `process_create()`, `process_start()`, `process_exit()`
- Ejecutar hello en modo usuario (ring 3)
- Hello hace syscall exit → kernel guarda eip y halt
- Diseño que permite agregar scheduler round-robin después

**Non-Goals:**
- Scheduler con context switching (se agrega después)
- Scheduling round-robin o cualquier algoritmo
- Múltiples procesos concurrentes
- Memoria dinámica para procesos (asignación estática)
- Syscalls além de exit (write, sleep, etc.)
- Carga de binarios desde disco
- Page tables por proceso (comparten espacio del kernel)

## Decisions

### 1. PCB mínima con solo eip para contexto

**Decisión:** PCB guarda solo `eip` (no todos los registros)

```c
typedef struct {
    uint32_t id;
    uint32_t state;
    uint32_t entry;
    uint32_t eip;
} pcb_t;
```

**Alternativa:** Guardar eax, ebx, ecx, edx, ebp, esp, etc.

**Rationale:** MVP no necesita context switch real. El scheduler futuro agregará `esp` y otros registros cuando implemente context_switch(). Mantener PCB mínima evita overengineering.

### 2. Estados simples: RUNNING, EXITED

**Decisión:** Solo dos estados

```c
#define PROC_RUNNING 0
#define PROC_EXITED  1
```

**Alternativa:** Ready, Waiting, Blocked, etc.

**Rationale:** Hello corre, termina, sale. No hay blocking ni waiting. Estados adicionales son para scheduler.

### 3. Hello embebido en kernel como binario plano

**Decisión:** Hello compilado como binario plano, convertido a array C, incluido en kernel

```c
uint8_t hello_bin[] = {
    0xb8, 0x01, 0x00, 0x00, 0x00,  // mov eax, 1  (sys_exit)
    0xbb, 0x00, 0x00, 0x00, 0x00,  // mov ebx, 0  (exit code)
    0xcd, 0x80                      // int 0x80
};
```

**Alternativa:** Cargar desde disco, linker script para embeber .bin

**Rationale:** Evitar complejidad de filesystem/disk. El array embebido permite compilar todo junto. En el futuro, `binary.c` del diseño original puede reemplazar esto.

### 4. Dirección fija para hello: 0x40000000

**Decisión:** Hello carga en 0x40000000

**Rationale:** Es la dirección usada en el diseño original. Page tables ya mapean esa dirección. Simplifica: no necesita loader dinámico.

### 5. Syscall exit via int 0x80

**Decisión:** Hello ejecuta `int 0x80` con eax=1, ebx=0

```
                  ┌─────────────────────────┐
                  │    syscall dispatch     │
                  ├─────────────────────────┤
    hello:        │  eax=1 → sys_exit()    │
        int 0x80 ──┤  guardar eip en PCB   │
                  │  volver al kernel       │
                  └─────────────────────────┘
```

**Alternativa:** sysenter/sysexit (más rápido), or opcode especial

**Rationale:** int 0x80 es simple y portable. El diseño futuro puede optimizar.

### 6. TSS para transición ring 0 → ring 3

**Decisión:** Usar TSS con SS0, ESP0 pointing a kernel stack

```c
tss_entry.ss0  = KERNEL_DATA_SEGMENT;
tss_entry.esp0 = KERNEL_STACK_TOP;
```

**Alternativa:** Jmp directo a dirección usuario sin TSS

**Rationale:** x86 requiere TSS para cambiar a ring 3 correctamente. El kernel ya tiene GDT configurada; solo agregamos TSS entry.

### 7. Tabla de procesos estática

**Decisión:** Array fijo de 4 PCBs

```c
#define MAX_PROCESSES 4

typedef struct {
    pcb_t processes[MAX_PROCESSES];
    uint32_t count;
    uint32_t next_pid;
    uint32_t running;  // índice del PCB actual
} process_table_t;
```

**Rationale:** Memoria estática, simple, sin allocator. Futuro scheduler puede usar esta tabla.

## Arquitectura general

```
┌─────────────────────────────────────────────────────────────────┐
│                     MEMORIA (simplificada)                      │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│   0x00000000 ─┐                                                 │
│               │  Kernel code + data                            │
│   0x00100000 ─┤  (4MB, identity mapped por paging)            │
│               │                                                 │
│   0x40000000 ─┤  Hello code (USER SPACE - ring 3)            │
│               │                                                 │
│   0xBFFFEFFF ─┤  Kernel stack (ring 0)                       │
│   0xBFFFF000 ─┘                                                 │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│                    FLUJO DE EJECUCIÓN                            │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│   kernel_main()                                                 │
│       pmm_init()                                                │
│       gdt_init()                                                │
│       idt_init()                                                │
│       paging_init()                                             │
│       tss_init()          // TSS para ring 3                   │
│       process_init()      // tabla vacía                       │
│                                                                 │
│       // Crear hello                                            │
│       pcb_t* hello = process_create("hello", 0x40000000);      │
│                                                                 │
│       // Copiar hello bin a 0x40000000                         │
│       memcpy((void*)0x40000000, hello_bin, hello_bin_size);    │
│                                                                 │
│       // Cambiar a modo usuario                                 │
│       process_start(hello);                                     │
│           // 1. Setup user stack                                │
│           // 2. ltr(TSS_SELECTOR)                              │
│           // 3. far jump a 0x40000000 con CPL=3                 │
│                                                                 │
│       // Hello corre en ring 3                                  │
│           ...                                                   │
│           int 0x80  // syscall exit                             │
│                                                                 │
│       // syscall_handler:                                       │
│           hello->eip = saved_eip;                               │
│           hello->state = PROC_EXITED;                          │
│                                                                 │
│       // Volver al kernel                                       │
│       kernel_halt();                                            │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

## Estructura de archivos

```
src/kernel/process/
├── process.h      // PCB, process_table_t, funciones públicas
├── process.c      // Implementación de process_*
├── context.S      // (placeholder para future context_switch)
└── syscall.c      // syscall_exit, syscall_handler

src/kernel/cpu/
├── tss.c          // TSS setup (nuevo)
└── tss.h          // (nuevo)

programs/hello/
├── hello.c        // Fuente C (alternativo a binario embebido)
└── Makefile       // Compila a hello.bin
```

## Syscall interface

```
| Num | Nombre   | Args              | Descripción                    |
|-----|----------|-------------------|--------------------------------|
| 1   | exit     | eax = código      | Termina proceso, vuelve kernel|
```

## Riesgos

| Riesgo | Probabilidad | Impacto | Mitigación |
|--------|---------------|---------|------------|
| TSS mal configurado causa GPF | Media | Alto | Verificar GDT + TSS juntos; testear con QEMU |
| Paging no mapea 0x40000000 | Baja | Alto | Verificar paging_init() mapea esa dirección |
| Hello corrupta kernel stack | Baja | Alto | Hello tiene stack separado en user space |
| Modo usuario no tiene permisos | Baja | Alto | Page tables deben tener U/S=1 para páginas usuario |

## Non-Goals (reconocimiento explícito)

- No se implementa `context_switch.S` — queda como stub para futuro
- No se implementa scheduler loop — el kernel halt tras exit
- No se implementa `process_wait()` — zombie collection queda para después
- No se implementa PID recycling — simple increment es suficiente

## Open Questions

1. **¿Usar hello.c compilado o binario embebido?**
   - Embebido: más simple, evita dependencia de cross-compiler
   - hello.c: más legible, demo más real
   
   Decisión pendiente.

2. **¿Stack de hello en qué dirección?**
   - Fija: 0xBFFFF000 - 4096 (arriba del kernel stack)
   - Dinámica: asignar frame del PMM
   
   Decisión pendiente.
