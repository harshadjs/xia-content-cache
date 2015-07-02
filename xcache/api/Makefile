CC=g++
AR=ar
CFLAGS=-g -I../../click/include/click
OBJS=XcacheAllocSlice.o XcacheGetChunk.o XcacheInit.o XcacheMain.o XcachePutChunk.o

lib: $(OBJS)
	$(AR) -cvq libxcache.a $(OBJS) ../bin/xcache_cmd.pb.o
	rm -f *.o

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $^ $(LDFLAGS)
clean:
	rm -f *.o
	rm -f libxcache.a
