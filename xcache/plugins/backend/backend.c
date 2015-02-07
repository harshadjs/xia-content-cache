#include <string.h>
#include <hash_table.h>
#include <stdio.h>
#include <xcache.h>
#include <fcntl.h>
#include <unistd.h>

typedef struct {
	char filename[512];
} backend_node_t;

static uint64_t bytes;
static uint32_t backend_index = 0;
static uint64_t max_cache_size;

xret_t backend_store(xcache_meta_t *meta, uint8_t *data)
{
	int fd;
	backend_node_t *node = xcache_alloc(sizeof(backend_node_t));

	printf("%s\n", __func__);
	backend_index++;
	sprintf(node->filename, "/tmp/xcache_backend/%d", backend_index);
	SET_PRIV(meta, node);

	fd = open(node->filename, O_RDWR | O_CREAT, 777);
	write(fd, data, meta->len);
	close(fd);

	bytes += meta->len;
	printf("Cached\n");
	return RET_CACHED;
}

uint8_t *backend_get(xcache_meta_t *meta, uint8_t *dest)
{
	backend_node_t *node = (backend_node_t *)GET_PRIV(meta);
	uint8_t *data = xcache_alloc(meta->len);
	int fd = open(node->filename, O_RDONLY);

	read(fd, data, meta->len);

	close(fd);
	printf("%s\n", __func__);
	return data;
}

xret_t backend_evict(xcache_meta_t *meta)
{
	free(GET_PRIV(meta));
	return RET_OK;
}

static struct xcache_plugin ht = {
	.name = "backend",
	.store = backend_store,
	.get = backend_get,
	.evict = backend_evict,
	.conf.max_size = UNLIMITED_SIZE,
	.conf.max_items = UNLIMITED_ITEMS,
	.conf.max_size = UNLIMITED_BANDWIDTH,
};

void _xcache_plugin_init(void)
{
	printf("Inside backend\n");
	max_cache_size = 1024 * 1; /* 1024 bytes */

	/* Plugin Init */
	xcache_register_plugin(&ht);
}
