#include "frame.h"
#include <stdio.h>


static struct list_elem * get_next_lru_clock (void) {
	if (list_empty (&lru_list))
		return NULL;

	if (lru_clock == NULL || lru_clock == list_end (&lru_list))
		return list_begin (&lru_list);

    struct list_elem *next_elem = list_next(lru_clock);
    if (next_elem == list_end(&lru_list))
        return list_begin(&lru_list);

	else
		return next_elem;

	return lru_clock;
}


void lru_list_init (void) {
	list_init (&lru_list);
	lock_init (&lru_list_lock);
	lru_clock = NULL;
}

void del_page_from_lru_list (struct page *page) {
	if (&page->lru == lru_clock) lru_clock = list_next (lru_clock);

	list_remove (&page->lru);
}


void add_page_to_lru_list (struct page *page) {
	list_push_back (&lru_list, &(page->lru));
}


void try_to_free_pages (enum palloc_flags flags UNUSED){
	
	lru_clock = get_next_lru_clock ();
	struct page *page, *target;

	page = list_entry (lru_clock, struct page, lru);
	while (page->vme->pinned || pagedir_is_accessed (page->thread->pagedir, page->vme->vaddr)) {
		pagedir_set_accessed (page->thread->pagedir, page->vme->vaddr, false);
		lru_clock = get_next_lru_clock ();
		page = list_entry (lru_clock, struct page, lru);
	}

	target = page;
	uint8_t type = target->vme->type;
	switch (type) {
		case VM_ANON:
			target->vme->swap_slot = swap_out (target->kaddr);
			break;

		case VM_BIN:
			if (pagedir_is_dirty (target->thread->pagedir, target->vme->vaddr)) {
				target->vme->swap_slot = swap_out (target->kaddr);
				target->vme->type = VM_ANON;
			}
			break;

	}
	target->vme->is_loaded = false;
	_free_page (target);
}

struct page * alloc_page (enum palloc_flags flags) {
	uint8_t *kpage;
	struct page *page;

	lock_acquire (&lru_list_lock);
	kpage = palloc_get_page (flags);

	while (kpage == NULL) {
		try_to_free_pages (flags);
		kpage = palloc_get_page (flags);
	}

	page = malloc (sizeof (struct page));
	page->thread = thread_current ();
	page->kaddr = kpage;

	add_page_to_lru_list (page);
	lock_release (&lru_list_lock);

	return page;

}

void free_page (void *kaddr) {
	lock_acquire (&lru_list_lock);

	struct page *page = NULL;

	struct list_elem *e = list_begin (&lru_list);
	while (e != list_end (&lru_list)) {
		struct page *tmp = list_entry (e, struct page, lru);
		if (tmp->kaddr == kaddr) {
			page = tmp;
			break;
		}
		e = list_next (e);
	}

	if (page != NULL)
		_free_page (page);

	lock_release (&lru_list_lock);
}

void _free_page (struct page *page) {
	del_page_from_lru_list (page);

	pagedir_clear_page (page->thread->pagedir, pg_round_down (page->vme->vaddr));
	palloc_free_page (page->kaddr);
	
	free (page);
}
