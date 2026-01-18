# Change: Add VGA text output with colored "hola mundo"

## Why
Verify the kernel boots correctly and Multiboot integration works by displaying text on screen. Each character in "hola mundo" will have a different color to test VGA write functionality.

## What Changes
- Add VGA text mode driver (write to 0xB8000 memory)
- Create `vga.c` with `vga_putchar()` and `vga_write()`
- Add `main.c` that prints "hola mundo" with rainbow colors
- Update `Makefile` to compile VGA driver

## Impact
- Affected specs: `vga-output` (new capability)
- Affected code:
  - `kernel/vga.c` (new)
  - `kernel/main.c` (modified)
  - `Makefile` (modified)
