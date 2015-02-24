#include "xcache.h"
#include "core.h"

int xcache_register_store(struct xcache_store *store)
{
	xcore_addtolist(store);
	return 0;
}
