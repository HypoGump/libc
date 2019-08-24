#ifndef LIBC_LOG_H_
#define LIBC_LOG_H_

#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define LIBC_LOG_ENABLE
#define LIBC_LOG_BUF_SIZE 512

// TODO:
//   1. async logging
//   2. rolling file auto matically

enum { LOG_LEVEL_TRACE, 
       LOG_LEVEL_DEBUG, 
       LOG_LEVEL_INFO, 
       LOG_LEVEL_WARN, 
       LOG_LEVEL_ERROR, 
       LOG_LEVEL_FATAL, 
       LOG_LEVEL_NUMS, };



#ifndef LIBC_LOG_ENABLE
  #define LOG_TRACE(fmt, ...)
  #define LOG_DEBUG(fmt, ...)
  #define LOG_INFO(fmt, ...)
  #define LOG_WARN(fmt, ...)
  #define LOG_ERROR(fmt, ...)
  #define LOG_FATAL(fmt, ...)
#else
  // Note: ##__VA_ARGS__ mean omit ',' when '...' is none
  #define LOG_TRACE(fmt, ...) \
          log_log(LOG_LEVEL_TRACE, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
  #define LOG_DEBUG(fmt, ...) \
          log_log(LOG_LEVEL_DEBUG, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
  #define LOG_INFO(fmt, ...) \
          log_log(LOG_LEVEL_INFO, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
  #define LOG_WARN(fmt, ...) \
          log_log(LOG_LEVEL_WARN, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
  #define LOG_ERROR(fmt, ...) \
          log_log(LOG_LEVEL_ERROR, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
  #define LOG_FATAL(fmt, ...) \
          log_log(LOG_LEVEL_FATAL, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#endif // LIBC_LOG_ENABLE


int log_init();
void log_log(int level, const char* srcfile, const char* func, 
              int line, const char* fmt, ...);

/* log_async.c */
void log_async_output(const char* msg, size_t len);

#endif