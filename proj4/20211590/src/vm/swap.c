#include "vm/swap.h"
#include "devices/block.h"
#include "vm/frame.h"
#include "vm/page.h"

const size_t PAGE_BLOCK_NUM = PGSIZE / BLOCK_SECTOR_SIZE;

void swap_init (void) {
	int num = 8*1024;
	swap_page = bitmap_create (num);
}

void swap_in (size_t used_index, void *kaddr) {
	struct block *swap_disk;

	swap_disk = block_get_role (BLOCK_SWAP);

	if (bitmap_test (swap_page, used_index)) {
		int i = 0;
		while ((size_t) i<PAGE_BLOCK_NUM) {
			block_read (swap_disk, PAGE_BLOCK_NUM * used_index + i, BLOCK_SECTOR_SIZE * i + kaddr);
			i++;
		}

		bitmap_reset (swap_page, used_index);
	}
}

size_t swap_out (void *kaddr) {
	struct block *swap_disk;
	size_t swap_idx;

	swap_disk = block_get_role (BLOCK_SWAP);
	swap_idx = bitmap_scan (swap_page, 0, 1, false);

	if (BITMAP_ERROR != swap_idx) {
		int i =0;
		while ((size_t) i<PAGE_BLOCK_NUM) {
			block_write (swap_disk, PAGE_BLOCK_NUM * swap_idx + i, BLOCK_SECTOR_SIZE * i + kaddr);
			i++;
		}
		bitmap_set (swap_page, swap_idx, true);
	}
	return swap_idx;
}
