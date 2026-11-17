#include "syscall.h"
.section .text
.globl _start
_start:
    xorq %rbp, %rbp
    movq (%rsp), %rdi
    leaq 8(%rsp), %rsi
    call main
    movl %eax, %edi
    movq $NR_EXIT, %rax
    int $0x80
    ud2
.section .note.GNU-stack,"",@progbits
