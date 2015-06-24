#ifndef __FIFO_H__
#define __FIFO_H__

#include "policy.h"

class FifoPolicy:public XcachePolicy {
public:
  int store(XcacheMeta *meta) {
    return 1;
  }
  int get(XcacheMeta *meta) {
    return 1;
  }
  int remove(XcacheMeta *meta) {
    return 1;
  }
  XcacheMeta *evict() {
    return NULL;
  }
};

#endif
