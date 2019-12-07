#ifndef LIBC_LOG_H_
#define LIBC_LOG_H_

#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

#define LIBC_LOG_ENABLE
#define LIBC_LOG_BUF_SIZE 512


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
  #define LOG_SYSERR(fmt, ...)
  #define LOG_SYSFATAL(fmt, ...)
#else
  // Note: ##__VA_ARGS__ mean omit ',' when '...' is none
  #define LOG_TRACE(fmt, ...) \
          log_log(LOG_LEVEL_TRACE, __FILE__, __FUNCTION__, __LINE__, 0, fmt, ##__VA_ARGS__)
  #define LOG_DEBUG(fmt, ...) \
          log_log(LOG_LEVEL_DEBUG, __FILE__, __FUNCTION__, __LINE__, 0, fmt, ##__VA_ARGS__)
  #define LOG_INFO(fmt, ...) \
          log_log(LOG_LEVEL_INFO, __FILE__, __FUNCTION__, __LINE__, 0, fmt, ##__VA_ARGS__)
  #define LOG_WARN(fmt, ...) \
          log_log(LOG_LEVEL_WARN, __FILE__, __FUNCTION__, __LINE__, 0, fmt, ##__VA_ARGS__)
  #define LOG_ERROR(fmt, ...) \
          log_log(LOG_LEVEL_ERROR, __FILE__, __FUNCTION__, __LINE__, 0, fmt, ##__VA_ARGS__)
  #define LOG_FATAL(fmt, ...) \
          log_log(LOG_LEVEL_FATAL, __FILE__, __FUNCTION__, __LINE__, 0, fmt, ##__VA_ARGS__)
  #define LOG_SYSERR(fmt, ...) \
          log_log(LOG_LEVEL_ERROR, __FILE__, __FUNCTION__, __LINE__, 1, fmt, ##__VA_ARGS__)
  #define LOG_SYSFATAL(fmt, ...) \
          log_log(LOG_LEVEL_FATAL, __FILE__, __FUNCTION__, __LINE__, 1, fmt, ##__VA_ARGS__)
#endif // LIBC_LOG_ENABLE


typedef void (*OutputFunc)(const char*, size_t);
typedef void (*FlushFunc)();

/* log settings */
void set_log_output_func(OutputFunc out);
void set_log_flush_func(FlushFunc flush);
void set_log_level(int level);
// if you want to use async log, call this before your app
void set_log_async_enabled();

typedef struct {
  const char* basename;
  size_t roll_size;
  time_t flush_interval;
} file_params_t;

#define set_log_file_params(...) var_file_params((file_params_t){.basename="libc-logfile", \
                                            .roll_size=500*1000*1000, \
                                            .flush_interval=3, ##__VA_ARGS__})

/* log.c */
void log_init(void);
void log_log(int level, const char* srcfile, const char* func,
              int line, int sysflag, const char* fmt, ...);
void log_exit(void);

/* log_async.c */
void log_async_init(void);
void log_async_exit(void);
void log_async_get_log(const char* msg, size_t len);

/* log_file.c */
void var_file_params(file_params_t init);
void log_file_append(const char* msg, size_t len);
void log_file_flush(void);
void log_file_close(void);
void log_file_roll(void);


#endif