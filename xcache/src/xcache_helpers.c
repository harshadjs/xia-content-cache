#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <xia_cache_req.h>
#include <xcache_helpers.h>

void print_cid(struct click_xia_xid *cid)
{
	int i;

	for(i = 0; i < CLICK_XIA_XID_ID_LEN; i++) {
		printf("%x", cid->id[i]);
	}
	printf("\n");
}

void dump_buf(char *buf, int len)
{
	int i;

	printf("Buf: ");
	for(i = 0; i < len; i++) {
		printf("0x%x", buf[i]);
	}
	printf("\n");
}

void dump_request(xcache_req_t *req)
{
	char *request = (req->request == XCACHE_STORE) ? ("STORE")
		:((req->request == XCACHE_SEARCH) ? ("SEARCH") : ("OTHER"));

	printf("Received %s request\n", request);
}

void *xcache_alloc(size_t size)
{
#ifdef __KERNEL__
	return kmalloc(GFP_KERNEL, size);
#else
	return malloc(size);
#endif
}

void *xcache_zalloc(size_t size)
{
	void *mem = xcache_alloc(size);

	memset(mem, 0, size);
	return mem;
}

void xcache_free(void *mem)
{
#ifdef __KERNEL__
	kfree(mem);
#else
	free(mem);
#endif
}
