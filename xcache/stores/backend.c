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

char xcache_dir[512];

static uint64_t max_cache_size;

static struct xcache_store store;

#define HEX2C(__hex) \
	(((__hex) >= 0xa) ? ('a' + ((__hex) - 0xa)) : ('0' + ((__hex) - 0x0)))

static void cid2filename(char *filename, struct click_xia_xid *cid)
{
	int i, dest_index = 0;

	dest_index = snprintf(filename, 512, "%s/", xcache_dir);
	for(i = 0; i < CLICK_XIA_XID_ID_LEN; i++) {
		filename[dest_index++] = HEX2C(cid->id[i] & 0xf);
		filename[dest_index++] = HEX2C((cid->id[i] & 0xf0) >> 4);
	}

	filename[dest_index] = 0;
}

xret_t backend_store(xcache_meta_t *meta, uint8_t *data)
{
	int fd;
	char filename[512];

	cid2filename(filename, &meta->cid);

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

	store.conf.current_size += meta->len;

	return RET_CACHED;
}

xret_t backend_get(xcache_meta_t *meta, uint8_t *dest)
{
	char filename[512];
	int fd;

	cid2filename(filename, &meta->cid);
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
	char filename[512];

	cid2filename(filename, &meta->cid);
	if(unlink(filename) < 0) {
		log(LOG_ERR, "Failed to delete file %s.\n", filename);
	}

	SET_STORE_PRIV(meta, NULL);

	store.conf.current_size -= meta->len;

	return RET_OK;
}

static struct xcache_store store = {
	.name = "backend",
	.store = backend_store,
	.get = backend_get,
	.evict = backend_evict,
	.conf.max_size = UNLIMITED_SIZE,
	.conf.max_items = UNLIMITED_ITEMS,
	.conf.max_size = UNLIMITED_BANDWIDTH,
	.conf.current_size = 0,
};

void _xcache_store_init(void)
{
	max_cache_size = 1024 * 1; /* 1024 bytes */

	sprintf(xcache_dir, "/tmp/backend-%d/", getpid());

	if((mkdir(xcache_dir, 0775) < 0) && (errno != EEXIST)) {
		log(LOG_ERR,
			"Backend Store: Could not be initialized. Got '%s' while creating %s\n",
			strerror(errno), xcache_dir);
		return;
	}

	log(LOG_INFO, "Backend Store Initialized.\n");
	/* Store Init */
	xcache_register_store(&store);
}
