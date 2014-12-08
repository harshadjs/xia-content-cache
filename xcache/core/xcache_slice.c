#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <xcache.h>
#include <xcache_helpers.h>
#include <xcache_slice.h>
#include <xcache_plugins.h>
#include "xcache_main.h"

#define BUCKETS 23
static int xslice_hash(void *key)
{
	int i, sum = 0;
	xcache_meta_t *c = (xcache_meta_t *)key;

	for(i = 0; i < CLICK_XIA_XID_ID_LEN; i++)
		sum += c->cid.id[i];

	return (sum);
}

/**
 * cmp_fn:
 * Compare function
 * @args
 * @returns
 */
static int xslice_cmp(void *val1, void *val2)
{
	xcache_meta_t *c1, *c2;

	c1 = (xcache_meta_t *)val1;
	c2 = (xcache_meta_t *)val2;

	return memcmp(c1->cid.id, c2->cid.id, CLICK_XIA_XID_ID_LEN);
}

/**
 * cleanup_fn:
 * Cleanup function
 * @args
 * @returns
 */
static void xslice_cleanup(void *val)
{
	xcache_meta_t *c = (xcache_meta_t *)val;

	if(!c)
		return;

	xcache_free(c);
}

static void xcache_addto_expiry_list(xslice_t *xslice, xcache_meta_t *meta)
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
	xcache_node->full = 0;
	/* TODO XXX: TTL = 50? */
	xcache_node->ttl = 50;
	xcache_node->ticks = ticks;

	return xcache_node;
}

uint8_t *xslice_search(xcache_meta_t **xcache_node, xslice_t *xslice,
					   xcache_req_t *req)
{
	xcache_meta_t c;

	c.cid = req->ch.cid;

	*xcache_node = (xcache_meta_t *)ht_search(xslice->meta_ht, &c);
	if(!*xcache_node)
		return NULL;

	return xcache_plugin_search(xslice, *xcache_node);
}

xcache_meta_t *xslice_store(xslice_t *xslice, xcache_req_t *req, uint8_t *data)
{
	xcache_meta_t *xcache_node;

	xslice_search(&xcache_node, xslice, req);

	if((xcache_node) /* && (xcache_node->full) */ ) {
		/* XXX: Should we modify it? */
		return NULL;
	}

	xcache_node = new_xcache_node(req);
	ht_add(xslice->meta_ht, xcache_node);

#ifdef __not_yet
	if(req->offset + req->len == xcache_node->len) {
		xcache_node->full = 1;
		dlist_insert_tail(&xslice->expiry_list, xcache_node);
	}
#endif

	xcache_addto_expiry_list(xslice, xcache_node);
	xcache_plugin_store(xslice, xcache_node, data);

	return xcache_node;
}

void xslice_free(xslice_t *xslice)
{
	dlist_flush(&xslice->expiry_list, NULL);
	ht_cleanup(xslice->meta_ht);
}

void xslice_send_timeout(xslice_t *xslice, xcache_meta_t *node)
{
	xcache_req_t req;

	printf("Node timed out\n");
	memset(&req, 0, sizeof(req));
	req.request = XCACHE_TIMEOUT;

	req.ch.cid = node->cid;
	req.ch.cid.type = htonl(CLICK_XIA_XID_TYPE_CID);

	req.hid = xslice->hid;
	req.hid.type = htonl(CLICK_XIA_XID_TYPE_HID);

	xcache_raw_send((uint8_t *)&req, sizeof(req));
}

void xslice_handle_timeout(xslice_t *xslice)
{
	dlist_node_t *iter;
	xcache_meta_t *node;

	for(iter = xslice->expiry_list; dlist_data(iter);
		iter = dlist_next(iter)) {
		node = (xcache_meta_t *)dlist_data(iter);
		if((ticks - node->ticks) < (node->ttl))
			continue;

		/* Timed out node */
		xslice_send_timeout(xslice, node);

		/* TODO: Expiry list must be sorted by the expected expiry times */
		ht_remove(xslice->meta_ht, node);
		dlist_remove_node(&xslice->expiry_list, &iter, NULL);
	}
}

xslice_t *new_xslice(xcache_req_t *req)
{
	xslice_t *xslice;

	xslice = xcache_alloc(sizeof(xslice_t));

	dlist_init(&xslice->expiry_list);
	xslice->meta_ht =
		ht_create(BUCKETS, xslice_hash, xslice_cmp, xslice_cleanup);

	xslice->hid = req->hid;
	xslice->max_size = DEFAULT_XCACHE_SIZ;
	xslice->cur_size = 0;

	if(!xslice->meta_ht) {
		xcache_free(xslice);
		return NULL;
	}

	return xslice;
}
