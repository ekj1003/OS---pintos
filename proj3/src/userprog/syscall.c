#include "userprog/syscall.h"
#include <stdio.h>
#include <string.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/synch.h"
#include "devices/shutdown.h"
#include "devices/input.h"
#include "filesys/off_t.h"

static void syscall_handler (struct intr_frame *);

struct file
  {
    struct inode *inode;
    off_t pos;                 
    bool deny_write;           
  };


void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}


static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  int syscall_no = *(uint32_t *)(f->esp);

  switch(syscall_no) {
    case SYS_HALT:
      halt();
      break;

    case SYS_EXIT:
      if (!is_user_vaddr(*(uint32_t*)(f->esp + 4))) {
        exit(-1);
      }
      exit(*(uint32_t *)(f->esp + 4));
      break;
    
    case SYS_EXEC:
      if (!is_user_vaddr(*(uint32_t*)(f->esp + 4))) {
        exit(-1);
      }
      f->eax = exec((const char *)(*(uint32_t *)(f->esp + 4)));
      break;

    case SYS_WAIT:
      if(!is_user_vaddr(*(uint32_t*)(f->esp + 4))) {
        exit(-1);
      }
      f->eax = wait((tid_t)*(uint32_t *)(f->esp + 4));
      break;

    case SYS_READ:
      if (!is_user_vaddr(*(uint32_t*)(f->esp + 20)) || !is_user_vaddr(*(uint32_t*)(f->esp + 24)) || !is_user_vaddr(*(uint32_t*)(f->esp + 28))) {
        exit(-1);
      }
      /* Added */
      lock_acquire(&flock);
      f->eax = read((int)*(uint32_t *)(f->esp + 20), (void *)*(uint32_t *)(f->esp + 24), (unsigned)*((uint32_t *)(f->esp + 28)));
      break;
    
    case SYS_WRITE:
      if(!is_user_vaddr(*(uint32_t*)(f->esp + 20)) || !is_user_vaddr(*(uint32_t*)(f->esp + 24)) || !is_user_vaddr(*(uint32_t*)(f->esp + 28))) {
        exit(-1);
      }
      /* Added */
      lock_acquire(&flock);
			f->eax = write((int)*(uint32_t *)(f->esp + 20), (void *)*(uint32_t *)(f->esp + 24), (unsigned)*((uint32_t *)(f->esp + 28)));
			break;

    case SYS_FIBO:
    	if(!is_user_vaddr(*(uint32_t*)(f->esp + 4))) {
        exit(-1);
      }
      f->eax = fibonacci((int)*(uint32_t *)(f->esp + 4));
      break;

    case SYS_MAX:
      if(!is_user_vaddr(*(uint32_t*)(f->esp + 28)) || !is_user_vaddr(*(uint32_t*)(f->esp + 32)) || !is_user_vaddr(*(uint32_t*)(f->esp + 36)) || !is_user_vaddr(*(uint32_t*)(f->esp + 40))) {
        exit(-1);
      }
      f->eax = max_of_four_int((int)*(uint32_t *)(f->esp + 28), (int)*(uint32_t *)(f->esp + 32), (int)*(uint32_t *)(f->esp + 36), (int)*(uint32_t *)(f->esp + 40));
      break;
    
    case SYS_CREATE:
      if(!is_user_vaddr(f->esp + 4) || !is_user_vaddr(f->esp + 8))
				exit(-1);
      f->eax = create((const char *)*(uint32_t *)(f->esp + 4), (unsigned)*(uint32_t *)(f->esp + 8));
      break;

    case SYS_REMOVE:
			if(!is_user_vaddr(f->esp + 4))
				exit(-1);
			f->eax = remove((const char *)*(uint32_t *)(f->esp + 4));
			break;
    
    case SYS_OPEN: 
      if(!is_user_vaddr(f->esp + 4))
				exit(-1);
      lock_acquire(&flock);
			f->eax = open((const char *)*(uint32_t *)(f->esp + 4));
			break;

    case SYS_CLOSE:
      if(!is_user_vaddr(f->esp + 4))
				exit(-1);

			close((int)*(uint32_t *)(f->esp + 4));
			break;

    case SYS_FILESIZE:
      if(!is_user_vaddr(f->esp + 4))
				exit(-1);

			f->eax = filesize((int)*(uint32_t *)(f->esp + 4));
			break;

    case SYS_SEEK:
      if(!is_user_vaddr(f->esp + 4) || !is_user_vaddr(f->esp + 8))
				exit(-1);

			seek((int)*(uint32_t *)(f->esp + 4), (unsigned)*(uint32_t *)(f->esp + 8));
			break;

    
    case SYS_TELL:
      if(!is_user_vaddr(f->esp + 4))
				exit(-1);

			f->eax = tell((int)*(uint32_t *)(f->esp + 4));
			break;


    default:
      break;
      
  }

}


void halt()
{
  shutdown_power_off();
}

void exit(int status)
{
  printf("%s: exit(%d)\n", thread_name(), status);
  thread_current()->exit_status = status;
  for (int i=3; i<128; i++) {
    if(thread_current()->fd[i] != NULL)
			close(i);
  }
  thread_exit();
}

tid_t exec(const char* file)
{
  return process_execute(file);
}

int wait (tid_t c_tid)
{
  return process_wait(c_tid);
}

int read (int fd, void *buffer, unsigned size)
{
 int ret = -1;
  if (fd == 0)
  {
    for (int i = 0; i < size; i++)
    {
      uint8_t c = input_getc();
      *((uint8_t *)buffer + i) = c;
      if (c == '\0')
      {
        ret = i;
        break;
      }
    }
    lock_release(&flock);
  }
  else if (3 <= fd)
  {
    if (thread_current()->fd[fd] == NULL)
    {
      lock_release(&flock);
      exit(-1);
    }
    lock_release(&flock);
    ret = file_read(thread_current()->fd[fd], buffer, size);
  }
  else
  {
    lock_release(&flock);
    ret = -1;
  }

  return ret;
}


int write (int fd, const void *buffer, unsigned size)
{
  int ret = -1;
  if (fd == 1)
  {
    putbuf(buffer, size);
    lock_release(&flock);
    ret = size;
  }
  else if (fd > 2)
  {
    lock_release(&flock);
    if (thread_current()->fd[fd] == NULL) {
      exit(-1);
    }
    
    if (thread_current()->fd[fd]->deny_write) {
        file_deny_write(thread_current()->fd[fd]);
    }
    
    ret = file_write(thread_current()->fd[fd], buffer, size);
  }
  else
  {
    lock_release(&flock);
    ret = -1;
  }

  return ret;
}


int fibonacci(int n)
{
  int i, x=0, y=1, z;

  if (n == 0) return x;

  for (i=2; i<=n; i++) {
    z=x+y;
    x=y;
    y=z;
  }
  return y;
}

int max_of_four_int(int a, int b, int c, int d)
{
  int maximum = a;

  if (maximum < b) maximum = b;
  if (maximum < c) maximum = c;
  if (maximum < d) maximum = d;

  return maximum;
}

bool create(const char *file, unsigned initial_size ){
	if(file == NULL || !is_user_vaddr(file))
		exit(-1);
    
	return filesys_create(file, initial_size);
}

bool remove (const char *file) {
	if(file == NULL || !is_user_vaddr(file))
		exit(-1);

	return filesys_remove(file);
}

int open(const char *file) {
  if (file == NULL) {
    lock_release(&flock);
    exit(-1);
  }

  struct file *fp = filesys_open(file);
  int ret = -1;
  int flag = 0;

  if (fp == NULL){
    lock_release(&flock);
    ret = -1;
  }
  else{
    for (int i = 3; i < 128; i++){
      if (thread_current()->fd[i] == NULL){
        if (strcmp(thread_current()->name, file) == 0) {
            file_deny_write(fp);
        }
        // else {
        //   lock_release(&flock);
        //   exit(-1);
        // }
        thread_current()->fd[i] = fp;
        lock_release(&flock);
        ret = i;
        flag = 1;
        break;
      }
    }
    if (!flag){
      lock_release(&flock);
      ret = -1;
    }
  }

  return ret;

}

void close(int fd) {

  struct file *fp;
	if(thread_current()->fd[fd] == NULL)
		exit(-1);

	fp = thread_current()->fd[fd];
  file_close(fp);
  thread_current()->fd[fd] = NULL;
}

int filesize(int fd) {
  if(thread_current()->fd[fd] == NULL)
		exit(-1);

	return file_length(thread_current()->fd[fd]);
}

void seek(int fd, unsigned position) {
  file_seek(thread_current()->fd[fd], position);
}

unsigned tell(int fd) {
  if(thread_current()->fd[fd] == NULL)
		exit(-1);

	return file_tell(thread_current()->fd[fd]);
}

