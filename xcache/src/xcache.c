#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include "xia_cache_req.h"
#include <errno.h>
#include "xcache.h"

static ht_t *hash_table;

#define UDP_PORT 1444

void print_cid(struct click_xia_xid *cid)
{
	int i;

	for(i = 0; i < CLICK_XIA_XID_ID_LEN; i++) {
		printf("%x", cid->id[i]);
	}
	printf("\n");
}

cid_node_t *process_store_request(cache_req_t *req, char *data)
{
	cid_node_t c, *cid_node;

	memcpy(&c.cid, &req->ch.cid, XID_STRUCT_LEN);
	memcpy(&c.hid, &req->hid, XID_STRUCT_LEN);

	cid_node = (cid_node_t *)ht_search(hash_table, &c);

	if(!cid_node) {
		cid_node = (cid_node_t *)malloc(sizeof(cid_node_t));
		cid_node->len = req->total_len;
		cid_node->data = (char *)malloc(cid_node->len);
		cid_node->cid = req->ch.cid;
		cid_node->hid = req->hid;
		cid_node->full = 0;
	} else {
		/* Check if node is already there */
		printf("Returning NULL\n");
		if(cid_node->full)
			return NULL;
	}
	if(req->offset + req->len == cid_node->len)
		cid_node->full = 1;
	memcpy(cid_node->data + req->offset, data, req->len);

	ht_add(hash_table, cid_node);
	printf("Returning node\n");

	return cid_node;
}

cid_node_t *process_search_request(cache_req_t *req, char *data)
{
	cid_node_t c, *cid_node;

	c.cid = req->ch.cid;
	c.hid = req->hid;

	cid_node = (cid_node_t *)ht_search(hash_table, &c);

	if(cid_node && cid_node->full)
		return cid_node;

	return NULL;
}

void dump_buf(char *buf, int len)
{
	int i;

	printf("Buf: ");
	for(i = 0; i < len; i++) {
		printf("%c", buf[i]);
	}
	printf("\n");
}

void send_response(int s, struct sockaddr_in *si_other, cid_node_t *c)
{
	char buf[8192], *dest;
	cache_req_t req;
	int len;

	req.request = RESPONSE;
	req.ch.cid = c->cid;
	req.hid = c->hid;

	req.offset = 0;
	req.len = req.total_len = c->len;

	dest = buf;
	memcpy(dest, &req, sizeof(cache_req_t));
	dest += sizeof(cache_req_t);

	memcpy(dest, c->data, c->len);
	dest += c->len;

	dump_buf(buf, dest - buf);

	printf("sendto returned %d\n",
		   (int)sendto(s, buf, (ssize_t)dest - (ssize_t)buf, 0,
					   (struct sockaddr *)si_other, sizeof(struct sockaddr)));
	printf("Error: %s\n", strerror(errno));
}

void dump_request(cache_req_t *req)
{
	char *request = (req->request == REQUEST_CACHE_STORE) ? ("STORE")
		:((req->request == REQUEST_CACHE_SEARCH) ? ("SEARCH") : ("OTHER"));

	printf("Received %s request\n", request);
}

int main_loop(void)
{
	struct sockaddr_in si_me, si_other;
	cache_req_t req;
    int s, i, slen = sizeof(si_other);
	cid_node_t *cid_node;
    char *buf;
	ht_iter_t iter;

    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
		return 0;

    memset((char *) &si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(UDP_PORT);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(s, (struct sockaddr *)&si_me, sizeof(si_me)) == -1)
		return 0;

	while(1) {
		if (recvfrom(s, &req, sizeof(req), 0,
					 (struct sockaddr *)&si_other, &slen) == -1)
			return 0;

		dump_request(&req);

		if(req.request == REQUEST_CACHE_STORE) {
			buf = malloc(req.len + 1);
			recvfrom(s, buf, req.len, 0, (struct sockaddr *)&si_other, &slen);
			buf[req.len] = 0;
			cid_node = process_store_request(&req, buf);
			free(buf);
		} else if(req.request == REQUEST_CACHE_SEARCH) {
			cid_node = process_search_request(&req, NULL);
			printf("Writing... [%d] %s\n", cid_node->len, cid_node->data);
			if(cid_node)
				send_response(s, &si_other, cid_node);
		}

		ht_iter_init(&iter, hash_table);
		while(ht_iter_data(&iter)) {
			cid_node_t *c;

			c = ht_iter_data(&iter);
			print_cid(&c->cid);
			ht_iter_next(&iter);
		}
	}
	return 0;
}

#define BUCKETS 23
int hash_fn(void *key)
{
	int i, sum = 0;
	cid_node_t *c = (cid_node_t *)key;

	for(i = 0; i < CLICK_XIA_XID_ID_LEN; i++)
		sum += c->cid.id[i];

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
int cmp_fn(void *val1, void *val2)
{
	cid_node_t *c1, *c2;

	c1 = (cid_node_t *)val1;
	c2 = (cid_node_t *)val2;

	return (memcmp(&c1->cid, &c2->cid, XID_STRUCT_LEN) ||
			memcmp(&c1->hid, &c2->hid, XID_STRUCT_LEN));
}

/**
 * cleanup_fn:
 * Cleanup function
 * @args
 * @returns
 */
void cleanup_fn(void *val)
{
	cid_node_t *c = (cid_node_t *)val;

	if(!c)
		return;

	if(c->data)
		free(c->data);
	free(c);
}

int main(int argc, char *argv[])
{
	hash_table = ht_create(BUCKETS, hash_fn, cmp_fn, cleanup_fn);

	return main_loop();
}
