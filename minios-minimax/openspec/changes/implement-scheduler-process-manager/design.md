## Context

MiniOS actualmente puede cargar binarios ELF pero no tiene gestión de procesos ni scheduling. El kernel inicia un proceso y permanece en él indefinidamente. Esta implementación añade multitasking cooperativo/ preemptive básico mediante:

- Driver de timer PIT para interrupciones periódicas
- Gestor de procesos con PCB y estados
- Scheduler round-robin con context switching
- Loader de binario plano con symbol table para memoria fija

## Goals / Non-Goals

**Goals:**
- Implementar timer interrupt driver simple usando PIT
- Crear estructura PCB con registros de CPU, estado, y metadata
- Implementar scheduler round-robin con time slice configurable
- Asignar direcciones de memoria fijas a programas via linker script
- Mantener compatibilidad con binarios ELF existentes

**Non-Goals:**
- Scheduling prioritario o multinivel
- Memoria virtual por proceso (comparten espacio del kernel)
- Señales o IPC
- Loading dinámico de programas
- Suspensión/resume de procesos

## Decisions

### 1. Timer Driver: PIT Channel 0

**Decision:** Usar PIT (Programmable Interval Timer) canal 0 con IRQ0

**Rationale:**
- Hardware disponible en todas las PCs compatibles
- Configuración simple con tres registros de datos
- No requiere APIC o HPET (más complejo)
- Compatible con modo protegido 32-bit

**Alternativas consideradas:**
- APIC timer: Requiere configuración APIC más compleja
- RTC: Resolución de 1Hz mínimo, demasiado lento

```c
// Puerto PIT
#define PIT_PORT_DATA    0x40
#define PIT_PORT_COMMAND 0x43

// Comandos
#define PIT_CMD_CHANNEL0 0x00
#define PIT_CMD_LATCH    0x00
#define PIT_CMD_MODE3   0x06  // Square wave generator
#define PIT_CMD_BOTH    0x30  // Access low/high byte
```

### 2. Process Control Block (PCB)

**Decision:** PCB con campos mínimos esenciales

```c
struct pcb {
    uint32_t id;
    uint32_t esp;        // Stack pointer
    uint32_t ebp;        // Frame pointer
    uint32_t eip;        // Instruction pointer
    uint32_t cr3;        // Page directory base
    uint8_t state;       // READY, RUNNING, BLOCKED
    uint8_t* stack;      // Kernel stack
    struct pcb* next;    // Linked list for scheduler
};
```

**Rationale:**
- Solo almacenar registros esenciales para context switch
- Linked list para scheduler O(1) circular
- cr3 permite快速的 page table switch

### 3. Scheduler: Round-Robin Simple

**Decision:** Lista circular con tick counter

```c
void scheduler_tick(void) {
    current_process->time_slice--;
    if (current_process->time_slice == 0) {
        switch_context();
    }
}
```

**Rationale:**
- Implementación más simple posible
- Time slice fijo (ej: 100 ticks = ~10ms)
- No requiere priority queues

### 4. Binary Format: Flat Binary with Symbol Table

**Decision:** Usar binarios planos compilados a dirección fija con symbol table en kernel

```c
// Program symbol table en kernel
struct program {
    const char* name;
    uint32_t base_addr;
    uint32_t size;
};

struct program programs[] = {
    {"hello", 0x40000000, 4096},
    {"shell", 0x40010000, 8192},
};
```

**Loader:**
```c
void load_program(const char* name) {
    struct program* p = find_program(name);
    read floppy/sector to p->base_addr;
}
```

**Rationale:**
- Elimina complejidad de parsing ELF
- Solo memcpy desde almacenamiento a memoria
- Cada programa conoce su dirección al compilar

**Alternativas consideradas:**
- ELF: Parsing complejo, relocations, secciones
- Header binario: Require build step extra
- Pre-linked: Similar pero sin symbol table centralizado

## Risks / Trade-offs

| Risk | Mitigation |
|------|------------|
| Stack overflow si programa excede espacio | límite de stack configurable |
| Solo 2GB direcciones usuario (0x40000000-0x7fffffff) | Suficiente para sistema educativo |
| Context switch sin save/restore completo | Solo esenciales; suficiente para simple programs |
| Timer interrupts pueden perderse bajo carga | Time slice conservador (~10ms) |

## Migration Plan

1. Crear driver PIT básico con interrupt handler
2. Implementar PCB con free list para hasta 16 procesos
3. Crear programs.c con symbol table (hello, shell addresses)
4. Implementar flat binary loader en binary.c
5. Añadir scheduler tick al PIT interrupt handler
6. Implementar context switch: save regs → switch stacks → restore regs → iret
7. Modificar main.c para inicializar scheduler y arrancar primer proceso
8. Actualizar Makefiles para compilar programas como binarios planos (-nostdlib -Wl,--entry=main -Wl,--format=binary)
