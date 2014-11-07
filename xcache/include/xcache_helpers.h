#ifndef __XCACHE_HELPERS_H__
#define __XCACHE_HELPERS_H__

#include <xia_cache_req.h>

void print_cid(struct click_xia_xid *cid);
void dump_buf(char *buf, int len);
void dump_request(xcache_req_t *req);
void *xcache_alloc(size_t size);
void *xcache_zalloc(size_t size);
void xcache_free(void *mem);

#endif
