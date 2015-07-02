CC=g++
CFLAGS=-g -I../../click/include/click
all:
	$(CC) $(CFLAGS) xcache_client.cc ../api/libxcache.a -o xcache_client -lprotobuf
clean:
	rm -f xcache_client
