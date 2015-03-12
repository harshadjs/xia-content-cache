#ifndef __LOGGER_H__
#define __LOGGER_H__

enum {
	LOG_INFO,
	LOG_DEBUG,
	LOG_ERR,
};

/* Set debug log level */
void set_log_level(int level);

#ifndef DISABLE_LOGS
/* Don't call this function. Use macro instead */
int _log(const char *func, int level, char *fmt, ...);
#define log(...) _log(__func__, __VA_ARGS__)
#else
#define log()
#endif

#endif
