#include "log.h"

#include <pthread.h>

#define AYSNC_BUF_SIZE 

static pthread_t log_async_thread;
static pthread_cond_t log_async_cond;

typedef struct {
  char buf[AYSNC_BUF_SIZE];
  size_t len;
} Buffer;

static char log_buf[AYSNC_BUF_SIZE] = {0};



