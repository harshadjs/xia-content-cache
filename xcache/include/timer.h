#ifndef __TIMER_H__
#define __TIMER_H__

struct timer_node {
	void (*timer)(void *);
	uint64_t timestamp;
	void *data;
};

uint32_t xcache_get_next_timeout(void);
uint32_t xcache_handle_timeout(void);
void xcache_register_timeout(uint32_t timeout, void (*timer)(void *), void *data);

#define TICKS_FROM_NOW(__time) ((ticks) + (__time))

#endif
