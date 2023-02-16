#include <stdint.h>

typedef struct {
	char name[65];
	uint32_t cell_size;
	void* data;
} Table;

typedef struct {
	uint32_t num_tables;
	Table* tables;
} Database;

Database* db_open();
void db_close(Database* db);
void db_create_table(Database* db, const char* name, uint32_t cell_size);
const char* db_first_table(Database* db);
const char* db_next_table(Database* db, const char* name);
void db_insert(Database* db, const char* table, void* data);
void db_select(Database* db, const char* table, uint32_t id, void* data);