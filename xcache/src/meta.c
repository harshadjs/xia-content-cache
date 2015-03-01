#include <stdio.h>
#include <string.h>
#include <hash_table.h>
#include "xcache.h"
#include "meta.h"


/* Hash table functions */
static int _hash(void *key)
{
	int i, sum = 0;
	xcache_meta_t *c = (xcache_meta_t *)key;

	for(i = 0; i < CLICK_XIA_XID_ID_LEN; i++)
		sum += c->cid.id[i];

	return (sum);
}

static int _compar(void *val1, void *val2)
{
	xcache_meta_t *c1, *c2;

	c1 = (xcache_meta_t *)val1;
	c2 = (xcache_meta_t *)val2;

	return memcmp(c1->cid.id, c2->cid.id, CLICK_XIA_XID_ID_LEN);
}

static void _cleanup(void *val)
{
	xcache_meta_t *c = (xcache_meta_t *)val;

	if(!c)
		return;

	xfree(c);
}

xcache_meta_t *xcache_req2meta(xcache_req_t *req)
{
	xcache_meta_t *meta;

	meta = (xcache_meta_t *)xalloc(sizeof(xcache_meta_t));
	meta->len = req->len;
	meta->cid = req->cid;
	meta->cticks = meta->aticks = ticks;

	return meta;
}

ht_t *xcache_new_metaht(int use_cleanup)
{
	/* TODO: 23?? */
	return (use_cleanup == METAHT_USE_CLEANUP)
		? ht_create(23, _hash, _compar, _cleanup) :
		ht_create(23, _hash, _compar, NULL);
}
