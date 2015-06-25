#ifndef __MEMHT_H__
#define __MEMHT_H__

#include "store.h"

/**
 * MemHt:
 * @brief Content Store - [In-memory hash table]
 * In memory hash table using C++'s stl
 */

class MemHt:public XcacheContentStore {
public:
  MemHt()
  {
  }

  int store(XcacheMeta *meta, std::string data)
  {
    std::cout << "Reached MemHt::" << __func__ << "\n";
    return 0;
  }

  XcacheMeta *get(std::string *&data)
  {
    return NULL;
  }

  void print(void)
  {
    
  }
};

#endif
