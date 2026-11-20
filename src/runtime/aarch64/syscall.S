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
.globl sys_ioctl
sys_ioctl:
    mov x8, #NR_IOCTL
    svc #0
    ret
.globl sys_pipe2
sys_pipe2:
    mov x8, #NR_PIPE2
    svc #0
    ret
.globl sys_wait_readable
sys_wait_readable:
    mov x8, #NR_WAIT_READABLE
    svc #0
    ret
.globl sys_uptime_ms
sys_uptime_ms:
    mov x8, #NR_UPTIME_MS
    svc #0
    ret
.globl sys_realtime
sys_realtime:
    mov x8, #NR_REALTIME
    svc #0
    ret
.globl sys_brk
sys_brk:
    mov x8, #NR_BRK
    svc #0
    ret
.globl sys_channel_listen
sys_channel_listen:
    mov x8, #NR_CHANNEL_LISTEN
    svc #0
    ret
.globl sys_channel_connect
sys_channel_connect:
    mov x8, #NR_CHANNEL_CONNECT
    svc #0
    ret
.globl sys_channel_accept
sys_channel_accept:
    mov x8, #NR_CHANNEL_ACCEPT
    svc #0
    ret
.globl sys_wait_readable_for
sys_wait_readable_for:
    mov x8, #NR_WAIT_READABLE_FOR
    svc #0
    ret
.globl sys_getrandom
sys_getrandom:
    mov x8, #NR_GETRANDOM
    svc #0
    ret
.globl sys_wait_io
sys_wait_io:
    mov x8, #NR_WAIT_IO
    svc #0
    ret
.globl sys_openpty
sys_openpty:
    mov x8, #NR_OPENPTY
    svc #0
    ret
.globl sys_dup2
sys_dup2:
    mov x8, #NR_DUP2
    svc #0
    ret
.globl sys_fcntl
sys_fcntl:
    mov x8, #NR_FCNTL
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
.globl sys_spawn_foreground
sys_spawn_foreground:
    mov x8, #NR_SPAWN_FOREGROUND
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
.globl sys_process_next
sys_process_next:
    mov x8, #NR_PROCESS_NEXT
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
.globl sys_mkdir
sys_mkdir:
    mov x8, #NR_MKDIR
    svc #0
    ret
.globl sys_rename
sys_rename:
    mov x8, #NR_RENAME
    svc #0
    ret
.globl sys_unlink
sys_unlink:
    mov x8, #NR_UNLINK
    svc #0
    ret
.globl sys_rmdir
sys_rmdir:
    mov x8, #NR_RMDIR
    svc #0
    ret
.globl sys_sync
sys_sync:
    mov x8, #NR_SYNC
    svc #0
    ret
.globl sys_shutdown
sys_shutdown:
    mov x8, #NR_SHUTDOWN
    svc #0
    ret
.globl sys_fstat
sys_fstat:
    mov x8, #NR_FSTAT
    svc #0
    ret
.globl sys_ftruncate
sys_ftruncate:
    mov x8, #NR_FTRUNCATE
    svc #0
    ret
.section .note.GNU-stack,"",%progbits
