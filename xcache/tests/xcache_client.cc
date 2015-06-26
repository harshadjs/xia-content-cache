#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../proto/xcache_cmd.pb.h"

#define UNIX_SERVER_SOCKET "/tmp/xcache.socket"

static int s;

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

int main(void)
{
  XcacheCommand cmd;
  std::string cmd_on_wire;

  xcache_init();

  cmd.set_cmd(XcacheCommand::XCACHE_STATUS);
  cmd.SerializeToString(&cmd_on_wire);
  
  write(s, cmd_on_wire.c_str(), cmd_on_wire.length());

  return 0;
}
