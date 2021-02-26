#define _GNU_SOURCE
#include <stdio.h>
#include "lib_shared.h"


void call_shared_lib()
{
  int status = -1;
 /* Make a function call to the local shared lib,
  * resides in ./lib on target
  */
  status = shared_lib_function();

  fprintf(stdout, "Shared lib returned %d\n", status);
  return;
}

/****************************************************************************/
int main(int argc, char **argv)
{
  call_shared_lib();
  return 0;
}

