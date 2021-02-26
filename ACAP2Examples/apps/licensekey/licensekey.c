#include <stdlib.h>
#include <stdio.h>
#include <libgen.h>
#include <licensekey.h>
#include <unistd.h>
#include <signal.h>
#include <syslog.h>
#include <sys/ptrace.h>

#include <string.h>
#include <getopt.h>
#include "customlicense.h"

#define APP_ID           569
#define MAJOR_VERSION    1
#define MINOR_VERSION    1

#define LOG(fmt, args...)   syslog(LOG_INFO, fmt, ## args)

static volatile sig_atomic_t exit_signal = 0;
static char* app_name = NULL;

static void show_help()
{
  printf("\n"
  "Usage: %s [OPTION]\n"
  "Example application utilizing the license framework.\n"
  "Application code also contains the base for hooking up\n"
  "a custom licensing scheme onto AXIS license framework,\n"
  "i.e the Web GUI and list.cgi.\n"
  "\n"
  " -h, --help               display this help and exit.\n"
  " -l, --license_status     display the applications license status and exit.\n"
  "\n", app_name );
}


static void
handle_sigterm(sig_atomic_t signo)
{
  exit_signal = 1;
}

static void
setup_signals(void)
{
  struct sigaction sa;

  sa.sa_flags = 0;

  sigemptyset(&sa.sa_mask);
  sa.sa_handler = handle_sigterm;
  sigaction(SIGTERM, &sa, NULL);
  sigaction(SIGINT, &sa, NULL);
}


static int
display_license_status()
{

  int ret;
  openlog(app_name, LOG_PID | LOG_CONS, LOG_USER);

  /* Axis License API, this call should be substituted in an
   * application using a custom/propritary licensing framework.
   * see customlicense.h for more information.
   * If a custom/propritary license framework is used, set the
   * package.conf variables LICENSEPAGE and LICENSE_CHECK_ARGS.
   * e.g:
   * LICENSEPAGE='custom'
   * LICENSE_CHECK_ARGS='--license_status'
   */

  ret = licensekey_verify(app_name, APP_ID, MAJOR_VERSION, MINOR_VERSION);

  if (ret == AXIS_LICENSE_CHECK_OK ) {
    printf("%s\n", CUSTOM_LICENSE_VALID);
    LOG("%s: License verification succeeded\n", app_name);
  }
  else {
    printf("%s\n", CUSTOM_LICENSE_INVALID);
    LOG("%s: License verification failed\n", app_name);
  }

  closelog();

  return ret;
}


int
main(int argc, char *argv[])
{
  int c;
  struct option long_options[] =
  {
    {"help", no_argument, NULL, 'h'},
    {"license_status", no_argument, NULL, 'l'},
    {0, 0, 0, 0},
  };

  /* Anti-debugging based on the ptrace system call */
  int is_debugged = (ptrace(PTRACE_TRACEME, 0, 0, 0) == -1);
  if (is_debugged) {
  	printf("This application is currently being debugged\n");
  	exit(EXIT_FAILURE);
  }

  app_name = basename(argv[0]);
  setup_signals();

  while ((c = getopt_long(argc, argv, "hl", long_options, NULL)) != -1) {
     switch (c) {
       case 'l' :
         display_license_status(); /* must only check license status and exit */
         exit(EXIT_SUCCESS);
         break;
       case '?' : /* fall-through to help on invalid option */
       case 'h' :
         show_help();
         c == '?' ? exit(EXIT_FAILURE):exit(EXIT_SUCCESS);
         break;
       case -1 : /* no more options */
         break;
       default :
         break;
     }
  }

  return EXIT_SUCCESS;
}

