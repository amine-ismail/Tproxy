#ifndef PROXY_LOG_H
#define PROXY_LOG_H
#include <stdio.h>
#include <stdarg.h>
#include "includes.h"
#define PROXY_LOG_ERROR 1
#define PROXY_LOG_WARNING 2
#define PROXY_LOG_INFO 3
#define PROXY_LOG_DEBUG 4

inline int logger (uint8_t level, const char * format, ...);
#ifdef DEBUG
#define logDebug(args...) (logger(PROXY_LOG_DEBUG, args))
#else 
#define logDebug(args...) 
#endif

#define logInfo(args...) (logger(PROXY_LOG_INFO, args))
#define logWarning(args...) (logger(PROXY_LOG_WARNING, args))
#define logError(...) (logger(PROXY_LOG_ERROR, __VA_ARGS__))

#endif
