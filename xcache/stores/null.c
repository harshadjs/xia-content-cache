#include <stdio.h>
#include <xcache.h>

static struct xcache_plugin null_plugin = {
	.name = "null",
	.store = NULL,
	.get = NULL,
	.evict = NULL
};

void _xcache_plugin_init(void)
{
	printf("Inside null\n");
	xcache_register_plugin(&null_plugin);
}
