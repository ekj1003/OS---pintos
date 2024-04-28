#include "userprog/syscall.h"
#include "threads/vaddr.h"
#include <stdio.h>
#include <string.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "devices/shutdown.h"
#include "devices/input.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "threads/synch.h"
#include "vm/page.h"

static void syscall_handler (struct intr_frame *);
struct lock filesys_lock;

struct file {
	struct inode *inode;
	off_t pos;
	bool deny_write;
};

void
syscall_init (void) 
{
	lock_init(&filesys_lock);
	intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

struct vm_entry *addr_check (void *addr, void *esp) {
	struct vm_entry *vme;

	if (addr < (void *) 0x08048000 || addr >= (void *) 0xc0000000) exit (-1);

	vme = find_vme (addr);
	if (vme == NULL) {
		if (!verify_stack (addr, esp)) exit (-1);
		grow_stack (addr);
		vme = find_vme (addr);
	}
	return vme;

}

void buffer_valid_check (void *buffer, unsigned size, void *esp, bool to_write) {
	void *ptr;
	ptr = pg_round_down (buffer);
	
	while (ptr < buffer + size) {
		struct vm_entry *vme;
		vme = addr_check(ptr, esp);
		if (vme == NULL || !(vme->writable)) exit(-1);
		ptr += PGSIZE;
	}
}

void string_valid_check (void *str, void *esp) {
	struct vm_entry *vme;
	void *ptr;
	int size = 0;
	vme = addr_check (str, esp);
	if (vme == NULL) exit (-1);

	
	while (((char *) str)[size] != '\0')
		size++;
	
	ptr = pg_round_down (str);
	while (ptr < str + size) {
    	vme = addr_check(ptr, esp);
    	if (vme == NULL) exit(-1);
   		ptr += PGSIZE;
	}
}

void string_size_valid_check (void *str, unsigned size, void *esp) {
	int i = 0;
	while (i < size ) {
		struct vm_entry *vme;
		vme = addr_check ((void *) (str++), esp);
		if (vme == NULL) exit (-1);
		i++;
	}

}

static void
syscall_handler (struct intr_frame *f)  {
	int sys_num = *(uint32_t *)(f->esp);

	if(sys_num < 0) {
		printf("not vaild system call number\n");
		exit(-1);
	}

	switch(sys_num) {
		case SYS_HALT:
			halt();
			break;

		case SYS_EXIT:
			exit(*(uint32_t *)(f->esp + 4));
			break;

		case SYS_EXEC:
			string_valid_check ((void *)*(uint32_t *)(f->esp + 4), f->esp);
			f->eax = exec((const char *)*(uint32_t *)(f->esp + 4));
			break;

		case SYS_WAIT:
			f->eax = wait((pid_t)*(uint32_t *)(f->esp + 4));
			break;

		case SYS_READ:
			string_size_valid_check ((void *)*(uint32_t *)(f->esp + 8), (unsigned)*((uint32_t *)(f->esp+12)), f->esp);
			f->eax = read((int)*(uint32_t *)(f->esp + 4), (void *)*(uint32_t *)(f->esp + 8), (unsigned)*((uint32_t *)(f->esp + 12)));
			break;

		case SYS_WRITE:
			string_size_valid_check ((void *)*(uint32_t *)(f->esp + 8), (unsigned)*((uint32_t *)(f->esp+12)), f->esp);
			f->eax = write((int)*(uint32_t *)(f->esp + 4), (char *)*(uint32_t *)(f->esp + 8), (unsigned)*((uint32_t *)(f->esp + 12)));
			break;

		case SYS_FIBO:
			f->eax = fibonacci((int)*(uint32_t *)(f->esp + 4));
			break;

		case SYS_MAX:
			f->eax = max_of_four_int((int)*(uint32_t *)(f->esp + 4), (int)*(uint32_t *)(f->esp + 8), (int)*(uint32_t *)(f->esp + 12), (int)*(uint32_t *)(f->esp + 16));
			break;

		case SYS_CREATE:
			string_valid_check ((void *)*(uint32_t *)(f->esp + 4), f->esp);
			f->eax = create((const char *)*(uint32_t *)(f->esp + 4), (unsigned)*(uint32_t *)(f->esp + 8));
			break;

		case SYS_REMOVE:
			string_valid_check ((void *)*(uint32_t *)(f->esp + 4), f->esp);
			f->eax = remove((const char *)*(uint32_t *)(f->esp + 4));
			break;

		case SYS_OPEN:
			string_valid_check ((void *)*(uint32_t *)(f->esp + 4), f->esp);
			f->eax = open((const char *)*(uint32_t *)(f->esp + 4));
			break;

		case SYS_CLOSE:
			close((int)*(uint32_t *)(f->esp + 4));
			break;

		case SYS_FILESIZE:
			f->eax = filesize((int)*(uint32_t *)(f->esp + 4));
			break;

		case SYS_SEEK:
			seek((int)*(uint32_t *)(f->esp + 4), (unsigned)*(uint32_t *)(f->esp + 8));
			break;

		case SYS_TELL:
			f->eax = tell((int)*(uint32_t *)(f->esp + 4));
			break;
	}
}

void 
halt ()
{
	shutdown_power_off();
}

void 
exit (int status)
{
	printf("%s: exit(%d)\n", thread_name(), status);
	thread_current()->exit_status = status;
	for(int i = 3; i < 128; i++) {
		if(thread_current()->fd[i] != NULL)
			close(i);
	}
	thread_exit();
}

pid_t 
exec (const char *file)
{
	return process_execute(file);
}

int 
wait (pid_t pid)
{
	return process_wait(pid);
}

int 
read (int fd, void *buffer, unsigned length)
{
	int i = 0;

	if(!is_user_vaddr(buffer))
		exit(-1);
	
	lock_acquire(&filesys_lock);

	if(fd == 0) {
		for(i=0; (unsigned int)i<length; i++) {
			if((((char *)buffer)[i] = input_getc()) == '\0') {
				break;
			}
		}
		lock_release(&filesys_lock);
		return i;
	}

	else if(fd >= 3) {
		if(thread_current()->fd[fd] == NULL){
			lock_release(&filesys_lock);
			exit(-1);
		}
		lock_release(&filesys_lock);
		return file_read(thread_current()->fd[fd], buffer, length);
	}

	else {
		lock_release(&filesys_lock);
		return -1;
	}
}

int 
write (int fd, const void *buffer, unsigned length)
{
	if(!is_user_vaddr(buffer))
		exit(-1);

	lock_acquire(&filesys_lock);

	if(fd == 1) {
		putbuf(buffer, length);
		lock_release(&filesys_lock);
		return length;
	}

	else if(fd >= 3) {
		if(thread_current()->fd[fd] == NULL) {
			lock_release(&filesys_lock);
			exit(-1);
		}
		if(thread_current()->fd[fd]->deny_write) {
			file_deny_write(thread_current()->fd[fd]);
		}
		lock_release(&filesys_lock);
		return file_write(thread_current()->fd[fd], buffer, length);
	}
	
	else {
		lock_release(&filesys_lock);
		return -1;
	}
}

int
fibonacci (int n)
{
	int i;
	int a = 1;
	int b = 1;
	int sum = 0;

	if(n == 0)
		return 0;
	if(n == 1)
		return 1;
	if (n == 2)
		return 1;
	for(i=2; i<n; i++) {
		sum = a + b;
		a = b;
		b = sum;
	}
	return sum;
}

int
max_of_four_int (int a, int b, int c, int d)
{
	int arr[4];
	int max = a;

	arr[0] = a;
	arr[1] = b;
	arr[2] = c;
	arr[3] = d;

	for(int i=1; i<4; i++) {
		if(max < arr[i]) {
			max = arr[i];
		}
	}

	return max;
}

bool 
create (const char *file, unsigned initial_size)
{
	return filesys_create(file, initial_size);
}

bool 
remove (const char *file)
{
	return filesys_remove(file);
}

int 
open (const char *file)
{
	int i;

	lock_acquire(&filesys_lock);
	struct file *fp = filesys_open(file);
	
	if(fp == NULL) {
		lock_release(&filesys_lock);
		return -1;
	}

	else {
		for(i = 3; i < 128; i++) {
			if(thread_current()->fd[i] == NULL) {
				if(!strcmp(thread_current()->name, file)) {
					file_deny_write(fp);
				}

				thread_current()->fd[i] = fp;
				lock_release(&filesys_lock);
				return i;
			}
		}
	}

	lock_release(&filesys_lock);
	return -1;
}

void
close (int fd)
{
	struct file *fp;

	if(thread_current()->fd[fd] == NULL)
		exit(-1);

	fp = thread_current()->fd[fd];
	thread_current()->fd[fd] = NULL;

	return file_close(fp);
}

int 
filesize (int fd)
{
	if(thread_current()->fd[fd] == NULL)
		exit(-1);

	return file_length(thread_current()->fd[fd]);
}

void
seek (int fd, unsigned position)
{
	if(thread_current()->fd[fd] == NULL)
		exit(-1);

	file_seek(thread_current()->fd[fd], position);
}

unsigned
tell (int fd)
{
	if(thread_current()->fd[fd] == NULL)
		exit(-1);

	return file_tell(thread_current()->fd[fd]);
}
