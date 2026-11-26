#include "syscall.h"
.section .text
.globl _start
_start:
    mov x29, xzr
    ldr x0, [sp]
    add x1, sp, #8
    bl main
    mov x8, #NR_EXIT
    svc #0
    brk #0
.section .note.GNU-stack,"",%progbits
