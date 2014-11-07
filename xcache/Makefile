CFLAGS=-g -I../click/include/click -Iinclude -Wall -Werror
all:
	gcc	$(CFLAGS) src/dlist.c src/xcache.c src/xcache_controller.c src/hash_table.c src/xcache_helpers.c src/xcache_slice.c -o xcache
clean:
	rm -f xcache
