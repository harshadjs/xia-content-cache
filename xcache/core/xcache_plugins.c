#include <stdio.h>
#include <xcache_plugins.h>
#include <xcache.h>

static dlist_node_t *plugins_list;

int xcache_register_plugin(struct xcache_plugin *plugin)
{
	dlist_insert_tail(&plugins_list, plugin);
	printf("Inserted into linked list\n");
	return 0;
}

void xcache_plugins_tick(uint64_t ticks)
{
	dlist_node_t *iter = plugins_list;
	struct xcache_plugin *plugin;

	while((plugin = (struct xcache_plugin *)dlist_data(iter)) != NULL) {
		if(plugin->__xcache_timer) {
			plugin->__xcache_timer(ticks);
		}
		iter = dlist_next(iter);
	}
}
