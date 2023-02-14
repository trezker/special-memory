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
	for(uint32_t i = 0; i<db->num_tables; ++i) {
		free(db->tables[i].columns);
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

void db_create_table(Database* db, const char* name) {
	if(db_find_table(db, name) != UINT32_MAX) {
		return;
	}
	db->tables = realloc(db->tables, sizeof(Table)*(db->num_tables+1));
	db->tables[db->num_tables].columns = NULL;
	strncpy(db->tables[db->num_tables].name, name, 64);
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

void db_add_column(Database* db, const char* tablename, const char* name, uint32_t size) {
	uint32_t i = db_find_table(db, tablename);
	if(i>=db->num_tables) {
		return;
	}
	Table* table = &db->tables[i];
	table->columns = realloc(
		table->columns, 
		sizeof(Column)*(table->num_columns+1)
	);
	strncpy(table->columns[table->num_columns].name, name, 64);
	table->columns[table->num_columns].size = size;
	table->num_columns++;
}

const char* db_get_first_column(Database* db, const char* tablename) {
	uint32_t i = db_find_table(db, tablename);
	if(i>=db->num_tables) {
		return NULL;
	}
	if(db->tables[i].num_columns==0) {
		return NULL;
	}
	return db->tables[i].columns[0].name;
}
