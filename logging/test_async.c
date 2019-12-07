#include "log.h"
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>

long g_total = 146888890;

void log_func()
{
  const int batch = 1000*1000;

  printf("start logging...\n");
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

int main()
{
  set_log_async_enabled();
  set_log_file_params(.basename="Hello");
  log_init();

  pthread_t log_threads[12];

  for (int i = 0; i < 12; ++i) {
    pthread_create(&log_threads[i], NULL, (void*)log_func, NULL);
  }

  for (int i = 0; i < 12; ++i) {
    pthread_join(log_threads[i], NULL);
  }

  log_exit();
}