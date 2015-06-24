#include "slice.h"
#include "meta.h"
#include "data.h"
#include "fifo.h"

XcacheSlice::XcacheSlice() {
  FifoPolicy fifo;

  maxSize = currentSize = contextID = ttl = 0;
  policy = fifo;
}
void XcacheSlice::addMeta(XcacheMeta *meta)
{
  metaMap[meta->getCid()] = meta;
}

bool XcacheSlice::hasRoom(XcacheMeta *meta)
{
  if(maxSize - currentSize >= meta->getLength())
    return true;
  return false;
}

void XcacheSlice::removeMeta(XcacheMeta *meta)
{
  std::map<xid_t, XcacheMeta *>::iterator iter;

  iter = metaMap.find(meta->getCid());
  metaMap.erase(iter);
}

void XcacheSlice::makeRoom(XcacheMeta *meta)
{
  while(!hasRoom(meta)) {
    XcacheMeta *toRemove = policy.evict();
    removeMeta(toRemove);
  }
}

void XcacheSlice::store(XcacheMeta *meta, XcacheData data)
{
  makeRoom(meta);
  addMeta(meta);
  policy.store(meta);

  //  return xcore_store(meta, data);
}

void XcacheSlice::setPolicy(XcachePolicy policy)
{
  this->policy = policy;
}


#if 0
XcacheMeta *
xslice_search(uint8_t **data, XcacheSlice *slice, xcache_req_t *req)
{
	XcacheMeta key, *meta;

	key.cid = req->cid;
	meta = ht_search(slice->meta_ht, &key);
	if(!meta)
		return NULL;

	slice->policy->get(slice, meta);

	return xcore_search(data, meta);
}

void
xslice_remove_meta(XcacheSlice *slice, XcacheMeta *meta)
{
	int i;

	ht_remove(slice->meta_ht, meta);

	slice->policy->remove(slice, meta);

	slice->cur_size -= meta->len;

	for(i = 0; i < meta->ref_count - 1; i++) {
		if(meta->slices[i] == slice)
			break;
	}

	for(; i < meta->ref_count - 1; i++) {
		meta->slices[i] = meta->slices[i + 1];
	}

	meta->ref_count--;
	if(meta->ref_count == 0) {
		xfree(meta->slices);
	} else {
		meta->slices = xrealloc(meta->slices, meta->ref_count);
	}

	if(meta->ref_count == 0)
		xctrl_remove(meta);
}

void xslice_flush(void *data)
{
	ht_iter_t iter;
	XcacheSlice *slice = (XcacheSlice *)data;
	XcacheMeta *meta;

	log(LOG_INFO, "Slice timed out at %d!\n", ticks);
	ht_iter_init(&iter, slice->meta_ht);
	while((meta = (XcacheMeta *)ht_iter_data(&iter)) != NULL) {
		xslice_remove_meta(slice, meta);
		ht_iter_next(&iter);
	}

	xctrl_remove_slice(slice);
}

#endif
