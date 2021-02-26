/**
 * This application is a simple motion detection example. It enables a client
 * to add and remove motion detection windows by means of a CGI called with
 * the following URL: http://<camera>/local/analyze/motion.cgi?<parameters>
 *
 * motion.cgi accepts parameters on the following form
 *
 * action     = "action", eq, action-id ;
 * action-id  = ( add-window |
 *                del-window |
 *                get-window |
 *                set-level );
 * add-window = "add-window", sep,
 *              x, sep,
 *              y, sep,
 *              width, sep,
 *              height, sep,
 *              level ;
 * del-window = "del-window", sep,
 *              window ;
 * get-window = "get-window", sep,
 *              window ;
 * set-level  = "set-level", sep,
 *              window, sep,
 *              level ;
 * window     = "window", eq, { digit } | all ;
 * level      = "level", eq, { digit } ;
 * all        = "all" ;
 * x          = "x", eq, { digit } ;
 * y          = "y", eq, { digit } ;
 * width      = "width", eq, nzdigit, { digit } ;
 * height     = "height", eq, nzdigit, { digit } ;
 * level      = "level", eq, nzdigit, { digit } ;
 * nzdigit    = "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9" ;
 * digit      = "0" | nzdigit ;
 * sep        = "&" ;
 * eq         = "=" ;
 *
 * Add a motion window
 * -------------------
 * A motion window is described by a coordinate representing the top left
 * corner of the window and the width / height representing the dimensions
 * of the window. Coordinates and width / height are expressed as pixel
 * coordinates and pixel width / height.
 *
 * Each window is associated with an event which is dynamically created or
 * destroyed when the window is added or removed. The event will be dispatched
 * when the motion detection intensity in a window rises above or falls below
 * the window's motion level.
 *
 * Example:
 * Add a new 200 pixel wide and 300 pixels high motion window. Position the
 * window's top left corner at the pixel coordinate 100, 200 and set the motion
 * detection level to 10.
 *
 * Request:
 * http://<camera>/local/analyze/motion.cgi?action=add-window&x=100&y=200&width=200&height=300&level=10
 *
 * Response:
 * <Window>
 *   <id>1</id>
 *   <x>100</x>
 *   <y>200</y>
 *   <Width>200</Width>
 *   <Height>300</Height>
 *   <Level>10</Level>
 *   <Motion>false</Motion>
 * </Window>
 *
 * Get window settings
 * -------------------
 *
 * Example:
 * Get the settings of a particual window
 *
 * Request:
 * http://<camera>/local/analyze/motion.cgi?action=get-window&window=1
 *
 * Response:
 * <Window>
 *   <id>1</id>
 *   <x>100</x>
 *   <y>200</y>
 *   <Width>200</Width>
 *   <Height>300</Height>
 *   <Level>10</Level>
 *   <Motion>false</Motion>
 * </Window>
 *
 * Example:
 * Gets the settings of all windows
 *
 * Request:
 * http://<camera>/local/analyze/motion.cgi?action=get-window&window=all
 *
 * Response:
 * <Windows>
 *   <Window>
 *     <id>2</id>
 *     <x>300</x>
 *     <y>300</y>
 *     <Width>400</Width>
 *     <Height>600</Height>
 *     <Level>10</Level>
 *     <Motion>false</Motion>
 *   </Window>
 *   <Window>
 *     <id>1</id>
 *     <x>100</x>
 *     <y>200</y>
 *     <Width>200</Width>
 *     <Height>300</Height>
 *     <Level>20</Level>
 *     <Motion>false</Motion>
 *   </Window>
 * </Windows>
 *
 * Example: set the motion detection level
 * ---------------------------------------
 * Sets the motion detection level of a particular window
 *
 * Request:
 * http://<camera>/local/analyze/motion.cgi?action=set-level&window=1&level=123
 *
 * Response:
 * <Window>
 *   <id>1</id>
 *   <x>100</x>
 *   <y>200</y>
 *   <Width>200</Width>
 *   <Height>300</Height>
 *   <Level>123</Level>
 *   <Motion>false</Motion>
 * </Window>
 *
 * Example: set the same motion detection level on all windows
 * -----------------------------------------------------------
 *
 * Request:
 * http://<camera>/local/analyze/motion.cgi?action=set-level&window=all&level=123
 *
 * Response:
 * <Windows>
 *   <Window>
 *     <id>2</id>
 *     <x>300</x>
 *     <y>300</y>
 *     <Width>400</Width>
 *     <Height>600</Height>
 *     <Level>123</Level>
 *     <Motion>false</Motion>
 *   </Window>
 *   <Window>
 *     <id>1</id>
 *     <x>100</x>
 *     <y>200</y>
 *     <Width>200</Width>
 *     <Height>300</Height>
 *     <Level>123</Level>
 *     <Motion>false</Motion>
 *   </Window>
 * </Windows>
 *
 * Example: remove a window
 * ------------------------
 *
 * Request:
 * http://<camera>/local/analyze/motion.cgi?action=del-window&window=0
 *
 * Response:
 * (empty)
 *
 * Example: remove all windows
 * ---------------------------
 *
 * Request:
 * http://<camera>/local/analyze/motion.cgi?action=del-window&window=all
 *
 * Response:
 * (empty)
 *
 */
#include <signal.h>
#include <string.h>
#include <capture.h>
#include <axsdk/axhttp.h>
#include <axsdk/axevent.h>
#include <axsdk/axparameter.h>
#include "analyze.h"
#include "motion_detection.h"
#include "analyze_util.h"
#include "analyze_error.h"
#include "storage.h"

#define APPNAME "Analyze"

#define DEFAULT_HTTP_HEADER \
  "Content-type: text/xml\r\n" \
  "Status: 200 OK\r\n\r\n"

#define XML_ERROR_RESP \
    "<AnalyzeError>"   \
      "<Code>"         \
        "%d"           \
      "</Code>"        \
      "<Message>"      \
        "%s"           \
      "</Message>"     \
    "</AnalyzeError>"  \

#define XML_WINDOW_RESP   \
  "<Window>"              \
    "<id>%d</id>"         \
    "<x>%d</x>"           \
    "<y>%d</y>"           \
    "<Width>%d</Width>"   \
    "<Height>%d</Height>" \
    "<Level>%d</Level>"   \
    "<Motion>%s</Motion>" \
  "</Window>"

#define RESOLUTION_PARAM "Image.I0.Appearance.Resolution"
#define CROP_MODIFIER
#define PARAM_ACTION    "action"
#define REQ_ADD_WINDOW  "add-window"
#define REQ_DEL_WINDOW  "del-window"
#define REQ_SET_LEVEL   "set-level"
#define REQ_GET_WINDOW  "get-window"
#define ARG_X           "x"
#define ARG_Y           "y"
#define ARG_WIDTH       "width"
#define ARG_HEIGHT      "height"
#define ARG_LEVEL       "level"
#define ARG_WINDOW      "window"
#define ARG_ALL         "all"

#define CAPTURE_PROP_FMT "resolution=%dx%d&sdk_format=Y800&fps=%d"
#define CAPTURE_FPS     10
/* Depending on the aspect ratio of the sensor you may want to change these
 * values if you're going to perform motion detection on the entire sensor.
 */
#define CAPTURE_WIDTH   320
#define CAPTURE_HEIGHT  240

#define ITERATION_PERIOD (1000 / CAPTURE_FPS)


/*
 * action=add-window&x=0&y=0&width=100&height=100&level=10
 *
 * action=del-window&window=1
 *
 * action=set-level&window=1&level=10
 *
 * action=get-window&window=1
 */

typedef struct _ApplicationState ApplicationState;
static GMainLoop *loop;

struct _ApplicationState {
  MotionDetection *detection;
  guint timer_source_id;
};

static void
http_request_handler(const gchar *path,
    const gchar *method,
    const gchar *query,
    GHashTable *params,
    GOutputStream *output_stream,
    gpointer user_data);

static gboolean
http_request_handle_set_level(GHashTable *params,
    GDataOutputStream *dos,
    ApplicationState *state,
    GError **error);

static gboolean
http_request_handle_add_window(GHashTable *params,
    GDataOutputStream *dos,
    ApplicationState *state,
    GError **error);

static gboolean
http_request_handle_del_window(GHashTable *params,
    GDataOutputStream *dos,
    ApplicationState *state,
    GError **error);

static gboolean
http_request_handler_get_window(GHashTable *params,
    GDataOutputStream *dos,
    ApplicationState *state,
    GError **error);

static void
http_request_handle_error(GDataOutputStream *dos,
    ApplicationState *state, GError *error);

static void
http_request_output_window(ApplicationState *state,
    guint window_id,
    GDataOutputStream *dos);

static void
http_request_output_all_windows(ApplicationState *state,
    GDataOutputStream *dos);

static void
http_request_output(GDataOutputStream *dos,
    const gchar *fmt, ...);

static gboolean
application_iterate(ApplicationState *state);

static gchar*
util_get_maxres(void);

static media_stream*
media_stream_new(guint width,
    guint height,
    guint fps);

static void
media_stream_free(media_stream *stream);

static void
application_state_start_iteration(ApplicationState *state);

static void
application_state_stop_iteration(ApplicationState *state);

static ApplicationState*
application_state_new(MotionDetection *detection);

static void
application_state_free(ApplicationState *state);

static gchar*
get_resolution(guint *width,
    guint *height,
    GError **error);

static void
handle_sigterm(G_GNUC_UNUSED int signo);

static void
init_signals(void);

static void
http_request_handler(G_GNUC_UNUSED const gchar *path,
    G_GNUC_UNUSED const gchar *method, /* We don't care about the method */
    G_GNUC_UNUSED const gchar *query,  /* We don't care about the raw query */
    GHashTable *params,
    GOutputStream *output_stream,
    gpointer user_data)
{
  ApplicationState *state = user_data;
  GDataOutputStream *dos;
  gchar *action;
  GError *error = NULL;
  gboolean result = FALSE;

  /* Create a GDataOutputStream from the GOutputStream. Writing to a
   * GDataOutputStream is easier if we're writing strings.
   */

  dos = g_data_output_stream_new(output_stream);

  if (params == NULL) {
    error = g_error_new(ANALYZE_ERROR, ANALYZE_ERROR_ILLEGAL_REQUEST,
        "Empty request");
    goto error;
  }

  action = g_hash_table_lookup(params, PARAM_ACTION);

  /* Try to figure out what action the client wants to perform. */
  if (g_strcmp0(action, REQ_SET_LEVEL) == 0) {
    /* The client wants modify the motion detection level of a window. */
    if (!http_request_handle_set_level(params, dos, state, &error)) {
      goto error;
    }

  } else if (g_strcmp0(action, REQ_ADD_WINDOW) == 0) {
    /* The client wants to create a new motion detection window. */
    if (!http_request_handle_add_window(params, dos, state, &error)) {
      goto error;
    }

  } else if (g_strcmp0(action, REQ_DEL_WINDOW) == 0) {
    /* The client wants to delete a motion detection window. */
    if (!http_request_handle_del_window(params, dos, state, &error)) {
      goto error;
    }

  } else if (g_strcmp0(action, REQ_GET_WINDOW) == 0) {
    /* The client wants to get information about a window */
    if (!http_request_handler_get_window(params, dos, state, &error)) {
      goto error;
    }
  } else {
    g_set_error(&error, ANALYZE_ERROR, ANALYZE_ERROR_ILLEGAL_ARGUMENT,
        "Illegal request: %s", query ? query:"(null)");
    goto error;
  }

  result = TRUE;
error:
  if (!result && error) {
    http_request_handle_error(dos, state, error);
    g_error_free(error);
  }

  /* The output stream must be unreferenced. This will close the connection
   * to the client and free the resources maintained by the output stream.
   */
  g_object_unref(dos);
}

static gboolean
http_request_handle_set_level(GHashTable *params,
    G_GNUC_UNUSED GDataOutputStream *dos,
    ApplicationState *state,
    GError **error)
{
  gchar *window_id_str;
  gchar *level_str;
  guint window_id;
  guint level;
  gboolean result = FALSE;

  if ((level_str = g_hash_table_lookup(params, ARG_LEVEL)) == NULL) {
    g_set_error(error, ANALYZE_ERROR, ANALYZE_ERROR_MISSING_ARGUMENT,
        "Missing argument: %s", ARG_LEVEL);

    goto error;
  } else if (!util_string_to_guint(level_str, &level)) {
    g_set_error(error, ANALYZE_ERROR, ANALYZE_ERROR_ILLEGAL_ARGUMENT,
        "Illegal argument: %s", level_str);
    goto error;
  }

  if ((window_id_str = g_hash_table_lookup(params, ARG_WINDOW)) == NULL) {
    g_set_error(error, ANALYZE_ERROR, ANALYZE_ERROR_MISSING_ARGUMENT,
        "Missing argument: %s", ARG_WINDOW);
    goto error;

  } else if (g_strcmp0(window_id_str, "all") == 0) {
    if (!motion_detection_set_all_levels(state->detection, level, error)) {
      goto error;
    }
    http_request_output(dos, DEFAULT_HTTP_HEADER);
    http_request_output_all_windows(state, dos);
  } else {

    if (!util_string_to_guint(window_id_str, &window_id)) {
      g_set_error(error, ANALYZE_ERROR, ANALYZE_ERROR_ILLEGAL_ARGUMENT,
          "Illegal argument: %s", window_id_str);
      goto error;
    }

    if (!motion_detection_set_level(state->detection, window_id, level,
          error)) {
      goto error;
    }
    http_request_output(dos, DEFAULT_HTTP_HEADER);
    http_request_output_window(state, window_id, dos);
  }

  result = TRUE;
error:
  return result;
}

static gboolean
http_request_handle_add_window(GHashTable *params,
    GDataOutputStream *dos,
    ApplicationState *state,
    GError **error)
{
  gchar *x_str;
  gchar *y_str;
  gchar *width_str;
  gchar *height_str;
  gchar *level_str;
  guint x;
  guint y;
  guint width;
  guint height;
  guint level;
  guint window_id;
  gboolean result = FALSE;

  struct arg_list {
    const gchar *arg;
    gchar **str_val;
    guint *int_val;
  } args[] = {
    { ARG_X,      &x_str,      &x },
    { ARG_Y,      &y_str,      &y },
    { ARG_WIDTH,  &width_str,  &width },
    { ARG_HEIGHT, &height_str, &height },
    { ARG_LEVEL,  &level_str,  &level }
  };

  struct arg_list *pargs = args;

  do {
    if ((*pargs->str_val = g_hash_table_lookup(params, pargs->arg)) == NULL) {
      g_set_error(error, ANALYZE_ERROR, ANALYZE_ERROR_MISSING_ARGUMENT,
          "Missing argument: %s", pargs->arg);
      goto error;
    }

    if (!util_string_to_guint(*pargs->str_val, pargs->int_val)) {
      g_set_error(error, ANALYZE_ERROR, ANALYZE_ERROR_ILLEGAL_ARGUMENT,
          "%s cannot be converted to an unsigned integer", *pargs->str_val);
      goto error;
    }

  } while (++pargs != &args[G_N_ELEMENTS(args)]);

  if (!motion_detection_add_window(state->detection, x, y, width, height,
        level, &window_id, error)) {
    goto error;
  }

  http_request_output(dos, DEFAULT_HTTP_HEADER);

  http_request_output_window(state, window_id, dos);

  if (motion_detection_get_nbr_windows(state->detection) > 0) {
    application_state_start_iteration(state);
    motion_detection_reset(state->detection);
  }
  result = TRUE;

error:

  return result;
}

static gboolean
http_request_handle_del_window(GHashTable *params,
    G_GNUC_UNUSED GDataOutputStream *dos,
    ApplicationState *state,
    GError **error)
{
  gchar *window_id_str;
  guint window_id;
  gboolean result = FALSE;

  if ((window_id_str = g_hash_table_lookup(params, ARG_WINDOW)) == NULL) {

    g_error_new(ANALYZE_ERROR, ANALYZE_ERROR_MISSING_ARGUMENT,
        "Missing argument: %s", ARG_WINDOW);

    goto error;

  } else if (g_strcmp0(window_id_str, ARG_ALL) == 0) {
    motion_detection_remove_all_windows(state->detection);
  } else {

    if (!util_string_to_guint(window_id_str, &window_id)) {
      g_set_error(error, ANALYZE_ERROR, ANALYZE_ERROR_ILLEGAL_ARGUMENT,
          "Illegal argument: %s", window_id_str);
      goto error;
    }

    if (!motion_detection_remove_window(state->detection, window_id, error)) {
      goto error;
    }
  }

  if (motion_detection_get_nbr_windows(state->detection) == 0) {
    application_state_stop_iteration(state);
  }

  result = TRUE;
error:
  return result;
}

static gboolean
http_request_handler_get_window(GHashTable *params,
    GDataOutputStream *dos,
    ApplicationState *state,
    GError **error)
{
  gchar *window_id_str;
  guint window_id;
  gboolean result = FALSE;

  if ((window_id_str = g_hash_table_lookup(params, ARG_WINDOW)) == NULL) {
    /* The window argument is missing */
    g_error_new(ANALYZE_ERROR, ANALYZE_ERROR_MISSING_ARGUMENT,
        "Missing argument: %s", ARG_WINDOW);
    goto error;
  } else if (g_strcmp0(window_id_str, ARG_ALL) == 0) {

    http_request_output(dos, DEFAULT_HTTP_HEADER);
    http_request_output_all_windows(state, dos);
  } else {
    if (!util_string_to_guint(window_id_str, &window_id)) {
      g_set_error(error, ANALYZE_ERROR, ANALYZE_ERROR_ILLEGAL_ARGUMENT,
          "%s cannot be converted into an unsigned integer", window_id_str);
      goto error;
    }
    http_request_output(dos, DEFAULT_HTTP_HEADER);
    http_request_output_window(state, window_id, dos);
  }

  result = TRUE;

error:
  return result;
}

static void
http_request_handle_error(G_GNUC_UNUSED GDataOutputStream *dos,
    G_GNUC_UNUSED ApplicationState *state, GError *error)
{
  http_request_output(dos, DEFAULT_HTTP_HEADER XML_ERROR_RESP,
      error->code, error->message);
}

static void
http_request_output_all_windows(ApplicationState *state,
    GDataOutputStream *dos)
{
  GList *windows;
  GList *iter;

  windows = motion_detection_get_all_windows(state->detection);
  iter = windows;
  http_request_output(dos, "<Windows>");

  while (iter != NULL) {
    guint window_id = GPOINTER_TO_UINT(iter->data);
    http_request_output_window(state, window_id, dos);
    iter = g_list_next(iter);
  }

  g_list_free(windows);
  http_request_output(dos, "</Windows>");
}

static void
http_request_output_window(ApplicationState *state,
    guint window_id,
    GDataOutputStream *dos)
{
  guint x;
  guint y;
  guint width;
  guint height;
  guint level;
  gboolean motion;
  GError *error = NULL;

  if (!motion_detection_get_window_settings(state->detection, window_id,
        &x, &y, &width, &height, &level, &motion, &error)) {
    goto error;
  }

  http_request_output(dos, XML_WINDOW_RESP,
      window_id, x, y, width, height, level, motion ? "true" : "false");

error:

  if (error != NULL) {
    g_warning("%s", error->message);
    g_error_free(error);
  }
}

static void
http_request_output(GDataOutputStream *dos,
    const gchar *fmt, ...)
{
  va_list ap;
  gchar *str;
  GError *error = NULL;

  va_start(ap, fmt);
  str = g_strdup_vprintf(fmt, ap);
  va_end(ap);
  g_data_output_stream_put_string(dos, str, NULL, &error);

  if (error != NULL) {
    g_warning("Failed to respond to client: %s", error->message);
    g_error_free(error);
  }

  g_free(str);
}

static gboolean
application_iterate(ApplicationState *state)
{
  GError *error = NULL;

  if (!motion_detection_iterate(state->detection, &error)) {
    g_warning("Motion detection analysis failed: %s", error->message);
    g_error_free(error);
  }

  /* The timer will continue to call this function every second as long as it
   * returns TRUE.
   */
  return TRUE;
}

static gchar*
util_get_maxres(void)
{
  int len = 0;
  gchar *tmp = NULL;
  gchar *res = NULL;
  char *buf = capture_get_optimal_resolutions_list(1);

  tmp = g_strstr_len(buf, strlen(buf), ",");
  len = strlen(buf);
  if (tmp) {
    len -= strlen(tmp);
  }
  res = g_strndup(buf, len);

  if(buf) {
    free(buf);
  }

  return res;
}


static media_stream*
media_stream_new(guint width,
    guint height,
    guint fps)
{
  media_stream *stream;
  gchar *capture_properties;

  capture_properties = g_strdup_printf(CAPTURE_PROP_FMT, width, height, fps);

  if ((stream = capture_open_stream(IMAGE_UNCOMPRESSED,
          capture_properties)) == NULL) {
    g_warning("Failed to initialize the capture interface:\n %s(\"%s\", \"%s\") == NULL",
		    "capture_open_stream",IMAGE_UNCOMPRESSED, capture_properties );
    goto error;
  }

error:
  g_free(capture_properties);

  return stream;
}

static void
media_stream_free(media_stream *stream)
{
  if (stream != NULL) {
    capture_close_stream(stream);
  }
}

static void
application_state_start_iteration(ApplicationState *state)
{
  /* Setup a timer to extract one image every second */
  if (state->timer_source_id == 0) {
    state->timer_source_id = g_timeout_add(ITERATION_PERIOD,
        (GSourceFunc)application_iterate, state);
  }
}

static void
application_state_stop_iteration(ApplicationState *state)
{
  if (state->timer_source_id != 0) {
    GSource *source;
    source = g_main_context_find_source_by_id(NULL, state->timer_source_id);
    if (source != NULL) {
      g_source_destroy(source);
    }
    state->timer_source_id = 0;
  }
}

static ApplicationState*
application_state_new(MotionDetection *detection)
{
  ApplicationState *state;
  state = g_slice_new(ApplicationState);
  state->detection = detection;
  state->timer_source_id = 0;

  return state;
}

static void
application_state_free(ApplicationState *state)
{
  if (state != NULL) {
    g_slice_free(ApplicationState, state);
  }
}

static gchar*
get_resolution(guint *width,
    guint *height,
    GError **error)
{
  gchar *resolution_str = NULL;
  gchar **resolution_parts = NULL;

  resolution_str = util_get_maxres();

  if (!resolution_str){
    g_set_error(error, ANALYZE_ERROR,
	  ANALYZE_ERROR_GET_RESOLUTION,	"Failed to get resolution");
    goto error;
  }

  if(g_strrstr(resolution_str, "x")) {
    resolution_parts = g_strsplit(resolution_str, "x", 0);

    if (!util_string_to_guint(resolution_parts[0], width) ||
        !util_string_to_guint(resolution_parts[1], height)) {
      g_set_error(error, ANALYZE_ERROR, ANALYZE_ERROR_CONVERT,
          "Failed to parse resolution: %s", resolution_str);
      goto error;
    }
  } else { /* fallback*/
    *width = CAPTURE_WIDTH;
	*height = CAPTURE_HEIGHT;
  }

error:
  if (resolution_parts != NULL) {
    g_strfreev(resolution_parts);
  }

  return resolution_str;
}

static void
handle_sigterm(G_GNUC_UNUSED int signo)
{
  if (loop != NULL) {
    g_main_loop_quit(loop);
  } else {
    exit(EXIT_FAILURE);
  }
}

static void
init_signals(void)
{
  struct sigaction sa_term;
  struct sigaction sa_ign;

  sa_term.sa_flags = 0;
  sigemptyset(&sa_term.sa_mask);
  sa_term.sa_handler = handle_sigterm;
  sigaction(SIGTERM, &sa_term, NULL);
  sigaction(SIGINT, &sa_term, NULL);

  sa_ign.sa_flags = 0;
  sigemptyset (&sa_ign.sa_mask);
  sa_ign.sa_handler = SIG_IGN;
  sigaction(SIGPIPE, &sa_ign, NULL);
}

int
main(void)
{
  ApplicationState *state = NULL;
  media_stream *stream = NULL;;
  MotionDetection *detection = NULL;
  AXHttpHandler *http_handler = NULL;
  AXEventHandler *event_handler = NULL;
  guint source_width;
  guint source_height;
  gchar *res_str = NULL;
  GError *error = NULL;

  init_signals();

  g_message("%s Started..", APPNAME);

  if (!(res_str = get_resolution(&source_width, &source_height, &error))) {
    goto error;
  }

  g_message("Starting video stream.. [%dx%d] %d fps ",CAPTURE_WIDTH, CAPTURE_HEIGHT, CAPTURE_FPS );
  stream = media_stream_new(CAPTURE_WIDTH, CAPTURE_HEIGHT, CAPTURE_FPS);
  g_message("Creating event handler..");
  event_handler = ax_event_handler_new();
  g_message("Creating motion detector source:%dx%d cropped:%dx%d",
      source_width, source_height, CAPTURE_WIDTH, CAPTURE_HEIGHT);
  detection = motion_detection_new(stream, event_handler,
      source_width, source_height, CAPTURE_WIDTH, CAPTURE_HEIGHT);

  state = application_state_new(detection);
  http_handler = ax_http_handler_new(http_request_handler, state);

  if (!storage_setup(&error)) {
    goto out;
  }

  loop = g_main_loop_new(NULL, FALSE);
  g_main_loop_run(loop);

  storage_release();

out:
  ax_http_handler_free(http_handler);
  motion_detection_free(detection);
  ax_event_handler_free(event_handler);
  media_stream_free(stream);
  application_state_free(state);
  if (res_str != NULL) {
    g_free(res_str);
  }

error:
  if (error != NULL) {
    g_warning("Failed to initialize: %s", error->message);
    g_error_free(error);
  }

  return error == NULL ? 0 : 1;
}
