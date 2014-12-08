#include <stdio.h>
#include <xcache_plugins.h>
#include <xcache.h>

static dlist_node_t *plugins_list;

#define foreach_plugin													\
	for(iter = plugins_list, plugin = NULL;								\
		(plugin = (struct xcache_plugin *)dlist_data(iter)) != NULL;	\
		iter = dlist_next(iter))


int xcache_register_plugin(struct xcache_plugin *plugin)
{
	dlist_insert_tail(&plugins_list, plugin);
	printf("Inserted into linked list\n");
	return 0;
}

void
xcache_plugin_store(xslice_t *xslice, xcache_meta_t *xcache_node, void *data)
{
	dlist_node_t *iter;
	struct xcache_plugin *plugin;

	foreach_plugin {
		if(plugin->__xcache_store) {
			plugin->__xcache_store(xslice, xcache_node, data);
		}
	}
}

uint8_t *
xcache_plugin_search(xslice_t *xslice, xcache_meta_t *node)
{
	uint8_t *data = NULL;
	dlist_node_t *iter;
	struct xcache_plugin *plugin;

	foreach_plugin {
		if(plugin->__xcache_search) {
			data = plugin->__xcache_search(xslice, node);
			if(data)
				return data;
		}
	}

	return NULL;
}


void xcache_plugins_tick(uint64_t ticks)
{
	dlist_node_t *iter;
	struct xcache_plugin *plugin;

	foreach_plugin {
		if(plugin->__xcache_timer) {
			plugin->__xcache_timer(ticks);
		}
	}
}

