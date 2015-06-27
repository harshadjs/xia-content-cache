#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../proto/xcache_cmd.pb.h"

#define UNIX_SERVER_SOCKET "/tmp/xcache.socket"

struct xcache_chunk_info {
  char cid[512];
};

static int s;
static int context_id;

int xcache_init(void)
{
  char buf[512];
  struct sockaddr_un xcache_addr;
  socklen_t len;

  s = socket(AF_UNIX, SOCK_SEQPACKET, 0);
  if(s < 0)
    return -1;

  /* Setup xcache's address */
  xcache_addr.sun_family = AF_UNIX;
  strcpy(xcache_addr.sun_path, UNIX_SERVER_SOCKET);

  if(connect(s, (struct sockaddr *)&xcache_addr, sizeof(xcache_addr)) < 0)
    return -1;

  return 0;
}

static int send_command(XcacheCommand *cmd)
{
  std::string cmd_on_wire;

  cmd->SerializeToString(&cmd_on_wire);
  return write(s, cmd_on_wire.c_str(), cmd_on_wire.length());
}

static int get_response_blocking(XcacheCommand *cmd)
{
  char buf[512] = "";
  std::string buffer;
  int ret;

  do {
    ret = read(s, buf, 512);
    if(ret == 0)
      break;

    buffer.append(buf);
  } while(ret == 512);

  if(ret == 0) {
    cmd->set_cmd(XcacheCommand::XCACHE_ERROR);
    return -1;
  } else {
    cmd->ParseFromString(buffer);
  }

  return 0;
}

/* API */
int XgetChunk(xcache_chunk_info *info, void **buf, size_t *len, int flags)
{
  XcacheCommand cmd;

  cmd.set_cmd(XcacheCommand::XCACHE_GETCHUNK);
  cmd.set_contextid(context_id);
  cmd.set_cid(std::string(info->cid));

  if(send_command(&cmd) < 0) {
    /* Error in Sending chunk */
    return -1;
  }

  if(get_response_blocking(&cmd) >= 0) {
    /* Got a valid response from Xcache */
    (*len) = cmd.data().length();
    (*buf) = malloc(*len);
    memcpy((*buf), cmd.data().c_str(), cmd.data().length());
    return 0;
  }

  return -1;
}

/* API */
int XallocateSlice(int32_t cache_size, int32_t ttl, int32_t cache_policy)
{
  int ret;
  XcacheCommand cmd;

  cmd.set_cmd(XcacheCommand::XCACHE_NEWSLICE);
  cmd.set_ttl(ttl);
  cmd.set_cachesize(cache_size);
  cmd.set_cachepolicy(cache_policy);

  if((ret = send_command(&cmd)) < 0) {
    return ret;
  }

  if((ret = get_response_blocking(&cmd)) >= 0) {
    context_id = cmd.contextid();
    std::cout << "Received contextid " << context_id << "\n";
  }

  return ret;
}

/* API */
int XputChunk(void *data, size_t len, xcache_chunk_info *info)
{
  int ret;
  XcacheCommand cmd;

  cmd.set_cmd(XcacheCommand::XCACHE_STORE);
  cmd.set_contextid(context_id);
  cmd.set_data(data, len);
  cmd.set_cid(std::string("CID:TODO"));
  strcpy(info->cid, "CID:TODO");

  return send_command(&cmd);
}

int main(void)
{
  XcacheCommand cmd;
  std::string cmd_on_wire;
  xcache_chunk_info info;
  void *buf;
  size_t len;

  xcache_init();

  XallocateSlice(1000, 10, 1);
  XputChunk((void *)"Harshad", strlen("Harshad"), &info);
  XgetChunk(&info, &buf, &len, 0);

  std::cout << "Received " << len << " bytes\n";

  return 0;
}
