#ifndef __XCACHE_CONTROLLER_H__
#define __XCACHE_CONTROLLER_H__
#include "xcache.h"
#include "xcache_slice.h"

xslice_t *xctrl_search(xcache_req_t *req);
xslice_t *xctrl_store(xcache_req_t *req, uint8_t *data);
int xctrl_add_xslice(xslice_t *xslice);
int xctrl_init(void);
void xctrl_handle_timeout(void);

#endif
