#ifndef __META_H__
#define __META_H__

#include "../clicknet/xid.h"
#include <map>
#include <iostream>
#include <stdint.h>
#include "data.h"
#include "store.h"

class XcacheSlice;

class XcacheMeta {
private:
  uint32_t refCount;
  uint64_t len;
  /* This map stores all the slices that this meta is a part of */
  std::map<uint32_t, XcacheSlice *> sliceMap;
  XcacheContentStore *store;
  xid_t cid;

  void ref(void) {
    refCount++;
  }
  void deref(void) {
    refCount--;
  }

public:
  XcacheMeta() {
    refCount = len = 0;
    store = NULL;
  }
  void setStore(XcacheContentStore *s) {
    store = s;
  }
  void addedToSlice(XcacheSlice *slice);

  void removedFromSlice(XcacheSlice *slice);

  void setLength(uint64_t length) {
    this->len = length;
  }

  uint64_t getLength() {
    return len;
  }

  /* For Debugging Purposes */
  void print(void) {
    std::cout << "RefCount = " << refCount << ", Length = " << len;
    if(store != NULL) {
      std::cout << ", Store = ";
      store->print();
    } else {
      std::cout << ", Store = NULL";
    }
    std::cout << std::endl;
  }

  xid_t getCid() {
    return cid;
  }
};

#endif
