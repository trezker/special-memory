#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "database.h"

const uint32_t PAGE_SIZE = 4096;
const uint32_t LEAF_NODE_NUM_CELLS_SIZE = sizeof(uint32_t);
const uint32_t LEAF_NODE_NUM_CELLS_OFFSET = 0;
const uint32_t LEAF_NODE_HEADER_SIZE = LEAF_NODE_NUM_CELLS_OFFSET + LEAF_NODE_NUM_CELLS_SIZE;

uint32_t* leaf_node_num_cells(void* node) {
	return node + LEAF_NODE_NUM_CELLS_OFFSET;
}

void* leaf_node_cell(void* node, uint32_t cell_num, uint32_t cell_size) {
	return node + LEAF_NODE_HEADER_SIZE + cell_num * cell_size;
}

Database* db_open() {
	Database* db = malloc(sizeof(Database));
	db->num_tables = 0;
	db->tables = NULL;
	return db;
}

void db_close(Database* db) {
	for(int i=0; i<db->num_tables; ++i) {
		free(db->tables[i].data);
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

	db->tables[db->num_tables].data = malloc(PAGE_SIZE);
	void *node = db->tables[db->num_tables].data;
	*leaf_node_num_cells(node) = 0;

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

	uint32_t num_cells = *leaf_node_num_cells(table->data);
	void* cell = leaf_node_cell(table->data, num_cells, table->cell_size);
	memcpy(cell, data, table->cell_size);
	*leaf_node_num_cells(table->data) += 1;
}

void db_select(Database* db, const char* tablename, uuid_t id, void* data) {
	uint32_t i = db_find_table(db, tablename);
	Table* table = &db->tables[i];

	uint32_t num_cells = *leaf_node_num_cells(table->data);
	for(uint32_t i=0; i<num_cells; ++i) {
		void* cell = leaf_node_cell(table->data, i, table->cell_size);
		if(uuid_compare(*(uuid_t*)cell, id) == 0) {
			memcpy(data, cell, table->cell_size);
			return;
		}
	}
}