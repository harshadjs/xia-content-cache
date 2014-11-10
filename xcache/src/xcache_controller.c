#include <stdio.h>
#include <string.h>
#include "xcache_slice.h"
#include "xcache_helpers.h"
#include "xcache_controller.h"

/*
 * Hash table containing all the slices
 * Key:HID, Value: xslice
 */
static ht_t *xcache_ctrl_ht;

#define BUCKETS 23
static int xctrl_hash(void *key)
{
	int i, sum = 0;
	xslice_t *c = (xslice_t *)key;

	for(i = 0; i < CLICK_XIA_XID_ID_LEN; i++)
		sum += c->hid.id[i];

	return (sum % BUCKETS);
}

/**
 * cmp_fn:
 * Compare function
 * @args
 * @returns
 */
static int xctrl_cmp(void *val1, void *val2)
{
	xslice_t *c1, *c2;

	c1 = (xslice_t *)val1;
	c2 = (xslice_t *)val2;

	return memcmp(&c1->hid, &c2->hid, XID_STRUCT_LEN);
}

/**
 * cleanup_fn:
 * Cleanup function
 * @args
 * @returns
 */
static void xctrl_cleanup(void *val)
{
	xslice_t *c = (xslice_t *)val;

	if(!c)
		return;

	xslice_free(c);
	xcache_free(c);
}

xslice_t *xctrl_search(xcache_req_t *req)
{
	xslice_t c;

	c.hid = req->hid;

	return (xslice_t *)ht_search(xcache_ctrl_ht, &c);
}

int xctrl_add_xslice(xslice_t *xslice)
{
	return ht_add(xcache_ctrl_ht, xslice);
}

void xctrl_handle_timeout(void)
{
	ht_iter_t iter;

	ht_iter_init(&iter, xcache_ctrl_ht);
	while(ht_iter_data(&iter)) {
		xslice_handle_timeout(ht_iter_data(&iter));
		ht_iter_next(&iter);
	}
}

int xctrl_init(void)
{
	xcache_ctrl_ht = ht_create(BUCKETS, xctrl_hash, xctrl_cmp, xctrl_cleanup);
	if(!xcache_ctrl_ht) {
		return -1;
	}
	return 0;
}
