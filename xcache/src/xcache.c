#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "xcache_helpers.h"
#include "xcache.h"
#include "xcache_controller.h"
#include "xia_cache_req.h"

static xcache_node_t *xcache_store(xcache_req_t *req, char *data)
{
	xslice_t *xslice = xctrl_search(req);

	if(!xslice) {
		/* First request for this client */
		xslice = new_xslice(req);
		xctrl_add_xslice(xslice);
	}

	return xslice_store(xslice, req, data);
}

static void
xcache_search_and_respond(int s,
						  struct sockaddr_in *si_other,
						  xcache_req_t *req)
{
	xslice_t *xslice = xctrl_search(req);
	xcache_node_t *xcache_node;
	char buf[8192], *dest;
	xcache_req_t response;

	if(!xslice) {
		/* First request for this client */
		xslice = new_xslice(req);
		xctrl_add_xslice(xslice);
		/* click knows if we have the object or not */
		/* XXX: But, we can send a DENY */
		return;
	}

	xcache_node = xslice_search(xslice, req);
	if(!xcache_node)
		/* XXX: Again, send a DENY */
		return;

	response.request = XCACHE_RESPONSE;
	response.ch.cid = xcache_node->cid;
	response.hid = xslice->hid;

	response.offset = 0;
	response.len = response.total_len = xcache_node->len;

	dest = buf;

	memcpy(dest, &response, sizeof(xcache_req_t));
	dest += sizeof(xcache_req_t);

	memcpy(dest, xcache_node->data, xcache_node->len);
	dest += xcache_node->len;

	dump_buf(buf, dest - buf);

	printf("sendto returned %d\n",
		   (int)sendto(s, buf, (ssize_t)dest - (ssize_t)buf, 0,
					   (struct sockaddr *)si_other, sizeof(struct sockaddr)));
	printf("Error: %s\n", strerror(errno));
}

static int xcache_main_loop(void)
{
	struct sockaddr_in si_me, si_other;
	xcache_req_t req;
    int s;
	socklen_t slen = sizeof(si_other);
    char *buf;

    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
		return 0;

    memset((char *) &si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(XCACHE_UDP_PORT);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(s, (struct sockaddr *)&si_me, sizeof(si_me)) == -1)
		return 0;

	while(1) {
		if (recvfrom(s, &req, sizeof(req), 0,
					 (struct sockaddr *)&si_other, &slen) == -1)
			return 0;

		dump_request(&req);

		if(req.request == XCACHE_STORE) {
			buf = xcache_alloc(req.len + 1);
			recvfrom(s, buf, req.len, 0, (struct sockaddr *)&si_other, &slen);
			buf[req.len] = 0;
			xcache_store(&req, buf);
			free(buf);
		} else if(req.request == XCACHE_SEARCH) {
			xcache_search_and_respond(s, &si_other, &req);
		}
	}
	return 0;
}

int main(int argc, char *argv[])
{
	xctrl_init();
	return xcache_main_loop();
}
