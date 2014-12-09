#include <string.h>
#include <hash_table.h>
#include <stdio.h>
#include <xcache.h>

typedef struct {
	xcache_meta_t *meta;
	xslice_t *xslice;
	uint8_t *data;
} memht_node_t;

static ht_t *memht;

static memht_node_t *new_memht_node(void)
{
	return xcache_alloc(sizeof(memht_node_t));
}

int mem_ht_store(xslice_t *xslice, xcache_meta_t *meta, uint8_t *data)
{
	memht_node_t *n = new_memht_node();

	printf("%s\n", __func__);
	n->meta = meta;
	n->xslice = xslice;
	n->data = data;

	ht_add(memht, n);
	return 0;
}

uint8_t *mem_ht_search(xslice_t *xslice, xcache_meta_t *meta)
{
	memht_node_t *n, key;

	printf("%s\n", __func__);
	key.meta = meta;
	key.xslice = xslice;

	n = ht_search(memht, &key);

	if(!n)
		return NULL;

	return n->data;
}

void mem_ht_evict(xslice_t *xslice, xcache_meta_t *node)
{
	return;
}

xcache_meta_t *mem_ht_timer(uint64_t ticks)
{
	printf("Timer called: %d\n", (int)ticks);
	return NULL;
}

static struct xcache_plugin ht = {
	.__xcache_store = mem_ht_store,
	.__xcache_search = mem_ht_search,
	.__xcache_timer = mem_ht_timer,
	.__xcache_evict = mem_ht_evict,
};

/* Hash table functions */
static int _ht_hash(void *val)
{
	int i, sum = 0;

	memht_node_t *node = (memht_node_t *)val;

	for(i = 0; i < CLICK_XIA_XID_ID_LEN; i++) {
		sum += node->xslice->hid.id[i];
		sum += node->meta->cid.id[i];
	}

	return sum;
}

static int _ht_cmp(void *val1, void *val2)
{
	memht_node_t *n1 = (memht_node_t *)val1,
		*n2 = (memht_node_t *)val2;

	return (memcmp(&n1->xslice->hid, &n2->xslice->hid, CLICK_XIA_XID_ID_LEN)
			|| memcmp(&n1->meta->cid, &n2->meta->cid, CLICK_XIA_XID_ID_LEN));
}

static void _ht_cleanup(void *val)
{
	memht_node_t *n = (memht_node_t *)val;

	free(n->data);
}

void _xcache_plugin_init(void)
{
	printf("Inside %s\n", __func__);
	memht = ht_create(57, _ht_hash, _ht_cmp, _ht_cleanup);

	/* Plugin Init */
	xcache_register_plugin(&ht);
}
