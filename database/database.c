#include <stdlib.h>
#include <string.h>
#include "database.h"

Database* db_open() {
	Database* db = malloc(sizeof(Database));
	db->num_tables = 0;
	db->tables = NULL;
	return db;
}

void db_close(Database* db) {
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

void db_create_table(Database* db, const char* name, uint32_t rowsize) {
	if(db_find_table(db, name) != UINT32_MAX) {
		return;
	}
	db->tables = realloc(db->tables, sizeof(Table)*(db->num_tables+1));
	strncpy(db->tables[db->num_tables].name, name, 64);
	db->tables[db->num_tables].rowsize = rowsize;
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
