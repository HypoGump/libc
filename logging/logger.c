#include "logger.h"

#include <sys/time.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>

__thread char t_time[32];
__thread time_t t_seconds;

static Logger logger;
static char log_buf[LIBC_LOG_BUF_SIZE] = {0};

static const char *level_names[LOG_LEVEL_NUMS] = {
  "TRACE ", 
  "DEBUG ", 
  "INFO  ", 
  "WARN  ", 
  "ERROR ", 
  "FATAL "
};

void default_output(const char* msg, int len)
{
  fwrite(msg, 1, len, stdout);
}

void default_flush()
{
  fflush(stdout);
}

int logger_init()
{
  logger.output = &default_output;
  logger.flush = &default_flush;
}


void record_log_time()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  time_t seconds = (time_t)tv.tv_sec;
  int microseconds = (int)tv.tv_usec;
  
  if (t_seconds != seconds) {
    t_seconds = seconds;
    struct tm tm_time;
    gmtime_r(&seconds, &tm_time); // FIXME TimeZone::fromUtcTime

    int len = snprintf(t_time, sizeof(t_time), "%4d%02d%02d %02d:%02d:%02d",
        tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
        tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
    assert(len == 17); (void)len;
  }
  snprintf(t_time + 17, ".%06dZ", microseconds);
}

/*
 * Log Filename Format:
 *     process name.datetime.hostname.process id.log
 * 
 * Log Format:
 *      date time tid loglevel message - sourcefile: line
 */
void log_log(int level, const char* srcfile, const char* func, 
              int line, const char* fmt, ...)
{
  size_t log_len = 0;
  /* 1. date and time*/
  record_log_time();
  log_len += snprintf(log_buf, sizeof(log_buf), "%s ", t_time);
  /* 2. thread id */
  // TODO
  /* 3. sourcefile and line */
  log_len += snprintf(log_buf + log_len, sizeof(log_buf+log_len), 
                      "%s/%s: %d - ", srcfile, func, line);
  /* 4. message */
  va_list args;
  va_start(args, fmt);
  log_len += vsnprinf(log_buf + log_len, sizeof(log_buf+log_len), fmt, args);
  
  /* 5. add '\0' */
  if (log_len < LIBC_LOG_BUF_SIZE) {
    *(log_buf+log_len) = '\0';
  }
  
  logger.output(log_buf, log_len);
  if (level == LOG_LEVEL_FATAL) {
    logger.flush();
    abort();
  }
}
