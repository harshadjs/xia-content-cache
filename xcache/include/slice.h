#ifndef __XCACHE_SLICE_H__
#define __XCACHE_SLICE_H__

#include <map>
#include "../clicknet/xid.h"
#include "policy.h"
#include <stdint.h>

class XcacheMeta;
class XcachePolicy;

class XcacheSlice {
private:
  std::map<xid_t, XcacheMeta *> metaMap;
  uint64_t maxSize, currentSize;
  uint32_t contextID;

  uint64_t ttl;
  XcachePolicy policy;

public:
  XcacheSlice();

  void setPolicy(XcachePolicy);

  void addMeta(XcacheMeta *);

  void store(XcacheMeta *, XcacheData);

  void search(XcacheData /*, request */);

  void removeMeta(XcacheMeta *);

  void flush(XcacheData);

  void makeRoom(XcacheMeta *);

  uint32_t getContextID(void) {
    return contextID;
  };

  bool hasRoom(XcacheMeta *);
};

#endif
