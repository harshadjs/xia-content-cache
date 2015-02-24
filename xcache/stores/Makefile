OBJS=memht.o
all: $(OBJS)
%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $^ $(LDFLAGS)

clean:
	rm -f $(OBJS)
