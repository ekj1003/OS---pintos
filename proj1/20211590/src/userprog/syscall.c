#include "userprog/syscall.h"
#include <stdio.h>
#include <string.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "devices/shutdown.h"
#include "devices/input.h"

static void syscall_handler (struct intr_frame *);
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
      f->eax = read((int)*(uint32_t *)(f->esp + 20), (void *)*(uint32_t *)(f->esp + 24), (unsigned)*((uint32_t *)(f->esp + 28)));
      break;
    
    case SYS_WRITE:
      if(!is_user_vaddr(*(uint32_t*)(f->esp + 20)) || !is_user_vaddr(*(uint32_t*)(f->esp + 24)) || !is_user_vaddr(*(uint32_t*)(f->esp + 28))) {
        exit(-1);
      }
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

int read (int fd, void *buffer, unsigned length)
{
  int i = 0;
  if(fd == 0)
  {
    while(i < length){
      if((((char *)buffer)[i] = input_getc()) == '\0') break;
      i++;
    }
  }
  return i;
}


int write (int fd, const void *buffer, unsigned length)
{
  if(fd == 1){
    putbuf(buffer, length);
    return length;
  }
  else return -1;
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