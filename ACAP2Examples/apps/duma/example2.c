#include <stdlib.h>
#include <duma.h>

int main()
{
  int *pi;
  int i;
  pi = (int*)malloc(10*sizeof(int));
  for(i=0; i<10; ++i)
    pi[i] = i;
  /* fix the memory leak */
  free(pi);
  return 0;
}
