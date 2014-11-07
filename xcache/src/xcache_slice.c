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

	return memcmp(&c1->cid, &c2->cid, XID_STRUCT_LEN);
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
	xcache_node->data = (char *)xcache_alloc(xcache_node->len);
	xcache_node->cid = req->ch.cid;
	xcache_node->full = 0;

	return xcache_node;
}

xcache_node_t *xslice_search(xslice_t *xslice, xcache_req_t *req)
{
	xcache_node_t c, *xcache_node;

	c.cid = req->ch.cid;

	xcache_node = (xcache_node_t *)ht_search(xslice->xcache_content_ht, &c);

	if(xcache_node && xcache_node->full)
		return xcache_node;

	return NULL;
}

xcache_node_t *xslice_store(xslice_t *xslice, xcache_req_t *req, char *data)
{
	xcache_node_t *xcache_node;

	xcache_node = xslice_search(xslice, req);

	if(!xcache_node) {
		xcache_node = new_xcache_node(req);
		ht_add(xslice->xcache_content_ht, xcache_node);
	} else {
		/* XXX: Should we modify it? */
		if(xcache_node->full)
			return NULL;
	}

	if(req->offset + req->len == xcache_node->len)
		xcache_node->full = 1;
	memcpy(xcache_node->data + req->offset, data, req->len);

	return xcache_node;
}

void xslice_free(xslice_t *xslice)
{
	dlist_flush(&xslice->xcache_lru_list, NULL);
	ht_cleanup(xslice->xcache_content_ht);
}

xslice_t *new_xslice(xcache_req_t *req)
{
	xslice_t *xslice;

	xslice = xcache_alloc(sizeof(xslice_t));

	dlist_init(&xslice->xcache_lru_list);
	xslice->xcache_content_ht =
		ht_create(BUCKETS, xslice_hash, xslice_cmp, xslice_cleanup);
	if(!xslice->xcache_content_ht) {
		xcache_free(xslice);
		return NULL;
	}

	return xslice;
}
