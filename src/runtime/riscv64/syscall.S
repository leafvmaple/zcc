/*
 * RISC-V 64 syscall stubs for custom OS
 *
 * Syscall convention:
 *   a7 = syscall number
 *   a0 = arg1, a1 = arg2, a2 = arg3
 *   Trap via: ecall
 */

#include "syscall.h"

    .section .text

/* long sys_read(int fd, void *buf, long count) */
    .globl sys_read
sys_read:
    li      a7, NR_READ
    ecall
    ret

/* long sys_write(int fd, const void *buf, long count) */
    .globl sys_write
sys_write:
    li      a7, NR_WRITE
    ecall
    ret

/* int sys_open(const char *path, int flags) */
    .globl sys_open
sys_open:
    li      a7, NR_OPEN
    ecall
    ret

/* int sys_close(int fd) */
    .globl sys_close
sys_close:
    li      a7, NR_CLOSE
    ecall
    ret

/* void sys_exit(int code) */
    .globl sys_exit
sys_exit:
    li      a7, NR_EXIT
    ecall
    j       sys_exit

/* int sys_pause(void) */
    .globl sys_pause
sys_pause:
    li      a7, NR_PAUSE
    ecall
    ret

    .section .note.GNU-stack,"",@progbits
