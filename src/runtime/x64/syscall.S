#include "syscall.h"
.section .text
.globl sys_read
sys_read:
    mov $NR_READ, %eax
    int $0x80
    ret
.globl sys_write
sys_write:
    mov $NR_WRITE, %eax
    int $0x80
    ret
.globl sys_open
sys_open:
    xor %edx, %edx
    mov $NR_OPEN, %eax
    int $0x80
    ret
.globl sys_close
sys_close:
    mov $NR_CLOSE, %eax
    int $0x80
    ret
.globl sys_exit
sys_exit:
    mov $NR_EXIT, %eax
    int $0x80
    ud2
.globl sys_pause
sys_pause:
    mov $NR_PAUSE, %eax
    int $0x80
    ret
.globl sys_spawn
sys_spawn:
    mov $NR_SPAWN, %eax
    int $0x80
    ret
.globl sys_spawn_foreground
sys_spawn_foreground:
    mov $NR_SPAWN_FOREGROUND, %eax
    int $0x80
    ret
.globl sys_wait
sys_wait:
    mov $NR_WAIT, %eax
    int $0x80
    ret
.globl sys_kill
sys_kill:
    mov $NR_KILL, %eax
    int $0x80
    ret
.globl sys_process_next
sys_process_next:
    mov $NR_PROCESS_NEXT, %eax
    int $0x80
    ret
.globl sys_getpid
sys_getpid:
    mov $NR_GETPID, %eax
    int $0x80
    ret
.globl sys_yield
sys_yield:
    mov $NR_YIELD, %eax
    int $0x80
    ret
.globl sys_stat
sys_stat:
    mov $NR_STAT, %eax
    int $0x80
    ret
.globl sys_readdir
sys_readdir:
    mov $NR_READDIR, %eax
    int $0x80
    ret
.globl sys_chdir
sys_chdir:
    mov $NR_CHDIR, %eax
    int $0x80
    ret
.globl sys_getcwd
sys_getcwd:
    mov $NR_GETCWD, %eax
    int $0x80
    ret
.globl sys_mkdir
sys_mkdir:
    mov $NR_MKDIR, %eax
    int $0x80
    ret
.globl sys_unlink
sys_unlink:
    mov $NR_UNLINK, %eax
    int $0x80
    ret
.globl sys_rmdir
sys_rmdir:
    mov $NR_RMDIR, %eax
    int $0x80
    ret
.globl sys_sync
sys_sync:
    mov $NR_SYNC, %eax
    int $0x80
    ret
.globl sys_shutdown
sys_shutdown:
    mov $NR_SHUTDOWN, %eax
    int $0x80
    ret
.globl sys_fstat
sys_fstat:
    mov $NR_FSTAT, %eax
    int $0x80
    ret
.globl sys_ftruncate
sys_ftruncate:
    mov $NR_FTRUNCATE, %eax
    int $0x80
    ret
.section .note.GNU-stack,"",@progbits
