#ifndef PAGER_H
#define PAGER_H

#include <stdint.h>

#define MAX_PAGES 100
#define PAGE_SIZE 4096

typedef struct {
	uint32_t num_pages;
	void* pages[MAX_PAGES];
} Pager;

Pager* db_open_pager();
void db_close_pager(Pager* pager);

uint32_t db_get_unused_page(Pager* pager);
void* db_get_page(Pager* pager, uint32_t n);

#endif
