#include "policies.h"

extern struct xcache_policy xcache_policy_fifo;

struct xcache_policy *policies[] = {
	[XCACHE_POLICY_FIFO] = &xcache_policy_fifo,
};
