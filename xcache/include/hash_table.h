#ifndef __HASH_TABLE_H__
#define __HASH_TABLE_H__

#include "dlist.h"
#include "stdlib.h"

#ifdef __KERNEL__
#define HT_ALLOC(__s) kmalloc((__s), GFP_KERNEL)
#define HT_FREE(__p) kfree(__p)
#else
#define HT_ALLOC(__s) malloc(__s)
#define HT_FREE(__p) free(__p)
#endif

typedef struct {
	int n_buckets, n_items;
	dlist_node_t **buckets;
	int (*hash)(void *);
	int (*cmp)(void *, void *);
	void (*cleanup)(void *);
} ht_t;

typedef struct {
	ht_t *table;
	int current_list;
	dlist_node_t *current_node;
} ht_iter_t;

#include "dlist.h"
ht_t *ht_create(int n_buckets, int (*hash)(void *),
				int (*cmp)(void *, void *),
				void (*cleanup)(void *));

int ht_add(ht_t *, void *);
void *ht_search(ht_t *, void *);
int ht_remove(ht_t *, void *);
int ht_cleanup(ht_t *);

int ht_iter_init(ht_iter_t *, ht_t *);
void *ht_iter_data(ht_iter_t *);
void ht_iter_next(ht_iter_t *);
#endif
