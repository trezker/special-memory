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
	if(strncmp(a, b, strlen(a)) != 0) { \
		printf("%s:%i: \n\tExpected: \"%s\" \n\tActual:   \"%s\"\n", __FILE__, __LINE__, a, b); \
		exit(EXIT_FAILURE); \
	} \
}

#define assert_equal(a, b) { \
	if(a != b) { \
		printf("%s:%i: \n\tExpected: %i \n\tActual:   %i\n", __FILE__, __LINE__, a, b); \
		exit(EXIT_FAILURE); \
	} \
}

#define assert_equal_uuid(a, b) { \
	if(uuid_compare(a, b) != 0) { \
		char uta[36]; \
		char utb[36]; \
		uuid_unparse(a, uta); \
		uuid_unparse(b, utb); \
		printf("%s:%i: \n\tExpected: \"%s\" \n\tActual:   \"%s\"\n", __FILE__, __LINE__, uta, utb); \
		exit(EXIT_FAILURE); \
	} \
}

#define assert_less_than_uuid(a, b) { \
	if(uuid_compare(a, b) > 0) { \
		char uta[36]; \
		char utb[36]; \
		uuid_unparse(a, uta); \
		uuid_unparse(b, utb); \
		printf("%s:%i: \n\tExpected <: \"%s\" \n\tActual:     \"%s\"\n", __FILE__, __LINE__, uta, utb); \
		exit(EXIT_FAILURE); \
	} \
}
