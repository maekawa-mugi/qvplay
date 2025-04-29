#include "config.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#if HAVE_UNISTD_H
# include <sys/types.h>
# include <unistd.h>
#endif
#if HAVE_SYS_PARAM_H
# include <sys/param.h>
#else
#define MAXPATHLEN 256
#endif
#include "common.h"
#include "command.h"
#include "command3.h"
#ifdef X68
#include "tty_x68.h"
#else
#ifdef WIN32
#include "tty_w32.h"
#include "getopt.h"
#else
#ifdef OS2
#include "tty_os2.h"
#else
#ifdef DOS
#include "tty_dos.h"
#else
#include "tty.h"
#endif /* DOS */
#endif /* OS2 */
#endif /* WIN32 */
#endif /* X68 */

#include "common.h"

extern int	optind, opterr;
extern char	*optarg;

static	int	all_pic_num = -1;
#ifndef DONTCAREUID
static	uid_t	uid, euid;
static	gid_t	gid, egid;
static	int	uidswapped = 0;
#endif


void usage()
{
  static	char	*usagestr[] =  {
    "qvalldel (Ver 0.95) (c)1996-2000 ken-ichi HAYASHI\n",
    "qvrec [options]\n",
    "\t -h           : show this usage.\n",
    "\t -f           : not show 'Are you sure ?' message.\n",
#ifndef X68
    "\t -D ttydevice : set tty(cua) device.\n",
#endif
    (char *)NULL,
  };
  char	**p;

  p = usagestr;
  while (*p)
    fprintf(stdout, *p++);
}

void Exit(code)
     int code;
{
  QVreset(1);
  if (!(QVgetfd() < 0))
    closetty(QVgetfd());
  exit(code);
} 

#ifndef DONTCAREUID
void
daemonuid()
{
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

void
useruid()
{
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

int
main(argc, argv)
     int	argc;
     char	**argv;
{
  char	*devpath = NULL;
  int	force = 0;
  char	c;
  

#ifndef DONTCAREUID
  uid = getuid();
  euid = geteuid();
  gid = getgid();
  egid = getegid();
  useruid();
#endif

  devpath = getenv("QVPLAYTTY");

  while ((c = getopt( argc, argv, "D:fh")) != -1){

    switch(c) {
    case 'h':
    case '?':
      usage();
      exit(-1);
    case 'D':
      devpath = optarg;
      break;
    case 'f':
      force = 1;
      break;
    }
  }

  if(devpath == NULL){
    devpath = malloc(sizeof(char) * (strlen(RSPORT) +1));
    if(devpath == NULL) {
      fprintf(stderr, "can't malloc\n");
      exit(1);
    }
    strcpy(devpath, RSPORT);
  }

  if(devpath){
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
  if(QVbattery() <= LOW_BATT){
    fprintf(stderr,"LOW BATTERY, change battery or connect AC adapter.\n");
    Exit(3);
  }
  c = '\0';
  if(force)
      QValldelete();
  else
    if(all_pic_num > 0){
      fprintf(stderr,"QV10 has %d picture(s).\n", all_pic_num);
      fprintf(stderr,"Are you sure ?(n/y)\n");
      c = getc(stdin);
      if((c =='y') || (c =='Y'))
	QValldelete();
    }
  QVreset(1);
  Exit (0);
}
