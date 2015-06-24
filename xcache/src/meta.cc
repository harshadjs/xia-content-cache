#include "slice.h"
#include "meta.h"

void XcacheMeta::addedToSlice(XcacheSlice *slice)
{
  ref();
  sliceMap[slice->getContextID()] = slice;
}

void XcacheMeta::removedFromSlice(XcacheSlice *slice)
{
  deref();  
}

