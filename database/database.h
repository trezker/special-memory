typedef struct {
} Database;

Database* db_open();
void db_close(Database* db);
