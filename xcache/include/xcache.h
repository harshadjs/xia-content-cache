#ifndef __XCACHE_H__
#define __XCACHE_H__
#include "hash_table.h"
#include <stddef.h>
#include <stdint.h>

#include "xia_cache_req.h"

#define XCACHE_UDP_PORT 1444

typedef struct {
	struct click_xia_xid cid;
	int len, full;
	char *data;
} xcache_node_t;

#endif
