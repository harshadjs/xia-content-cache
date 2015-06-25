#include <fcntl.h>
#include <string>
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


void XcacheController::handleUdp(int s)
{
  char buf[500];
  XcacheCommand cmd;
  int ret;

  /* TODO: Handle big packets */
  ret = recvfrom(s, buf, 500, MSG_DONTWAIT, NULL, 0);

  bool parseSuccess = cmd.ParseFromArray(buf, ret);
  if(!parseSuccess) {
    std::cout << "[ERROR] Protobuf could not parse\n;";
    return;
  }

  if(cmd.cmd() == XcacheCommand::XCACHE_STORE) {
    ret = store(&cmd);
  } else if(cmd.cmd() == XcacheCommand::XCACHE_NEWSLICE) {
    ret = newSlice(&cmd);
  } else if(cmd.cmd() == XcacheCommand::XCACHE_SEARCH) {
    XcacheCommand response = search(&cmd);
  }

}


XcacheSlice *XcacheController::lookupSlice(XcacheCommand *cmd)
{
  std::map<int32_t, XcacheSlice *>::iterator i = sliceMap.find(cmd->contextid());

  if(i != sliceMap.end()) {
    std::cout << "[Success] Slice Exists.\n";
    return i->second;
  } else {
    /* TODO use default slice */
    std::cout << "Slice does not exist. Falling back to default slice.\n";
    return NULL;
  }
}

int XcacheController::newSlice(XcacheCommand *cmd)
{
  /* TODO: Check if the slice already exists */
  XcacheSlice *slice;

  slice = new XcacheSlice(cmd->contextid());
  /* TODO: Set slice config */

  sliceMap[cmd->contextid()] = slice;
  std::cout << "New Slice." << std::endl;
  return 0;
}

int XcacheController::store(XcacheCommand *cmd)
{
  XcacheSlice *slice;
  XcacheMeta *meta;
  std::map<std::string, XcacheMeta *>::iterator i = metaMap.find(cmd->cid());

  std::cout << "Reached " << __func__ << std::endl;

  if(i != metaMap.end()) {
    meta = i->second;
    std::cout << "Meta Exsits." << std::endl;
  } else {
    /* New object - Allocate a meta */
    meta = new XcacheMeta(cmd);
    metaMap[cmd->cid()] = meta;
    std::cout << "New Meta." << std::endl;
  }

  slice = lookupSlice(cmd);
  if(!slice)
    return -1;

  if(slice->store(meta, cmd->data()) < 0) {
    std::cout << "Slice store failed\n";
    return -1;
  }

  return storeManager.store(meta, cmd->data());
}

XcacheCommand XcacheController::search(XcacheCommand *cmd)
{
  XcacheCommand response;
  std::cout << "Search Request\n";
  return response;
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
      handleUdp(s);
      command_received = 1;
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
