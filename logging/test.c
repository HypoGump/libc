#include "log.h"
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>

long g_total;
FILE* g_file;

void dummy_output(const char* msg, size_t len)
{
  g_total += len;
  if (g_file) {
    fwrite(msg, 1, len, g_file);
  }
  else {
    log_file_append(msg, len);
  }
}

void bench()
{
  set_log_output_func(dummy_output);
  const int batch = 1000*1000;
  g_total = 0;
  
  // start
  struct timeval start;
  gettimeofday(&start, NULL);
  for (int i = 0; i < batch; ++i)
  {
    LOG_INFO("Hello 0123456789 abcdefghijklmnopqrstuvwxyz"
            " ABCDEFGHIJKLMNOPQRSTUVWXYZ %d", i);
  }
  struct timeval end;
  gettimeofday(&end, NULL);
  // end
  time_t usenconds = (end.tv_sec * 1000 * 1000 + end.tv_usec) - (start.tv_sec * 1000 * 1000 + start.tv_usec);
  double seconds = (double)usenconds/(1000*1000);
  printf("%f seconds, %ld bytes, %.2f msg/s, %.2f MiB/s\n",
         seconds, g_total, batch / seconds, g_total / seconds / 1024 / 1024);
}

int main(void)
{
  printf("Hello World!\n");
  LOG_TRACE("LOG_TRACE");
  LOG_DEBUG("LOG_DEBUG");
  LOG_INFO("LOG_INFO");
  LOG_WARN("LOG_WARN");
  LOG_ERROR("LOG_ERROR");
  
  char buffer[64*1024];
  
  g_file = fopen("/dev/null", "w");
  setbuffer(g_file, buffer, sizeof buffer);
  bench();
  fclose(g_file);
  
  g_file = fopen("/tmp/log", "w");
  setbuffer(g_file, buffer, sizeof buffer);
  bench();
  fclose(g_file);
  
  g_file = NULL;
  
  set_log_file_basename("test_log");
  log_file_roll();
  bench();
  log_file_close();
  
  return 0;
}