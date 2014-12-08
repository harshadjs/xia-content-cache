#ifndef __XCACHE_H__
#define __XCACHE_H__
#include "hash_table.h"
#include "dlist.h"
#include <stddef.h>
#include <stdint.h>
#include "xia_cache_req.h"

/* Content object */
typedef struct {
	/* Content ID */
	struct click_xia_xid cid;

	/* Length of the data object */
	int len;

	/* A boolean, set if the entire object is present */
	int full;

	/* Time at which this object was inserted */
	uint32_t ticks;

	/* Time to live */
	uint32_t ttl;

	/* Number of plugins holding this data */
	int ref_count;
} xcache_meta_t;

/** xslice_t: Identfies one cache slice **/
typedef struct {
	/* Host identifier */
	struct click_xia_xid hid;

	/* Current size in bytes of cache */
	uint32_t cur_size;

	/*
	 * Maximum allowed size, after which
	 * policy should be applied
	 */
	uint32_t max_size;

	/*
	 * xcache_timeout_list:
	 * LRU sorted list of data blocks.
	 * key: CID, value: xcache_meta_t (please see @xcache.h)
	 */
	dlist_node_t *expiry_list;

	/*
	 * xcache_content_ht:
	 * Hash table of all the content objects
	 * Key: CID, value: xcache_meta_t
	 */
	ht_t *meta_ht;

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
	 * @xcache_meta_t: Object metadata to be stored
	 * @uint8_t *: Data to be stored, must be of length xcache_meta_t->len
	 * @returns 1 if the object was cached
	 * @returns 0 if the object was not cached
	 *
	 * Note: The function's return value is used by the xcache-core
	 * to keep object references.
	 */
	int (*__xcache_store)(xslice_t *, xcache_meta_t *, uint8_t *);

	/**
	 * __xcache_search:
	 * Search function
	 * @xslice_t: Corresponding slice
	 * @xcache_req_t: Received request
	 * @returns Data for searched object
	 * @returns NULL if not found
	 *
	 * Note: __xcache_search will be called for plugin 'n' only if
	 * (n-1) plugins before it returned NULL. That's why the order of plugins
	 * matters. The order of plugins is defined in plugins/order.conf.
	 */
	uint8_t *(*__xcache_search)(xslice_t *, xcache_meta_t *);

	/**
	 * __xcache_timer:
	 * Timer function for this plugin. Called after every TICK_INTERVAL.
	 * @args ticks: Current number of ticks
	 * @returns cache_node, if this plugin decides to evict it.
	 */
	xcache_meta_t *(*__xcache_timer)(uint64_t ticks);

	/**
	 * __xcache_force_evict:
	 * Force evict xcache_node. If this function is called, the plugin must
	 * remove the xcache_node from xslice. The xcache object will be freed after
	 * the function call and will not be safe to access anymore.
	 * @xslice_t: Corresponding slice
	 * @xcache_meta_t: Object to be evicted
	 */
	void (*__xcache_evict)(xslice_t *, xcache_meta_t *);
};

/**
 * xcache_register_plugin:
 * Register Xcache plugin.
 */
int xcache_register_plugin(struct xcache_plugin *plugin);

void *xcache_alloc(size_t size);

#endif
