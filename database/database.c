#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "database.h"

typedef struct {
	uint32_t num_cells;
} Leaf;

void* leaf_node_cell(void* node, uint32_t cell_num, uint32_t cell_size) {
	return node + sizeof(Leaf) + cell_num * cell_size;
}

Database* db_open() {
	Database* db = malloc(sizeof(Database));
	db->num_tables = 0;
	db->tables = NULL;
	return db;
}

void db_close(Database* db) {
	for(int i=0; i<db->num_tables; ++i) {
		db_close_pager(db->tables[i].pager);
	}
	free(db->tables);
	free(db);
}

uint32_t db_find_table(Database* db, const char* name) {
	for(uint32_t i = 0; i<db->num_tables; ++i) {
		if(strncmp(name, db->tables[i].name, 64) == 0) {
			return i;
		}
	}
	return UINT32_MAX;
}

void db_create_table(Database* db, const char* name, uint32_t cell_size) {
	if(db_find_table(db, name) != UINT32_MAX) {
		return;
	}
	db->tables = realloc(db->tables, sizeof(Table)*(db->num_tables+1));
	strncpy(db->tables[db->num_tables].name, name, 64);
	db->tables[db->num_tables].cell_size = cell_size;

 	Pager* pager = db_open_pager();
	db->tables[db->num_tables].pager = pager;
	db_get_unused_page(pager);

	Leaf *node = db_get_page(pager, 0);
	node->num_cells = 0;
	//*leaf_node_num_cells(node) = 0;

	db->num_tables++;
}

const char* db_first_table(Database* db) {
	return db->tables[0].name;
}

const char* db_next_table(Database* db, const char* name) {
	uint32_t i = db_find_table(db, name);
	if(i+1 < db->num_tables) {
		return db->tables[i+1].name;
	}
	return NULL;
}

void db_insert(Database* db, const char* tablename, void* data) {
	uint32_t i = db_find_table(db, tablename);
	Table* table = &db->tables[i];
	Leaf* node = db_get_page(table->pager, 0);

	void* cell = leaf_node_cell(node, node->num_cells, table->cell_size);
	memcpy(cell, data, table->cell_size);
	node->num_cells += 1;
}

void db_select(Database* db, const char* tablename, uuid_t id, void* data) {
	uint32_t i = db_find_table(db, tablename);
	Table* table = &db->tables[i];
	Leaf* node = db_get_page(table->pager, 0);

	for(uint32_t i=0; i<node->num_cells; ++i) {
		void* cell = leaf_node_cell(node, i, table->cell_size);
		if(uuid_compare(*(uuid_t*)cell, id) == 0) {
			memcpy(data, cell, table->cell_size);
			return;
		}
	}
}
