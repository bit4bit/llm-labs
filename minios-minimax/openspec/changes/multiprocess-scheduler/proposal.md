## Why

El scheduler actual puede lanzar un proceso por primera vez pero no puede hacer context switch real entre procesos ya en ejecucion. `scheduler_switch()` es un stub que solo hace `ret`, el guardado de registros en el timer handler captura valores post-interrupcion (incorrectos), y toda la memoria de usuario (codigo y stack) es un solo mapping compartido. Para correr multiples programas diferentes concurrentemente se necesita context switching funcional, stacks separados, y regiones de codigo independientes.

## What Changes

- **Implementar context switch real via stack switching**: guardar ESP del kernel stack en el PCB del proceso saliente, restaurar ESP del PCB del proceso entrante. El kernel stack ya contiene el frame completo (pushal + iret frame), no se necesita copiar registro por registro.
- **Kernel stacks separados por proceso**: cada proceso recibe su propio kernel stack de 4KB. Al crear un proceso se prepara un "fake interrupt frame" en su kernel stack para que el primer switch lo lance a user mode.
- **Eliminar guardado registro-por-registro en timer_handler**: el `pushal` en `timer_handler_asm` ya preserva los registros correctos en el stack. Solo se necesita guardar ESP en el PCB.
- **Regiones de codigo de usuario separadas por proceso**: cada proceso se carga en un PDE diferente (PDE 256, 257, 258, 259) con su propio frame fisico. El linker script se parametriza para generar binarios en la direccion correcta.
- **User stacks separados por proceso**: cada proceso recibe su propio PDE para stack (PDE 767, 766, 765, 764) con frame fisico independiente.
- **Actualizar TSS.ESP0 en cada context switch**: apuntar al kernel stack del proceso entrante.
- **BREAKING**: el PCB cambia de estructura (nuevos campos `kernel_esp`, `kernel_stack_top`; se pueden eliminar los campos individuales de registros).
- **BREAKING**: el linker script de programas se parametriza; cada programa se compila para una direccion virtual diferente.

## Capabilities

### New Capabilities
- `context-switch`: mecanismo de cambio de contexto entre procesos via stack switching en assembly. Incluye guardado/restauracion de ESP, actualizacion de TSS, y soporte para "fake interrupt frame" en procesos nuevos.
- `per-process-memory`: asignacion de regiones de memoria independientes (codigo y stack) por proceso usando PDEs separados. Incluye parametrizacion del linker script para direcciones de carga por proceso.

### Modified Capabilities
- `preemptive-scheduler`: el timer handler cambia de guardar registros individualmente a guardar solo ESP. El scheduler usa el context switch real en vez del stub.
- `process-management`: el PCB se extiende con `kernel_esp` y `kernel_stack_top`. `process_create` asigna kernel stack y prepara fake interrupt frame. Se elimina la distincion entre "primera ejecucion" y "continuacion".

## Impact

- **Kernel**: `process.h` (PCB), `process.c` (scheduler, create, start), `trampoline.S` (scheduler_switch), `interrupts.c` (timer handler), `vmm.c` (mappings por proceso)
- **Build system**: `programs/user.ld` parametrizado, `tools/build-programs.rb` modificado para pasar direccion base por programa
- **Tests**: selfcheck debe seguir pasando; se necesita un nuevo test que demuestre dos programas corriendo concurrentemente con output intercalado
- **Compatibilidad**: cambios breaking en la estructura del PCB y en el build de programas
