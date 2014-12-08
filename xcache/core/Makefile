OBJS = dlist.o
OBJS += xcache.o
OBJS += hash_table.o
OBJS += xcache_controller.o
OBJS += xcache_slice.o
OBJS += xcache_helpers.o
OBJS += xcache_plugins.o

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $^ $(LDFLAGS)

xcache: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f xcache
	rm -f *.o
