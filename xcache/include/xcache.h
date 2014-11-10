#ifndef __XCACHE_H__
#define __XCACHE_H__
#include "hash_table.h"
#include <stddef.h>
#include <stdint.h>

#include "xia_cache_req.h"

#define XCACHE_UDP_PORT 1444

/*
 * This limit is currently kept for testing
 * Can be extended upto 65536 bytes.
 */
#define UDP_MAX_PKT 4096

/* Content object */
typedef struct {
	struct click_xia_xid cid;		/* Content ID */
	int len,	/* Length of the data object */
		full;	/* A boolean, set if the entire object is present */
	uint32_t ticks,	/* Time at which this object was inserted */
		ttl;	/* Time to live */
	uint8_t *data;	/* Actual data */
} xcache_node_t;

/* Global time counter. This is incremented every second */
extern uint32_t ticks;

/**
 * xcache_raw_send:
 * Send data over UDP socket towards click.
 * @args data: Data to be sent
 * @args len: Length
 */
int xcache_raw_send(uint8_t *data, int len);
#endif
