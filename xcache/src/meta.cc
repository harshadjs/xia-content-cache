#include "slice.h"
#include "meta.h"

XcacheMeta::XcacheMeta(XcacheCommand *cmd)
{
  refCount = 0;
  store = NULL;
  cid = cmd->cid();
}

XcacheMeta::XcacheMeta()
{
  refCount = 0;
  store = NULL;
}

void XcacheMeta::addedToSlice(XcacheSlice *slice)
{
  ref();
  sliceMap[slice->getContextID()] = slice;
}

void XcacheMeta::removedFromSlice(XcacheSlice *slice)
{
  deref();  
}

