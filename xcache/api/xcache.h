#ifndef __XCACHE_H__
#define __XCACHE_H__

#include <stddef.h>
#include <stdint.h>

#define CIDLEN 512

#define UNIX_SERVER_SOCKET "/tmp/xcache.socket"

struct xcacheChunk {
  char cid[CIDLEN];
  void *buf;
  size_t len;
};

struct xcacheSlice {
  uint32_t contextId;
};

int XcacheInit(void);
int XcacheGetChunk(xcacheSlice *slice, xcacheChunk *chunk, int flags);
int XcacheAllocateSlice(struct xcacheSlice *slice, int32_t cache_size, int32_t ttl, int32_t cache_policy);
int XcachePutChunk(xcacheSlice *slice, xcacheChunk *chunk);

#endif
