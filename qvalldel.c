#include "config.h"
#include "version.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
#include <sys/types.h>
#include <unistd.h>
#endif
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#else
#define MAXPATHLEN 256
#endif
#include "command.h"
#include "command3.h"
#include "common.h"
#ifdef _WIN32
#include "win32/getopt.h"
#include "win32/tty_w32.h"
#else
#include <getopt.h>
#include "tty.h"
#endif

#include "common.h"

extern int optind, opterr;
extern char *optarg;

static int all_pic_num = -1;

void version(void) {
  static char *usagestr[] = {
      QVALLDEL_USAGE_HEADER,
      (char *)NULL,
  };
  char **p;

  p = usagestr;
  while (*p)
    fprintf(stdout, "%s", *p++);
}

void usage(void) {
  static char *usagestr[] = {
      QVALLDEL_USAGE_HEADER,
      "qvalldel [options]\n",
      "\t -f           : not show 'Are you sure ?' message.\n",
      "\t -h           : show this usage.\n",
      "\t -V           : show version information.\n",
      "\t -D ttydevice : set tty(cua) device.\n",
      (char *)NULL,
  };
  char **p;

  p = usagestr;
  while (*p)
    fprintf(stdout, "%s", *p++);
}

void Exit(int code) {
  if (QVgetfd() >= 0) {
    closetty(QVgetfd());
    QVsetfd(-1);
  }
  exit(code);
}

int main(int argc, char **argv) {
  char *devpath = NULL;
  int force = 0;
  int c;

  devpath = getenv("QVPLAYTTY");

  while ((c = getopt(argc, argv, "D:fhV")) != -1) {

    switch (c) {
    case 'V':
      version();
      exit(0);
    case 'h':
    case '?':
      usage();
      exit(0);
    case 'D':
      devpath = optarg;
      break;
    case 'f':
      force = 1;
      break;
    }
  }

  if (devpath == NULL)
    devpath = RSPORT;

  if (devpath) {
    QVsetfd(opentty(devpath));
    if (QVgetfd() < 0)
      Exit(1);
    if (all_pic_num < 0)
      all_pic_num = QVhowmany();
    if (all_pic_num < 0)
      Exit(1);
  }
  if (QVbattery() <= LOW_BATT) {
    fprintf(stderr, "LOW BATTERY, change battery or connect AC adapter.\n");
    Exit(3);
  }
  c = '\0';
  if (force)
    QValldelete();
  else if (all_pic_num > 0) {
    fprintf(stderr, "QV10 has %d picture(s).\n", all_pic_num);
    fprintf(stderr, "Are you sure ?(n/y)\n");
    c = getc(stdin);
    if ((c == 'y') || (c == 'Y'))
      QValldelete();
  }
  if (QVreset(1) < 0)
    Exit(1);
  Exit(0);
}
