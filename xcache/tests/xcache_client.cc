#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../api/xcache.h"

int main(void)
{
  xcacheSlice slice;
  xcacheChunk chunk;

  XcacheInit();

  XcacheAllocateSlice(&slice, 1000, 10, 1);
  chunk.buf = (void *)"Harshad";
  chunk.len = strlen("harshad");
  XcachePutChunk(&slice, &chunk);
  XcacheGetChunk(&slice, &chunk, 0);

  std::cout << "Received " << chunk.len << " bytes\n";

  return 0;
}
