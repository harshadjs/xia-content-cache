#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <xcache.h>
#include <xcache_helpers.h>
#include <xcache_slice.h>

#define BUCKETS 23
static int xslice_hash(void *key)
{
	int i, sum = 0;
	xcache_node_t *c = (xcache_node_t *)key;

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
	xcache_node_t *c1, *c2;

	c1 = (xcache_node_t *)val1;
	c2 = (xcache_node_t *)val2;

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
	xcache_node_t *c = (xcache_node_t *)val;

	if(!c)
		return;

	if(c->data)
		xcache_free(c->data);
	xcache_free(c);
}

static xcache_node_t *new_xcache_node(xcache_req_t *req)
{
	xcache_node_t *xcache_node;

	xcache_node = (xcache_node_t *)xcache_alloc(sizeof(xcache_node_t));
	xcache_node->len = req->total_len;
	xcache_node->data = (uint8_t *)xcache_alloc(xcache_node->len);
	xcache_node->cid = req->ch.cid;
	xcache_node->cid.type = CLICK_XIA_XID_TYPE_CID;
	xcache_node->full = 0;
	xcache_node->ttl = 50;
	xcache_node->ticks = ticks;

	return xcache_node;
}

xcache_node_t *xslice_search(xslice_t *xslice, xcache_req_t *req)
{
	xcache_node_t c, *xcache_node;

	c.cid = req->ch.cid;

	xcache_node = (xcache_node_t *)ht_search(xslice->xcache_content_ht, &c);

	if(xcache_node)
		return xcache_node;

	return NULL;
}

static void xslice_apply_policy(xslice_t *xslice)
{
	dlist_node_t *tobe_removed;
	xcache_node_t *node;

	if(xslice->cur_size <= xslice->max_size)
		return;

	while(xslice->cur_size > xslice->max_size) {
		tobe_removed = xslice->xcache_lru_list;
		if(!tobe_removed)
			return;

		node = dlist_data(tobe_removed);
		dlist_remove_head(&xslice->xcache_lru_list, NULL);
		ht_remove(xslice->xcache_content_ht, dlist_data(tobe_removed));
		xslice->cur_size -= node->len;
	}
}

xcache_node_t *xslice_store(xslice_t *xslice, xcache_req_t *req, uint8_t *data)
{
	xcache_node_t *xcache_node;

	xcache_node = xslice_search(xslice, req);

	if((xcache_node) && (xcache_node->full)) {
		/* XXX: Should we modify it? */
		return NULL;
	}

	if(!xcache_node) {
		xcache_node = new_xcache_node(req);
		ht_add(xslice->xcache_content_ht, xcache_node);
		xslice->cur_size += xcache_node->len;
		xslice_apply_policy(xslice);
	}

	if(req->offset + req->len == xcache_node->len) {
		xcache_node->full = 1;
		dlist_insert_tail(&xslice->xcache_lru_list, xcache_node);
	}

	memcpy(xcache_node->data + req->offset, data, req->len);

	return xcache_node;
}

void xslice_free(xslice_t *xslice)
{
	dlist_flush(&xslice->xcache_lru_list, NULL);
	ht_cleanup(xslice->xcache_content_ht);
}

void xslice_send_timeout(xslice_t *xslice, xcache_node_t *node)
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
	dlist_node_t *iter = xslice->xcache_lru_list;
	xcache_node_t *node;

	for(iter = xslice->xcache_lru_list; dlist_data(iter);
		iter = dlist_next(iter)) {
		node = (xcache_node_t *)dlist_data(iter);
		if((ticks - node->ticks) < (node->ttl))
			continue;

		/* Timed out node */
		xslice_send_timeout(xslice, node);
		ht_remove(xslice->xcache_content_ht, node);
		dlist_remove_node(&xslice->xcache_lru_list, &iter, NULL);
	}
}

xslice_t *new_xslice(xcache_req_t *req)
{
	xslice_t *xslice;

	xslice = xcache_alloc(sizeof(xslice_t));

	dlist_init(&xslice->xcache_lru_list);
	xslice->xcache_content_ht =
		ht_create(BUCKETS, xslice_hash, xslice_cmp, xslice_cleanup);

	xslice->hid = req->hid;
	xslice->max_size = DEFAULT_XCACHE_SIZ;
	xslice->cur_size = 0;

	if(!xslice->xcache_content_ht) {
		xcache_free(xslice);
		return NULL;
	}

	return xslice;
}
