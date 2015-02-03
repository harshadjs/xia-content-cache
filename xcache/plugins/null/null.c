#include <stdio.h>
#include <xcache.h>

static struct xcache_plugin null_plugin = {
	.name = "null",
	.__xcache_store = NULL,
	.__xcache_search = NULL,
	.__xcache_timer = NULL,
	.__xcache_force_evict = NULL,
	.__xcache_notify_evict = NULL,
	.__xcache_exists = NULL,
};

void _xcache_plugin_init(void)
{
	printf("Inside null\n");
	xcache_register_plugin(&null_plugin);
}
