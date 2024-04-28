#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <list.h>
#include <hash.h>
#include "threads/vaddr.h"
#include "threads/thread.h"

#define VM_BIN 0
#define VM_ANON 1

struct vm_entry {
	uint8_t type;
	void *vaddr;
	bool writable;

	bool pinned;
	bool is_loaded;

	char *file;
//	struct file *file;
	size_t offset;
	size_t read_bytes;
	size_t zero_bytes;

	size_t swap_slot;
	
	struct hash_elem elem;
};

struct page {
	void *kaddr;
	struct vm_entry *vme;
	struct thread *thread;
	struct list_elem lru;
};

static unsigned vm_hash_func (const struct hash_elem *e, void *aux);
static bool vm_less_func (const struct hash_elem *a, const struct hash_elem *b, void *aux);

void vm_init (struct hash *vm);
bool insert_vme (struct hash *vm, struct vm_entry *vme);
bool delete_vme (struct hash *vm, struct vm_entry *vme);

struct vm_entry *find_vme (void *vaddr);
void vm_destroy (struct hash *vm);

bool load_file (void *kaddr, struct vm_entry *vme);

#endif
