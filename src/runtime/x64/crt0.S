/*
 * x86_64 C runtime startup for custom OS
 * Entry point: _start → calls main() → sys_exit(retval)
 */

    .section .text
    .globl _start
_start:
    /* Clear frame pointer for clean stack traces */
    xorq    %rbp, %rbp

    /* Call main() */
    call    main

    /* Exit with main's return value */
    movq    %rax, %rdi          /* exit code = main() return value */
    movq    $1, %rax            /* NR_EXIT = 1 */
    int     $0x80
    hlt

    .section .note.GNU-stack,"",@progbits
