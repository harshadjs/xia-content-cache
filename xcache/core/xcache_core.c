#include <stdio.h>
#include <xcache_core.h>
#include <xcache.h>


static dlist_node_t *plugins_list;

#define foreach_plugin													\
	for(iter = plugins_list, plugin = NULL;								\
		(plugin = (struct xcache_plugin *)dlist_data(iter)) != NULL;	\
		iter = dlist_next(iter))


void xcore_addtolist(struct xcache_plugin *plugin)
{
	dlist_insert_tail(&plugins_list, plugin);
}

static struct xcache_plugin *xcore_get_plugin(xcache_meta_t *meta)
{
	static dlist_node_t *iter = NULL;
	struct xcache_plugin *plugin;

	if(!iter)
		iter = plugins_list;

	plugin = (struct xcache_plugin *)dlist_data(iter);

	iter = dlist_next(iter);

	if(plugin->get)
		return plugin;
	else
		return xcore_get_plugin(meta);
}

xcache_meta_t *xcore_store(xcache_meta_t *meta, uint8_t *data)
{
	struct xcache_plugin *plugin = xcore_get_plugin(meta);

	if(!plugin)
		return NULL;

	meta->plugin = plugin;
	plugin->store(meta, data);

	return meta;
}

uint8_t *xcore_search(xcache_meta_t *meta, uint8_t *data)
{
	if(!meta)
		return NULL;

	meta->plugin->get(meta, data);

	return NULL;
}

void xcore_remove(xcache_meta_t *meta)
{
	meta->plugin->evict(meta);
}
