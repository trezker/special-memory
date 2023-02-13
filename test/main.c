#include <stdio.h>
#include <stdlib.h>
#include "../database/database.h"

#define assert_not_null(x) { \
	if(x == NULL) { \
		printf("%s:%i: Expected NOT NULL\n", __FILE__, __LINE__); \
		exit(EXIT_FAILURE); \
	} \
}

void test_database_can_be_opened_and_closed() {
	Database* db = db_open();
	assert_not_null(db);
	db_close(db);
}

int main(int argc, char* argv[]) {
	test_database_can_be_opened_and_closed();

	printf("TESTING COMPLETE\n");
	return EXIT_SUCCESS;
}
