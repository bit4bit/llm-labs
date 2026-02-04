ENTRY(kernel_main)

SECTIONS
{
    /* Higher-half kernel at 2MB */
    . = 0xC0100000;

    .text : AT(ADDR(.text) - 0xC0000000)
    {
        *(.text)
    }

    .rodata : AT(ADDR(.rodata) - 0xC0000000)
    {
        *(.rodata)
    }

    .data : AT(ADDR(.data) - 0xC0000000)
    {
        *(.data)
    }

    .bss : AT(ADDR(.bss) - 0xC0000000)
    {
        *(.bss)
        *(COMMON)
    }

    end = .; _end = .; __end = .;
}