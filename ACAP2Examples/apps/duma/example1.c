#include <stdlib.h>
#include "duma.h"

int main()
{
  int i;
  int *pi;
  /* simulate a memory leak */
  pi = (int*)malloc(10*sizeof(int));
  for(i=0; i<10; ++i)
    pi[i] = i;
  return 0;
}
