#include <stdlib.h>
#include "duma.h"

int main()
{
  int *pi;
  int i;
  pi = (int*)malloc(10*sizeof(int));
  /* Simulate a buffer overrun that will trigger a 
   * segfault in your program when receiving signal SIGSEGV 
   * do on target export DUMA_PROTECT_BELOW=0 
   * Use the debugger backtrace to find where it happens */
  for(i=0; i<11; ++i)
    pi[i] = i;
  free(pi);
  return 0;
}
