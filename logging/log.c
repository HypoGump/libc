#include "log.h"

#include <sys/time.h>
#include <time.h>
#include <inttypes.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <errno.h>
#include <signal.h>
#include <execinfo.h>

__thread char t_time[32];
__thread time_t t_seconds;
__thread pid_t t_cachetid = 0;
__thread char t_errnobuf[512];
__thread int t_savedErrno = 0;

typedef struct {
  uint8_t level;
  void (*output)(const char*, size_t);
  void (*flush)();
  bool async_enabled;
} Logger;

__thread char log_buf[LIBC_LOG_BUF_SIZE] = {0};

static const char *level_names[LOG_LEVEL_NUMS] = {
  "TRACE ",
  "DEBUG ",
  "INFO  ",
  "WARN  ",
  "ERROR ",
  "FATAL "
};

/*
 * different level highlight
 */
const char* log_level_color[LOG_LEVEL_NUMS] =
{
  "\033[1;37m",     /* TRACE - WHITE */
  "\033[0;36m",     /* DEBUG - CYAN */
  "\033[0;32;32m",  /* INFO - GREEN */
  "\033[1;33m",     /* WARN - YELLOW */
  "\033[0;35m",     /* ERROR - PURPLE */
  "\033[0;32;31m",  /* FATAL - RED */
};

const char* end_color = "\033[m";

/*
 * set output and flush func
 */
void default_output(const char* msg, size_t len)
{
  fwrite(msg, 1, len, stdout);
}

void default_flush()
{
  fflush(stdout);
}

/*
 * default settings, print log on stdout
 */
static Logger logger = {
      .level = 0,
      .output = &default_output,
      .flush = &default_flush
};

/*
 * signal handler
 */
void segmentfault_handler(int sig) {
  void *array[16];
  char** msgs = NULL;
  size_t size;

  // get void*'s for all entries on the stack
  // every elem points to the return address from the corresponding frame
  size = backtrace(array, 16);

  // print out all the frames to stderr
  // translates the address into an array of strings that describe the
  // address symbolically, including the function name and a offset into
  // the function and the actual return address
  //
  // @param msgs: malloced by backtrace_symbols and must be freed by caller
  msgs = backtrace_symbols(array, size);
  fprintf(stderr, "Error: signal %d:\n"
                "[backtrace] Execution path:\n", sig);

  for (size_t i = 0; i < size; ++i) {
    fprintf(stderr, "#%d %s\n", (int)i, msgs[i]);
  }

  free(msgs);
  exit(1);
}

/*
 * setting interfaces
 */
void set_log_output_func(OutputFunc out)
{
  logger.output = out;
}

void set_log_flush_func(FlushFunc flush)
{
  logger.flush = flush;
}

void set_log_level(int level)
{
  logger.level = level;
}

void set_log_async_enabled()
{
  logger.async_enabled = true;
}


/*
 * call this func before app, if you want to config log settings
 */
void log_init()
{
  signal(SIGSEGV, segmentfault_handler);
  if (logger.async_enabled) {
    set_log_output_func(log_async_get_log);
    log_async_init();
  }
}

void log_exit()
{
  if (logger.async_enabled) {
    log_async_exit();
  }
}

/*
 *  get thread id
 */
static pid_t gettid()
{
  if (t_cachetid == 0) {
    t_cachetid = (pid_t)syscall(SYS_gettid);
  }
  return t_cachetid;
}

/*
 *  generate formated log datetime
 */
static void record_log_time()
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
  sprintf(t_time + 17, ".%06dZ", microseconds);
}

/*
 * Log Filename Format:
 *     process name.datetime.hostname.process id.log
 *
 * Log Format:
 *      date time tid loglevel sourcefile:line - message
 */
void log_log(int level, const char* srcfile, const char* func,
              int line, int sysflag, const char* fmt, ...)
{
  if (level < logger.level) {
    return;
  }

  if (sysflag && level >= LOG_LEVEL_ERROR) {
    t_savedErrno = errno;
  }

  size_t log_len = 0;
  /* 1. date and time*/
  record_log_time();
  log_len += snprintf(log_buf, sizeof(log_buf), "%s ", t_time);

  /* 2. thread id */
  log_len += snprintf(log_buf + log_len, LIBC_LOG_BUF_SIZE - log_len,
                      "%05d ", gettid());

  /* 3. sourcefile and line */
  log_len += snprintf(log_buf + log_len, LIBC_LOG_BUF_SIZE - log_len,
                      "%s%s%s%s/%s:%d - ", log_level_color[level],
                      level_names[level], end_color,
                      srcfile, func, line);

  /* 4. message */
  va_list args;
  va_start(args, fmt);
  log_len += vsnprintf(log_buf + log_len, LIBC_LOG_BUF_SIZE - log_len, fmt, args);

  /* 5. record errno */
  if (sysflag && level >= LOG_LEVEL_ERROR && t_savedErrno != 0) {
    strerror_r(t_savedErrno, t_errnobuf, sizeof(t_errnobuf));
    log_len += snprintf(log_buf + log_len, LIBC_LOG_BUF_SIZE - log_len,
                        "(errno=%d) %s", t_savedErrno, t_errnobuf);
  }

  /* 6. add '\n' and '\0' */
  if (log_len < LIBC_LOG_BUF_SIZE) {
    *(log_buf+log_len) = '\n';
    ++log_len;
  }
  if (log_len < LIBC_LOG_BUF_SIZE) {
    *(log_buf+log_len) = '\0';
  }

  logger.output(log_buf, log_len);
  if (level == LOG_LEVEL_FATAL) {
    logger.flush();
    abort();
  }
}
