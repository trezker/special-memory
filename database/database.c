#include <stdlib.h>
#include <string.h>
#include "database.h"

Database* db_open() {
	return malloc(sizeof(Database));
}

void db_close(Database* db) {
	free(db);
}

void db_create_table(Database* db, const char* name) {
	strncpy(db->table.name, name, 64);
}

const char* db_first_table(Database* db) {
	return db->table.name;
}