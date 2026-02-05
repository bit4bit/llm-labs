## Why

El proyecto MiniOS está definido en `project.md` pero no existe código base. Sin un Makefile funcional y los archivos mínimos de boot, no es posible compilar ni ejecutar el sistema operativo en QEMU. Este change establece la base para todo desarrollo futuro.

## What Changes

- Crear `Makefile` con targets para compilar kernel, crear ISO, ejecutar en QEMU
- Crear `src/kernel/boot/multiboot.S`: Header multiboot para que GRUB reconozca el kernel
- Crear `src/kernel/boot/boot.s`: Bootstrap que entra en modo protegido de 32 bits
- Crear `src/kernel/main.c`: Entry point del kernel que imprime "MinOS Loaded" en VGA
- Crear `link.ld`: Linker script para posicionar el kernel en dirección válida
- Crear estructura de directorios según `project.md`
- Crear imagen ISO bootable con GRUB

## Capabilities

### New Capabilities
- `kernel-boot`: Capacidad del kernel para bootear via GRUB multiboot y mostrar mensaje inicial

## Impact

- **Archivos nuevos**: `Makefile`, `link.ld`, `src/kernel/boot/multiboot.S`, `src/kernel/boot/boot.s`, `src/kernel/main.c`
- **Herramientas requeridas**: gcc-multilib (i686-linux-gnu), grub, xorriso, qemu-system-i386
- **Dependencias inversas**: Todos los demás componentes del kernel dependen de esta base
