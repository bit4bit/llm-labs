OUTPUT_FORMAT("binary")
OUTPUT_ARCH("i386")
ENTRY(_start)

SECTIONS
{
    /* Per-process base address: set via --defsym=_user_base=<addr>
     * Defaults to 0x40000000 (process 0) if not defined. */
    . = DEFINED(_user_base) ? _user_base : 0x40000000;

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
