#include "syscall.h"
.section .text
.globl _start
_start:
    ld a0, 0(sp)
    addi a1, sp, 8
    call main
    li a7, NR_EXIT
    ecall
    unimp
.section .note.GNU-stack,"",@progbits
