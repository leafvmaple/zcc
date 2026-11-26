#include "syscall.h"
.section .text
.globl sys_read
sys_read:
    li a7, NR_READ
    ecall
    ret
.globl sys_write
sys_write:
    li a7, NR_WRITE
    ecall
    ret
.globl sys_open
sys_open:
    li a2, 0
    li a7, NR_OPEN
    ecall
    ret
.globl sys_close
sys_close:
    li a7, NR_CLOSE
    ecall
    ret
.globl sys_ioctl
sys_ioctl:
    li a7, NR_IOCTL
    ecall
    ret
.globl sys_pipe2
sys_pipe2:
    li a7, NR_PIPE2
    ecall
    ret
.globl sys_wait_readable
sys_wait_readable:
    li a7, NR_WAIT_READABLE
    ecall
    ret
.globl sys_uptime_ms
sys_uptime_ms:
    li a7, NR_UPTIME_MS
    ecall
    ret
.globl sys_realtime
sys_realtime:
    li a7, NR_REALTIME
    ecall
    ret
.globl sys_brk
sys_brk:
    li a7, NR_BRK
    ecall
    ret
.globl sys_channel_listen
sys_channel_listen:
    li a7, NR_CHANNEL_LISTEN
    ecall
    ret
.globl sys_channel_connect
sys_channel_connect:
    li a7, NR_CHANNEL_CONNECT
    ecall
    ret
.globl sys_channel_accept
sys_channel_accept:
    li a7, NR_CHANNEL_ACCEPT
    ecall
    ret
.globl sys_wait_readable_for
sys_wait_readable_for:
    li a7, NR_WAIT_READABLE_FOR
    ecall
    ret
.globl sys_getrandom
sys_getrandom:
    li a7, NR_GETRANDOM
    ecall
    ret
.globl sys_wait_io
sys_wait_io:
    li a7, NR_WAIT_IO
    ecall
    ret
.globl sys_openpty
sys_openpty:
    li a7, NR_OPENPTY
    ecall
    ret
.globl sys_dup2
sys_dup2:
    li a7, NR_DUP2
    ecall
    ret
.globl sys_fcntl
sys_fcntl:
    li a7, NR_FCNTL
    ecall
    ret
.globl sys_exit
sys_exit:
    li a7, NR_EXIT
    ecall
    unimp
.globl sys_pause
sys_pause:
    li a7, NR_PAUSE
    ecall
    ret
.globl sys_spawn
sys_spawn:
    li a7, NR_SPAWN
    ecall
    ret
.globl sys_spawn_foreground
sys_spawn_foreground:
    li a7, NR_SPAWN_FOREGROUND
    ecall
    ret
.globl sys_wait
sys_wait:
    li a7, NR_WAIT
    ecall
    ret
.globl sys_kill
sys_kill:
    li a7, NR_KILL
    ecall
    ret
.globl sys_process_next
sys_process_next:
    li a7, NR_PROCESS_NEXT
    ecall
    ret
.globl sys_getpid
sys_getpid:
    li a7, NR_GETPID
    ecall
    ret
.globl sys_yield
sys_yield:
    li a7, NR_YIELD
    ecall
    ret
.globl sys_stat
sys_stat:
    li a7, NR_STAT
    ecall
    ret
.globl sys_readdir
sys_readdir:
    li a7, NR_READDIR
    ecall
    ret
.globl sys_chdir
sys_chdir:
    li a7, NR_CHDIR
    ecall
    ret
.globl sys_getcwd
sys_getcwd:
    li a7, NR_GETCWD
    ecall
    ret
.globl sys_mkdir
sys_mkdir:
    li a7, NR_MKDIR
    ecall
    ret
.globl sys_rename
sys_rename:
    li a7, NR_RENAME
    ecall
    ret
.globl sys_unlink
sys_unlink:
    li a7, NR_UNLINK
    ecall
    ret
.globl sys_rmdir
sys_rmdir:
    li a7, NR_RMDIR
    ecall
    ret
.globl sys_sync
sys_sync:
    li a7, NR_SYNC
    ecall
    ret
.globl sys_shutdown
sys_shutdown:
    li a7, NR_SHUTDOWN
    ecall
    ret
.globl sys_reboot
sys_reboot:
    li a7, NR_REBOOT
    ecall
    ret
.globl sys_set_priority
sys_set_priority:
    li a7, NR_SET_PRIORITY
    ecall
    ret
.globl sys_fstat
sys_fstat:
    li a7, NR_FSTAT
    ecall
    ret
.globl sys_ftruncate
sys_ftruncate:
    li a7, NR_FTRUNCATE
    ecall
    ret
.section .note.GNU-stack,"",@progbits
