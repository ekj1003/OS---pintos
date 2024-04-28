#ifndef VM_FRAME_H
#define VM_FRAME_H

#include <hash.h>
#include <list.h>
#include "filesys/file.h"
#include "userprog/pagedir.h"
#include "threads/synch.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "vm/page.h"
#include "vm/swap.h"
#include "vm/swap.h"


struct list lru_list;
struct lock lru_list_lock;
struct list_elem *lru_clock;

/* Initializes the Least Recently Used (LRU) list
, setting it up for usage within the page replacement algorithm.*/
void lru_list_init (void);

/* Adds a given page to the LRU list, 
marking it as one of the candidates for replacement in the page eviction process.*/
void add_page_to_lru_list (struct page *page);

/* Removes a specified page from the LRU list, 
typically after it has been chosen for eviction or when it's no longer in use.*/
void del_page_from_lru_list (struct page *page);

/* Attempts to free pages in the system based on the provided allocation flags. 
It tries to reclaim unused or less frequently used pages to make them available for future allocations. */
void try_to_free_pages (enum palloc_flags flags);

/* Allocates a page based on the specified allocation flags, 
preparing it for use within the system. */
struct page *alloc_page (enum palloc_flags flags);

/* Frees the page associated with the provided kernel virtual address, 
making it available for further use.*/
void free_page (void *kdaar);

/* Internally frees the specified page, which might involve additional operations 
or clearing certain data structures associated with the page before its deallocation.*/
void _free_page (struct page *page);

#endif
