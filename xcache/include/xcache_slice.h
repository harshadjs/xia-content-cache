#ifndef __XCACHE_SLICE_H__
#define __XCACHE_SLICE_H__

#include <stdint.h>
#include "hash_table.h"
#include "xia_cache_req.h"
#include "xcache.h"

/* bytes to KB */
#define KB(__bytes) ((__bytes) * 1024)

/* bytes to MB */
#define MB(__bytes) ((KB(__bytes)) * 1024)

/*
 * XXX: This is for testing,
 * Increase this limit
 */
#define DEFAULT_XCACHE_SIZ MB(1)

/** xslice_t: Identfies one cache slice **/
typedef struct {
	struct click_xia_xid hid;	/* Host identifier */
	uint32_t cur_size,	/* Current size in bytes of cache */
		max_size;		/*
						 * Maximum allowed size, after which
						 * policy should be applied
						 */
	/*
	 * xcache_lru_list:
	 * LRU sorted list of data blocks.
	 * key: CID, value: xcache_node_t (please see @xcache.h)
	 */
	dlist_node_t *xcache_lru_list;
	/*
	 * xcache_content_ht:
	 * Hash table of all the content objects
	 * Key: CID, value: xcache_node_t
	 */
	ht_t *xcache_content_ht;
} xslice_t;

/** xslice layer APIs */

/* Store @data as per @req in cache slice @xslice */
xcache_node_t *xslice_store(xslice_t *xslice, xcache_req_t *req, uint8_t *data);

/* Search for xcache_node_t in @xslice as per @req (req contains CID) */
xcache_node_t *xslice_search(xslice_t *xslice, xcache_req_t *req);

/* Create a new cache slice as per @req. (req contains HID) */
xslice_t *new_xslice(xcache_req_t *req);

/* Free up xslice */
void xslice_free(xslice_t *xslice);

/* Find out timed out nodes in xslice, and send back timeout
 * messages to click
 */
void xslice_handle_timeout(xslice_t *xslice);

#endif
