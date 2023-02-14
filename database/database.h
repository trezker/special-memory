#include <stdint.h>

typedef struct {
	char name[65];
	uint32_t size;
} Column;

typedef struct {
	char name[65];
	uint32_t num_columns;
	Column* columns;
} Table;

typedef struct {
	uint32_t num_tables;
	Table* tables;
} Database;

Database* db_open();
void db_close(Database* db);
void db_create_table(Database* db, const char* name);
const char* db_first_table(Database* db);
const char* db_next_table(Database* db, const char* name);
void db_add_column(Database* db, const char* table, const char* name, uint32_t size);
const char* db_get_first_column(Database* db, const char* table);