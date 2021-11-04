#include <stdio.h>
#include <stdarg.h>
#include "includes.h"
#include "log.h"
#ifdef DEBUG
uint8_t runningLevel = PROXY_LOG_DEBUG;
#else
uint8_t runningLevel = PROXY_LOG_INFO;
#endif
#define RED    "31"
#define GREEN  "32"
#define ORANGE "33"
#define BLUE   "34"
#define NCOLOR "0"

#define clearscreen() fprintf(stderr, "\033[H\033[2J")
#define color(param) fprintf(stderr, "\033[%sm",param)

int logger (uint8_t level, const char * format, ...) {
  va_list arg;
  switch (level) {
  case PROXY_LOG_ERROR:
    color (RED);
    break;
  case PROXY_LOG_WARNING:
    color (ORANGE);
    break;
  case PROXY_LOG_INFO:
    color (GREEN);
    break;
  case PROXY_LOG_DEBUG:
    break;
  default:
    color (NCOLOR);
  }
    
  if (runningLevel >= level) {
    va_start (arg, format);
    vfprintf(stderr, format, arg);
    va_end(arg);
    fflush(stderr);
  }
  color (NCOLOR);
  return 0;
  
}




