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

#define SET_PRIV(__xcache_meta, __plugin_obj)				\
	(__xcache_meta)->priv								\
				   = (void *)(__plugin_obj)

#define GET_PRIV(__xcache_meta)	((__xcache_meta)->priv)

#endif /* XCACHE_PLUGIN_PRIO */

#include <_plugins.autogen.h>

typedef enum {
	RET_CACHED,
	RET_UNCACHED,
	RET_EVICTED,
	RET_OK,
	RET_FAIL,
} xret_t;

struct xcache_plugin;

/* Content object */
typedef struct {
	/* Content ID */
	struct click_xia_xid cid;

	/* Length of the data object */
	int len;

	/* Time at which this object was created */
	uint32_t cticks;

	/* Time at which this object was inserted */
	uint32_t aticks;

	/* Time to live */
	uint32_t ttl;

	/* Number of plugins holding this data */
	int ref_count;

	/* Plugin that stores this content */
	struct xcache_plugin *plugin;

	/* xcache plugin objects */
	void *priv;
} xcache_meta_t;

#define UNLIMITED64 (~((uint64_t)(0x0)))
#define UNLIMITED32 (~((uint32_t)(0x0)))

#define UNLIMITED_SIZE UNLIMITED64
#define UNLIMITED_ITEMS UNLIMITED32
#define UNLIMITED_BANDWIDTH UNLIMITED32

struct xcache_plugin_conf {
	uint64_t max_size;		/* in bytes */
	uint32_t max_items; 	/* */
	uint32_t bandwidth; 	/* in bps */
};

struct xcache_plugin {
	char *name;

	struct xcache_plugin_conf conf;
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
	xret_t (*store)(xcache_meta_t *, uint8_t *);

	/**
	 * __xcache_get:
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
	uint8_t *(*get)(xcache_meta_t *, uint8_t *);

	xret_t (*evict)(xcache_meta_t *);
};

/**
 * xcache_register_plugin:
 * Register Xcache plugin.
 */
int xcache_register_plugin(struct xcache_plugin *);

void *xcache_alloc(size_t size);
void xcache_free(void *);

#endif
