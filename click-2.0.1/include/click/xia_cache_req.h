#ifndef __XIA_CACHE_REQ_H__
#define __XIA_CACHE_REQ_H__
#include <stddef.h>

#include "../clicknet/xid.h"
#include "../clicknet/xid_types.h"

enum request_t {
	XCACHE_STORE = 0,
	XCACHE_SEARCH,
	XCACHE_TIMEOUT,
	XCACHE_CLEAR,
	XCACHE_RESPONSE,
};

typedef struct {
	uint32_t context_id,
		ttl,
		cache_size,
		cache_policy;
} __attribute__((packed)) xcache_context_t;

typedef struct {
	uint8_t request;
	struct click_xia_xid cid;
	xcache_context_t context;
	uint32_t len;
} __attribute__((packed)) xcache_req_t;

#define XID_STRUCT_LEN (sizeof(struct click_xia_xid))

#define MAX_TRANSFER_SIZE 4096

#endif
