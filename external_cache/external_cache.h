#ifndef __EXTERNAL_CACHE_H__
#define __EXTERNAL_CACHE_H__
#include "hash_table.h"
#include <stddef.h>
#include <stdint.h>

#include "xia_cache_req.h"

typedef struct {
	struct click_xia_xid cid;
	struct click_xia_xid hid;
	int len, full;
	char *data;
} cid_node_t;

typedef struct {
	context_id_t context_id;
	ht_t *cid_ht;
} context_node_t;

#endif
