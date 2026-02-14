## Context

MiniOS tiene un scheduler round-robin que invoca `scheduler()` en cada tick del timer (100Hz). Puede lanzar un proceso nuevo via `enter_user_mode()` pero `scheduler_switch()` es un stub (`ret`). El timer handler guarda registros del contexto kernel (post-interrupcion), no los valores originales del usuario. Hay un solo mapping de memoria para codigo (`PDE 256 → 0x40000000`) y stack (`PDE 767 → 0xBFC00000`) de usuario, compartido por todos los procesos. Los programas se compilan como flat binary a direccion fija `0x40000000`.

Estado actual de archivos relevantes:
- `process.h`: PCB con campos individuales para cada registro (eax, ebx, etc.)
- `process.c`: scheduler() con rama especial para primer arranque vs continuacion
- `trampoline.S`: `enter_user_mode()` funcional, `scheduler_switch()` stub
- `interrupts.c`: `timer_handler()` guarda registros uno por uno con inline asm
- `vmm.c`: un PDE para codigo de usuario, un PDE para stack de usuario
- `programs/user.ld`: direccion fija `. = 0x40000000`
- `tools/build-programs.rb`: misma configuracion de link para todos los programas

## Goals / Non-Goals

**Goals:**
- Ejecutar 2-4 programas diferentes concurrentemente con preemption real
- Context switch correcto: guardar y restaurar estado completo del proceso
- Cada proceso con su propia memoria de codigo y stack
- Un solo camino de codigo para arranque y continuacion de procesos (fake interrupt frame)
- Selfcheck existente sigue pasando

**Non-Goals:**
- Page directories separados por proceso (un solo page directory compartido)
- Paginas de 4KB (se mantienen paginas de 4MB PSE)
- Aislamiento de memoria entre procesos (todos ven el mismo espacio de direcciones)
- Prioridades, time slices variables, o algoritmos de scheduling avanzados
- Soporte para mas de MAX_PROCESSES (4) procesos

## Decisions

### D1: Stack switching en vez de register-copy

**Decision**: el context switch guarda solo `kernel_esp` en el PCB. El kernel stack ya contiene el frame completo (segment regs + pushal + iret frame). Para restaurar, se carga ESP del siguiente proceso y se ejecuta `pop segments + popal + iret`.

**Alternativa descartada**: copiar registros individualmente al PCB (como hace ahora). Esto es mas fragil, propenso a errores (los valores capturados son post-interrupcion), y requiere reconstruir el stack para el iret.

**Razon**: mas simple, mas correcto, y es el patron estandar en kernels educativos (xv6, JOS).

### D2: Kernel stacks como array estatico en BSS

**Decision**: declarar `static char kernel_stacks[MAX_PROCESSES][4096] __attribute__((aligned(4096)))` en process.c. El kernel stack del proceso `i` tiene su tope en `&kernel_stacks[i][4096]`.

**Alternativa descartada**: asignar kernel stacks dinamicamente via PMM. Esto introduce complejidad innecesaria dado que MAX_PROCESSES=4 y solo se necesitan 16KB totales.

**Razon**: simple, sin fragmentacion, sin fallo de asignacion posible. 16KB en BSS es insignificante.

### D3: Fake interrupt frame para procesos nuevos

**Decision**: cuando `process_create()` inicializa un proceso, prepara su kernel stack con un frame identico al que dejaria un timer interrupt: segment regs + pushal + iret frame (SS, ESP, EFLAGS, CS, EIP). El campo `kernel_esp` del PCB apunta al tope de este frame.

Esto elimina la rama especial `if (next->eip == 0)` en el scheduler. Todo proceso (nuevo o suspendido) se reanuda de la misma forma: restaurar ESP, pop, iret.

**Layout del fake frame** (stack crece hacia abajo, direcciones decrecientes):

```
kernel_stack_top (ej. &kernel_stacks[i][4096])
    ├── SS      = 0x23 (user data)
    ├── ESP     = user_stack_initial del proceso
    ├── EFLAGS  = 0x202 (IF=1, reserved bit 1)
    ├── CS      = 0x1B (user code)
    ├── EIP     = entry point del programa
    ├── EAX     = 0  ┐
    ├── ECX     = 0  │
    ├── EDX     = 0  │ pushal order
    ├── EBX     = 0  │
    ├── ESP_ign = 0  │ (ignorado por popal)
    ├── EBP     = 0  │
    ├── ESI     = 0  │
    ├── EDI     = 0  ┘
    ├── DS      = 0x23 (user data)
    ├── ES      = 0x23
    ├── FS      = 0x23
    └── GS      = 0x23  ◄── kernel_esp apunta aqui
```

### D4: PDEs separados para codigo y stack de usuario

**Decision**: cada proceso recibe un PDE diferente para codigo y otro para stack:

| Proceso | PDE codigo | Vaddr codigo  | PDE stack | Vaddr stack   |
|---------|-----------|---------------|-----------|---------------|
| 0       | 256       | 0x40000000    | 767       | 0xBFC00000    |
| 1       | 257       | 0x40400000    | 766       | 0xBF800000    |
| 2       | 258       | 0x40800000    | 765       | 0xBF400000    |
| 3       | 259       | 0x40C00000    | 764       | 0xBF000000    |

Cada PDE apunta a un frame fisico de 4MB asignado por PMM. Los mappings se crean durante `process_create()` o en `vmm_init()`.

**Alternativa descartada**: un page directory por proceso. Esto requiere duplicar las entradas del kernel en cada PD, cambiar CR3 en cada switch, y es significativamente mas complejo.

**Razon**: con un solo page directory, todos los procesos coexisten en el mismo espacio de direcciones. No hay aislamiento, pero es la solucion mas simple y suficiente para el objetivo.

### D5: Linker script parametrizado

**Decision**: cambiar `user.ld` para aceptar la direccion base como variable del linker:

```
. = DEFINED(_user_base) ? _user_base : 0x40000000;
```

El build script pasa la direccion por programa:

```
ld --defsym=_user_base=0x40400000 -T user.ld -o prog.bin prog.o
```

Se necesita un mecanismo para asignar direcciones a programas. La forma mas simple: un mapa en el build script o derivar la direccion del orden de descubrimiento.

**Alternativa descartada**: compilar como PIC (-fpic). Esto cambia el modelo de codigo, requiere GOT/PLT o relocaciones, y el formato flat binary no soporta relocaciones.

**Alternativa descartada**: un linker script por programa. Duplicacion innecesaria.

### D6: Reestructurar timer_handler_asm y scheduler_switch

**Decision**: el flujo del timer interrupt cambia a:

```
timer_handler_asm:
    pushal
    push ds, es, fs, gs
    mov $0x10, segments        # kernel data segments
    call timer_handler_c       # EOI, tick++, run_count++
    call scheduler             # elige next, hace el switch
    pop gs, fs, es, ds
    popal
    iret
```

`scheduler()` ahora:
1. Marca prev como READY
2. Busca next READY
3. Si next != prev: llama `scheduler_switch(prev, next)`
4. `scheduler_switch` (assembly) guarda ESP en prev->kernel_esp, carga ESP de next->kernel_esp, actualiza TSS.ESP0, retorna (el ret va al punto donde next fue suspendido, que es dentro de timer_handler_asm, que hace pop+iret)

Si next == prev, no hace nada y el flujo normal de pop+iret retoma el mismo proceso.

### D7: PCB simplificado

**Decision**: el PCB pierde los campos individuales de registros y gana:

```c
typedef struct {
    uint32_t id;
    uint32_t state;
    uint32_t entry;           // entry point address
    char name[32];
    uint32_t kernel_esp;      // saved ESP in kernel stack
    uint32_t kernel_stack_top; // top of this process's kernel stack
    uint32_t user_stack;      // user stack initial address
    uint32_t run_count;
} pcb_t;
```

Los campos `eip`, `esp`, `ebp`, `eax-edi`, `ds-gs` se eliminan porque el estado vive en el kernel stack, no en el PCB.

## Risks / Trade-offs

**[Sin aislamiento de memoria]** → Aceptado. Todos los procesos ven la memoria de los demas. Un proceso malicioso podria corromper otro. Mitigation: no es objetivo para esta fase.

**[Espacio de direcciones fijo por slot]** → Cada proceso tiene una direccion virtual fija basada en su indice en la process table. Si un programa espera estar en 0x40000000 pero se carga en 0x40400000, fallara. Mitigation: el linker script parametrizado asegura que cada binario se compila para su direccion correcta.

**[4MB por region de codigo]** → Cada proceso consume un frame de 4MB para codigo aunque use pocos KB. Mitigation: aceptable con MAX_PROCESSES=4 (16MB total). El PMM tiene cientos de frames disponibles.

**[Kernel stack de 4KB]** → Si un proceso anida muchas interrupciones o syscalls, podria desbordar su kernel stack. Mitigation: 4KB es estandar para kernels de 32-bit. Las funciones del kernel son pocas y no recursivas.

**[Build system acoplado a slots]** → El build script necesita saber que direccion asignar a cada programa. Si se reordena la tabla de procesos, las direcciones cambian. Mitigation: definir un mapa explicito programa→direccion en el build script o en un archivo de configuracion.

## Open Questions

- ¿Como se asigna la direccion de carga a cada programa en el build? ¿Mapa explicito en el build script, archivo de configuracion, o derivado del nombre del programa?
- ¿Se necesita actualizar `process_exit_return()` en main.c para manejar multiples procesos terminando en vez de halt inmediato?
- ¿El selfcheck actual necesita cambios para funcionar con el nuevo esquema de memoria, o puede seguir en el slot 0 (0x40000000) sin modificacion?
