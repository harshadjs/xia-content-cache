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

/** xslice layer APIs */

/* Store @data as per @req in cache slice @xslice */
xcache_meta_t *xslice_store(xslice_t *xslice, xcache_req_t *req, uint8_t *data);

/* Search for xcache_meta_t in @xslice as per @req (req contains CID) */
uint8_t *
xslice_search(xcache_meta_t **xcache_node, xslice_t *xslice, xcache_req_t *req);

/* Create a new cache slice as per @req. (req contains HID) */
xslice_t *new_xslice(xcache_req_t *req);

/* Free up xslice */
void xslice_free(xslice_t *xslice);

/* Find out timed out nodes in xslice, and send back timeout
 * messages to click
 */
void xslice_handle_timeout(xslice_t *xslice);

#endif
