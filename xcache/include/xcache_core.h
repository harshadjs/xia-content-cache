#ifndef __XCACHE_CORE_H__
#define __XCACHE_CORE_H__
#include <xcache.h>

/* DONE add a plugin to list */
void xcore_addtolist(struct xcache_plugin *plugin);

/* DONE Store in a plugin */
xcache_meta_t *xcore_store(xcache_meta_t *meta, uint8_t *data);

/* Done search from a plugin */
uint8_t *xcore_search(xcache_meta_t *meta, uint8_t *data);

/* Done remove from a plugin */
void xcore_remove(xcache_meta_t *meta);

#endif
