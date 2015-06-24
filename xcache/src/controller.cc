#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "controller.h"
#include <iostream>
#include "xcache_cmd.pb.h"

static int xcache_creat_sock(int port)
{
	struct sockaddr_in si_me;
    int s;

    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
		return -1;

    memset((char *)&si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(port);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(s, (struct sockaddr *)&si_me, sizeof(si_me)) == -1)
		return -1;

    return s;
}

static void xcache_set_timeout(struct timeval *t)
{
	t->tv_sec = 1;
	t->tv_usec = 0;
}

void XcacheController::handleCli(void)
{
  char buf[512], *ret;
  
  ret = fgets(buf, 512, stdin);

  if(ret) {
    std::cout << "Received: " << buf;
    if(strncmp(buf, "store", strlen("store")) == 0) {
      std::cout << "Store Request" << std::endl;
    } else if(strncmp(buf, "search", strlen("search")) == 0) {
      std::cout << "Search Request" << std::endl;
    } else if(strncmp(buf, "status", strlen("status")) == 0) {
      std::cout << "status Request" << std::endl;
    } 
  }
}

void XcacheController::store(XcacheCommand *cmd)
{
  
}

void XcacheController::run(void)
{
  fd_set fds;
  struct timeval timeout;
  int max, s, n, command_received = 1;

  //  ticks = 1;
  s = xcache_creat_sock(1444);

  FD_ZERO(&fds);
  xcache_set_timeout(&timeout);

  while(1) {
    FD_SET(s, &fds);
    FD_SET(fileno(stdin), &fds);
#define MAX(_a, _b) ((_a > _b) ? (_a) : (_b))
    max = MAX(s, fileno(stdin));

    if(command_received == 1) {
      /* Display prompt */
      fprintf(stderr, "xcache > ");
      command_received = 0;
    }

    n = select(max + 1, &fds, NULL, NULL, &timeout);
    command_received = 0;

    if(FD_ISSET(s, &fds)) {
      std::cout << "Action on UDP" << std::endl;
    }

    if(FD_ISSET(fileno(stdin), &fds)) {
      std::cout << "Action on stdin" << std::endl;
      handleCli();
      command_received = 1;
    }

    if((n == 0) && (timeout.tv_sec == 0) && (timeout.tv_usec == 0)) {
      // std::cout << "Timeout" << std::endl;
    }
  }
}
