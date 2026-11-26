/*
 * User-mode linker script for Zonix OS (64-bit targets)
 *
 * Programs are loaded at low virtual addresses (0x400000) — well below
 * the kernel's higher-half mapping.  The ELF loader creates PT_LOAD
 * segments from .text / .rodata / .data / .bss and maps them with
 * PTE_U so they are accessible from ring 3.
 */

ENTRY(_start)

USER_BASE = 0x400000;

SECTIONS {
    . = USER_BASE;

    .text : {
        *(.text .text.*)
    }

    .rodata : {
        *(.rodata .rodata.*)
    }

    . = ALIGN(0x1000);
    .data : {
        *(.data .data.*)
    }

    .bss : {
        *(.bss .bss.*)
        *(COMMON)
    }

    /DISCARD/ : {
        *(.note.GNU-stack)
        *(.comment)
        *(.eh_frame)
    }
}
