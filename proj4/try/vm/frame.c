#include "frame.h"
#include <stdio.h>


void lru_list_init (void) {
	list_init (&lru_list);
	lock_init (&lru_list_lock);
	lru_clock = NULL;
}


void add_page_to_lru_list (struct page *page) {
	list_push_back (&lru_list, &(page->lru));
}


void del_page_from_lru_list(struct page *page) {
	/* If the page we want to delete is same with lru_clock pointing,
	   then lru_clock should move to next */
	if(lru_clock == &page->lru)
		lru_clock = list_next(lru_clock);

	list_remove(&page->lru);
}


struct page *alloc_page(enum palloc_flags flags) {
    uint8_t *kpage;
    struct page *new_page;
	lock_acquire(&lru_list_lock);

    kpage = palloc_get_page(flags);

	for(;kpage == NULL;){
		try_to_free_pages(flags);
		kpage = palloc_get_page(flags);
	}

	new_page = malloc(sizeof(struct page));
	new_page->thread = thread_current();
	new_page->kaddr = kpage;

	add_page_to_lru_list(new_page);
	lock_release(&lru_list_lock);

	return new_page;
}


void free_page(void *kaddr) {
	lock_acquire(&lru_list_lock);

	struct page *page = NULL;
	struct list_elem *elem = list_begin(&lru_list);

	while(elem!=list_end(&lru_list)){
		struct page *cand_page = list_entry(elem, struct page, lru);
		if(cand_page->kaddr == kaddr){
			page = cand_page;
			break;
		}
		elem = list_next(elem);
	}
	if(page != NULL)
		_free_page(page);

	lock_release(&lru_list_lock);
}


void _free_page (struct page *page) {

	del_page_from_lru_list (page);
    if (page !=NULL) {
        pagedir_clear_page (page->thread->pagedir, pg_round_down (page->vme->vaddr));
	    palloc_free_page (page->kaddr);
    }
	free (page);
}


static struct list_elem * get_next_lru_clock (void) {
    struct list_elem *ans;

	if (list_empty (&lru_list))
		return NULL;

	if (lru_clock == NULL || lru_clock == list_end (&lru_list)) {
        ans = list_begin (&lru_list);
        return ans;
    }
		

	if (list_next (lru_clock) != list_end (&lru_list)) {
        ans = list_next(&lru_list);
        return ans;
    }
		
    else {
        ans = list_next (lru_clock);
        return ans;
    } 

	return lru_clock;
}


void try_to_free_pages (enum palloc_flags flags UNUSED) {
	
	struct page *found_free;
	lru_clock = get_next_lru_clock ();
	struct page *page = list_entry (lru_clock, struct page, lru);

	while (page->vme->pinned || pagedir_is_accessed (page->thread->pagedir, page->vme->vaddr)) {
		pagedir_set_accessed (page->thread->pagedir, page->vme->vaddr, false);
		struct list_elem *ptr = get_next_lru_clock ();
		lru_clock = ptr;
		page = list_entry (lru_clock, struct page, lru);
	}

	found_free = page;
	switch (found_free->vme->type) {

		case VM_ANON:
			found_free->vme->swap_slot = swap_out (found_free->kaddr);
			break;

		case VM_BIN:
			if (pagedir_is_dirty (found_free->thread->pagedir, found_free->vme->vaddr)) {
				found_free->vme->swap_slot = swap_out (found_free->kaddr);
				found_free->vme->type = VM_ANON;
			}
			break;


	}
	found_free->vme->is_loaded = false;
	_free_page (found_free);
}

