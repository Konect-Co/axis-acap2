#ifndef _STORAGE_H_
#define _STORAGE_H_

#include <glib.h>
#include <axsdk/axstorage.h>

gboolean
storage_setup(GError **error);

void
storage_release(void);

void
storage_write(const gchar* message);

#endif /* _STORAGE_H_ */
