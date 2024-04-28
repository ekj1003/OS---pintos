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




static unsigned vm_hash_func (const struct hash_elem *e, void *aux)
{
	struct vm_entry *vmEntry;
	vmEntry = hash_entry(e, struct vm_entry, elem);

	return hash_int((int)vmEntry->vaddr);
}

static bool vm_less_func (const struct hash_elem *a, const struct hash_elem *b)
{
	struct vm_entry *a_Entry;
	struct vm_entry *b_Entry;
	a_Entry = hash_entry(a, struct vm_entry, elem);
	b_Entry = hash_entry(b, struct vm_entry, elem);

	return (a_Entry->vaddr < b_Entry->vaddr);
}


struct vm_entry *find_vme (void *vaddr) {

    struct vm_entry target;
    struct hash_elem *e;

	struct thread *cur = thread_current ();
	struct vm_entry search;

	target.vaddr = pg_round_down (vaddr);
	e = hash_find (&(cur->vm), &(target.elem));

	if (e == NULL) return NULL;
	else return hash_entry (e, struct vm_entry, elem);
}

void vm_init (struct hash *vm) {
	hash_init (vm, vm_hash_func, vm_less_func, NULL);
}


bool insert_vme (struct hash *vm, struct vm_entry *vme) {
	vme->pinned = false;

	struct hash_elem *elem;
    
    elem = hash_insert (vm, &(vme->elem));

	if (elem != NULL) return false;
	else return true;
}

bool delete_vme (struct hash *vm, struct vm_entry *vme) {
	struct hash_elem *elem;
    elem = hash_delete (vm, & (vme->elem));

	if (elem != NULL) {
		palloc_free_page (pagedir_get_page (thread_current ()->pagedir, vme->vaddr));
		free (vme);
		return true;
	}
	else return false;

}


void vm_destroy (struct hash *vm) {
	hash_destroy (vm, NULL);
}


bool load_file (void *kaddr, struct vm_entry *vme) {
	struct file *file;
    int n_read;

    file = filesys_open (vme->file_name);
	n_read = file_read_at (file, kaddr, vme->read_bytes, vme->offset);

	if (n_read != (int)(vme->read_bytes)) return false;
    
	memset (kaddr + n_read, 0, vme->zero_bytes);
	return true;
}


