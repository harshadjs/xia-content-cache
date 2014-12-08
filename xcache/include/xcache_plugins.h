#ifndef __XCACHE_PLUGINS_H__
#define __XCACHE_PLUGINS_H__
#include <xcache.h>

void xcache_plugins_tick(uint64_t ticks);
uint8_t *
xcache_plugin_search(xslice_t *xslice, xcache_meta_t *xcache_node);
void
xcache_plugin_store(xslice_t *xslice, xcache_meta_t *xcache_node, void *data);
#endif
