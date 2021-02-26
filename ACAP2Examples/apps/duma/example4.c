#include <stdlib.h>
#include "duma.h"

int main()
{
  int *pi;
  int i;
  pi = (int*)malloc(10*sizeof(int));
  /* Fix the buffer overrun in example3.c */
  for(i=0; i<10; ++i)
    pi[i] = i;
  free(pi);
  return 0;
}
