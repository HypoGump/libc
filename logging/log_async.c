#include "log.h"
#include "container/list.h"

#include <pthread.h>
#include <string.h>

#define AYSNC_BUF_SIZE 4000*1000  // 4MB
#define LOG_ASYNC_OUTPUT_PTHREAD_PRIORITY  (sched_get_priority_max(SCHED_RR) - 1)

static pthread_t log_async_thread;
static pthread_cond_t log_async_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t log_buf_mutex = PTHREAD_MUTEX_INITIALIZER;

// make sure log backend thread run before app
static pthread_cond_t backend_running_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t backend_running_mutex = PTHREAD_MUTEX_INITIALIZER;
static int count = 1;

typedef struct {
  struct list_node node;
  char buf[AYSNC_BUF_SIZE];
  size_t avail;
  size_t len;
} Buffer;

static Buffer* currentBuffer;
static Buffer* nextBuffer;
LIST_HEAD(buffers);  // buffer list
static size_t buffers_len = 0;

/* log param */
static const int flush_interval = 3;  // flush per 3 seconds

/*
 * allocate new buffer
 */
static inline Buffer* buf_alloc()
{
  Buffer* newbuf = (Buffer*)malloc(sizeof(Buffer));
  memset(newbuf->buf, 0, sizeof(newbuf->buf));
  newbuf->avail = AYSNC_BUF_SIZE;
  newbuf->len = 0;
  return newbuf;
}

static Buffer* ptr_move(Buffer* obj)
{
  Buffer* temp = obj;
  obj = NULL;
  return temp;
}

/*
 * record log line into buffer
 *
 * @param buffer: struct buffer
 * @param msg: log message ready to be output
 * @param len: length of msg
 */
static void append(Buffer* buffer, const char* msg, size_t len)
{
  if (buffer->avail > len) {
    memcpy(buffer->buf, msg, len);
    buffer->len += len;
    buffer->avail -= len;
  }
}

/*
 * output interface to log front-end
 *
 * @param msg: log message ready to be output
 * @param len: length of msg
 */
void log_async_get_log(const char* msg, size_t len)
{
  pthread_mutex_lock(&log_buf_mutex);
  
  if (currentBuffer->avail > len) {
    append(currentBuffer, msg, len);
  }
  else {
    list_push_back(&buffers, &currentBuffer->node);
    ++buffers_len;
    
    if (nextBuffer) {
      currentBuffer = ptr_move(nextBuffer);
    }
    else {
      currentBuffer = buf_alloc();
    }
    append(currentBuffer, msg, len);
    pthread_cond_signal(&log_async_cond);
  }
  
  pthread_mutex_unlock(&log_buf_mutex);
}


/*
 * output backend (log thread func)
 */
static void *log_async_output()
{
  pthread_mutex_lock(&backend_running_mutex);
  --count;
  if (count == 0) pthread_cond_broadcast(&backend_running_cond);
  pthread_mutex_unlock(&backend_running_mutex);
  
  // TODO: open a file 
  log_file_roll();
  Buffer* newBuffer1 = buf_alloc();
  Buffer* newBuffer2 = buf_alloc();
  
  LIST_HEAD(buffersToWrite);
  while (1) {
    pthread_mutex_lock(&log_buf_mutex);
    if (buffers_len == 0) {
      struct timespec abstime;
      clock_gettime(CLOCK_REALTIME, &abstime);
      abstime.tv_sec = flush_interval;
      abstime.tv_nsec = 0;
      pthread_cond_timedwait(&log_async_cond, &log_buf_mutex, &abstime);
    }
    list_push_back(&buffers, &currentBuffer->node);
    currentBuffer = ptr_move(newBuffer1);
    
    list_swap(&buffersToWrite, &buffers);
    buffers_len = 0;
    
    if (!nextBuffer) nextBuffer = ptr_move(newBuffer2);
    pthread_mutex_unlock(&log_buf_mutex);
    
    struct list_node *p;
    list_for_each(p, &buffersToWrite) {
      Buffer* bufptr = list_entry(p, Buffer, node);
      log_file_append(bufptr->buf, bufptr->len);
    }
    
    struct list_node *q;
    list_for_each_safe(p, q, &buffersToWrite) {
      Buffer* bufptr = list_entry(p, Buffer, node);
      free(bufptr);
    }
    
    newBuffer1 = buf_alloc();
    newBuffer2 = buf_alloc();
    
    log_file_flush();
  }
  
  log_file_flush();
  log_file_close();
}

void log_async_init()
{
  currentBuffer = buf_alloc();
  nextBuffer = buf_alloc();
  
  pthread_attr_t thread_attr;
  struct sched_param thread_sched_param;
  
  pthread_attr_init(&thread_attr);
  pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
  
  pthread_attr_setschedpolicy(&thread_attr, SCHED_RR);
  thread_sched_param.sched_priority = LOG_ASYNC_OUTPUT_PTHREAD_PRIORITY;
  pthread_attr_setschedparam(&thread_attr, &thread_sched_param);
  
  pthread_create(&log_async_thread, &thread_attr, log_async_output, NULL);
  
  pthread_attr_destroy(&thread_attr);
  
  // Note: make sure backend start before app
  pthread_mutex_lock(&backend_running_mutex);
  while (count > 0) {
    pthread_cond_wait(&backend_running_cond, &backend_running_mutex);
  }
  pthread_mutex_unlock(&backend_running_mutex);
}

