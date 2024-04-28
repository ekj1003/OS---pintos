#ifndef VM_PAGE_H
#define VM_PAGE_H

#include "threads/thread.h"
#include "threads/vaddr.h"
#include <hash.h>
#include <list.h>


#define VM_ANON 1
#define VM_BIN 0		
	

struct vm_entry {

	size_t offset;		    /* 읽어야할 file offset */		
	size_t read_bytes;      			
	size_t zero_bytes;
	size_t swap_slot;			

	uint8_t type;				
	void *vaddr;

	bool writable;				
    bool is_loaded;         /* phys addr의 load 여부 check */
	bool pinned;   
	         /* True 해당 주소에 write 가능 */
	struct file* file;	
	char *file_name;
	struct hash_elem elem;		
};

struct page {
	void *kaddr;
	struct vm_entry *vme;
	struct list_elem lru;
	struct thread *thread;
	
};



void vm_init (struct hash *vm);
bool insert_vme (struct hash *vm, struct vm_entry *vme);
bool delete_vme (struct hash *vm, struct vm_entry *vme);

struct vm_entry *find_vme (void *vaddr);
void vm_destroy (struct hash *vm);
bool load_file (void *kaddr, struct vm_entry *vme);

static unsigned vm_hash_func (const struct hash_elem *e, void *aux);
static bool vm_less_func (const struct hash_elem *a, const struct hash_elem *b, void *aux);


#endif
