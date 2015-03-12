#include <stdio.h>
#include <stdarg.h>
#include "logger.h"

static int log_level;

int _log(const char *func, int level, char *fmt, ...)
{
	char new_fmt[512];
	va_list ap;
	int ret;

	if(level < log_level) {
		return 0;
	}

	sprintf(new_fmt, "%s: %s", func, fmt);
	va_start(ap, fmt);

	if(level == LOG_ERR) {
		ret = vfprintf(stderr, new_fmt, ap);
	} else {
		ret = vprintf(new_fmt, ap);
	}
	va_end(ap);

	return ret;
}

void set_log_level(int level)
{
	log_level = LOG_INFO + (level % (LOG_ERR - LOG_INFO));
}
