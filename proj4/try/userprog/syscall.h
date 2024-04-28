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
int read (int fd, void *buffer, unsigned size);
int write (int fd, const void *buffer, unsigned size);

int fibonacci(int n);
int max_of_four_int(int a, int b, int c, int d);

bool create(const char *file, unsigned initial_size);
bool remove(const char *file);
int open(const char *file);
void close(int fd);
int filesize(int fd);
void seek(int fd, unsigned position);
unsigned tell(int fd);

struct vm_entry * check_address (void *addr, void *esp);
void check_valid_buffer (void *buffer, unsigned size, void *esp, bool to_write);
void check_valid_string (void *str, void *esp);
void check_valid_string_size (void *str, unsigned size, void *esp);

