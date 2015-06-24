#ifndef __POLICY_H__
#define __POLICY_H__

#include "meta.h"
#include "data.h"

class FifoPolicy;

class XcachePolicy  {
public:
  virtual int store(XcacheMeta *) {
    return 0;
  };
  virtual int get(XcacheMeta *) {
    return 0;
  };
  virtual int remove(XcacheMeta *) {
    return 0;
  };
  virtual XcacheMeta *evict() {
    return NULL;
  };
};

#endif /* __POLICY_H__ */
