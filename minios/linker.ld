OUTPUT_FORMAT("elf64-x86-64")
ENTRY(_start)

SECTIONS
{
    /* Place Multiboot2 header at the very start of file */
    .multiboot2_header ALIGN(8) : {
        KEEP(*(.multiboot2_header))
    }

    /* Then start actual code at 0x100000 in memory */
    . = 0x100000;

    .text ALIGN(16) : { *(.text*) }
    .rodata ALIGN(16) : { *(.rodata*) }
    .data ALIGN(16) : { *(.data*) }
    .bss ALIGN(4096) : { *(.bss*) }
}
