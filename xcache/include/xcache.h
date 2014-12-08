#ifndef __XCACHE_H__
#define __XCACHE_H__
#include "hash_table.h"
#include "dlist.h"
#include <stddef.h>
#include <stdint.h>
#include "xia_cache_req.h"

/* Content object */
typedef struct {
	struct click_xia_xid cid;		/* Content ID */
	int len,	/* Length of the data object */
		full;	/* A boolean, set if the entire object is present */
	uint32_t ticks,	/* Time at which this object was inserted */
		ttl;	/* Time to live */
	uint8_t *data;	/* Actual data */
	int ref_count;
} xcache_node_t;

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

/* Xcache plugin initialization macro */
#ifdef XCACHE_PLUGIN_PRIO
#define PRIO XCACHE_PLUGIN_PRIO
#define _xcache_plugin_init \
	__attribute__((constructor((XCACHE_PLUGIN_PRIO) + 100))) _XCACHE_PLUGIN_INIT
#endif /* XCACHE_PLUGIN_PRIO */

struct xcache_plugin {
	/**
	 * __xcache_store:
	 * New content object arrived.
	 * @xslice_t: Corresponding slice
	 * @xcache_node_t: Object to be stored
	 * @returns 1 if the object was cached
	 * @returns 0 if the object was not cached
	 *
	 * Note: The function's return value is used by the xcache-core
	 * to keep object references.
	 */
	int (*__xcache_store)(xslice_t *, xcache_node_t *);

	/**
	 * __xcache_search:
	 * Search function
	 * @xslice_t: Corresponding slice
	 * @xcache_req_t: Received request
	 * @returns the result of search if search successful
	 * @returns NULL if not found
	 *
	 * Note: __xcache_search will be called for plugin 'n' only if
	 * (n-1) plugins before it returned NULL. That's why the order of plugins
	 * matters. The order of plugins is defined in plugins/order.conf.
	 */
	xcache_node_t *(*__xcache_search)(xslice_t *, xcache_req_t *);

	/**
	 * __xcache_timer:
	 * Timer function for this plugin. Called after every TICK_INTERVAL.
	 * @args ticks: Current number of ticks
	 * @returns cache_node, if this plugin decides to evict it.
	 */
	xcache_node_t *(*__xcache_timer)(uint64_t ticks);

	/**
	 * __xcache_force_evict:
	 * Force evict xcache_node. If this function is called, the plugin must
	 * remove the xcache_node from xslice. The xcache object will be freed after
	 * the function call and will not be safe to access anymore.
	 * @xslice_t: Corresponding slice
	 * @xcache_node_t: Object to be evicted
	 */
	void (*__xcache_force_evict)(xslice_t *, xcache_node_t *);
};

/**
 * xcache_register_plugin:
 * Register Xcache plugin.
 */
int xcache_register_plugin(struct xcache_plugin *plugin);

#endif
