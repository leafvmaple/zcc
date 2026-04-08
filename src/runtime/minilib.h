#ifndef _MINILIB_H
#define _MINILIB_H

/* Minimal C runtime for custom OS — no libc dependency */

long sys_read(int fd, void* buf, long count);
long sys_write(int fd, const void* buf, long count);
int sys_open(const char* path, int flags);
int sys_close(int fd);
void sys_exit(int code) __attribute__((noreturn));
int sys_pause(void);

int printf(const char* fmt, ...);

#endif
