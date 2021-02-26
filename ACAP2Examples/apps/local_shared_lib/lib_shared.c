#include <stdio.h>
#include "lib_shared.h"

int shared_lib_function(void)
{
  fprintf(stdout, "local_shared_lib: \n");
  return SHARED_LIB_OK;
}

