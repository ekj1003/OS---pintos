#ifndef VM_SWAP_H
#define VM_SWAP_H

#include <list.h>
#include <hash.h>
#include <bitmap.h>

struct bitmap *swap_page;

/* Sets up and allocates space on a storage device 
to be used as swap space during system initialization. */
void swap_init (void);

/* Retrieves data from swap space to the main memory 
using the provided swap slot index and kernel virtual address. */
void swap_in (size_t used_index, void *kaddr);

/* Moves data from the main memory to the swap space, returning the index of the used swap slot. 
This function helps free up space in the main memory by transferring pages to the swap space.*/
size_t swap_out (void *kaddr);

#endif
