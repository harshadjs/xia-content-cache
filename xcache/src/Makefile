OBJS = dlist.o
OBJS += xcache.o
OBJS += hash_table.o
OBJS += controller.o
OBJS += core.o
OBJS += helpers.o
OBJS += api.o
OBJS += slice.o
OBJS += meta.o
OBJS += logger.o
OBJS += timer.o
OBJS += cli.o

all: $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $^ $(LDFLAGS)


clean:
	rm -f xcache
	rm -f *.o
