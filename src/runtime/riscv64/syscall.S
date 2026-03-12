/*
 * RISC-V 64 syscall stubs for custom OS
 *
 * Syscall convention:
 *   a7 = syscall number
 *   a0 = arg1, a1 = arg2, a2 = arg3
 *   Trap via: ecall
 */

    .section .text

/* long sys_write(int fd, const void *buf, long count)
 * C calling convention: a0=fd, a1=buf, a2=count
 * These already match the syscall register layout.
 */
    .globl sys_write
sys_write:
    li      a7, 4               /* NR_WRITE = 4 */
    ecall
    ret

/* void sys_exit(int code)
 * C calling convention: a0=code
 */
    .globl sys_exit
sys_exit:
    li      a7, 1               /* NR_EXIT = 1 */
    ecall
    /* Should not return */
    j       sys_exit

    .section .note.GNU-stack,"",@progbits
