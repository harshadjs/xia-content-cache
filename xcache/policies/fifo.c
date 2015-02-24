#include <stdio.h>
#include "dlist.h"
#include "xcache.h"
#include "policies.h"

static xret_t fifo_store(xcache_slice_t *slice, xcache_meta_t *meta)
{
	printf("%s\n", __func__);
	return RET_OK;
}

static xret_t fifo_get(xcache_slice_t *slice, xcache_meta_t *meta)
{
	printf("%s\n", __func__);
	return RET_OK;
}

static xret_t fifo_remove(xcache_slice_t *slice, xcache_meta_t *meta)
{
	printf("%s\n", __func__);
	return RET_OK;
}

static xret_t fifo_make_room(xcache_slice_t *slice, xcache_meta_t *meta)
{
	printf("%s\n", __func__);
	return RET_OK;
}

struct xcache_policy xcache_policy_fifo = {
	.store = fifo_store,
	.get = fifo_get,
	.remove = fifo_remove,
	.make_room = fifo_make_room,
};
