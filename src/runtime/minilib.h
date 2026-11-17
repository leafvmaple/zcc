#ifndef ZCC_MINILIB_H
#define ZCC_MINILIB_H
#include "syscall.h"
#include "process.h"
#include "fcntl.h"
long sys_read(int fd, void* buffer, long count);
long sys_write(int fd, void* buffer, long count);
int sys_open(char* path, int flags);
int sys_close(int fd);
void sys_exit(int code);
int sys_pause();
long sys_spawn(char* path, int argc, char** argv);
long sys_wait(int pid, int* status);
long sys_kill(int pid);
long sys_getpid();
long sys_yield();
long sys_stat(char* path, struct FileStat* result);
long sys_readdir(char* path, long index, struct DirectoryEntry* result);
long strlen(char* text);
int strcmp(char* first, char* second);
int memcmp(void* first, void* second, long size);
void* memcpy(void* target, void* source, long size);
void* memset(void* target, int value, long size);
void* memmove(void* target, void* source, long size);
#ifndef __ZCC__
int printf(const char* format, ...);
#endif
#endif
