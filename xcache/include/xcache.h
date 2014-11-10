#ifndef __XCACHE_H__
#define __XCACHE_H__
#include "hash_table.h"
#include <stddef.h>
#include <stdint.h>

#include "xia_cache_req.h"

#define XCACHE_UDP_PORT 1444
#define UDP_MAX_PKT 4096

typedef struct {
	struct click_xia_xid cid;
	int len, full;
	uint32_t ticks, ttl;
	uint8_t *data;
} xcache_node_t;

extern uint32_t ticks;
int xcache_raw_send(uint8_t *data, int len);
#endif
