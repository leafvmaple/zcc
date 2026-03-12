/*
 * x86_64 syscall stubs for custom OS
 *
 * Syscall convention (matching user's custom OS):
 *   rax = syscall number
 *   rdi = arg1, rsi = arg2, rdx = arg3
 *   Trap via: int $0x80
 */

    .section .text

/* long sys_write(int fd, const void *buf, long count)
 * C calling convention: rdi=fd, rsi=buf, rdx=count
 * These already match the syscall register layout.
 */
    .globl sys_write
sys_write:
    movq    $4, %rax            /* NR_WRITE = 4 */
    int     $0x80
    ret

/* void sys_exit(int code)
 * C calling convention: rdi=code
 */
    .globl sys_exit
sys_exit:
    movq    $1, %rax            /* NR_EXIT = 1 */
    int     $0x80
    /* Should not return, but just in case */
    hlt

    .section .note.GNU-stack,"",@progbits
