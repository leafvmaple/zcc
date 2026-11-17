#ifndef ZCC_MINILIB_H
#define ZCC_MINILIB_H
#include "syscall.h"
#include "process.h"
#include "fcntl.h"
#include "terminal.h"
#ifdef __cplusplus
extern "C" {
#endif
long sys_read(int fd, void* buffer, long count);
long sys_write(int fd, void* buffer, long count);
int sys_open(char* path, int flags);
int sys_close(int fd);
long sys_ioctl(int fd, int request, long argument);
long sys_dup2(int old_fd, int new_fd);
long sys_fcntl(int fd, int command, long argument);
void sys_exit(int code);
int sys_pause();
long sys_chdir(char* path);
long sys_getcwd(char* buffer, long capacity);
long sys_spawn(char* path, int argc, char** argv);
long sys_spawn_foreground(char* path, int argc, char** argv);
long sys_wait(int pid, int* status);
long sys_kill(int pid);
long sys_process_next(long after_pid, struct ProcessInfo* result);
long sys_getpid();
long sys_yield();
long sys_mkdir(char* path);
long sys_unlink(char* path);
long sys_rmdir(char* path);
long sys_sync();
long sys_shutdown();
long sys_fstat(int fd, struct DescriptorStat* result);
long sys_ftruncate(int fd, long length);
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
#ifdef __cplusplus
}
#endif
#endif
