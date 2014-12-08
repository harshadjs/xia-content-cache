#ifndef __XCACHE_MAIN_H__
#define __XCACHE_MAIN_H__
#include "xcache.h"

#define XCACHE_UDP_PORT 1444

/*
 * This limit is currently kept for testing
 * Can be extended upto 65536 bytes.
 */
#define UDP_MAX_PKT 4096

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
