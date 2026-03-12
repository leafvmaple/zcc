/* Minimal linker script for custom OS — RISC-V 64 */

ENTRY(_start)

SECTIONS {
    . = 0x80000000;

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
