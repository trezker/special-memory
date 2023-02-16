#include <stdlib.h>
#include "pager.h"

Pager* db_open_pager() {
	Pager* pager = malloc(sizeof(Pager));
	return pager;
}