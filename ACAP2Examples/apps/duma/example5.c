#include <stdlib.h>
#include "duma.h"

int main()
{
  int *pi;
  pi = (int*)malloc(10*sizeof(int));
  /* simulate a buffer underrun that will trigger a SIGSEGV */
  /* set this variable on target: export DUMA_PROTECT_BELOW=1 */
  /* Use the debugger backtrace to find where it happens */
  pi[-2] = 0;
  return 0;
}
