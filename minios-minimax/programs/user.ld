OUTPUT_FORMAT("binary")
OUTPUT_ARCH("i386")
ENTRY(_start)

SECTIONS
{
    . = 0x40000000;  /* User program base address */

    .text : {
        *(.text.startup)
        *(.text)
    }

    .rodata : {
        *(.rodata*)
    }

    .data : {
        *(.data)
    }

    .bss : {
        *(.bss)
        *(COMMON)
    }

    /* Discard unnecessary sections */
    /DISCARD/ : {
        *(.comment)
        *(.note*)
        *(.eh_frame)
        *(.eh_frame_hdr)
    }
}
