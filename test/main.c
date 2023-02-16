#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "assert.h"
#include "memorydebug.h"
#include "../database/database.h"

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

typedef struct {
	void (*f)(void);
} Test;

/*
TODO:
Fixed keysize
Table creation take rowsize
Don't define columns, but create index giving type and offset.
*/
int main(int argc, char* argv[]) {
	clear_allocations();

	int num_tests = 0;
	Test tests[16];
	tests[num_tests++].f = &test_database_can_be_opened_and_closed;
	tests[num_tests++].f = &test_database_can_create_a_table;
	tests[num_tests++].f = &test_database_can_create_multiple_tables;
	tests[num_tests++].f = &test_database_can_not_create_duplicate_tables;

	for(int i=0; i<num_tests; ++i) {
		tests[i].f();
		if(!check_allocations()) {
			printf("Memory fault on test #%i\n", i);
			return EXIT_FAILURE;
		}
	}

	printf("TESTING COMPLETE\n");
	return EXIT_SUCCESS;
}
