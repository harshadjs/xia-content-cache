#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "xcache.h"
#include "hash_table.h"

static uint64_t bytes;
static uint64_t max_cache_size;

xret_t memht_store(xcache_meta_t *meta, uint8_t *data)
{
	printf("%s\n", __func__);
	SET_STORE_PRIV(meta, data);


	bytes += meta->len;
	printf("Cached\n");
	return RET_CACHED;
}

xret_t memht_get(xcache_meta_t *meta, uint8_t *data)
{
	printf("%s\n", __func__);
	memcpy(data, GET_STORE_PRIV(meta), meta->len);
	return RET_OK;
}

xret_t memht_evict(xcache_meta_t *meta)
{
	free(GET_STORE_PRIV(meta));
	return RET_OK;
}

static struct xcache_store ht = {
	.name = "memht",
	.store = memht_store,
	.get = memht_get,
	.evict = memht_evict,
	.conf.max_size = UNLIMITED_SIZE,
	.conf.max_items = UNLIMITED_ITEMS,
	.conf.max_size = UNLIMITED_BANDWIDTH,
};

void _xcache_store_init(void)
{
	printf("Inside memht\n");
	max_cache_size = 1024 * 1; /* 1024 bytes */

	/* Store Init */
	xcache_register_store(&ht);
}
