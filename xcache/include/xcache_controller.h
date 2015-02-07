#ifndef __XCACHE_CONTROLLER_H__
#define __XCACHE_CONTROLLER_H__
#include "xcache.h"

/** Xcache controller APIs
 ** ----------------------
 ** Following are the APIs provided by this layer.
 ** This is the thinnest layer. In future, if this code moves to kernel,
 ** this layer will go away.
 **/

/* DONE Store data in cache slice corresponding to  @req */
xcache_meta_t *xctrl_store(xcache_req_t *req, uint8_t *data);

/* DONE Get meta from request */
xcache_meta_t *xctrl_get_meta(xcache_req_t *req);

/* DONE Get meta from request */
uint8_t *xctrl_get_data(xcache_meta_t *meta, uint8_t *data);

/* DONE Timer routine: Must be called once every timeout*/
void xctrl_timer(void);

/* DONE Init function for this layer: Initializes hash table */
int xctrl_init(void);

/* DONE Remove */
void xctrl_remove(xcache_meta_t *meta);

/* DONE Send timeout (Change name) */
void xctrl_send_timeout(xcache_meta_t *meta);
#endif
