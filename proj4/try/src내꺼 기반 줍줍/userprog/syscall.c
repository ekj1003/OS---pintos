// #include "userprog/syscall.h"
// #include <stdio.h>
// #include <string.h>
// #include <syscall-nr.h>
// #include "threads/interrupt.h"
// #include "threads/thread.h"
// #include "threads/vaddr.h"
// #include "threads/synch.h"
// #include "devices/shutdown.h"
// #include "devices/input.h"
// #include "filesys/off_t.h"
// #include "vm/page.h"

// static void syscall_handler (struct intr_frame *);

// struct file
//   {
//     struct inode *inode;
//     off_t pos;                 
//     bool deny_write;           
//   };


// void
// syscall_init (void) 
// {
//   intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
// }


// static void
// syscall_handler (struct intr_frame *f UNUSED) 
// {
//   int syscall_no = *(uint32_t *)(f->esp);

//   switch(syscall_no) {
//     case SYS_HALT:
//       halt();
//       break;

//     case SYS_EXIT:
//       if (!is_user_vaddr(*(uint32_t*)(f->esp + 4))) {
//         exit(-1);
//       }
//       exit(*(uint32_t *)(f->esp + 4));
//       break;
    
//     case SYS_EXEC:
//       if (!is_user_vaddr(*(uint32_t*)(f->esp + 4))) {
//         exit(-1);
//       }
//       f->eax = exec((const char *)(*(uint32_t *)(f->esp + 4)));
//       break;

//     case SYS_WAIT:
//       if(!is_user_vaddr(*(uint32_t*)(f->esp + 4))) {
//         exit(-1);
//       }
//       f->eax = wait((tid_t)*(uint32_t *)(f->esp + 4));
//       break;

//     case SYS_READ:
//       if (!is_user_vaddr(*(uint32_t*)(f->esp + 20)) || !is_user_vaddr(*(uint32_t*)(f->esp + 24)) || !is_user_vaddr(*(uint32_t*)(f->esp + 28))) {
//         exit(-1);
//       }
//       /* Added */
//       lock_acquire(&flock);
//       f->eax = read((int)*(uint32_t *)(f->esp + 20), (void *)*(uint32_t *)(f->esp + 24), (unsigned)*((uint32_t *)(f->esp + 28)));
//       break;
    
//     case SYS_WRITE:
//       if(!is_user_vaddr(*(uint32_t*)(f->esp + 20)) || !is_user_vaddr(*(uint32_t*)(f->esp + 24)) || !is_user_vaddr(*(uint32_t*)(f->esp + 28))) {
//         exit(-1);
//       }
//       /* Added */
//       lock_acquire(&flock);
// 			f->eax = write((int)*(uint32_t *)(f->esp + 20), (void *)*(uint32_t *)(f->esp + 24), (unsigned)*((uint32_t *)(f->esp + 28)));
// 			break;

//     case SYS_FIBO:
//     	if(!is_user_vaddr(*(uint32_t*)(f->esp + 4))) {
//         exit(-1);
//       }
//       f->eax = fibonacci((int)*(uint32_t *)(f->esp + 4));
//       break;

//     case SYS_MAX:
//       if(!is_user_vaddr(*(uint32_t*)(f->esp + 28)) || !is_user_vaddr(*(uint32_t*)(f->esp + 32)) || !is_user_vaddr(*(uint32_t*)(f->esp + 36)) || !is_user_vaddr(*(uint32_t*)(f->esp + 40))) {
//         exit(-1);
//       }
//       f->eax = max_of_four_int((int)*(uint32_t *)(f->esp + 28), (int)*(uint32_t *)(f->esp + 32), (int)*(uint32_t *)(f->esp + 36), (int)*(uint32_t *)(f->esp + 40));
//       break;
    
//     case SYS_CREATE:
//       if(!is_user_vaddr(f->esp + 4) || !is_user_vaddr(f->esp + 8))
// 				exit(-1);
//       f->eax = create((const char *)*(uint32_t *)(f->esp + 4), (unsigned)*(uint32_t *)(f->esp + 8));
//       break;

//     case SYS_REMOVE:
// 			if(!is_user_vaddr(f->esp + 4))
// 				exit(-1);
// 			f->eax = remove((const char *)*(uint32_t *)(f->esp + 4));
// 			break;
    
//     case SYS_OPEN: 
//       if(!is_user_vaddr(f->esp + 4))
// 				exit(-1);
//       lock_acquire(&flock);
// 			f->eax = open((const char *)*(uint32_t *)(f->esp + 4));
// 			break;

//     case SYS_CLOSE:
//       if(!is_user_vaddr(f->esp + 4))
// 				exit(-1);

// 			close((int)*(uint32_t *)(f->esp + 4));
// 			break;

//     case SYS_FILESIZE:
//       if(!is_user_vaddr(f->esp + 4))
// 				exit(-1);

// 			f->eax = filesize((int)*(uint32_t *)(f->esp + 4));
// 			break;

//     case SYS_SEEK:
//       if(!is_user_vaddr(f->esp + 4) || !is_user_vaddr(f->esp + 8))
// 				exit(-1);

// 			seek((int)*(uint32_t *)(f->esp + 4), (unsigned)*(uint32_t *)(f->esp + 8));
// 			break;

    
//     case SYS_TELL:
//       if(!is_user_vaddr(f->esp + 4))
// 				exit(-1);

// 			f->eax = tell((int)*(uint32_t *)(f->esp + 4));
// 			break;


//     default:
//       break;
      
//   }

// }


// void halt()
// {
//   shutdown_power_off();
// }

// void exit(int status)
// {
//   printf("%s: exit(%d)\n", thread_name(), status);
//   thread_current()->exit_status = status;
//   for (int i=3; i<128; i++) {
//     if(thread_current()->fd[i] != NULL)
// 			close(i);
//   }
//   thread_exit();
// }

// tid_t exec(const char* file)
// {
//   return process_execute(file);
// }

// int wait (tid_t c_tid)
// {
//   return process_wait(c_tid);
// }

// int read (int fd, void *buffer, unsigned size)
// {
//  int ret = -1;
//   if (fd == 0)
//   {
//     for (int i = 0; i < size; i++)
//     {
//       uint8_t c = input_getc();
//       *((uint8_t *)buffer + i) = c;
//       if (c == '\0')
//       {
//         ret = i;
//         break;
//       }
//     }
//     lock_release(&flock);
//   }
//   else if (3 <= fd)
//   {
//     if (thread_current()->fd[fd] == NULL)
//     {
//       lock_release(&flock);
//       exit(-1);
//     }
//     lock_release(&flock);
//     ret = file_read(thread_current()->fd[fd], buffer, size);
//   }
//   else
//   {
//     lock_release(&flock);
//     ret = -1;
//   }

//   return ret;
// }


// int write (int fd, const void *buffer, unsigned size)
// {
//   int ret = -1;
//   if (fd == 1)
//   {
//     putbuf(buffer, size);
//     lock_release(&flock);
//     ret = size;
//   }
//   else if (fd > 2)
//   {
//     lock_release(&flock);
//     if (thread_current()->fd[fd] == NULL) {
//       exit(-1);
//     }
    
//     if (thread_current()->fd[fd]->deny_write) {
//         file_deny_write(thread_current()->fd[fd]);
//     }
    
//     ret = file_write(thread_current()->fd[fd], buffer, size);
//   }
//   else
//   {
//     lock_release(&flock);
//     ret = -1;
//   }

//   return ret;
// }


// int fibonacci(int n)
// {
//   int i, x=0, y=1, z;

//   if (n == 0) return x;

//   for (i=2; i<=n; i++) {
//     z=x+y;
//     x=y;
//     y=z;
//   }
//   return y;
// }

// int max_of_four_int(int a, int b, int c, int d)
// {
//   int maximum = a;

//   if (maximum < b) maximum = b;
//   if (maximum < c) maximum = c;
//   if (maximum < d) maximum = d;

//   return maximum;
// }

// bool create(const char *file, unsigned initial_size ){
// 	if(file == NULL || !is_user_vaddr(file))
// 		exit(-1);
    
// 	return filesys_create(file, initial_size);
// }

// bool remove (const char *file) {
// 	if(file == NULL || !is_user_vaddr(file))
// 		exit(-1);

// 	return filesys_remove(file);
// }

// int open(const char *file) {
//   if (file == NULL) {
//     lock_release(&flock);
//     exit(-1);
//   }

//   struct file *fp = filesys_open(file);
//   int ret = -1;
//   int flag = 0;

//   if (fp == NULL){
//     lock_release(&flock);
//     ret = -1;
//   }
//   else{
//     for (int i = 3; i < 128; i++){
//       if (thread_current()->fd[i] == NULL){
//         if (strcmp(thread_current()->name, file) == 0) {
//             file_deny_write(fp);
//         }
//         // else {
//         //   lock_release(&flock);
//         //   exit(-1);
//         // }
//         thread_current()->fd[i] = fp;
//         lock_release(&flock);
//         ret = i;
//         flag = 1;
//         break;
//       }
//     }
//     if (!flag){
//       lock_release(&flock);
//       ret = -1;
//     }
//   }

//   return ret;

// }

// void close(int fd) {

//   struct file *fp;
// 	if(thread_current()->fd[fd] == NULL)
// 		exit(-1);

// 	fp = thread_current()->fd[fd];
//   file_close(fp);
//   thread_current()->fd[fd] = NULL;
// }

// int filesize(int fd) {
//   if(thread_current()->fd[fd] == NULL)
// 		exit(-1);

// 	return file_length(thread_current()->fd[fd]);
// }

// void seek(int fd, unsigned position) {
//   file_seek(thread_current()->fd[fd], position);
// }

// unsigned tell(int fd) {
//   if(thread_current()->fd[fd] == NULL)
// 		exit(-1);

// 	return file_tell(thread_current()->fd[fd]);
// }

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

struct vm_entry *check_address (void *addr, void *esp) {
	struct vm_entry *vme;

	if (addr < (void *) 0x08048000 || addr >= (void *) 0xc0000000) {
		exit (-1);
	}
	vme = find_vme (addr);
	if (vme == NULL) {
		if (!verify_stack (addr, esp)) exit (-1);
		expand_stack (addr);
		vme = find_vme (addr);
	}
	return vme;

}

void check_valid_buffer (void *buffer, unsigned size, void *esp, bool to_write) {
	void *ptr;
	ptr = pg_round_down (buffer);
	
	for (; ptr < buffer + size; ptr += PGSIZE) {
		struct vm_entry * vme;
		vme = check_address (ptr, esp);
		if (vme == NULL || !(vme->writable)) exit (-1);
	}
}

void check_valid_string (void *str, void *esp) {
	struct vm_entry *vme;
	void *ptr;
	int size = 0;
	vme = check_address (str, esp);
	if (vme == NULL) exit (-1);

	
	while (((char *) str)[size] != '\0')
		size++;
	
	ptr = pg_round_down (str);
	for (; ptr < str + size; ptr += PGSIZE) {
		vme = check_address (ptr, esp);
		if (vme == NULL) exit (-1);
	}
}

void check_valid_string_size (void *str, unsigned size, void *esp) {
	int i = 0;
	while (i < size ) {
		struct vm_entry *vme;
		vme = check_address ((void *) (str++), esp);
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
			check_valid_string ((void *)*(uint32_t *)(f->esp + 4), f->esp);
			f->eax = exec((const char *)*(uint32_t *)(f->esp + 4));
			break;

		case SYS_WAIT:
			f->eax = wait((pid_t)*(uint32_t *)(f->esp + 4));
			break;

		case SYS_READ:
			check_valid_string_size ((void *)*(uint32_t *)(f->esp + 8), (unsigned)*((uint32_t *)(f->esp+12)), f->esp);
			f->eax = read((int)*(uint32_t *)(f->esp + 4), (void *)*(uint32_t *)(f->esp + 8), (unsigned)*((uint32_t *)(f->esp + 12)));
			break;

		case SYS_WRITE:
			check_valid_string_size ((void *)*(uint32_t *)(f->esp + 8), (unsigned)*((uint32_t *)(f->esp+12)), f->esp);
			f->eax = write((int)*(uint32_t *)(f->esp + 4), (char *)*(uint32_t *)(f->esp + 8), (unsigned)*((uint32_t *)(f->esp + 12)));
			break;

		case SYS_FIBO:
			f->eax = fibonacci((int)*(uint32_t *)(f->esp + 4));
			break;

		case SYS_MAX:
			f->eax = max_of_four_int((int)*(uint32_t *)(f->esp + 4), (int)*(uint32_t *)(f->esp + 8), (int)*(uint32_t *)(f->esp + 12), (int)*(uint32_t *)(f->esp + 16));
			break;

		case SYS_CREATE:
			check_valid_string ((void *)*(uint32_t *)(f->esp + 4), f->esp);
			f->eax = create((const char *)*(uint32_t *)(f->esp + 4), (unsigned)*(uint32_t *)(f->esp + 8));
			break;

		case SYS_REMOVE:
			check_valid_string ((void *)*(uint32_t *)(f->esp + 4), f->esp);
			f->eax = remove((const char *)*(uint32_t *)(f->esp + 4));
			break;

		case SYS_OPEN:
			check_valid_string ((void *)*(uint32_t *)(f->esp + 4), f->esp);
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
