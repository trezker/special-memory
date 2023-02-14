#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <stdbool.h>
#include "../database/database.h"

typedef struct {
	void* ptr;
	bool freed;
} Allocation;

#define MAX_ALLOCATIONS 1024
Allocation allocations[MAX_ALLOCATIONS];
int num_allocations = 0;

void free_allocation(void* p) {
	for(int i=0; i<MAX_ALLOCATIONS; ++i) {
		if(allocations[i].ptr == p) {
			allocations[i].freed = true;
			break;
		}
	}
}

void *malloc(size_t size) {
	void* (*original_malloc)(size_t) = dlsym(RTLD_NEXT, "malloc");
	void* p = original_malloc(size);
	allocations[num_allocations].ptr = p;
	num_allocations++;
	return p;
}

void *realloc(void* ptr, size_t size) {
	void* (*original_realloc)(void*, size_t) = dlsym(RTLD_NEXT, "realloc");
	void* p = original_realloc(ptr, size);
	if(p != ptr) {
		allocations[num_allocations].ptr = p;
		num_allocations++;
		if(ptr != NULL) {
			free_allocation(ptr);
		}
	}
	return p;
}

void free(void* ptr) {
	void (*original_free)(void*) = dlsym(RTLD_NEXT, "free");
	original_free(ptr);
	free_allocation(ptr);
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

void clear_allocations() {
	for(int i=0; i<num_allocations; ++i) {
		allocations[i].ptr = NULL;
		allocations[i].freed = false;
	}
	num_allocations = 0;
}

bool check_allocations() {
	for(int i=0; i<num_allocations; ++i) {
		if(allocations[i].freed == false) {
			printf("Allocations: %i\n", num_allocations);
			printf("Allocation #%i not freed: %p\n", i, allocations[i].ptr);
			return false;
		}
	}
	clear_allocations();
	return true;
}

typedef struct {
	void (*f)(void);
} Test;

int main(int argc, char* argv[]) {
	clear_allocations();

	int num_tests = 0;
	Test tests[16];
	tests[num_tests++].f = &test_database_can_be_opened_and_closed;
	tests[num_tests++].f = &test_database_can_create_a_table;
	tests[num_tests++].f = &test_database_can_create_multiple_tables;
	tests[num_tests++].f = &test_database_can_not_create_duplicate_tables;
	tests[num_tests++].f = &test_database_can_add_columns;

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
