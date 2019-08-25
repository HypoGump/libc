#include "log.h"

#include <time.h>
#include <unistd.h>
#include <string.h>

static FILE* fp = NULL;
static char buffer[64*1024];
static size_t written_bytes = 0;

static time_t start_day = 0;  // need initialization
static time_t last_flush_time = 0;
static int flush_interval = 3;
static size_t roll_size = 500*1000*1000; // 500MB

static char filename[128];
static char basename[128] = "libc-logfile";

static const int SECONDS_PER_DAY = 60*60*24;

/*
 * set basename
 */
void set_log_file_basename(const char* name)
{
  size_t n = snprintf(basename, sizeof(basename), "%s", name);
  *(basename+n) = '\0';
}

void set_log_file_roll_size(size_t size)
{
  roll_size = size;
}


/*
 * get log file name
 * 
 * log file name format:   basename.datetime.hostname.pid.log
 * @param basename: 
 * @param now:
 */
static void get_log_file_name(const char* basename, time_t* now)
{
  memset(filename, 0, sizeof(filename));
  int pos = 0;
  /* 1. basename */
  pos += snprintf(filename, sizeof(filename), "%s", basename);
  /* 2.datetime */
  char timebuf[32];
  struct tm tm;
  *now = time(NULL);
  gmtime_r(now, &tm); // FIXME: localtime_r ?
  strftime(timebuf, sizeof(timebuf), ".%Y%m%d-%H%M%S.", &tm);
  pos += snprintf(filename+pos, 128-pos, "%s", timebuf);
  /* 3. hostname */
  char hostname[128];
  gethostname(hostname, sizeof(hostname));
  pos += snprintf(filename+pos, 128-pos, "%s", hostname);
  /* 4. process id */
  char pidbuf[32];
  snprintf(pidbuf, sizeof(pidbuf), ".%d", getpid());
  pos += snprintf(filename+pos, 128-pos, "%s", pidbuf);
  
  snprintf(filename+pos, 128-pos, ".log");
}

/*
 * write to file without lock
 */
static size_t write_file_unlocked(const char* msg, size_t len) 
{
  return fwrite_unlocked(msg, 1, len, fp);
}

void log_file_flush()
{
  fflush(fp);
}

/*
 * append to file
 */
static void file_append(const char* msg, size_t len)
{
  size_t n = write_file_unlocked(msg, len);
  size_t remain = len - n;
  
  while (remain > 0) {
    size_t x = write_file_unlocked(msg + n, len);
    if (x == 0) {
      int err = ferror(fp);
      if (err) {
        char buf[128];
        strerror_r(err, buf, sizeof(buf));
        fprintf(stderr, "[log_file] log_file_append() failed %s\n", buf);
      }
      break;
    }
    n += x;
    remain -= x;
  }
  
  written_bytes += len;
}

/*
 * interface to log_async_output 
 */
void log_file_append(const char* msg, size_t len)
{
  file_append(msg, len);
  
  if (written_bytes > roll_size) {
    log_file_roll();
  }
  else {
    time_t now = time(NULL);
    time_t this_day = now / SECONDS_PER_DAY;
    
    if (this_day != start_day) {
      log_file_roll();
    }
    else if (now - last_flush_time > flush_interval) {
      last_flush_time = now;
      log_file_flush();
    }
  }
}

/*
 * close old file and roll a new file
 */
void log_file_roll()
{
  if (fp) fclose(fp);
  
  time_t now;
  get_log_file_name(basename, &now);
  
  start_day = now / SECONDS_PER_DAY;
  last_flush_time = now;
  written_bytes = 0;
  
  fp = fopen(filename, "ae");
  setbuffer(fp, buffer, sizeof(buffer));
}

void log_file_close()
{
  fclose(fp);
}
