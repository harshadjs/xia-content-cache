#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <hash_table.h>
#include <stdio.h>
#include <xcache.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "logger.h"

#define XCACHE_DIR "/tmp/xcache_backend/"

static uint64_t bytes;
static uint32_t backend_index = 0;
static uint64_t max_cache_size;

xret_t backend_store(xcache_meta_t *meta, uint8_t *data)
{
	int fd;
	char *filename;

	backend_index++;
	filename = xalloc(128);

	snprintf(filename, 128, XCACHE_DIR"/%d", backend_index);
	SET_STORE_PRIV(meta, filename);

	fd = open(filename, O_RDWR | O_CREAT, 0777);

	if(fd < 0) {
		log(LOG_ERR, "Failed to open file.\n");
		return RET_FAIL;
	}

	if(write(fd, data, meta->len) < meta->len) {
		log(LOG_ERR, "Could not complete write.\n");
		close(fd);
		return RET_FAIL;
	}
	close(fd);

	bytes += meta->len;

	return RET_CACHED;
}

xret_t backend_get(xcache_meta_t *meta, uint8_t *dest)
{
	char *filename = GET_STORE_PRIV(meta);
	int fd;

	fd = open(filename, O_RDONLY);

	if(fd < 0) {
		log(LOG_ERR, "Failed to open file %s.\n", filename);
		return RET_FAIL;
	}

	if(read(fd, dest, meta->len) < meta->len) {
		log(LOG_ERR, "Incomplete read.\n");
		close(fd);
		return RET_FAIL;
	}

	close(fd);

	return RET_OK;
}

xret_t backend_evict(xcache_meta_t *meta)
{
	char *filename = GET_STORE_PRIV(meta);

	if(unlink(filename) < 0) {
		log(LOG_ERR, "Failed to delete file %s.\n", filename);
	}

	xfree(GET_STORE_PRIV(meta));
	SET_STORE_PRIV(meta, NULL);

	return RET_OK;
}

static struct xcache_store ht = {
	.name = "backend",
	.store = backend_store,
	.get = backend_get,
	.evict = backend_evict,
	.conf.max_size = UNLIMITED_SIZE,
	.conf.max_items = UNLIMITED_ITEMS,
	.conf.max_size = UNLIMITED_BANDWIDTH,
};

void _xcache_store_init(void)
{
	max_cache_size = 1024 * 1; /* 1024 bytes */

	if((mkdir(XCACHE_DIR, 0775) < 0) && (errno != EEXIST)) {
		log(LOG_ERR,
			"Backend Store: Could not be initialized. Got '%s' while creating %s\n",
			strerror(errno), XCACHE_DIR);
		return;
	}

	log(LOG_INFO, "Backend Store Initialized.\n");
	/* Store Init */
	xcache_register_store(&ht);
}
