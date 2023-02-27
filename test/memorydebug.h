#include <stdbool.h>

void clear_allocations();
bool check_allocations();
void hexDump(char *desc, void *addr, int len);