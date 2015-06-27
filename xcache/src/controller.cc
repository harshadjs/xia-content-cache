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

#define OK_SEND_RESPONSE 1
#define OK_NO_RESPONSE 0
#define BAD -1

#define UNIX_SERVER_SOCKET "/tmp/xcache.socket"

static int context_id = 0;

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

int XcacheController::handleCmd(XcacheCommand *resp, XcacheCommand *cmd)
{
  int ret = OK_NO_RESPONSE;
  XcacheCommand response;

  if(cmd->cmd() == XcacheCommand::XCACHE_STORE) {
    std::cout << "Received STORE command \n";
    ret = store(cmd);
  } else if(cmd->cmd() == XcacheCommand::XCACHE_NEWSLICE) {
    std::cout << "Received NEWSLICE command \n";
    ret = newSlice(resp, cmd);
  } else if(cmd->cmd() == XcacheCommand::XCACHE_GETCHUNK) {
    std::cout << "Received GETCHUNK command \n";
    ret = search(resp, cmd);
  } else if(cmd->cmd() == XcacheCommand::XCACHE_STATUS) {
    std::cout << "Received STATUS command \n";
    status();
  }

  return ret;
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


int XcacheController::newSlice(XcacheCommand *resp, XcacheCommand *cmd)
{
  XcacheSlice *slice;

  if(lookupSlice(cmd)) {
    /* Slice already exists */
    return -1;
  }

  slice = new XcacheSlice(++context_id);

  //  slice->setPolicy(cmd->cachepolicy());
  slice->setTtl(cmd->ttl());
  slice->setSize(cmd->cachesize());

  sliceMap[slice->getContextId()] = slice;
  resp->set_cmd(XcacheCommand::XCACHE_RESPONSE);
  resp->set_contextid(slice->getContextId());

  return OK_SEND_RESPONSE;
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

int XcacheController::search(XcacheCommand *resp, XcacheCommand *cmd)
{
  XcacheSlice *slice;

  std::cout << "Search Request\n";
  slice = lookupSlice(cmd);
  if(!slice) {
    resp->set_cmd(XcacheCommand::XCACHE_ERROR);
  } else {
    std::string data = slice->search(cmd);

    resp->set_cmd(XcacheCommand::XCACHE_RESPONSE);
    resp->set_data(data);
    std::cout << "Looked up Data = " << data << "\n";
  }
  return OK_SEND_RESPONSE;
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
        XcacheCommand resp, cmd;
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
            if(handleCmd(&resp, &cmd) == OK_SEND_RESPONSE) {
              resp.SerializeToString(&buffer);
              write(*iter, buffer.c_str(), buffer.length());
            }
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
