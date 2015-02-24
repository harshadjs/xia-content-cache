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
};


extern struct xcache_policy *policies[];

#define POLICY_ID_TO_OBJ(__id) (policies[__id])

#endif /* __POLICIES_H__ */
