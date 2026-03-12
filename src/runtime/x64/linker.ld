/* Minimal linker script for custom OS — x86_64 */

ENTRY(_start)

SECTIONS {
    . = 0x400000;

    .text : {
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
    }
}
