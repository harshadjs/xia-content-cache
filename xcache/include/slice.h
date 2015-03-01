#ifndef __XCACHE_SLICE_H__
#define __XCACHE_SLICE_H__

#include <stdint.h>
#include "hash_table.h"

struct xcache_policy;

typedef struct {
	ht_t *meta_ht;
	uint32_t max_size;
	uint32_t cur_size;

	/* Context / Slice identifier */
	uint32_t context_id;

	/* Time to live for this slice */
	uint32_t ttl;

	struct xcache_policy *policy;
	/*
	 * Note: Recommeded to access using SET/GET_POLICY_PRIV macro.
	 * See xcache.h
	 */
	void *policy_priv;
} xcache_slice_t;

#include "policies.h"
#include "xcache.h"

xcache_meta_t *
xslice_store(xcache_slice_t *slice, xcache_meta_t *meta, uint8_t *data);
xcache_meta_t *
xslice_search(uint8_t **data, xcache_slice_t *slice, xcache_req_t *req);
void xslice_flush(void *data);
int xslice_init(xcache_slice_t *slice);
#endif
