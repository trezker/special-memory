#include <stdint.h>

typedef struct {
	char name[65];
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