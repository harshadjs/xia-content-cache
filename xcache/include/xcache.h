#ifndef __XCACHE_H__
#define __XCACHE_H__
#include "hash_table.h"
#include "dlist.h"
#include <stddef.h>
#include <stdint.h>
#include "xia_cache_req.h"

/**
 * Xcache Plugin Macros:
 */
#ifdef XCACHE_PLUGIN_PRIO
#define PRIO XCACHE_PLUGIN_PRIO
/* Initializtion macro */
#define _xcache_plugin_init \
	__attribute__((constructor((XCACHE_PLUGIN_PRIO) + 100))) _XCACHE_PLUGIN_INIT

#define SET_REF(__xcache_meta, __plugin_obj)				\
	(__xcache_meta)->__xcache_plugin_obj[XCACHE_PLUGIN_PRIO]	\
				   = (void *)(__plugin_obj)

#define GET_REF(__xcache_meta)									\
	(__xcache_meta)->__xcache_plugin_obj[XCACHE_PLUGIN_PRIO]

#define xcache_evict(__xslice, __xmeta)						\
	__xcache_evict(XCACHE_PLUGIN_PRIO, __xslice, __xmeta)

#endif /* XCACHE_PLUGIN_PRIO */

#include <_plugins.autogen.h>

typedef struct {
	uint16_t bm[(XCACHE_N_PLUGINS + 1) / 16];
} xplugin_bm_t;

#define __BM_INDEX(__plugin) ((__plugin) % 16)
#define __BM_OFF(__plugin) ((__plugin) % 16)

#define BM_SET(__xplugin_bm, __plugin) \
	(((__xplugin_bm).bm[__BM_INDEX(__plugin)]) |= (1 << (__BM_OFF(__plugin))))
#define BM_CLR(__xplugin_bm, __plugin) \
	(((__xplugin_bm).bm[__BM_INDEX(__plugin)]) &= ~(1 << (__BM_OFF(__plugin))))
#define BM_GET(__xplugin_bm, __plugin) \
	(((__xplugin_bm).bm[__BM_INDEX(__plugin)]) & (1 << (__BM_OFF(__plugin))))

typedef enum {
	RET_CACHED,
	RET_UNCACHED,
	RET_EVICTED,
	RET_OK,
	RET_FAIL,
} xret_t;

/* Content object */
typedef struct {
	/* Content ID */
	struct click_xia_xid cid;

	/* Length of the data object */
	int len;

	/* A boolean, set if the entire object is present */
	int full;

	/* Time at which this object was created */
	uint32_t cticks;

	/* Time at which this object was inserted */
	uint32_t aticks;

	/* Time to live */
	uint32_t ttl;

	/* Number of plugins holding this data */
	int ref_count;

	/* xcache plugin objects */
	void *__xcache_plugin_obj[XCACHE_N_PLUGINS];
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

struct xcache_plugin {
	char *name;

	/**
	 * __xcache_store:
	 * New content object arrived.
	 * @xslice_t: Corresponding slice
	 * @xcache_meta_t: Object metadata to be stored
	 * @uint8_t *: Data to be stored, must be of length xcache_meta_t->len
	 * @returns RET_CACHED if the object was cached
	 * @returns RET_UNCACHED if the object was not cached
	 *
	 * Note: The function's return value is used by the xcache-core
	 * to keep object references.
	 */
	xret_t (*__xcache_store)(xslice_t *, xcache_meta_t *, uint8_t *);

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
	xret_t (*__xcache_timer)(uint64_t ticks);

	/**
	 * __xcache_notify_evict:
	 * evict xcache_node.
	 * @int: The node who has evicted this object
	 * @xslice_t: Corresponding slice
	 * @xcache_meta_t: Object to be evicted
	 */
	xret_t
	(*__xcache_notify_evict)(int, xslice_t *, xcache_meta_t *);

	/**
	 * __xcache_evicted:
	 * Some plugin evicted xcache_node.
	 * @xslice_t: Corresponding slice
	 * @xcache_meta_t: Object to be evicted
	 */
	xret_t
	(*__xcache_force_evict)(xslice_t *, xcache_meta_t *);

	/**
	 * __xcache_exists:
	 * Some plugin evicted xcache_node.
	 * @xslice_t: Corresponding slice
	 * @xcache_meta_t: Object to be evicted
	 */
	xret_t
	(*__xcache_exists)(xslice_t *, xcache_meta_t *);
};

/**
 * xcache_register_plugin:
 * Register Xcache plugin.
 */
int xcache_register_plugin(struct xcache_plugin *);

void *xcache_alloc(size_t size);
void xcache_free(void *);


#endif
