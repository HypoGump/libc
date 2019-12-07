#include "log.h"
#include <stdio.h>

void seg()
{
  int *foo = (int*)-1;
  printf("%d\n", *foo);
}

void bar() { seg(); }
void foo() { bar(); }

int main(int argc, char** argv)
{
  printf("%s\n", argv[0]+2);
  log_init(argv[0]+2);

  foo();
}