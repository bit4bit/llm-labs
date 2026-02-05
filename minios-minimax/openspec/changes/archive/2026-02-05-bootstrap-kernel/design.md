## Context

MiniOS es un proyecto greenfield definido en `project.md`. El objetivo inmediato es establecer la infraestructura de build y los archivos mínimos para que el kernel pueda bootear via GRUB y mostrar un mensaje inicial. El proyecto usa gcc-multilib para compilar código de 32 bits nativo en un sistema de 64 bits.

## Goals / Non-Goals

**Goals:**
- Makefile funcional con targets: build, iso, qemu, clean
- Kernel compilable que pasa el header multiboot de GRUB
- Transición exitosa a modo protegido de 32 bits
- kernel.bin que muestra "MinOS Loaded" en pantalla VGA
- ISO bootable mediante GRUB

**Non-Goals:**
- Soporte de paging o memoria virtual
- Sistema de archivos
- Drivers de teclado o disco
- Syscalls o gestión de procesos
- Multiboot2 o bootloader propio

## Decisions

| Decisión | Alternativas | Justificación |
|----------|--------------|---------------|
| GRUB legacy (multiboot1) | GRUB2, custom bootloader | Simplicidad, header mínimo, bien documentado |
| Modo protegido 32-bit | Modo real 16-bit, long mode 64-bit | gcc-multilib facilita 32-bit, suficiente para educativo |
| Formato ELF | flat binary, COFF | Soporte nativo de GRUB, permite segmentación |
| Memoria baja (1MB) | Alta memoria (>1MB) | Compatibilidad máxima con BIOS, simple |

```
┌─────────────────────────────────────────────────────────────┐
│                    MEMORY MAP (1MB)                         │
├─────────────────────────────────────────────────────────────┤
│ 0x00000 - 0x00400       │ IVT (Interrupt Vector Table)     │
│ 0x00400 - 0x00500       │ BDA (BIOS Data Area)             │
│ 0x07C00 - 0x07E00       │ Bootloader (MBR)                 │
│ 0x07E00 - 0x80000       │ свободное пространство          │
│ 0x100000 - ...          │ Kernel (cargado por GRUB)        │
├─────────────────────────────────────────────────────────────┤
│ GRUB carga kernel en dirección > 1MB según header multiboot │
└─────────────────────────────────────────────────────────────┘
```

## Risks / Trade-offs

| Riesgo | Impacto | Mitigación |
|--------|---------|------------|
| gcc-multilib no produce código relocalizable | Kernel no carga | Usar `-fno-pie -no-pie` en flags |
| Header multiboot mal alineado | GRUB no carga kernel | Asegurar alineación de 4 bytes en linker script |
| Modo VGA texto vs gráfico | Texto no visible | Usar modo texto estándar (0x03), escribir en 0xB8000 |
