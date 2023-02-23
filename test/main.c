#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "assert.h"
#include "memorydebug.h"
#include "../database/database.h"
#include "../database/pager.h"

typedef struct {
	uuid_t id;
	char text[257];
} Stuff;

void test_database_can_be_opened_and_closed() {
	Database* db = db_open();
	assert_not_null(db);
	db_close(db);
}

void test_database_can_create_a_table() {
	Database* db = db_open();

	const char* table_name = "test";
	db_create_table(db, table_name, 100);
	const char* first_table = db_first_table(db);
	assert_not_null(first_table);
	assert_equal_string(table_name, first_table);

	db_close(db);
}

void test_database_can_create_multiple_tables() {
	Database* db = db_open();

	const char* table1 = "one";
	const char* table2 = "two";
	db_create_table(db, table1, sizeof(uuid_t));
	db_create_table(db, table2, 2);
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
	db_create_table(db, table1, 1);
	db_create_table(db, table2, 2);
	db_create_table(db, table1, 3);
	db_create_table(db, table2, 4);
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

void test_database_can_insert_and_select_data() {
	Database* db = db_open();
	const char* table = "stuff";
	db_create_table(db, table, sizeof(Stuff));

	Stuff in1;
	uuid_generate(in1.id);
	strncpy(in1.text, "Hagrid", 256);
	db_insert(db, table, &in1);
	Stuff in2;
	uuid_generate(in2.id);
	strncpy(in2.text, "Harry", 256);
	db_insert(db, table, &in2);

	Stuff out;
	db_select(db, table, in1.id, &out);
	assert_equal_uuid(in1.id, out.id);

	db_select(db, table, in2.id, &out);
	assert_equal_uuid(in2.id, out.id);

	db_close(db);
}

void test_pager_can_be_opened_and_closed() {
	Pager* pager = db_open_pager();
	db_close_pager(pager);
}

void test_pager_provides_writable_pages() {
	Pager* pager = db_open_pager();

	uint32_t n1 = db_get_unused_page(pager);
	uint32_t n2 = db_get_unused_page(pager);
	void* page1 = db_get_page(pager, n1);
	void* page2 = db_get_page(pager, n2);
	memset(page1, 0, 4096);
	memset(page2, 1, 4096);

	db_close_pager(pager);
}

void test_cursor_can_step_through_a_table() {
	Database* db = db_open();
	const char* table = "stuff";
	db_create_table(db, table, sizeof(Stuff));

	Stuff in1;
	uuid_generate(in1.id);
	strncpy(in1.text, "Hagrid", 256);
	db_insert(db, table, &in1);
	Stuff in2;
	uuid_generate(in2.id);
	strncpy(in2.text, "Harry", 256);
	db_insert(db, table, &in2);

	Cursor cursor;
	db_table_start(db, "stuff", &cursor);
	Stuff out;
	db_cursor_value(&cursor, &out);

	assert_equal_uuid(in1.id, out.id);

	db_close(db);
}

typedef struct {
	void (*f)(void);
	uint32_t line;
} Test;

#define add_test(function) { \
	tests[num_tests].f = function; \
	tests[num_tests].line = __LINE__; \
	num_tests++; \
}

/*
TODO:
Write tests to implement functions using Cursor.
Implement btree.
Don't define columns, but create index giving type and offset.
*/
int main(int argc, char* argv[]) {
	printf("Stuff: %li\n", sizeof(Stuff));
	printf("uuid: %li\n", sizeof(uuid_t));
	printf("System pagesize: %ld\n", sysconf(_SC_PAGESIZE));
	
	clear_allocations();

	int num_tests = 0;
	Test tests[16];
	add_test(test_database_can_be_opened_and_closed);
	add_test(test_database_can_create_a_table);
	add_test(test_database_can_create_multiple_tables);
	add_test(test_database_can_not_create_duplicate_tables);
	add_test(test_database_can_insert_and_select_data);

	add_test(test_pager_can_be_opened_and_closed);
	add_test(test_pager_provides_writable_pages);

	add_test(test_cursor_can_step_through_a_table);

	for(int i=0; i<num_tests; ++i) {
		tests[i].f();
		if(!check_allocations()) {
			printf("Memory fault on test at line #%i\n", tests[i].line);
			return EXIT_FAILURE;
		}
	}

	printf("TESTING COMPLETE\n");
	return EXIT_SUCCESS;
}
