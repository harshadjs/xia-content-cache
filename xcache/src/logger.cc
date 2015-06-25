#include "logger.h"

Logger::Logger()
{
  prefix = filename = "";
  logToFile = logToStdout = false;
  currentLevel = LOG_INFO;
}

void Logger::setLogFile(std::string filename)
{

}

void Logger::setLogLevel(int level)
{

}

int Logger::log(int level, char *fmt, ...)
{
  int ret;
  va_list ap;

  va_start(ap, fmt);
  ret = vprintf(fmt, ap);
  va_end(ap);
  return ret;
}
