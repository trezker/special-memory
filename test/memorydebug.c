#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <dlfcn.h>

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

void hexDump (char *desc, void *addr, int len) {
    int i;
    unsigned char buff[17];       // stores the ASCII data
    unsigned char *pc = addr;     // cast to make the code cleaner.

    // Output description if given.

    if (desc != NULL)
        printf ("%s:\n", desc);

    // Process every byte in the data.

    for (i = 0; i < len; i++) {
        // Multiple of 16 means new line (with line offset).

        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line.

            if (i != 0)
                printf ("  %s\n", buff);

            // Output the offset.

            printf ("  %04x ", i);
        }

        // Now the hex code for the specific character.

        printf (" %02x", pc[i]);

        // And store a printable ASCII character for later.

        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }

    // Pad out last line if not exactly 16 characters.

    while ((i % 16) != 0) {
        printf ("   ");
        i++;
    }

    // And print the final ASCII bit.

    printf ("  %s\n", buff);
}
