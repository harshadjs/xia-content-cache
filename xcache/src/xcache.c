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
#include "xcache_main.h"
#include "xia_cache_req.h"

/* Global time counter: incremented every second */
uint32_t ticks;

/* UDP socket for communication with click */
static int s;

/* Socket address for click */
static struct sockaddr_in click_addr;

int xcache_raw_send(uint8_t *data, int len)
{
	return sendto(s, data, len, 0, (struct sockaddr *)&click_addr,
				  sizeof(struct sockaddr));
}

static void
xcache_fragment_and_send(xcache_meta_t *meta,
						 uint8_t *data)
{
	uint8_t packet[UDP_MAX_PKT], *payload;
	xcache_req_t *response = (xcache_req_t *)packet;

#define MIN(__a, __b) (((__a) < (__b)) ? (__a) : (__b))

	printf("%s: Meta->len = %d\n", __func__, meta->len);

	payload = packet;
	payload += sizeof(xcache_req_t);
	response->request = XCACHE_RESPONSE;
	response->cid = meta->cid;
	response->cid.type = htonl(CLICK_XIA_XID_TYPE_CID);

	response->len = meta->len;

	memcpy(payload, data, response->len);
	sendto(s, packet, sizeof(xcache_req_t) + response->len, 0,
		   (struct sockaddr *)&click_addr, sizeof(struct sockaddr));
}

static void
xcache_search_and_respond(xcache_req_t *req)
{
	xcache_meta_t *meta;
	uint8_t *data;

	meta = xctrl_get_meta(req);
	printf("[Meta: %p]\n", meta);
	data = xcache_alloc(meta->len);
	xctrl_get_data(meta, data);
	if(!meta || !data /* || !xcache_node->full */)
		/* XXX: Again, send a DENY */
		return;

	xcache_fragment_and_send(meta, data);
}

static void xcache_inbound_udp(void)
{
	socklen_t slen = sizeof(click_addr);
	uint8_t packet[UDP_MAX_PKT], *payload;
	xcache_req_t *req;

	recvfrom(s, packet, UDP_MAX_PKT, 0,
			 (struct sockaddr *)&click_addr, &slen);

	req = (xcache_req_t *)packet;
	payload = &packet[sizeof(xcache_req_t)];

	xcache_dump_req(req);
	if(req->request == XCACHE_STORE) {
		xctrl_store(req, payload);
	} else if(req->request == XCACHE_SEARCH) {
		xcache_search_and_respond(req);
	}
}

static void xcache_set_timeout(struct timeval *t)
{
	t->tv_sec = 1;
	t->tv_usec = 0;
}

static void xcache_creat_sock(void)
{
	struct sockaddr_in si_me;

    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
		return;

    memset((char *)&si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(XCACHE_UDP_PORT);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(s, (struct sockaddr *)&si_me, sizeof(si_me)) == -1)
		return;
}

static int xcache_main_loop(void)
{
    int n;
	fd_set fds;
	struct timeval timeout;

	ticks = 1;
	xcache_creat_sock();

	FD_ZERO(&fds);
	xcache_set_timeout(&timeout);
	while(1) {
		FD_SET(s, &fds);
		n = select(s + 1, &fds, NULL, NULL, &timeout);

		if(FD_ISSET(s, &fds)) {
			xcache_inbound_udp();
		}

		if((n == 0) && (timeout.tv_sec == 0) && (timeout.tv_usec == 0)) {
			/* It's a timeout */
			ticks ++;
			xcache_set_timeout(&timeout);
			xctrl_timer();
		}

	}
	return 0;
}

int main(int argc, char *argv[])
{
	xctrl_init();
	return xcache_main_loop();
}
