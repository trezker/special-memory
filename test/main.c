#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <stdbool.h>
#include "../database/database.h"

int out_of_memory = false;

void *malloc(size_t size) {
	if(out_of_memory) {
		return NULL;
	}
	void* (*original_malloc)(size_t size) = dlsym(RTLD_NEXT, "malloc");
	return original_malloc(size);
}

#define assert_not_null(x) { \
	if(x == NULL) { \
		printf("%s:%i: Expected NOT NULL\n", __FILE__, __LINE__); \
		exit(EXIT_FAILURE); \
	} \
}

#define assert_null(x) { \
	if(x != NULL) { \
		printf("%s:%i: Expected NULL\n", __FILE__, __LINE__); \
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

void test_database_can_create_multiple_tables() {
	Database* db = db_open();

	const char* table1 = "one";
	const char* table2 = "two";
	db_create_table(db, table1);
	db_create_table(db, table2);
	const char* first_table = db_first_table(db);
	const char* next_table = db_next_table(db, first_table);
	const char* end_of_tables = db_next_table(db, next_table);
	assert_not_null(first_table);
	assert_equal_string(table1, first_table);
	assert_not_null(next_table);
	assert_equal_string(table2, next_table);
	assert_null(end_of_tables);

	db_close(db);
}

void test_database_can_not_create_duplicate_tables() {
	Database* db = db_open();

	const char* table1 = "one";
	const char* table2 = "two";
	db_create_table(db, table1);
	db_create_table(db, table2);
	db_create_table(db, table1);
	db_create_table(db, table2);
	const char* first_table = db_first_table(db);
	const char* next_table = db_next_table(db, first_table);
	const char* end_of_tables = db_next_table(db, next_table);
	assert_not_null(first_table);
	assert_equal_string(table1, first_table);
	assert_not_null(next_table);
	assert_equal_string(table2, next_table);
	assert_null(end_of_tables);

	db_close(db);
}

void test_database_can_add_columns() {
	Database* db = db_open();
	const char* table = "one";
	db_create_table(db, table);

	db_add_column(db, table, "key", sizeof(uint32_t));
	const char* column = db_get_first_column(db, table);
	assert_not_null(column);
	assert_equal_string("key", column);

	db_close(db);
}

int main(int argc, char* argv[]) {
	test_database_can_be_opened_and_closed();
	test_database_can_create_a_table();
	test_database_can_create_multiple_tables();
	test_database_can_not_create_duplicate_tables();
	test_database_can_add_columns();

	printf("TESTING COMPLETE\n");
	return EXIT_SUCCESS;
}
