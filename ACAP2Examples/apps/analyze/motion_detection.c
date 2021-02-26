#include "motion_detection.h"
#include "analyze_error.h"
#include "storage.h"

#define EVENT_TEMPLATE_NAME "com.example.motion"
#define EVENT_WINDOW_KEY    "window"
#define EVENT_MOTION_KEY    "motion"

struct _MotionWindow {
  guint x;
  guint y;
  guint width;
  guint height;
  guint crop_x;
  guint crop_y;
  guint crop_width;
  guint crop_height;
  guint level;
  gboolean motion;

  guint declaration_id;
};

struct _MotionDetection {
  media_stream *stream;
  AXEventHandler *event_handler;
  media_frame *prev_frame;
  guint source_width;
  guint source_height;
  guint crop_width;
  guint crop_height;
  gchar *delta_matrix;

  GHashTable *windows;
  guint last_window;
};

static MotionWindow*
motion_window_new(guint x,
    guint y,
    guint width,
    guint height,
    guint crop_x,
    guint crop_y,
    guint crop_width,
    guint crop_height,
    guint level);

static void
motion_window_free(MotionWindow *window);

static void
motion_window_update_motion(MotionWindow *window,
    guint window_id,
    AXEventHandler *event_handler,
    gboolean motion);

static void
motion_detection_analyze_frame(MotionDetection *detection,
    media_frame *curr_frame);

static void
motion_detection_analyze_windows(MotionDetection *detection);

MotionDetection*
motion_detection_new(media_stream *stream,
    AXEventHandler *event_handler,
    guint source_width,
    guint source_height,
    guint crop_width,
    guint crop_height)
{
  MotionDetection *detection;

  detection = g_slice_new0(MotionDetection);

  detection->stream        = stream;
  detection->event_handler = event_handler;

  detection->source_width  = source_width;
  detection->source_height = source_height;
  detection->crop_width    = crop_width;
  detection->crop_height   = crop_height;

  detection->delta_matrix = g_malloc(sizeof(gchar) * crop_width *
      crop_height);

  detection->windows = g_hash_table_new_full(g_direct_hash, g_direct_equal,
      NULL, (GDestroyNotify)motion_window_free);

  detection->last_window = 0;

  return detection;
}

void
motion_detection_free(MotionDetection *detection)
{
  if (detection != NULL) {
    g_free(detection->delta_matrix);

    if (detection->prev_frame) {
      capture_frame_free(detection->prev_frame);
    }

    g_hash_table_unref(detection->windows);

    g_slice_free(MotionDetection, detection);
  }
}

gboolean
motion_detection_add_window(MotionDetection *detection,
    guint x,
    guint y,
    guint width,
    guint height,
    guint level,
    guint *window_id,
    GError **error)
{
  MotionWindow *window = NULL;
  AXEventKeyValueSet *set = NULL;
  gchar *message = NULL;
  guint crop_x;
  guint crop_y;
  guint crop_width;
  guint crop_height;
  gdouble horiz_modifier;
  gdouble vert_modifier;
  guint declaration_id;
  gboolean result = FALSE;

  if (width == 0 || height == 0) {
    g_set_error(error, ANALYZE_ERROR, ANALYZE_ERROR_DIMENSIONS,
        "The window has no area");

    goto error;
  }
  if (x + width > detection->source_width ||
      y + height > detection->source_height) {

    g_set_error(error, ANALYZE_ERROR, ANALYZE_ERROR_DIMENSIONS,
        "Window does not fit within the image");

    goto error;
  }

  horiz_modifier = (((gdouble)detection->crop_width) /
      detection->source_width);

  vert_modifier = (((gdouble)detection->crop_height) /
      detection->source_height);

  crop_x = (guint)(x * horiz_modifier);

  crop_y = (guint)(y * vert_modifier);

  crop_width = (guint)(width * horiz_modifier);

  /* If the width is zero after cropping, change it to one */
  if (crop_width == 0) {
    crop_width = 1;
  }
  crop_height = (guint)(height * vert_modifier);
  /* If the height is zero after cropping, change it to one */
  if (crop_height == 0) {
    crop_height = 1;
  }

  window = motion_window_new(x, y, width, height,
      crop_x, crop_y, crop_width, crop_height, level);

  *window_id = detection->last_window++;

  set = ax_event_key_value_set_new();

  if (!ax_event_key_value_set_add_key_values(set, error,
        EVENT_WINDOW_KEY, NULL, window_id, AX_VALUE_TYPE_INT,
        EVENT_MOTION_KEY, NULL, &window->motion, AX_VALUE_TYPE_BOOL,
        NULL)) {
    goto error;
  }

  if (!ax_event_handler_declare_from_template(detection->event_handler,
        EVENT_TEMPLATE_NAME, set, &declaration_id, NULL, NULL, error)) {
    goto error;
  }

  window->declaration_id = declaration_id;

  g_hash_table_insert(detection->windows, GUINT_TO_POINTER(*window_id),
      window);

  message = g_strdup_printf("Motion detection window '%d' added", *window_id);
  storage_write(message);

  result = TRUE;

error:

  if (result == FALSE) {
    if (window != NULL) {
      motion_window_free(window);
    }
  }

  if (set != NULL) {
    ax_event_key_value_set_free(set);
  }

  if (message != NULL) {
    g_free(message);
  }

  return result;
}

gboolean
motion_detection_remove_window(MotionDetection *detection,
    guint window_id,
    GError **error)
{
  MotionWindow *window;
  gboolean result = FALSE;
  gchar* message = NULL;

  if ((window = g_hash_table_lookup(detection->windows,
          GUINT_TO_POINTER(window_id))) == NULL) {

    g_set_error(error, ANALYZE_ERROR, ANALYZE_ERROR_NOT_FOUND,
        "Window %d not found", window_id);

    goto error;
  }

  result = ax_event_handler_undeclare(detection->event_handler,
        window->declaration_id, error);

  message = g_strdup_printf("Motion detection window '%d' removed",
      window_id);
  storage_write(message);

  g_hash_table_remove(detection->windows,
      GUINT_TO_POINTER(window_id));

error:
  if (message != NULL) {
    g_free(message);
  }

  return result;
}

gboolean
motion_detection_get_window(MotionDetection *detection,
    guint window_id,
    const MotionWindow **window,
    GError **error)
{
  gboolean result = FALSE;
  gchar *message = NULL;

  if ((*window = g_hash_table_lookup(detection->windows,
          GUINT_TO_POINTER(window_id))) == NULL) {

    g_set_error(error, ANALYZE_ERROR, ANALYZE_ERROR_NOT_FOUND,
        "Window %d not found", window_id);

    goto error;
  }

  message = g_strdup_printf("Get motion detection window '%d'", window_id);
  storage_write(message);

  result = TRUE;
error:
  if (message != NULL) {
    g_free(message);
  }

  return result;
}

GList*
motion_detection_get_all_windows(MotionDetection *detection)
{
  gchar *message;

  message = g_strdup_printf("Get all motion detection windows");
  storage_write(message);

  g_free(message);

  return g_hash_table_get_keys(detection->windows);
}

void
motion_detection_remove_all_windows(MotionDetection *detection)
{
  MotionWindow *window;
  GHashTableIter iter;
  gchar *message;

  g_hash_table_iter_init(&iter, detection->windows);

  while (g_hash_table_iter_next(&iter, NULL, (gpointer*)&window)) {
    g_hash_table_iter_remove(&iter);
  }

  message = g_strdup_printf("Removed all motion detection windows");
  storage_write(message);

  g_free(message);

  detection->last_window = 0;
}

gboolean
motion_detection_set_level(MotionDetection *detection,
    guint window_id,
    guint level,
    GError **error)
{
  MotionWindow *window;
  gboolean result = FALSE;
  gchar *message = NULL;

  if ((window = g_hash_table_lookup(detection->windows,
          GUINT_TO_POINTER(window_id))) == NULL) {
    g_set_error(error, ANALYZE_ERROR, ANALYZE_ERROR_NOT_FOUND,
        "Window %d not found", window_id);

    goto error;
  }

  window->level = level;
  message = g_strdup_printf("Set level '%d' to motion detection window '%d'",
      level, window_id);
  storage_write(message);

  result = TRUE;
error:
  if (message != NULL) {
    g_free(message);
  }

  return result;
}

gboolean
motion_detection_set_all_levels(MotionDetection *detection,
    guint level,
    GError **error)
{
  GHashTableIter iter;
  guint window_id;
  gboolean result = FALSE;
  gchar *message = NULL;

  g_hash_table_iter_init(&iter, detection->windows);

  while (g_hash_table_iter_next(&iter, (gpointer*)&window_id, NULL)) {
    if (!motion_detection_set_level(detection, window_id, level, error)) {
      goto error;
    }
  }

  message = g_strdup_printf("Level '%d' has been set to all motion detection "
      "windows", level);
  storage_write(message);

  result = TRUE;
error:
  if (message != NULL) {
    g_free(message);
  }
  return result;
}

gboolean
motion_detection_get_window_settings(MotionDetection *detection,
    guint window_id,
    guint *x,
    guint *y,
    guint *width,
    guint *height,
    guint *level,
    gboolean *motion,
    GError **error)
{
  MotionWindow *window;
  gboolean result = FALSE;
  gchar *message = NULL;

  if ((window = g_hash_table_lookup(detection->windows,
          GUINT_TO_POINTER(window_id))) == NULL) {
    g_set_error(error, ANALYZE_ERROR, ANALYZE_ERROR_NOT_FOUND,
        "Window %d not found", window_id);

    goto error;
  }

  if (x != NULL) {
    *x = window->x;
  }

  if (y != NULL) {
    *y = window->y;
  }

  if (width != NULL) {
    *width = window->width;
  }

  if (height != NULL) {
    *height = window->height;
  }

  if (level != NULL) {
    *level = window->level;
  }

  if (motion != NULL) {
    *motion = window->motion;
  }

  if(window->motion){
    message = g_strdup_printf("Motion detection window settings for window '%d' "
        "x: '%u', y: '%u', width: '%u', height '%u', level '%u', motion '%s'",
        window_id, x ? *x : 0, y ? *y : 0, width ? *width : 0,
        height ? *height : 0, level ? *level : 0,
        motion ? (*motion ? "true" : "false") : "no value");
    storage_write(message);
  }

  result = TRUE;

error:
  if (message != NULL) {
    g_free(message);
  }
  return result;
}

guint
motion_detection_get_nbr_windows(MotionDetection *detection)
{
  return g_hash_table_size(detection->windows);
}

void
motion_detection_reset(MotionDetection *detection)
{
  if (detection->prev_frame != NULL) {
    capture_frame_free(detection->prev_frame);

    detection->prev_frame = NULL;
  }
}

gboolean
motion_detection_iterate(MotionDetection *detection,
    GError **error)
{
  media_frame *curr_frame;
  gboolean result = FALSE;

  if ((curr_frame = capture_get_frame(detection->stream)) == NULL) {
    g_set_error(error, ANALYZE_ERROR, ANALYZE_ERROR_CAPTURE,
        "capture_get_frame() returned NULL");

    goto error;
  }

  if (detection->prev_frame == NULL) {

    /* If this is the first time we're called then there's nothing to compare
     * this frame to. Just save the frame and return.
     */
    detection->prev_frame = curr_frame;

  } else {
    /* Perform motion detection analysis on the entire frame */
    motion_detection_analyze_frame(detection, curr_frame);

    capture_frame_free(detection->prev_frame);
    detection->prev_frame = curr_frame;

    /* Check each individual window for motion */
    motion_detection_analyze_windows(detection);
  }

  result = TRUE;
error:
  return result;
}

static void
motion_detection_analyze_frame(MotionDetection *detection,
    media_frame *curr_frame)
{
  guint y;
  guint prev_stride = capture_frame_stride(detection->prev_frame);
  guint curr_stride = capture_frame_stride(curr_frame);
  gchar *prev_data  = capture_frame_data(detection->prev_frame);
  gchar *curr_data  = capture_frame_data(curr_frame);

  /* Poor man's motion detection. Compare and save the difference in pixel
   * values between the current frame and the previous frame.
   */
  for (y = 0; y < detection->crop_height; y++) {
    guint x;
    guint prev_pos_y = y * prev_stride;
    guint curr_pos_y = y * curr_stride;
    guint delta_h    = y * detection->crop_height;

    for (x = 0; x < detection->crop_width; x++) {
      guint prev_pos = prev_pos_y + x;
      guint curr_pos = curr_pos_y + x;

      detection->delta_matrix[delta_h + x] =
        ABS(curr_data[curr_pos] - prev_data[prev_pos]);
    }
  }
}

static void
motion_detection_analyze_windows(MotionDetection *detection)
{
  guint window_id;
  GHashTableIter iter;
  MotionWindow *window;

  g_hash_table_iter_init(&iter, detection->windows);

  while (g_hash_table_iter_next(&iter, (gpointer*)&window_id,
        (gpointer*)&window)) {
    guint y;
    guint level = 0;
    gboolean motion;

    for (y = window->crop_y; y < window->crop_height; y++) {
      guint x;
      guint delta_y = y * window->crop_height;

      for (x = window->crop_x; x < window->crop_width; x++) {
        level += detection->delta_matrix[delta_y + x];
      }
    }

    level /= window->crop_width * window->crop_height;

    g_message("Window %d: level %d", window_id, level);

    motion = level >= window->level;

    if (motion != window->motion) {
      g_message("Window %d: %d -> %d", window_id, window->motion, motion);
      motion_window_update_motion(window, window_id, detection->event_handler,
           motion);
    }
  }
}

MotionWindow*
motion_window_new(guint x,
    guint y,
    guint width,
    guint height,
    guint crop_x,
    guint crop_y,
    guint crop_width,
    guint crop_height,
    guint level)
{
  MotionWindow *window;

  window = g_slice_new(MotionWindow);

  window->x           = x;
  window->y           = y;
  window->width       = width;
  window->height      = height;
  window->crop_x      = crop_x;
  window->crop_y      = crop_y;
  window->crop_width  = crop_width;
  window->crop_height = crop_height;
  window->level       = level;
  window->motion      = FALSE;

  return window;
}

void
motion_window_free(MotionWindow *window)
{
  if (window != NULL) {
    g_slice_free(MotionWindow, window);
  }
}

static void
motion_window_update_motion(MotionWindow *window,
    guint window_id,
    AXEventHandler *event_handler,
    gboolean motion)
{
  AXEvent *event = NULL;
  AXEventKeyValueSet *set;
  GTimeVal time_stamp;
  GError *error = NULL;
  gchar *message = NULL;

  window->motion = motion;

  set = ax_event_key_value_set_new();

  if (!ax_event_key_value_set_add_key_value(set,
        EVENT_MOTION_KEY, NULL, &motion, AX_VALUE_TYPE_BOOL, &error)) {
    goto error;
  }

  g_get_current_time(&time_stamp);

  event = ax_event_new(set, &time_stamp);

  ax_event_handler_send_event(event_handler, window->declaration_id,
      event, &error);

  message = g_strdup_printf("An event has occured for motion detection window "
      " '%d'", window_id);
  storage_write(message);

error:
  if (error != NULL) {
    g_warning("%s", error->message);
    g_error_free(error);
  }

  if (set != NULL) {
    ax_event_key_value_set_free(set);
  }

  if (event != NULL) {
    ax_event_free(event);
  }

  if (message != NULL) {
    g_free(message);
  }
}

