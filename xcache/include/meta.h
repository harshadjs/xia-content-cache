#ifndef __META_H__
#define __META_H__

enum {
	METAHT_USE_CLEANUP,
	METAHT_DONTUSE_CLEANUP,
};

ht_t *xcache_new_metaht(int);
xcache_meta_t *xcache_req2meta(xcache_req_t *req);

#endif
