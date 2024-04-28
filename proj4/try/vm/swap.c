#include "vm/swap.h"
#include "vm/page.h"
#include "vm/frame.h"
#include "devices/block.h"

const size_t NUM_PAGE_BLOCK = PGSIZE / BLOCK_SECTOR_SIZE;

void swap_init (void) {
    int num = 1024 * 8;
	swap_bitmap = bitmap_create (num);
}

void swap_in(size_t used_index, void *kaddr) {
	struct block *disk_swapped;
    disk_swapped = block_get_role (BLOCK_SWAP);

    if(!bitmap_test(swap_bitmap, used_index)) return;
	else {
        int i = 0;
        while((size_t) i<NUM_PAGE_BLOCK) {
            block_read (disk_swapped, NUM_PAGE_BLOCK * used_index + i, BLOCK_SECTOR_SIZE * i + kaddr);
            i++;
        }
		bitmap_reset (swap_bitmap, used_index);
	}
}

size_t swap_out(void *kaddr) {
	struct block *disk_swapped;
	size_t idx_swapped;

    disk_swapped = block_get_role (BLOCK_SWAP);
    idx_swapped = bitmap_scan (swap_bitmap, 0, 1, false);

	if (BITMAP_ERROR != idx_swapped) {
        int i =0;
        while ((size_t) i<NUM_PAGE_BLOCK) {
            block_write (disk_swapped, NUM_PAGE_BLOCK * idx_swapped + i, BLOCK_SECTOR_SIZE * i + kaddr);
            i++;
        }
		bitmap_set (swap_bitmap, idx_swapped, true);
	}
	return idx_swapped;
}