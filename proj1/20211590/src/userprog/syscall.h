#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H
#include "lib/user/syscall.h"
#include "process.h"

void syscall_init (void);
#endif /* userprog/syscall.h */

void halt (void);
void exit (int status);
tid_t exec (const char *file);
int wait (tid_t c_tid);
int read (int fd, void *buffer, unsigned length);
int write (int fd, const void *buffer, unsigned length);

int fibonacci(int n);
int max_of_four_int(int a, int b, int c, int d);