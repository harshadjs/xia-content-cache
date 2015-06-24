#ifndef __DATA_H__
#define __DATA_H__

#include <stdint.h>

class XcacheData {
private:
  void *data;
  uint64_t len;

public:
  XcacheData() {
    data = NULL;
    len = 0;
  }
  void setXcacheData(void *p, uint64_t l) {
    data = p;
    len = l;
  }
};

#endif
