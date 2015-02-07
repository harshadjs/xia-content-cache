#include <arpa/inet.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xia_cache_req.h"

#define UDP_MAX_PKT 4096

struct sockaddr_in dest_addr;
uint8_t cid[CLICK_XIA_XID_ID_LEN];
int sock;
char *input_file, *output_file;

#define TEST_START_LOG(__msg) do {								\
		printf("============== %s =============\n", __func__);	\
		printf("[%s]\n", __msg);								\
	} while(0)

#define TEST_END_LOG do {								\
		printf("===================================\n\n");	\
	} while(0)

struct click_xia_xid my_hid = {
	1,
	{1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
	 11, 12, 13, 14, 15, 16, 17, 18, 19, 20}
};

void generate_random_xid(uint8_t *xid)
{
	int i;

	for(i = 0; i < CLICK_XIA_XID_ID_LEN; i++)
		xid[i] = (uint8_t)random();
}

void create_search_req(xcache_req_t *req, uint8_t *cid)
{
	req->request = XCACHE_SEARCH;
	req->ch.cid.type = 0;
	memcpy(req->ch.cid.id, cid, CLICK_XIA_XID_ID_LEN);
	req->ch.ttl = 0;
	req->hid = my_hid;
	req->offset = 0;
	req->len = 0;
	req->total_len = 0;
}

void create_store_req(xcache_req_t *req, uint8_t *cid)
{
	req->request = XCACHE_STORE;
	req->ch.cid.type = 0;
	memcpy(req->ch.cid.id, cid, CLICK_XIA_XID_ID_LEN);
	req->ch.ttl = 100;
	req->hid = my_hid;
	req->offset = 0;
	req->len = 0;
	req->total_len = 0;
}

int create_socket(void)
{
	return socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
}

void dump_request(xcache_req_t *req, char *str)
{
	printf("%s request: %s\n", (req->request == XCACHE_STORE) ?
		   "STORE" : "SEARCH", str);
}


void send_data(uint8_t *data, int len) {
	int n;

	n = sendto(sock, data, len, 0, (struct sockaddr *)&dest_addr,
			   sizeof(struct sockaddr));
	printf("\tRequested: %d, Sent %d bytes\n", len, n);
}

void test_1(void)
{
	uint8_t packet[UDP_MAX_PKT], buf[UDP_MAX_PKT], *payload;
	xcache_req_t req;
	int fd, n;
	int file_size;
	off_t cur_off = 0;

	TEST_START_LOG("Sending data to xcache");

	fd = open(input_file, O_RDWR);
	if(!fd) {
		printf("Invalid input file: %s\n", input_file);
		return;
	}
	file_size = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);

	generate_random_xid(cid);

	payload = packet;
	payload += sizeof(xcache_req_t);
	do {
		n = read(fd, buf, 1000);

		create_store_req(&req, cid);
		req.len = n;
		printf("n = %d, filesize = %d\n", n, file_size);
		req.total_len = file_size;
		req.offset = cur_off;

		memcpy(packet, &req, sizeof(xcache_req_t));
		memcpy(payload, buf, n);
		send_data(packet, sizeof(xcache_req_t) + n);

		cur_off += n;
	} while(n == 1000);

	create_search_req(&req, cid);
	send_data((uint8_t *)&req, sizeof(xcache_req_t));
	close(fd);
	TEST_END_LOG;
}

void test_2()
{
	int fd = open(output_file, O_RDWR | O_CREAT);
	int recvd = 0, total_len;
	uint8_t packet[UDP_MAX_PKT], *payload;
	xcache_req_t *hdr = (xcache_req_t *)packet;
	socklen_t socklen;

	TEST_START_LOG("Sending search request");
	payload = packet;
	payload += sizeof(xcache_req_t);

	if(fd < 0) {
		printf("%s: Invalid output file: %s\n", __func__, output_file);
		return;
	}
	do {
		recvfrom(sock, packet, UDP_MAX_PKT, 0,
				 (struct sockaddr *)&dest_addr, &socklen);
		total_len = hdr->total_len;
		recvd += hdr->len;
		write(fd, payload, hdr->len);
		printf("recvd = %d\n", recvd);
	} while(recvd < total_len);

	close(fd);
	TEST_END_LOG;
}

int main(int argc, char *argv[])
{
	if(argc < 3) {
		printf("Usage: %s <input_file> <output_file>\n", argv[0]);
		return 1;
	}

	input_file = argv[1];
	output_file = argv[2];
	sock = create_socket();
	dest_addr.sin_family = AF_INET;
	inet_aton("127.0.0.1", (struct in_addr *)&dest_addr.sin_addr.s_addr);
	dest_addr.sin_port = htons(1444);

	test_1();
	test_2();
	return 0;
}
