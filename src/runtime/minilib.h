#ifndef _MINILIB_H
#define _MINILIB_H

/* Minimal C runtime for custom OS — no libc dependency */

long sys_write(int fd, const void *buf, long count);
void sys_exit(int code) __attribute__((noreturn));
int printf(const char *fmt, ...);

#endif
