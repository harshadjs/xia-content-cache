#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <xcache.h>
#include "slice.h"
#include "helpers.h"
#include "controller.h"
#include "core.h"
#include "xcache_main.h"
#include "meta.h"

#define DEFAULT_XCACHE_SIZ 1000 // TODO: Really?
#define BUCKETS 23

/*
** @brief Makes room for accommodating meta in slice. Thus, applies
** policy rules
** TODO: Fill
*/
int xslice_make_room(xcache_slice_t *slice, xcache_meta_t *meta)
{
	return 0;
}

xcache_meta_t *
xslice_store(xcache_slice_t *slice, xcache_meta_t *meta, uint8_t *data)
{
	if(!meta)
		return NULL;

	if(slice->max_size - slice->cur_size < meta->len) {
		if(xslice_make_room(slice, meta) < 0) {
			printf("%s: Could not make room.\n", __func__);
			return NULL;
		}
	}

	ht_add(slice->meta_ht, meta);
	meta->ref_count++;

	slice->policy->store(slice, meta);

	return xcore_store(meta, data);
}

xcache_meta_t *
xslice_search(uint8_t **data, xcache_slice_t *slice, xcache_req_t *req)
{
	xcache_meta_t key, *meta;

	key.cid = req->cid;
	meta = ht_search(slice->meta_ht, &key);
	if(!meta)
		return NULL;

	slice->policy->get(slice, meta);

	return xcore_search(data, meta);
}

void
xslice_remove_meta(xcache_slice_t *slice, xcache_meta_t *meta)
{
	ht_remove(slice->meta_ht, meta);
	meta->ref_count--;

	slice->policy->remove(slice, meta);

	slice->cur_size -= meta->len;
	if(meta->ref_count == 0)
		xctrl_remove(meta);
}

/* TODO: Fill */
void xslice_flush(xcache_slice_t *slice)
{
}

int xslice_init(xcache_slice_t *slice)
{
	slice->meta_ht = xcache_new_metaht();
	slice->max_size = DEFAULT_XCACHE_SIZ;
	slice->cur_size = 0;

	return 0;
}
