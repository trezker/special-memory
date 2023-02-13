typedef struct {
	char name[64];
} Table;

typedef struct {
	Table table;
} Database;

Database* db_open();
void db_close(Database* db);
void db_create_table(Database* db, const char* name);
const char* db_first_table(Database* db);