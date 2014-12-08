#include <stdio.h>
#include <xcache.h>

int plugin1_store(xslice_t *xslice, xcache_node_t *node)
{
	return 0;
}

xcache_node_t *plugin1_search(xslice_t *xslice, xcache_req_t *req)
{
	return NULL;
}

void plugin1_force_evict(xslice_t *xslice, xcache_node_t *node)
{
	return;
}

xcache_node_t *plugin1_timer(uint64_t ticks)
{
	printf("Timer called: %d\n", (int)ticks);
	return NULL;
}

static struct xcache_plugin plugin1 = {
	.__xcache_store = plugin1_store,
	.__xcache_search = plugin1_search,
	.__xcache_timer = plugin1_timer,
	.__xcache_force_evict = plugin1_force_evict,
};

void _xcache_plugin_init(void)
{
	printf("Inside %s\n", __func__);
	/* Plugin Init */
	xcache_register_plugin(&plugin1);
}
