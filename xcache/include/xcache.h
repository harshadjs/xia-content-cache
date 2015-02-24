#ifndef __XCACHE_H__
#define __XCACHE_H__
#include "hash_table.h"
#include "dlist.h"
#include <stddef.h>
#include <stdint.h>
#include "xia_cache_req.h"

/**
 * Xcache Store Macros:
 */

/* Initializtion macro */
#define _xcache_store_init						\
	static __attribute__((constructor)) _init

#define SET_STORE_PRIV(__xcache_meta, __store_obj)	\
	(__xcache_meta)->store_priv						\
				   = (void *)(__store_obj)

#define GET_STORE_PRIV(__xcache_meta)	((__xcache_meta)->store_priv)

#define SET_POLICY_PRIV(__xcache_meta_or_slice, __policy_obj)	\
	(__xcache_meta_or_slice)->policy_priv						\
							= (void *)(__policy_obj)

#define GET_POLICY_PRIV(__xcache_meta_or_slice)	\
	((__xcache_meta_or_slice)->policy_priv)

typedef enum {
	RET_CACHED,
	RET_UNCACHED,
	RET_EVICTED,
	RET_OK,
	RET_FAIL,
} xret_t;

struct xcache_store;

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

	/* Number of slices holding this data */
	int ref_count;

	/* Store that stores this content */
	struct xcache_store *store;

	/* xcache store objects */
	void *store_priv;

	void *policy_priv;
} xcache_meta_t;

#define UNLIMITED64 (~((uint64_t)(0x0)))
#define UNLIMITED32 (~((uint32_t)(0x0)))

#define UNLIMITED_SIZE UNLIMITED64
#define UNLIMITED_ITEMS UNLIMITED32
#define UNLIMITED_BANDWIDTH UNLIMITED32

struct xcache_store_conf {
	uint64_t max_size;		/* in bytes */
	uint32_t max_items; 	/* */
	uint32_t bandwidth; 	/* in bps */
};

struct xcache_store {
	char *name;

	struct xcache_store_conf conf;
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
	 * Note: __xcache_search will be called for store 'n' only if
	 * (n-1) stores before it returned NULL. That's why the order of stores
	 * matters. The order of stores is defined in stores/order.conf.
	 */
	xret_t (*get)(xcache_meta_t *, uint8_t *);

	xret_t (*evict)(xcache_meta_t *);
};

/**
 * xcache_register_store:
 * Register Xcache store.
 */
int xcache_register_store(struct xcache_store *);

void *xalloc(size_t size);
void xfree(void *);

extern uint32_t ticks;
#endif
