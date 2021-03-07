/*
* - Hello Glib -
*
* This application is a basic 'hello world' type of application.
* It will set up a CGI interface on http://<cam-ip>/local/hello_glib/example.cgi
* -The CGI-path is specified by the HTTPCGIPATH variable in package.conf,
* which in this case points to the file "cgi.txt"
*
* The application will also register a timer, which on timeout calls a
* function. In this case 'on_timeout()' ever second. The current timer value
* is printed when example.cgi is accessed.
*
*/

#include <glib.h>
#include <gio/gio.h>
#include <string.h>
#include <axsdk/axhttp.h>
#include <iostream>

void randFunc() { std::cout << "Yooo" << std::endl; }


/*
 * This function will be called when our CGI is accessed.
 */
static void
request_handler(const gchar *path,
    const gchar *method,
    const gchar *query,
    GHashTable *params,
    GOutputStream *output_stream,
    gpointer user_data)
{
  GDataOutputStream *dos;
  guint *timer = (guint*) user_data;
  gchar msg[128];

  dos = g_data_output_stream_new(output_stream);

  /* Send out the HTTP response status code */
  g_data_output_stream_put_string(dos,"Content-Type: text/plain\r\n", NULL, NULL);
  g_data_output_stream_put_string(dos,"Status: 200 OK\r\n\r\n", NULL, NULL);

  /* Our custom message */
  g_snprintf(msg, sizeof(msg),"You have accessed '%s'\n", path ? path:"(NULL)");
  g_data_output_stream_put_string(dos, msg, NULL, NULL);

  g_snprintf(msg, sizeof(msg), "\n%s() [%s:%d] \n\n"
                               "-  Hello World! -\n"
                               "I've been running for %d seconds\n",
                               __FUNCTION__, __FILE__, __LINE__, *timer );
  g_data_output_stream_put_string(dos, msg, NULL, NULL);

  g_object_unref(dos);
}

/*
 * This function will increase our timer and print the current value to stdout.
 */
static gboolean
on_timeout(gpointer data)
{
  guint *timer = (guint*) data;

  (*timer)++; /* increase the timer */

  g_message("%s() [%s:%d] - this app has been running for %d seconds.",
            __FUNCTION__, __FILE__, __LINE__, *timer);

  return TRUE; /* FALSE removes the event source */
}

/*
 * Our main function
 */
int
main(void)
{
  GMainLoop *loop;
  AXHttpHandler *handler;
  guint timer = 0;

  loop    = g_main_loop_new(NULL, FALSE);
  handler = ax_http_handler_new(request_handler, &timer);

  g_message("Created a HTTP handler: %p", handler);

  /* Periodically call 'on_timeout()' every second */
  g_timeout_add(1000, on_timeout, &timer);

  /* start the main loop */
  g_main_loop_run(loop);

  /* free up resources */
  g_main_loop_unref(loop);
  ax_http_handler_free(handler);

  return 0;
}

