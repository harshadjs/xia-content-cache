#ifndef __POLICIES_H__
#define __POLICIES_H__

#include "xcache.h"

#include "slice.h"

struct xcache_policy {
	xret_t (*store)(xcache_slice_t *, xcache_meta_t *);
	xret_t (*get)(xcache_slice_t *, xcache_meta_t *);
	xret_t (*remove)(xcache_slice_t *, xcache_meta_t *);
 	xret_t (*make_room)(xcache_slice_t *, xcache_meta_t *);
};

enum {
	XCACHE_POLICY_FIFO,
	XCACHE_POLICY_MAX
};


extern struct xcache_policy *policies[];

#define POLICY_ID_TO_OBJ(__id) (((__id) >= XCACHE_POLICY_MAX) ? \
								policies[XCACHE_POLICY_FIFO] :	\
								policies[__id])

#endif /* __POLICIES_H__ */
