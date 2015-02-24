#ifndef __XCACHE_CORE_H__
#define __XCACHE_CORE_H__
#include <xcache.h>

/* DONE add a store to list */
void xcore_addtolist(struct xcache_store *store);

/* DONE Store in a store */
xcache_meta_t *xcore_store(xcache_meta_t *meta, uint8_t *data);

/* Done search from a store */
xcache_meta_t *xcore_search(uint8_t **data, xcache_meta_t *meta);

/* Done remove from a store */
void xcore_remove(xcache_meta_t *meta);

#endif
