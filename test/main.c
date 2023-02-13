#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../database/database.h"

#define assert_not_null(x) { \
	if(x == NULL) { \
		printf("%s:%i: Expected NOT NULL\n", __FILE__, __LINE__); \
		exit(EXIT_FAILURE); \
	} \
}

#define assert_equal_string(a, b) { \
	if(strncmp(a, b, strlen(a) != 0)) { \
		printf("%s:%i: Expected \"%s\", got \"%s\"\n", __FILE__, __LINE__, a, b); \
		exit(EXIT_FAILURE); \
	} \
}

void test_database_can_be_opened_and_closed() {
	Database* db = db_open();
	assert_not_null(db);
	db_close(db);
}

void test_database_can_create_a_table() {
	Database* db = db_open();

	const char* table_name = "test";
	db_create_table(db, table_name);
	const char* first_table = db_first_table(db);
	assert_not_null(first_table);
	assert_equal_string(table_name, first_table);

	db_close(db);
}

int main(int argc, char* argv[]) {
	test_database_can_be_opened_and_closed();
	test_database_can_create_a_table();

	printf("TESTING COMPLETE\n");
	return EXIT_SUCCESS;
}
