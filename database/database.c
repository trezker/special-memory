#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "database.h"

typedef struct {
	uuid_t key;
	uint32_t page;
} Child;

#define NODE_HEADER struct { \
	uint8_t type; \
	uint8_t num_cells; \
	uint32_t parent; \
	union { \
		uint32_t next_leaf; \
		uint32_t last_child; \
	}; \
}

#define NODE_SPACE_FOR_CELLS (PAGE_SIZE-sizeof(NODE_HEADER))
#define INTERNAL_NODE_MAX_CELLS (NODE_SPACE_FOR_CELLS/sizeof(Child))

typedef struct {
	NODE_HEADER;
	union {
		uint8_t cellspace[NODE_SPACE_FOR_CELLS];
		Child children[INTERNAL_NODE_MAX_CELLS];
	};
} Node;

uint32_t leaf_max_cells(Table* table) {
	return NODE_SPACE_FOR_CELLS / table->cell_size;
}

void* leaf_node_cell(Node* node, uint32_t cell_num, uint32_t cell_size) {
	return node->cellspace + cell_num * cell_size;
}

Database* db_open() {
//	printf("Internal node size: %li\n", sizeof(Internal));
//	printf("Internal max_cells: %li\n", INTERNAL_NODE_MAX_CELLS);
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
//	printf("Leaf max cells: %i\n", leaf_max_cells(&db->tables[db->num_tables]));
//	printf("Leaf node size: %li\n", sizeof(uint32_t)*2 + cell_size*(leaf_max_cells(&db->tables[db->num_tables])));

 	Pager* pager = db_open_pager();
	db->tables[db->num_tables].pager = pager;
	db_get_unused_page(pager);

	Node *node = db_get_page(pager, 0);
	node->num_cells = 0;
	node->next_leaf = 0;

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
	Node* node = db_get_page(table->pager, 0);
	uint32_t max_cells = leaf_max_cells(table);
	if(node->num_cells == max_cells) {
		uint32_t next_page = db_get_unused_page(table->pager);
		Node* next_node = db_get_page(table->pager, next_page);
		next_node->num_cells = max_cells/2;
		node->num_cells -= next_node->num_cells;
		void* from = leaf_node_cell(node, node->num_cells, table->cell_size);
		memcpy(next_node->cellspace, from, next_node->num_cells*table->cell_size);
		next_node->next_leaf = node->next_leaf;
		node->next_leaf = next_page;
		node = next_node;
	}
	void* cell = leaf_node_cell(node, node->num_cells, table->cell_size);
	memcpy(cell, data, table->cell_size);
	node->num_cells += 1;
}

void db_select(Database* db, const char* tablename, uuid_t id, void* data) {
	uint32_t i = db_find_table(db, tablename);
	Table* table = &db->tables[i];
	Node* node = db_get_page(table->pager, 0);

	for(uint32_t i=0; i<node->num_cells; ++i) {
		void* cell = leaf_node_cell(node, i, table->cell_size);
		if(uuid_compare(*(uuid_t*)cell, id) == 0) {
			memcpy(data, cell, table->cell_size);
			return;
		}
	}
}

void db_table_start(Database* db, const char* tablename, Cursor* cursor) {
	uint32_t i = db_find_table(db, tablename);
	cursor->table = &db->tables[i];
	cursor->page = 0;
	cursor->cell = 0;
	cursor->end = false;
}

void db_cursor_value(Cursor* cursor, void* out) {
	Table* table = cursor->table;
	Node* node = db_get_page(table->pager, cursor->page);
	void* cell = leaf_node_cell(node, cursor->cell, table->cell_size);
	memcpy(out, cell, table->cell_size);
}

void db_cursor_next(Cursor* cursor) {
	++cursor->cell;
	Table* table = cursor->table;
	Node* node = db_get_page(table->pager, cursor->page);
	if(cursor->cell >= node->num_cells) {
		if(node->next_leaf == 0) {
			cursor->end = true;
			return;
		}
		cursor->cell = 0;
		cursor->page = node->next_leaf;
	}
}