#include <arpa/inet.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xia_cache_req.h"

#define UDP_MAX_PKT (64 * 1024)

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

void generate_random_xid(uint8_t *xid)
{
	int i;

	for(i = 0; i < CLICK_XIA_XID_ID_LEN; i++)
		xid[i] = (uint8_t)rand();
}

uint8_t *str2cid(char *xid_str)
{
	int i, j;
	static uint8_t cid[CLICK_XIA_XID_ID_LEN];

	memset(cid, 0, CLICK_XIA_XID_ID_LEN);
	j = CLICK_XIA_XID_ID_LEN - 1;

	for(i = strlen(xid_str) - 1; i >= 0; i--) {
		cid[j--] = xid_str[i] - '0';
	}

	return cid;
}

void create_search_req(xcache_req_t *req, uint8_t *cid)
{
	req->request = XCACHE_SEARCH;
	req->cid.type = 0;
	memcpy(req->cid.id, cid, CLICK_XIA_XID_ID_LEN);
	req->context.ttl = 0;
	req->len = 0;
}

void create_store_req(xcache_req_t *req, uint8_t *cid)
{
	req->request = XCACHE_STORE;
	req->cid.type = 0;
	memcpy(req->cid.id, cid, CLICK_XIA_XID_ID_LEN);
	req->context.ttl = 100;
	req->len = 0;
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

#if 0
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
	n = read(fd, buf, file_size);

	create_store_req(&req, cid);
	req.len = n;
	printf("n = %d, filesize = %d\n", n, file_size);

	memcpy(packet, &req, sizeof(xcache_req_t));
	memcpy(payload, buf, n);
	send_data(packet, sizeof(xcache_req_t) + n);

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
	recvfrom(sock, packet, UDP_MAX_PKT, 0,
			 (struct sockaddr *)&dest_addr, &socklen);
	recvd = hdr->len;
	write(fd, payload, hdr->len);
	printf("recvd = %d\n", recvd);

	close(fd);
	TEST_END_LOG;
}
#endif

void dump_received(uint8_t *data)
{
	xcache_req_t *req = (xcache_req_t *)data;
	int i;

	printf("Size = %d\n", req->len);
	for(i = 0; i < req->len; i++) {
		printf("%c", data[sizeof(xcache_req_t) + i]);
	}
	printf("\n");
}

int main(int argc, char *argv[])
{
	uint8_t packet[UDP_MAX_PKT], *payload;
	xcache_req_t req;
	xcache_req_t *hdr = (xcache_req_t *)packet;
	int fd;
	int file_size;
	socklen_t socklen;

	if(argc < 2) {
		printf("Usage: %s search|store <cid> <filename> <ttl>\n", argv[0]);
		return 0;
	}

	payload = packet;
	payload += sizeof(xcache_req_t);

	sock = create_socket();
	dest_addr.sin_family = AF_INET;
	inet_aton("127.0.0.1", (struct in_addr *)&dest_addr.sin_addr.s_addr);
	dest_addr.sin_port = htons(1444);

	if(strcmp(argv[1], "search") == 0) {
		create_search_req(&req, str2cid(argv[2]));
		send_data((uint8_t *)&req, sizeof(req));
		printf("Received: %d\n",
			   recvfrom(sock, packet, UDP_MAX_PKT, 0,
						(struct sockaddr *)&dest_addr, &socklen));
		dump_received(packet);
	} else if(strcmp(argv[1], "store") == 0) {
		/* xcache_client store <cid> <filename> <ttl> */
		create_store_req(hdr, str2cid(argv[2]));
		fd = open(argv[3], O_RDWR);
		file_size = lseek(fd, 0, SEEK_END);
		lseek(fd, 0, SEEK_SET);

		hdr->len = read(fd, packet + sizeof(xcache_req_t), file_size);
		hdr->context.ttl = strtol(argv[4], NULL, 10);
		send_data(packet, sizeof(xcache_req_t) + hdr->len);
	}

	return 0;
}
