#include "xcache.h"
#include "timer.h"
#include "logger.h"
#include "dlist.h"

static dlist_node_t *timeout_list;

int timer_list_cmp(void *t1, void *t2)
{
	struct timer_node *n1 = (struct timer_node *)t1,
		*n2 = (struct timer_node *)t2;

	if(n1->timestamp < n2->timestamp)
		return 1;

	if(n1->timestamp > n2->timestamp)
		return -1;

	return 0;
}

void xcache_register_timeout(uint32_t timeout, void (*timer)(void *), void *data)
{
	struct timer_node *node;

	node = (struct timer_node *)xalloc(sizeof(struct timer_node));
	if(!node)
		return;

	node->timer = timer;
	node->timestamp = timeout;
	node->data = data;

	dlist_insert_sorted(&timeout_list, node, timer_list_cmp);
}

uint32_t xcache_handle_timeout(void)
{
	struct timer_node *node;

	if(!timeout_list)
		return 0;

	node = (struct timer_node *)dlist_data(timeout_list);

	while(node && (node->timestamp == ticks)) {
		log(LOG_INFO, "Loop1\n");
		node->timer(node->data);

		dlist_remove_head(&timeout_list, free);
		node = (struct timer_node *)dlist_data(timeout_list);
	}
	return 0;
}

uint32_t xcache_get_next_timeout(void)
{
	struct timer_node *node;

	if(!timeout_list)
		return 0;

	node = (struct timer_node *)dlist_data(timeout_list);
	if(!node)
		return 0;

	return node->timestamp;
}
