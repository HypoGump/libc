#include "log.h"


int main(void)
{
  printf("Hello World!\n");
  log_init();
  
  LOG_TRACE("LOG_TRACE");
  LOG_DEBUG("LOG_DEBUG");
  LOG_INFO("LOG_INFO");
  LOG_WARN("LOG_WARN");
  LOG_ERROR("LOG_ERROR");
  //LOG_FATAL("LOG_FATAL");
  return 0;
}