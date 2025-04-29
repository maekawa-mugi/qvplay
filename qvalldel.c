#include "config.h"
#include "version.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if HAVE_UNISTD_H
#include <sys/types.h>
#include <unistd.h>
#endif
#if HAVE_SYS_PARAM_H
#include <sys/param.h>
#else
#define MAXPATHLEN 256
#endif
#include "command.h"
#include "command3.h"
#include "common.h"
#ifdef X68K
#include "x68k/tty_x68.h"
#else
#ifdef _WIN32
#include "win32/getopt/getopt.h"
#include "win32/tty_w32.h"
#else
#ifdef OS2
#include "os2/tty_os2.h"
#else
#ifdef DOS
#include "dos/tty_dos.h"
#else
#include "tty.h"
#endif /* DOS */
#endif /* OS2 */
#endif /* WIN32 */
#endif /* X68 */

#include "common.h"

extern int optind, opterr;
extern char *optarg;

static int all_pic_num = -1;
#ifndef DONTCAREUID
static uid_t uid, euid;
static gid_t gid, egid;
static int uidswapped = 0;
#endif

void version() {
  static char *usagestr[] = {
      QVALLDEL_USAGE_HEADER,
      (char *)NULL,
  };
  char **p;

  p = usagestr;
  while (*p)
    fprintf(stdout, *p++);
}

void usage() {
  static char *usagestr[] = {
      QVALLDEL_USAGE_HEADER,
      "qvalldel [options]\n",
      "\t -f           : not show 'Are you sure ?' message.\n",
      "\t -h           : show this usage.\n",
      "\t -V           : show version information.\n",
#ifndef X68
      "\t -D ttydevice : set tty(cua) device.\n",
#endif
      (char *)NULL,
  };
  char **p;

  p = usagestr;
  while (*p)
    fprintf(stdout, *p++);
}

void Exit(code) int code;
{
  QVreset(1);
  if (!(QVgetfd() < 0))
    closetty(QVgetfd());
  exit(code);
}

#ifndef DONTCAREUID
void daemonuid() {
  if (uidswapped) {
#ifdef HAVE_SETREUID
    setreuid(uid, euid);
    setregid(gid, egid);
#else
    setuid(uid);
    seteuid(euid);
    setgid(gid);
    setegid(egid);
#endif
    uidswapped = 0;
  }
}

void useruid() {
  if (!uidswapped) {
#ifdef HAVE_SETREUID
    setregid(egid, gid);
    setreuid(euid, uid);
#else
    setgid(egid);
    setegid(gid);
    setuid(euid);
    seteuid(uid);
#endif
    uidswapped = 1;
  }
}
#endif

int main(argc, argv)
int argc;
char **argv;
{
  char *devpath = NULL;
  int force = 0;
  char c;

#ifndef DONTCAREUID
  uid = getuid();
  euid = geteuid();
  gid = getgid();
  egid = getegid();
  useruid();
#endif

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

  if (devpath == NULL) {
    devpath = malloc(sizeof(char) * (strlen(RSPORT) + 1));
    if (devpath == NULL) {
      fprintf(stderr, "can't malloc\n");
      exit(1);
    }
    strcpy(devpath, RSPORT);
  }

  if (devpath) {
#ifndef DONTCAREUID
    daemonuid();
#endif
    QVsetfd(opentty(devpath));
#ifndef DONTCAREUID
    useruid();
#endif
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
  QVreset(1);
  Exit(0);
}
