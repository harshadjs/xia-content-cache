#include <xcache.h>
#include <xcache_core.h>

int xcache_register_plugin(struct xcache_plugin *plugin)
{
	xcore_addtolist(plugin);
	return 0;
}
