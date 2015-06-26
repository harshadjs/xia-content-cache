CC=g++
CFLAGS=-g -I../../click/include/click
all:
	$(CC) $(CFLAGS) ../bin/xcache_cmd.pb.o xcache_client.cc -o xcache_client -lprotobuf
clean:
	rm -f xcache_client
