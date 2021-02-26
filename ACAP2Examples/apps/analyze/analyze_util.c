#include <glib.h>
#include "analyze_error.h"
#include "analyze_util.h"

gboolean
util_string_to_guint(const gchar *str,
    guint *val)
{
  const gchar *p;
  guint lval;
  gboolean result = FALSE;

  p = str;
  lval = 0;

  while (*p) {
    if (!g_ascii_isdigit(*p)) {
      goto error;
    }

    lval = lval * 10 + (*p - '0');

    p++;
  }

  *val = lval;

  result = TRUE;
error:
  return result;
}
