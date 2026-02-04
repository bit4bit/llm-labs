ENTRY(_start)

SECTIONS
{
    /* Begin at 1MB */
    . = 1M;

    .text : ALIGN(4K)
    {
        *(.multiboot)
        *(.text)
    }

    .rodata : ALIGN(4K)
    {
        *(.rodata)
    }

    .data : ALIGN(4K)
    {
        *(.data)
    }

    .bss : ALIGN(4K)
    {
        *(.bss)
        *(COMMON)
    }

    end = .; _end = .; __end = .;
}