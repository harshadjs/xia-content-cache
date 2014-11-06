#ifndef __XIA_CACHE_REQ_H__
#define __XIA_CACHE_REQ_H__
#include <stddef.h>

#include "../clicknet/xid.h"

enum request_t {
		REQUEST_CACHE_STORE = 0,
		REQUEST_CACHE_SEARCH,
		REQUEST_CACHE_CLEAR_CONTEXT,
		REQUEST_CACHE_CLEAR_CID,
		RESPONSE,
};

struct content_header {
	/* CID of the content */
	struct click_xia_xid cid;

	/* Time to live */
	uint32_t ttl;
} __attribute__((packed));

typedef struct {
	enum request_t request;
	struct content_header ch;
	struct click_xia_xid hid;
	off_t offset;
	size_t len;
	size_t total_len;
} __attribute__((packed)) cache_req_t;

#define XID_STRUCT_LEN (sizeof(struct click_xia_xid))

#define MAX_TRANSFER_SIZE 4096

#endif
