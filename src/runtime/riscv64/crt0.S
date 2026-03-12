/*
 * RISC-V 64 C runtime startup for custom OS
 * Entry point: _start → calls main() → sys_exit(retval)
 */

    .section .text
    .globl _start
_start:
    /* Call main() */
    call    main

    /* Exit with main's return value (already in a0) */
    li      a7, 1               /* NR_EXIT = 1 */
    ecall
    j       _start              /* Should not reach here */

    .section .note.GNU-stack,"",@progbits
