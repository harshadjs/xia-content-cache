#ifndef __XCACHE_SLICE_H__
#define __XCACHE_SLICE_H__

#include <stdint.h>
#include "hash_table.h"
#include "xia_cache_req.h"
#include "xcache.h"

#define KB(__bytes) ((__bytes) * 1024)
#define MB(__bytes) ((KB(__bytes)) * 1024)

#define DEFAULT_XCACHE_SIZ MB(1)

typedef struct {
	struct click_xia_xid hid;
	uint32_t cur_size, max_size;
	dlist_node_t *xcache_lru_list;
	ht_t *xcache_content_ht;
} xslice_t;

xcache_node_t *xslice_store(xslice_t *xslice, xcache_req_t *req, uint8_t *data);
xcache_node_t *xslice_search(xslice_t *xslice, xcache_req_t *req);
xslice_t *new_xslice(xcache_req_t *req);
void xslice_free(xslice_t *xslice);
void xslice_handle_timeout(xslice_t *xslice);

#endif
