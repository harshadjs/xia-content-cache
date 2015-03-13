#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include "logger.h"
#include "xcache.h"
#include "slice.h"
#include "helpers.h"
#include "controller.h"
#include "core.h"
#include "meta.h"
#include "xcache_main.h"
#include "policies.h"
#include "cli.h"

static xctrl_t xctrl;

#define DEFAULT_XCACHE_SIZ  ((uint64_t)(500 * 1024 * 1024))
#define BUCKETS 23

static xcache_slice_t *new_slice(xcache_req_t *req)
{
	xcache_slice_t *slice;

	slice = (xcache_slice_t *)xalloc(sizeof(xcache_slice_t));
	slice->ttl = req->context.ttl;
	slice->context_id = req->context.context_id;
	slice->max_size = req->context.cache_size;
	slice->policy = POLICY_ID_TO_OBJ(req->context.cache_policy);

	return slice;
}

static xcache_slice_t *lookup_slice(xcache_req_t *req)
{
	xcache_slice_t key;

	key.context_id = req->context.context_id;

	return ht_search(xctrl.slice_ht, &key);
}

static xcache_slice_t *
create_or_lookup_slice(xcache_req_t *req)
{
	xcache_slice_t *slice = lookup_slice(req);

	if(slice)
		return slice;

	slice = new_slice(req);
	if(!slice)
		return NULL;

	xslice_init(slice);

	ht_add(xctrl.slice_ht, slice);
	return slice;
}

xcache_meta_t *xctrl_store(xcache_req_t *req, uint8_t *data)
{
	xcache_slice_t *slice;
	xcache_meta_t key, *meta;

	slice = create_or_lookup_slice(req);
	if(!slice) {
		printf("%s: Could not create or lookup slice, context_id = %d.\n",
			   __func__, req->context.context_id);
		return NULL;
	}

	key.cid = req->cid;
	meta = ht_search(xctrl.meta_ht, &key);
	if(!meta) {
		meta = xcache_req2meta(req);
		ht_add(xctrl.meta_ht, meta);
	}

	return xslice_store(slice, meta, data);
}

xcache_meta_t *xctrl_search(uint8_t **data, xcache_req_t *req)
{
	xcache_slice_t *slice = lookup_slice(req);

	if(!slice) {
		xcache_meta_t key, *meta;

		log(LOG_DEBUG, "Slice lookup failed.\n");
		key.cid = req->cid;
		meta = ht_search(xctrl.meta_ht, &key);
		if(!meta) {
			log(LOG_DEBUG, "Meta lookup failed - Should not happen.\n");
			return NULL;
		}
		slice = meta->slices[0];
	}

	return xslice_search(data, slice, req);
}

void xctrl_remove_slice(xcache_slice_t *slice)
{
	ht_remove(xctrl.slice_ht, slice);
}

void
xctrl_remove(xcache_meta_t *meta)
{
	xcore_remove(meta);
	ht_remove(xctrl.meta_ht, meta);
}

void xctrl_send_timeout(xcache_meta_t *meta)
{
	xcache_req_t req;

	printf("Node timed out\n");
	memset(&req, 0, sizeof(req));
	req.request = XCACHE_TIMEOUT;

	req.cid = meta->cid;
	req.cid.type = htonl(CLICK_XIA_XID_TYPE_CID);

	xcache_raw_send((uint8_t *)&req, sizeof(req));
}

/* Hash table functions */
static int _hash(void *key)
{
	return ((xcache_slice_t *)key)->context_id;
}

static int _compar(void *val1, void *val2)
{
	xcache_slice_t *s1 = (xcache_slice_t *)val1,
		*s2 = (xcache_slice_t *)val2;

	return (s1->context_id == s2->context_id) ? 0 : 1;
}

static void _cleanup(void *val)
{
	xcache_slice_t *slice = (xcache_slice_t *)val;

	if(!slice)
		return;

	xfree(slice);
}

void xctrl_cli_list_meta(void)
{
	ht_iter_t iter;
	xcache_meta_t *meta;

	ht_iter_init(&iter, xctrl.meta_ht);
	while((meta = (xcache_meta_t *)ht_iter_data(&iter)) != NULL) {
		char cid[512];

		memset(cid, 0, 512);
		cid2str(cid, &meta->cid);
		printf("[%s]\t%d\t%d\n", cid, meta->stats.hits, ticks - meta->ticks);
		ht_iter_next(&iter);
	}
}

int xctrl_init(void)
{
	xctrl.slice_ht = ht_create(BUCKETS, _hash, _compar, _cleanup);
	xctrl.meta_ht = xcache_new_metaht(METAHT_USE_CLEANUP);
	xctrl.max_size = DEFAULT_XCACHE_SIZ;
	xctrl.cur_size = 0;

	return 0;
}
