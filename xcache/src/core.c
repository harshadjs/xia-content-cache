#include <stdio.h>
#include "core.h"
#include "xcache.h"


static dlist_node_t *stores_list;

#define foreach_store													\
	for(iter = stores_list, store = NULL;								\
		(store = (struct xcache_store *)dlist_data(iter)) != NULL;	\
		iter = dlist_next(iter))


void xcore_addtolist(struct xcache_store *store)
{
	dlist_insert_tail(&stores_list, store);
}

static struct xcache_store *xcore_get_store(xcache_meta_t *meta)
{
	static dlist_node_t *iter = NULL;
	struct xcache_store *store;

	if(!iter)
		iter = stores_list;

	store = (struct xcache_store *)dlist_data(iter);

	iter = dlist_next(iter);

	if(store->get)
		return store;
	else
		return xcore_get_store(meta);
}

xcache_meta_t *xcore_store(xcache_meta_t *meta, uint8_t *data)
{
	struct xcache_store *store = xcore_get_store(meta);

	if(!store)
		return NULL;

	meta->store = store;
	store->store(meta, data);

	return meta;
}

/* NOTE: Must be freed */
xcache_meta_t *xcore_search(uint8_t **data, xcache_meta_t *meta)
{
	if(!meta)
		return NULL;

	*data = xalloc(meta->len);
	if(!*data)
		return NULL;

	meta->store->get(meta, *data);

	return meta;
}

/** CLI Commands **/
void xcore_list_stores(void)
{
	dlist_node_t *iter;
	struct xcache_store *store;

	for(iter = stores_list, store = NULL;
		(store = (struct xcache_store *)dlist_data(iter)) != NULL;
		iter = dlist_next(iter)) {
		printf("%s\n", store->name);
		printf("\tCapacity: 0x%Lx\n", (unsigned long long)store->conf.max_size);
		printf("\tCurrent size: 0x%Lx\n",
			   (unsigned long long)store->conf.current_size);
		printf("\tMaximum items: 0x%x\n", store->conf.max_items);
		printf("\tbandwidth: %d\n", store->conf.bandwidth);
	}
}

/* TODO: Should not directly remove, what about slices ? */
void xcore_remove(xcache_meta_t *meta)
{
	meta->store->evict(meta);
}

