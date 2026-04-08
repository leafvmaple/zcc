/*
 * x86_64 syscall stubs for custom OS
 *
 * Syscall convention (matching user's custom OS):
 *   rax = syscall number
 *   rdi = arg1, rsi = arg2, rdx = arg3
 *   Trap via: int $0x80
 */

#include "syscall.h"

    .section .text

/* long sys_read(int fd, void *buf, long count) */
    .globl sys_read
sys_read:
    movq    $NR_READ, %rax
    int     $0x80
    ret

/* long sys_write(int fd, const void *buf, long count) */
    .globl sys_write
sys_write:
    movq    $NR_WRITE, %rax
    int     $0x80
    ret

/* int sys_open(const char *path, int flags) */
    .globl sys_open
sys_open:
    movq    $NR_OPEN, %rax
    int     $0x80
    ret

/* int sys_close(int fd) */
    .globl sys_close
sys_close:
    movq    $NR_CLOSE, %rax
    int     $0x80
    ret

/* void sys_exit(int code) */
    .globl sys_exit
sys_exit:
    movq    $NR_EXIT, %rax
    int     $0x80
    hlt

/* int sys_pause(void) */
    .globl sys_pause
sys_pause:
    movq    $NR_PAUSE, %rax
    int     $0x80
    ret

    .section .note.GNU-stack,"",@progbits
