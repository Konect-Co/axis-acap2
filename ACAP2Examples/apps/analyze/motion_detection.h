#ifndef _MOTION_DETECTION_H_
#define _MOTION_DETECTION_H_

#include <glib.h>
#include <axsdk/axevent.h>
#include <capture.h>

typedef struct _MotionWindow MotionWindow;

typedef struct _MotionDetection MotionDetection;

MotionDetection*
motion_detection_new(media_stream *stream,
    AXEventHandler *event_handler,
    guint source_width,
    guint source_height,
    guint crop_width,
    guint crop_height);

void
motion_detection_free(MotionDetection *detection);

gboolean
motion_detection_add_window(MotionDetection *detection,
    guint x,
    guint y,
    guint width,
    guint height,
    guint level,
    guint *window_id,
    GError **error);

gboolean
motion_detection_remove_window(MotionDetection *detection,
    guint window_id,
    GError **error);

gboolean
motion_detection_get_window(MotionDetection *detection,
    guint window_id,
    const MotionWindow **window,
    GError **error);

GList*
motion_detection_get_all_windows(MotionDetection *detection);

gboolean
motion_detection_set_level(MotionDetection *detection,
    guint window_id,
    guint level,
    GError **error);

gboolean
motion_detection_set_all_levels(MotionDetection *detection,
    guint level,
    GError **error);

gboolean
motion_detection_get_window_settings(MotionDetection *detection,
    guint window_id,
    guint *x,
    guint *y,
    guint *width,
    guint *height,
    guint *level,
    gboolean *motion,
    GError **error);

void
motion_detection_remove_all_windows(MotionDetection *detection);

guint
motion_detection_get_nbr_windows(MotionDetection *detection);

void
motion_detection_reset(MotionDetection *detection);

gboolean
motion_detection_iterate(MotionDetection *detection,
    GError **error);

gboolean
motion_window_detect(MotionWindow *window,
    gpointer data,
    guint width,
    guint height,
    guint stride);

#endif /* _MOTION_DETECTION_H_ */
