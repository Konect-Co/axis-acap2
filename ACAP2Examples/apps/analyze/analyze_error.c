#include "analyze_error.h"

#define ANALYZE_ERROR_QUARK_STRING "AnalyzeError"

GQuark
analyze_error_quark(void)
{
  return g_quark_from_string(ANALYZE_ERROR_QUARK_STRING);
}
