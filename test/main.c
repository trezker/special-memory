#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "assert.h"
#include "memorydebug.h"
#include "../database/database.h"
#include "../database/pager.h"

uuid_t last_reverse_id;

void uuid_generate_reverse(uuid_t u) {
	uuid_copy(u, last_reverse_id);
	int i = 15;
	while(last_reverse_id[i] == 0) {
		--last_reverse_id[i];
		--i;
	}
	--last_reverse_id[i];
}

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
	uuid_generate_time(in1.id);
	strncpy(in1.text, "Hagrid", 256);
	db_insert(db, table, &in1);
	Stuff in2;
	uuid_generate_time(in2.id);
	strncpy(in2.text, "Harry", 256);
	db_insert(db, table, &in2);

	Stuff out;
	Cursor cursor;
	db_table_start(db, "stuff", &cursor);
	db_cursor_value(&cursor, &out);
	assert_equal(false, cursor.end)
	assert_equal_uuid(in1.id, out.id);

	db_cursor_next(&cursor);
	db_cursor_value(&cursor, &out);
	assert_equal(false, cursor.end)
	assert_equal_uuid(in2.id, out.id);

	db_cursor_next(&cursor);
	assert_equal(true, cursor.end)

	db_close(db);
}

void test_cursor_can_traverse_pages() {
	Database* db = db_open();
	const char* table = "stuff";
	db_create_table(db, table, sizeof(Stuff));

	int num_items = 30;
	char suuid[36];
	for(int i=0; i<num_items; ++i) {
		Stuff in;
		uuid_generate(in.id);
		
		uuid_unparse(in.id, suuid);
		printf("%s\n", suuid);

		sprintf(in.text, "name%i", i);
		db_insert(db, table, &in);
	}

	int i = 0;
	Stuff out;
	uuid_t prev;
	Cursor cursor;
	db_table_start(db, "stuff", &cursor);
	db_cursor_value(&cursor, &out);
	while(cursor.end == false) {
		uuid_unparse(out.id, suuid);
		uuid_copy(prev, out.id);
//		printf("Checking %i Page: %i Cell: %i UUID: %s, Text: %s\n", i, cursor.page, cursor.cell, suuid, out.text);
//		hexDump(NULL, &out, sizeof(Stuff));
		++i;
		db_cursor_next(&cursor);
		if(cursor.end == false) {
			db_cursor_value(&cursor, &out);
//			uuid_unparse(out.id, suuid);
//			printf("Checking %i Page: %i Cell: %i UUID: %s, Text: %s\n", i, cursor.page, cursor.cell, suuid, out.text);
//			hexDump(NULL, &out, sizeof(Stuff));
			sprintf(suuid, "Iterator: %i", i);
			assert_less_than_uuid_m(prev, out.id, suuid);
		}
	}

	assert_equal(num_items, i);

	db_close(db);
}

void test_key_dataset(char keys[][37], uint32_t num_items) {
	Database* db = db_open();
	const char* table = "stuff";
	db_create_table(db, table, sizeof(Stuff));

	char suuid[36];
	for(int i=0; i<num_items; ++i) {
		Stuff in;
		uuid_parse(keys[i], in.id);
		
//		uuid_unparse(in.id, suuid);
//		printf("%s\n", keys[i]);

		sprintf(in.text, "name%i", i);
		db_insert(db, table, &in);
	}

	int i = 0;
	Stuff out;
	uuid_t prev;
	Cursor cursor;
	db_table_start(db, "stuff", &cursor);
	db_cursor_value(&cursor, &out);
	while(cursor.end == false) {
		uuid_unparse(out.id, suuid);
		uuid_copy(prev, out.id);
//		printf("Checking %i Page: %i Cell: %i UUID: %s, Text: %s\n", i, cursor.page, cursor.cell, suuid, out.text);
//		hexDump(NULL, &out, sizeof(Stuff));
		++i;
		db_cursor_next(&cursor);
		if(cursor.end == false) {
			db_cursor_value(&cursor, &out);
//			uuid_unparse(out.id, suuid);
//			printf("Checking %i Page: %i Cell: %i UUID: %s, Text: %s\n", i, cursor.page, cursor.cell, suuid, out.text);
//			hexDump(NULL, &out, sizeof(Stuff));
			sprintf(suuid, "Iterator: %i", i);
			assert_less_than_uuid_m(prev, out.id, suuid);
		}
	}

	assert_equal(num_items, i);

	db_close(db);
}

void test_key_dataset_1() {
	char keystrings[33][37] = {
		"aad4d207-4ebc-42d2-b69b-6aa6c8270f1c",
		"4e55239b-a8c8-4a5f-98d1-b0cdd67b295c",
		"aaa8158a-4c5f-4ac8-87fa-6b98d38f742f",
		"7ed4640f-fa46-45b2-bf6a-56ffa262a2cf",
		"e8a91d10-04c2-4a2f-aca9-6b941a571819",
		"35fa16d8-85e0-4728-9a68-648008a2170e",
		"080c27c3-f4c0-41b9-a2cd-c7b51420c5bb",
		"a339aecd-e1f9-4823-bbcc-852da343e483",
		"3f80ad14-0bba-4d6d-8578-cfa58b354726",
		"b754b986-78f3-4a66-ace8-f9dada8ad74e",
		"93c59117-17ea-4f65-bcba-d1c70ac3611a",
		"f48b7e1d-619f-45eb-b1a8-1804c67d9f2a",
		"d496a0ff-ab95-4b27-bb90-de98be7ba2c4",
		"5b669e29-3988-41a7-8a1b-b6d98f81f84c",
		"49b976cf-9f96-4dce-979b-b5a7564704ff",
		"eed357ae-be30-4616-b7bb-7fbe715be6db",
		"c9efdcc6-71b3-4eda-963a-60d593151cc4",
		"c6d01e2e-37f4-4833-8b71-d5a6649f7a9b",
		"b1d48292-2bd1-4447-8bbb-47731499284a",
		"32135faf-d984-4459-82e8-b7ba7216edd1",
		"ec01633a-394a-4963-901d-f574621f724f",
		"915edf39-00e6-4ecb-9d0d-4eeb90eff787",
		"490c75c9-1b7f-431d-b5f1-4c0d27b91c75",
		"f017a070-6fa6-4530-bffb-2bcd7005b9ef",
		"37de3626-5395-4247-98ab-1518768b6600",
		"8e1963bd-d474-4cd5-9a0e-ba9bb62a8406",
		"bfe43dad-ee1b-4525-abb5-5117fb22aa1c",
		"bc01c22d-57a6-4ac3-8ed8-b231249ac332",
		"f13d8a62-9ce7-4350-9205-5ef524df5f7d",
		"5bfddc97-3bdd-4c05-9c39-eb411a6dc19d",
		"890bdf25-56af-438d-a2ac-83da73e65e06",
		"1e831198-a7ca-441d-9af5-bde849b7169a",
		"99678cee-e2bf-485b-87b6-b835d678edd9",
	};
	test_key_dataset(keystrings, 33);
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
	//add_test(test_cursor_can_traverse_pages);

	add_test(test_key_dataset_1);

	for(int i=0; i<num_tests; ++i) {
		memset(last_reverse_id, 255, sizeof(uuid_t));
		tests[i].f();
		if(!check_allocations()) {
			printf("Memory fault on test at line #%i\n", tests[i].line);
			return EXIT_FAILURE;
		}
	}

	printf("TESTING COMPLETE\n");
	return EXIT_SUCCESS;
}
