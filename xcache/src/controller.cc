#include <fcntl.h>
#include <string>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "controller.h"
#include <iostream>

#define UNIX_SERVER_SOCKET "/tmp/xcache.socket"


static int xcache_create_click_socket(int port)
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

static int xcache_create_lib_socket(void)
{
  struct sockaddr_un my_addr;
  int s;

  s = socket(AF_UNIX, SOCK_SEQPACKET, 0);

  my_addr.sun_family = AF_UNIX;
  strcpy(my_addr.sun_path, UNIX_SERVER_SOCKET);

  bind(s, (struct sockaddr *)&my_addr, sizeof(my_addr));
  listen(s, 100);

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


void XcacheController::status(void)
{
  std::map<int32_t, XcacheSlice *>::iterator i;

  std::cout << "[Status]\n";
  for(i = sliceMap.begin(); i != sliceMap.end(); ++i) {
    i->second->status();
  }
}


void XcacheController::handleUdp(int s)
{
}

void XcacheController::handleCmd(XcacheCommand *cmd)
{
  int ret;

  if(cmd->cmd() == XcacheCommand::XCACHE_STORE) {
    ret = store(cmd);
  } else if(cmd->cmd() == XcacheCommand::XCACHE_NEWSLICE) {
    ret = newSlice(cmd);
  } else if(cmd->cmd() == XcacheCommand::XCACHE_SEARCH) {
    XcacheCommand response = search(cmd);
  } else if(cmd->cmd() == XcacheCommand::XCACHE_STATUS) {
    status();
  }

  ret = ret;
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
  XcacheSlice *slice;

  if(lookupSlice(cmd)) {
    /* Slice already exists */
    return -1;
  }

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
  XcacheSlice *slice;

  std::cout << "Search Request\n";
  slice = lookupSlice(cmd);
  if(!slice) {
    response.set_cmd(XcacheCommand::XCACHE_ERROR);
  } else {
    response.set_cmd(XcacheCommand::XCACHE_RESPONSE);
    response.set_data(slice->search(cmd));
  }
  return response;
}

void XcacheController::run(void)
{
  fd_set fds, allfds;
  struct timeval timeout;
  int max, libsocket, s, n;
  std::vector<int> activeConnections;
  std::vector<int>::iterator iter;

  s = xcache_create_click_socket(1444);
  libsocket = xcache_create_lib_socket();

  FD_ZERO(&fds);
  FD_SET(s, &allfds);
  FD_SET(libsocket, &allfds);
#define MAX(_a, _b) ((_a > _b) ? (_a) : (_b))

  xcache_set_timeout(&timeout);

  while(1) {
    memcpy(&fds, &allfds, sizeof(fd_set));

    max = MAX(libsocket, s);
    for(iter = activeConnections.begin(); iter != activeConnections.end(); ++iter) {
      max = MAX(max, *iter);
    }

    n = select(max + 1, &fds, NULL, NULL, &timeout);

    if(FD_ISSET(s, &fds)) {
      std::cout << "Action on UDP" << std::endl;
      handleUdp(s);
    }

    if(FD_ISSET(libsocket, &fds)) {
      int new_connection = accept(libsocket, NULL, 0);
      std::cout << "Action on libsocket" << std::endl;
      activeConnections.push_back(new_connection);
      FD_SET(new_connection, &allfds);
    }

    for(iter = activeConnections.begin(); iter != activeConnections.end();) {
      if(FD_ISSET(*iter, &fds)) {
        char buf[512] = "";
        std::string buffer;
        XcacheCommand cmd;
        int ret;

        do {
          ret = read(*iter, buf, 512);
          if(ret == 0)
            break;

          buffer.append(buf);
        } while(ret == 512);

        if(ret != 0) {
          bool parseSuccess = cmd.ParseFromString(buffer);
          if(!parseSuccess) {
            std::cout << "[ERROR] Protobuf could not parse\n;";
          } else {
            handleCmd(&cmd);
          }
        }

        if(ret == 0) {
          std::cout << "Closing\n";
          close(*iter);
          FD_CLR(*iter, &allfds);
          activeConnections.erase(iter);
          continue;
        }
      }
      ++iter;
    }

    if((n == 0) && (timeout.tv_sec == 0) && (timeout.tv_usec == 0)) {
      // std::cout << "Timeout" << std::endl;
    }
  }
}
