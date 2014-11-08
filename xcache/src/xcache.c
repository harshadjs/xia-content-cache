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

static xcache_node_t *xcache_store(xcache_req_t *req, uint8_t *data)
{
	xslice_t *xslice = xctrl_search(req);

	printf("%s: Store request received\n", __func__);
	if(!xslice) {
		/* First request for this client */
		xslice = new_xslice(req);
		xctrl_add_xslice(xslice);
	}

	return xslice_store(xslice, req, data);
}

static void
xcache_fragment_and_send(int s, struct sockaddr_in *si_other,
						 xcache_node_t *xcache_node, xslice_t *xslice)
{
	uint8_t packet[UDP_MAX_PKT], *payload;
	xcache_req_t *response = (xcache_req_t *)packet;
	int sent = 0;

#define MIN(__a, __b) (((__a) < (__b)) ? (__a) : (__b))

	payload = packet;
	payload += sizeof(xcache_req_t);
	do {
		printf("%s: Sending cache response\n", __func__);
		response->request = XCACHE_RESPONSE;
		response->ch.cid = xcache_node->cid;
		response->hid = xslice->hid;

		response->offset = sent;
		response->len = MIN(1000, (xcache_node->len - sent));
		response->total_len = xcache_node->len;

		memcpy(payload, xcache_node->data + sent, response->len);
		sendto(s, packet, sizeof(xcache_req_t) + response->len, 0,
			   (struct sockaddr *)si_other, sizeof(struct sockaddr));
		sent += response->len;
		printf("%s: sent=%d\n", __func__, sent);
	} while(sent < xcache_node->len);
}

static void
xcache_search_and_respond(int s,
						  struct sockaddr_in *si_other,
						  xcache_req_t *req)
{
	xslice_t *xslice = xctrl_search(req);
	xcache_node_t *xcache_node;

	printf("%s: Search request received\n", __func__);

	if(!xslice) {
		/* First request for this client */
		xslice = new_xslice(req);
		xctrl_add_xslice(xslice);
		/* click knows if we have the object or not */
		/* XXX: But, we can send a DENY */
		return;
	}

	xcache_node = xslice_search(xslice, req);
	if(!xcache_node || !xcache_node->full)
		/* XXX: Again, send a DENY */
		return;

	xcache_fragment_and_send(s, si_other, xcache_node, xslice);
}

static int xcache_main_loop(void)
{
	struct sockaddr_in si_me, si_other;
	uint8_t packet[UDP_MAX_PKT], *payload;
	xcache_req_t *req;
    int s;
	socklen_t slen = sizeof(si_other);

    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
		return 0;

    memset((char *)&si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(XCACHE_UDP_PORT);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(s, (struct sockaddr *)&si_me, sizeof(si_me)) == -1)
		return 0;

	while(1) {
		recvfrom(s, packet, UDP_MAX_PKT, 0,
					  (struct sockaddr *)&si_other, &slen);

		printf("Something recevied\n");
		req = (xcache_req_t *)packet;
		payload = &packet[sizeof(xcache_req_t)];

		if(req->request == XCACHE_STORE) {
			xcache_store(req, payload);
		} else if(req->request == XCACHE_SEARCH) {
			xcache_search_and_respond(s, &si_other, req);
		}
	}
	return 0;
}

int main(int argc, char *argv[])
{
	xctrl_init();
	return xcache_main_loop();
}
