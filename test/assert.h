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

#define assert_equal(a, b) { \
	if(a != b) { \
		printf("%s:%i: Expected \"%i\", got \"%i\"\n", __FILE__, __LINE__, a, b); \
		exit(EXIT_FAILURE); \
	} \
}
