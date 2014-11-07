#ifndef __XCACHE_SLICE_H__
#define __XCACHE_SLICE_H__

#include "hash_table.h"
#include "xia_cache_req.h"
#include "xcache.h"

typedef struct {
	struct click_xia_xid hid;
	dlist_node_t *xcache_lru_list;
	ht_t *xcache_content_ht;
} xslice_t;

xcache_node_t *xslice_store(xslice_t *xslice, xcache_req_t *req, char *data);
xcache_node_t *xslice_search(xslice_t *xslice, xcache_req_t *req);
xslice_t *new_xslice(xcache_req_t *req);
void xslice_free(xslice_t *xslice);

#endif
