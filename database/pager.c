#include <stdlib.h>
#include "pager.h"

Pager* db_open_pager() {
	Pager* pager = malloc(sizeof(Pager));
	for(uint32_t i=0; i<MAX_PAGES; ++i) {
		pager->pages[i] = NULL;
	}
	pager->num_pages = 0;
	return pager;
}

void db_close_pager(Pager* pager) {
	for(uint32_t i=0; i<MAX_PAGES; ++i) {
		free(pager->pages[i]);
	}
	free(pager);
}

uint32_t db_get_unused_page(Pager* pager) {
	pager->pages[pager->num_pages] = malloc(PAGE_SIZE);
	return pager->num_pages++;
}

void* db_get_page(Pager* pager, uint32_t n) {
	return pager->pages[n];
}
