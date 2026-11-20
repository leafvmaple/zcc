#ifndef ZCC_MINILIB_H
#define ZCC_MINILIB_H
#include "syscall.h"
#include "process.h"
#include "fcntl.h"
#include "terminal.h"
#include "display.h"
#include "channel.h"
#ifdef __cplusplus
extern "C" {
#endif
long sys_read(int fd, void* buffer, long count);
long sys_write(int fd, void* buffer, long count);
int sys_open(char* path, int flags);
int sys_close(int fd);
long sys_ioctl(int fd, int request, long argument);
long sys_pipe2(int* ends, int flags);
// Named local streams; flags accept O_CLOEXEC and O_NONBLOCK. Each returns a descriptor or a negative error.
long sys_channel_listen(char* name, int flags);
long sys_channel_connect(char* name, int flags);
long sys_channel_accept(int fd, int flags);
long sys_wait_readable(long mask);
long sys_wait_readable_for(long mask, long timeout_ms);
long sys_wait_io(long read_mask, long write_mask, long timeout_ms);
long sys_uptime_ms();
long sys_realtime();
// Moves the program break and returns it; 0 returns the current break.
long sys_brk(long end);
long sys_getrandom(void* output, long size, long flags);
long sys_openpty(int* ends, struct TerminalSize* size);
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
long sys_rename(char* path, char* new_name);
long sys_unlink(char* path);
long sys_rmdir(char* path);
long sys_sync();
long sys_shutdown();
long sys_fstat(int fd, struct DescriptorStat* result);
long sys_ftruncate(int fd, long length);
long sys_stat(char* path, struct FileStat* result);
long sys_readdir(char* path, long index, struct DirectoryEntry* result);
int strcmp(char* first, char* second);
#ifdef __ZCC__
long strlen(char* text);
int memcmp(void* first, void* second, long size);
void* memcpy(void* target, void* source, long size);
void* memset(void* target, int value, long size);
void* memmove(void* target, void* source, long size);
#else
__SIZE_TYPE__ strlen(const char* text);
int memcmp(const void* first, const void* second, __SIZE_TYPE__ size);
void* memcpy(void* target, const void* source, __SIZE_TYPE__ size);
void* memset(void* target, int value, __SIZE_TYPE__ size);
void* memmove(void* target, const void* source, __SIZE_TYPE__ size);
#endif
#ifndef __ZCC__
int printf(const char* format, ...);
#endif
#ifdef __cplusplus
}
#endif
#endif
