#include "syscall.h"
.section .text
.globl sys_read
sys_read:
    mov x8, #NR_READ
    svc #0
    ret
.globl sys_write
sys_write:
    mov x8, #NR_WRITE
    svc #0
    ret
.globl sys_open
sys_open:
    mov x2, xzr
    mov x8, #NR_OPEN
    svc #0
    ret
.globl sys_close
sys_close:
    mov x8, #NR_CLOSE
    svc #0
    ret
.globl sys_exit
sys_exit:
    mov x8, #NR_EXIT
    svc #0
    brk #0
.globl sys_pause
sys_pause:
    mov x8, #NR_PAUSE
    svc #0
    ret
.globl sys_spawn
sys_spawn:
    mov x8, #NR_SPAWN
    svc #0
    ret
.globl sys_wait
sys_wait:
    mov x8, #NR_WAIT
    svc #0
    ret
.globl sys_kill
sys_kill:
    mov x8, #NR_KILL
    svc #0
    ret
.globl sys_getpid
sys_getpid:
    mov x8, #NR_GETPID
    svc #0
    ret
.globl sys_yield
sys_yield:
    mov x8, #NR_YIELD
    svc #0
    ret
.globl sys_stat
sys_stat:
    mov x8, #NR_STAT
    svc #0
    ret
.globl sys_readdir
sys_readdir:
    mov x8, #NR_READDIR
    svc #0
    ret
.globl sys_chdir
sys_chdir:
    mov x8, #NR_CHDIR
    svc #0
    ret
.globl sys_getcwd
sys_getcwd:
    mov x8, #NR_GETCWD
    svc #0
    ret
.section .note.GNU-stack,"",%progbits
