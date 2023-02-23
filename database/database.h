#ifndef DATABASE_H
#define DATABASE_H

#include <stdint.h>
#include <stdbool.h>
#include <uuid/uuid.h>
#include "pager.h"

typedef struct {
	char name[65];
	uint32_t cell_size;
	Pager* pager;
} Table;

typedef struct {
	uint32_t num_tables;
	Table* tables;
} Database;

typedef struct {
	Table* table;
	uint32_t page;
	uint32_t cell;
	bool end_of_table;
} Cursor;

Database* db_open();
void db_close(Database* db);
void db_create_table(Database* db, const char* name, uint32_t cell_size);
const char* db_first_table(Database* db);
const char* db_next_table(Database* db, const char* name);
void db_insert(Database* db, const char* table, void* data);
void db_select(Database* db, const char* table, uuid_t id, void* data);

void db_table_start(Database* db, const char* table, Cursor* cursor);
void db_cursor_value(Cursor* cursor, void* out);

#endif
