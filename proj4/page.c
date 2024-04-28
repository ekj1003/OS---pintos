#include "vm/page.h"
#include "vm/frame.h"
#include "vm/swap.h"
#include "threads/malloc.h"
#include "userprog/pagedir.h"
#include "filesys/file.h"
#include "threads/interrupt.h"
#include "filesys/filesys.h"
#include <stdio.h>
#include <string.h>


void vm_init (struct hash *vm) {
	hash_init (vm, vm_hash_func, vm_less_func, NULL);
}

bool insert_vme (struct hash *vm, struct vm_entry *vme) {
	struct hash_elem *e;
	vme->pinned = false;

	e = hash_insert (vm, &(vme->elem));

	if (e != NULL) return false;
	else
		return true;
}

bool delete_vme (struct hash *vm, struct vm_entry *vme) {
	struct hash_elem *e;
	bool ans;
	e = hash_delete (vm, & (vme->elem));

	
	if (e != NULL) {
		palloc_free_page(pagedir_get_page (thread_current ()->pagedir, vme->vaddr));
		free (vme);
		ans = true;
	}

	else ans = false;

	return ans;
}

struct vm_entry *find_vme (void *vaddr) {
	struct thread *th;
	struct vm_entry search;
	struct hash_elem *e;
	th = thread_current ();

	search.vaddr = pg_round_down (vaddr);
	e = hash_find (&(th->vm), &(search.elem));

	
	if (e == NULL) return NULL;
	else return hash_entry (e, struct vm_entry, elem);
		
}

void vm_destroy (struct hash *vm) {
	hash_destroy (vm, NULL);
}

bool load_file (void *kaddr, struct vm_entry *vme) {
	struct file *file;
	int read_bytes;

	file = filesys_open (vme->file_name);
	read_bytes = file_read_at(file, kaddr, vme->read_bytes, vme->offset);

	if (read_bytes != (int) (vme->read_bytes))
		return false;
	memset (kaddr + vme->read_bytes, 0, vme->zero_bytes);

	return true;
}



static unsigned vm_hash_func (const struct hash_elem *e, void *aux) {

	struct vm_entry *vme;
	vme = hash_entry (e, struct vm_entry, elem);

	return hash_int ((int) vme->vaddr);
}

static bool vm_less_func (const struct hash_elem *a, const struct hash_elem *b, void *aux) {
	struct vm_entry *vme_a;
	struct vm_entry *vme_b;
	vme_a = hash_entry (a, struct vm_entry, elem);
	vme_b = hash_entry (b, struct vm_entry, elem);

	return vme_a->vaddr < vme_b->vaddr;
}











