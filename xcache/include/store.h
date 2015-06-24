#ifndef __STORE_H__
#define __STORE_H__

#include "data.h"

class XcacheMeta;

class XcacheContentStore {
private:
  /* Configuration parameters to be added here */
public: 
  virtual int store(XcacheMeta *, XcacheData *);
  virtual XcacheMeta *get(XcacheData *&);
  virtual void print(void);
};

#endif
