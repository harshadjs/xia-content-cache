#include <xcache.h>

static struct xcache_plugin null_plugin = {
	.__xcache_store = NULL,
	.__xcache_search = NULL,
	.__xcache_timer = NULL,
	.__xcache_evict = NULL,
};

void _xcache_plugin_init(void)
{
	xcache_register_plugin(&null_plugin);
}
