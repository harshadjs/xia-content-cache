#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <xcache.h>
#include <xcache_helpers.h>
#include <xcache_controller.h>
#include <xcache_core.h>
#include "xcache_main.h"

typedef struct {
	ht_t *meta_ht;
	dlist_node_t *expiry_list;
	uint64_t max_size;
	uint64_t cur_size;
} xctrl_t;

static xctrl_t xctrl;

#define DEFAULT_XCACHE_SIZ 1000 // TODO: Really?
#define BUCKETS 23

static void xcache_addto_expiry_list(xcache_meta_t *meta)
{
	/* TODO: Fill */
	return;
}

static xcache_meta_t *new_xcache_node(xcache_req_t *req)
{
	xcache_meta_t *xcache_node;

	xcache_node = (xcache_meta_t *)xcache_alloc(sizeof(xcache_meta_t));
	xcache_node->len = req->total_len;
	xcache_node->cid = req->ch.cid;
	xcache_node->cid.type = CLICK_XIA_XID_TYPE_CID;
	xcache_node->ttl = req->ch.ttl;
	xcache_node->cticks = xcache_node->aticks = ticks;

	return xcache_node;
}

xcache_meta_t *xctrl_get_meta(xcache_req_t *req)
{
	xcache_meta_t key;

	key.cid = req->ch.cid;

	return ht_search(xctrl.meta_ht, (void *)&key);
}

uint8_t *xctrl_get_data(xcache_meta_t *meta, uint8_t *data)
{
	return xcore_search(meta, data);
}

xcache_meta_t *xctrl_store(xcache_req_t *req, uint8_t *data)
{
	xcache_meta_t *meta;

	meta = new_xcache_node(req);
	if(!meta)
		return NULL;

	ht_add(xctrl.meta_ht, meta);

	xcache_addto_expiry_list(meta);
	return xcore_store(meta, data);
}

void
xctrl_remove(xcache_meta_t *meta)
{
	xcore_remove(meta);
	ht_remove(xctrl.meta_ht, meta);
}

void xctrl_timer(void)
{
	dlist_node_t *iter;
	xcache_meta_t *meta;

	for(iter = xctrl.expiry_list; dlist_data(iter);
		iter = dlist_next(iter)) {
		meta = (xcache_meta_t *)dlist_data(iter);
		if((ticks - meta->cticks) < (meta->ttl))
			continue;
		xctrl_remove(meta);
	}
}

void xctrl_send_timeout(xcache_meta_t *meta)
{
	xcache_req_t req;

	printf("Node timed out\n");
	memset(&req, 0, sizeof(req));
	req.request = XCACHE_TIMEOUT;

	req.ch.cid = meta->cid;
	req.ch.cid.type = htonl(CLICK_XIA_XID_TYPE_CID);

	xcache_raw_send((uint8_t *)&req, sizeof(req));
}

/* Hash table functions */
static int _hash(void *key)
{
	int i, sum = 0;
	xcache_meta_t *c = (xcache_meta_t *)key;

	for(i = 0; i < CLICK_XIA_XID_ID_LEN; i++)
		sum += c->cid.id[i];

	return (sum);
}

static int _compar(void *val1, void *val2)
{
	xcache_meta_t *c1, *c2;

	c1 = (xcache_meta_t *)val1;
	c2 = (xcache_meta_t *)val2;

	return memcmp(c1->cid.id, c2->cid.id, CLICK_XIA_XID_ID_LEN);
}

static void _cleanup(void *val)
{
	xcache_meta_t *c = (xcache_meta_t *)val;

	if(!c)
		return;

	xcache_free(c);
}

int xctrl_init(void)
{
	xctrl.meta_ht = ht_create(BUCKETS, _hash, _compar, _cleanup);
	if(!xctrl.meta_ht)
		return -1;

	xctrl.max_size = DEFAULT_XCACHE_SIZ;
	xctrl.cur_size = 0;

	return 0;
}
