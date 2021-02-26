#include <axsdk/axstorage.h>
#include <glib/gstdio.h>
#include "storage.h"

#define FILENAME "analyze.log"

typedef struct _StorageParams StorageParams;

struct _StorageParams {
  AXStorage *axstorage;
  guint subscription_id;
  gchar* storage_path;
  gboolean writable;
  gboolean full;
  gboolean exiting;
  gboolean available;
};

static StorageParams *storage_params = NULL;

static void
setup_storage_callback(AXStorage *storage,
    gpointer user_data,
    GError *error);

static void
subscribe_storage_events_callback(gchar *storage_id,
    gpointer user_data,
    GError *error);

static void
release_storage_callback(gpointer user_data, GError *error);

static void
release_storage(void);

static void
setup_storage_callback(AXStorage *storage,
    G_GNUC_UNUSED gpointer user_data,
    GError *error)
{
  GError *err = NULL;

  if (error != NULL) {
    g_warning("Failed to setup storage callback: '%s'", error->message);
    g_error_free(error);
    goto error;
  }

  if (storage == NULL || storage_params == NULL) {
    g_warning("Unexpected error during setup");
    goto error;
  }

  storage_params->axstorage = storage;
  storage_params->storage_path = ax_storage_get_path(storage, &err);

error:
  if (err != NULL) {
    g_warning("Failed to get path to storage: '%s'", err->message);
    g_error_free(err);
  }
}

static void
subscribe_storage_events_callback(gchar *storage_id,
    G_GNUC_UNUSED gpointer user_data,
    GError *error)
{
  gboolean available;
  gboolean exiting;
  gboolean full;
  gboolean writable;
  GError *err = NULL;

  if (error != NULL) {
    g_warning("Failed to subsribe to events: %s", error->message);
    g_error_free(error);
    goto error;
  }

  exiting = ax_storage_get_status(storage_id, AX_STORAGE_EXITING_EVENT, &err);
  if (err != NULL) {
    goto error;
  }
  writable = ax_storage_get_status(storage_id, AX_STORAGE_WRITABLE_EVENT, &err);
  if (err != NULL) {
    goto error;
  }
  full = ax_storage_get_status(storage_id, AX_STORAGE_FULL_EVENT, &err);
  if (err != NULL) {
    goto error;
  }

  available = ax_storage_get_status(storage_id, AX_STORAGE_AVAILABLE_EVENT,
    &err);
  if (err != NULL) {
    goto error;
  }

  if (storage_params != NULL) {
    storage_params->exiting = exiting;
    storage_params->writable = writable;
    storage_params->full = full;
    storage_params->available = available;
  }

  if (exiting){
    release_storage();
  } else if (writable && !full && !exiting) {
    gint counter = 3;
    while (counter > 0) {
      if (ax_storage_setup_async(storage_id, setup_storage_callback, NULL, &err)) {
        break;
      }

      if (err != NULL) {
        if (counter == 1) {
          g_warning("Failed to setup storage: '%s'", err->message);
        }
        g_error_free(err);
        err = NULL;
      }

      counter--;
    }
  }

error:
  if (err != NULL) {
    g_warning("Failed to get status for event: '%s'", err->message);
    g_error_free(err);
  }
}

gboolean
storage_setup(GError **error)
{
  GList *storage_list = NULL;
  GList *node = NULL;
  gboolean result = FALSE;
  gchar *storage_id;
  GError *err= NULL;

  storage_list = ax_storage_list(&err);
  if (storage_list  == NULL) {
    goto error;
  }

  storage_params = g_slice_new0(StorageParams);

  /* For this application, we don't care which type of disk we use so we just
     pick the first in list */
  storage_id = g_list_first(storage_list)->data;
  if ((storage_params->subscription_id = ax_storage_subscribe(storage_id,
      subscribe_storage_events_callback, NULL, &err)) == 0) {
    goto error;
  }

  for (node = g_list_first(storage_list); node != NULL;
     node = g_list_next(node)) {
    g_free(node->data);
  }
  g_list_free(storage_list);

  result = TRUE;
error:
  if (err != NULL) {
    if (storage_params != NULL) {
      g_slice_free(StorageParams, storage_params);
      storage_params = NULL;
    }
    g_set_error(error, err->domain, err->code,
      "Failed to setup storage: '%s'", err->message);
    g_error_free(err);
  }

  return result;
}

static void
release_storage_callback(G_GNUC_UNUSED gpointer user_data, GError *error)
{
  if (error) {
    g_warning("Something went wrong in release storage callback: '%s'",
      error->message);
    g_error_free(error);
  }
}

static void
release_storage(void)
{
  GError *error = NULL;

  if (storage_params == NULL) {
    goto error;
  }

  ax_storage_release_async(storage_params->axstorage,
      release_storage_callback, NULL, &error);

error:
  if (error != NULL) {
    g_warning("Failed to release storage: '%s'", error->message);
    g_error_free(error);
  }
}

void
storage_release(void)
{
  if (storage_params == NULL) {
    goto error;
  }

  release_storage();

  ax_storage_unsubscribe(storage_params->subscription_id, NULL);

  if (storage_params->storage_path != NULL) {
    g_free(storage_params->storage_path);
  }
  g_slice_free(StorageParams, storage_params);
  storage_params = NULL;

error:
  return;
}

void
storage_write(const gchar* message)
{
  FILE *file;
  gchar *filename = NULL;
  gchar *time_str = NULL;
  GTimeVal time_val;

  if (storage_params == NULL || storage_params->storage_path == NULL ||
      storage_params->full == TRUE || storage_params->exiting == TRUE ||
      storage_params->writable == FALSE || storage_params->available == FALSE){
    g_warning("Could not write to disk");
    goto error;
  }

  g_get_current_time(&time_val);
  time_str = g_time_val_to_iso8601(&time_val);
  filename = g_strdup_printf("%s/%s", storage_params->storage_path, FILENAME);

  if ((file = g_fopen(filename, "a")) == NULL) {
    goto error;
  }

  g_fprintf(file, "[  %s  ]: %s\n", time_str, message);
  fclose(file);

error:
  if (time_str != NULL) {
    g_free(time_str);
  }

  if (filename != NULL) {
    g_free(filename);
  }
}
