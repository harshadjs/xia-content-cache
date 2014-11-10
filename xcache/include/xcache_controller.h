#ifndef __XCACHE_CONTROLLER_H__
#define __XCACHE_CONTROLLER_H__
#include "xcache.h"
#include "xcache_slice.h"

/** Xcache controller APIs
 ** ----------------------
 ** Following are the APIs provided by this layer.
 ** This is the thinnest layer. In future, if this code moves to kernel,
 ** this layer will go away.
 **/

/* Get xcache slice for request @req */
xslice_t *xctrl_search(xcache_req_t *req);

/* Store data in cache slice corresponding to  @req */
xslice_t *xctrl_store(xcache_req_t *req, uint8_t *data);

/* Add @xslice to the slice hash table */
int xctrl_add_xslice(xslice_t *xslice);

/* Init function for this layer: Initializes hash table */
int xctrl_init(void);

/*
 * Timeout function for this layer: This should be called on every
 * timeout (currently it is 1 second). It will subsequently invoke
 * appropriate xslice_* functions
 */
void xctrl_handle_timeout(void);

#endif
