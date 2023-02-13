#include <stdlib.h>
#include "database.h"

Database* db_open() {
	return malloc(sizeof(Database));
}

void db_close(Database* db) {
	free(db);
}
