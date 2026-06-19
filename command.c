#include "common.h"
#include "config.h"
#include <setjmp.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>

#ifdef X68K
#include "x68k/tty_x68.h"
#else
#ifdef _WIN32
#include "win32/tty_w32.h"
#else
#ifdef OS2
#include "os2/tty_os2.h"
#else
#ifdef DOS
#include "dos/tty_dos.h"
#else
#include "tty.h"
#include <termios.h>
#endif /* DOS */
#endif /* OS2 */
#endif /* WIN32 */
#endif /* X68K */
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#if 0
#define dprintf(x) fprintf x
#else
#define dprintf(x)
#endif

int qvverbose;
int qvhasvgamode;
int qv7xxprotocol;

static int QVfd = -1;
static int dotrap = 0;
static jmp_buf errtrap;
static int check_sum = 0;

/* for VC++1.5 */
#ifdef DOS
void sleep(int sec) {
  time_t lt;
  time_t lt2;
  time(&lt);

  while (1) {
    time(&lt2);
    if ((lt2 - lt) >= sec)
      return;
  }
}
#endif

void QVsetfd(int fd) {
  dprintf((stderr, "QVfd = %x\n", fd));
  QVfd = fd;
}

int QVgetfd(void) { return QVfd; }

/*------------------------------------------------------------*/
static int calcsum(uint8_t *p, int len) {
  uint8_t *q;
  int sum = 0;
  int i;
  q = p;
  for (i = 0; i < len; i++) {
    sum = sum + *q;
    q++;
  }
  return (sum);
}

void wbyte(uint8_t c) {
  dprintf((stderr, "> %02x\n", c));
  if (writetty(QVfd, &c, 1) < 0) {
    perror("writetty");
    if (dotrap)
      longjmp(errtrap, 1);
    else
      Exit(1);
  }
  check_sum = check_sum + (int)c;
}

uint8_t rbyte() {
  uint8_t c;

  if (readtty(QVfd, &c, 1) < 0) {
    perror("readtty");
    if (dotrap)
      longjmp(errtrap, 1);
    else
      Exit(1);
  }
  dprintf((stderr, "< %02x\n", c));
  return c;
}

int checksum(uint8_t u) {
  uint8_t s;
  s = 0xff & (~check_sum);
  if (u != s) {
    if (s == rbyte())
      return (1);
    fprintf(stderr, "checksum error.\n");
    return (-1);
  }
  return (1);
}

void wstr(uint8_t *p, int len) {
  dprintf((stderr, "> len=%d\n", len));
  if (writetty(QVfd, p, len) < 0) {
    perror("writetty");
    if (dotrap)
      longjmp(errtrap, 1);
    else
      Exit(1);
  }
  check_sum = check_sum + calcsum(p, len);
}

void rstr(uint8_t *p, int len) {

  dprintf((stderr, "< len=%d\n", len));
  if (readtty(QVfd, p, len) < 0) {
    perror("readtty");
    if (dotrap)
      longjmp(errtrap, 1);
    else
      Exit(1);
  }
}

/*------------------------------------------------------------*/

int QVok(void) {
  int retrycount = RETRY;

  while (retrycount--) {
    wbyte(ENQ);
    if (rbyte() == ACK) {
      check_sum = 0;
      return 1; /*ok*/
    }
  }
  return 0; /*ng*/
}

int QVreset(int flag) {
  uint8_t s;

  if (!QVok())
    return -1; /*ng*/

  if (flag)
    wstr("QR", 2);
  else
    wstr("QE", 2);

  s = rbyte();            /*supposed to be 0x5c('Z') or 0x69*/
#if !defined(__FreeBSD__) /* Why ? */
  if (checksum(s) == -1)
    return (-1);
#endif
  wbyte(ACK);

  return (int)s; /*ok*/
}

int QVhowmany(void) {
  uint8_t s;
  uint8_t n;
  int retrycount = RETRY;

  while (retrycount--) {
    if (!QVok())
      return -1; /*ng*/
    wstr("MP", 2);
    s = rbyte(); /*supposed to be 0x62 */
    if (s == 0x62)
      break;
  }
  wbyte(ACK);
  n = rbyte(); /*# of pictures*/
  return (int)n;
}

int QVremain(int n) /* 100 only may be*/
{
  uint8_t s;

  int p;

  if (!QVok())
    return -1; /*ng*/

  if (qvhasvgamode == 0) { /* QV10/30/10a/11/70 */
    p = QVhowmany();
    if (p < 0)
      return -1;
    return (MAX_PICTURE_NUM_QV10 - p);
  }

  if (qv7xxprotocol == 0) {
    /* QV100/300/200 */
    if (n)
      wstr("Eb", 2); /* fine picture remain */
    else
      wstr("EB", 2); /* normal picture remain */
    s = rbyte();
    if (checksum(s) == -1)
      return (-1);
    wbyte(ACK);
    s = rbyte(); /* remain picture num */
    return (int)s;
  } else {
    /* QV700/770 */
    /* not supported yet */
    return (100);
  }
}

int QVshowpicture(int n) {
  uint8_t s;

  if (!QVok())
    return -1; /*ng*/
  wstr("DA", 2);
  wbyte(n);
  s = rbyte(); /*supposed to be 0x7a - n*/
  if (checksum(s) == -1)
    return (-1);
  wbyte(ACK);
  return 1; /*ok*/
}

int QVswstat(void) {
  uint8_t s;
  int r;

  if (!QVok())
    return -1; /*ng*/
  wstr("DS", 2);
  wbyte(0x02);
  s = rbyte();
  if (checksum(s) == -1)
    return (-1);
  wbyte(ACK);
  s = rbyte(); /* SW status ?*/
  r = s << 8;
  s = rbyte(); /* SW status ?*/
  r = r | s;
  return (int)r;
}

long QVrevision() {
  uint8_t s;
  long r;

  if (!QVok())
    return -1; /*ng*/
  wstr("SU", 2);
  s = rbyte();
  if (checksum(s) == -1)
    return (-1);
  wbyte(ACK);
  s = rbyte(); /* revision ? */
  r = s;
  s = rbyte();
  r = r << 8;
  r = r | s;
  s = rbyte(); /* revision ? */
  r = r << 8;
  r = r | s;
  s = rbyte();
  r = r << 8;
  r = r | s;
  return (long)r;
}

static int current_speed = DEFAULT;
int QVchangespeed(int speed) {
  int n;
  uint8_t s;
  int baud;

  if (current_speed == speed)
    return (1);

  if (!QVok())
    return -1; /*ng*/

  switch (speed) {
  case LIGHT: /* 115200 baud */
    n = 3;
#ifdef CANNOTUSEHIGHSPEED
    /* some linux distribution cannot use B115200,
         so you should use setserial command  */
    baud = B38400;
#else
    baud = B115200;
#endif
    break;
  case TOP: /* 57600 baud */
    n = 7;
#ifdef CANNOTUSEHIGHSPEED
    baud = B38400;
#else
    /* some linux distribution cannot use B57600,
         so you should use setserial command  */
    baud = B57600;
#endif
    break;
  case HIGH: /* 38400 baud */
#ifdef X68K
    if (qvhasvgamode)
      n = 11; /* QV-100/300  */
    else
      n = 10; /* 39062.5 baud */
#else
    n = 11;
#endif
    baud = B38400;
    break;
  case MID: /* 19200 baud */
#ifdef X68K
    n = 23;
#else
    n = 22;
#endif
    baud = B19200;
    break;
  case DEFAULT:
  default:
    n = 46;
    baud = B9600;
    break;
  }
  wstr("CB", 2);
  wbyte(n);

  s = rbyte();
  if (checksum(s) == -1)
    return (-1);
  wbyte(ACK);
  sleep(1); /* ??? */
  changespeed(QVfd, baud);
  current_speed = speed;
  if (!QVok())
    return -1; /*ng*/
  return 1;    /*ok*/
}

/*------------------------------------------------------------*/
int QVsectorsize(int n) {
  uint8_t s;
  uint8_t t;
  s = (uint8_t)(n >> 8) & 0xff;
  t = (uint8_t)n & 0xff;
  if (!QVok())
    return -1; /*ng*/
  wstr("PP", 2);
  wbyte(s);
  wbyte(t);
  s = rbyte();
  if (checksum(s) == -1)
    return (-1);
  wbyte(ACK);
  return 1;
}

#ifdef USEWORKFILE
int QVblockrecv_file(FILE *fp, int filesize) {
  uint8_t s;
  uint8_t t;
  uint8_t *p;
  int i = 0;
  u_int sectorsize;
  int retrycount = RETRY;
  int sum;
  uint8_t buf[SECTOR];

  wbyte(DC2);

  while (1) {
    if (qvverbose)
      fprintf(stderr, "%6d\b\b\b\b\b\b", p - buf);
    else
      fprintf(stderr, "%6d/%6d\b\b\b\b\b\b\b\b\b\b\b\b\b", p - buf, filesize);

    /* x: fault handlers */
    dotrap = 1;
    if (setjmp(errtrap) != 0) {
      wbyte(NAK);
      dprintf((stderr, "*********retry*********\n"));
    }

  retry:;
    p = buf;
    /* 1: obtain sector size */
    if ((s = rbyte()) != STX) {
      dprintf((stderr, "NG sector size(%02x)\n", s));
      flushtty(QVfd);
      wbyte(NAK);
      retrycount--;
      if (retrycount)
        goto retry;
      return -1; /*ng*/
    }
    s = rbyte();
    sum = s;
    t = rbyte();
    sum = sum + t;
    sectorsize = ((u_int)s << 8) | t;

    /* 2: drain it */
    rstr(p, sectorsize);

    sum = sum + calcsum(p, sectorsize);
    /* 3: finalize sector */
    s = rbyte();           /*sector type?*/
    t = 0xff & (~rbyte()); /*checksum?*/
    sum = 0xff & (sum + s);
    if (sum != t) {
      flushtty(QVfd);
      wbyte(NAK);
      goto retry;
    }
    i += sectorsize;
    fwrite(p, sizeof(uint8_t), sectorsize, fp);

    dotrap = 0;

    if (s == ETX) {
      /* final sector... terminate transfer */
      wbyte(ACK);
      break;
    } else if (s == ETB) {
      /* block cleanup */
      wbyte(ACK);
    } else {
      /* strange condition... retry this sector */
      flushtty(QVfd);
      wbyte(NAK);
      goto retry;
    }
  }

  if (qvverbose)
    fprintf(stderr, "\n");

  return i;
}
#endif

int QVblockrecv(uint8_t *buf, int filesize) {
  uint8_t s;
  uint8_t t;
  uint8_t *p;
  u_int sectorsize;
  int retrycount = RETRY;
  int sum;

  wbyte(DC2);

  p = buf;
  while (1) {
    if (qvverbose)
      if (filesize == 0)
        fprintf(stderr, "%6d\b\b\b\b\b\b", p - buf);
      else
        fprintf(stderr, "%6d/%6d\b\b\b\b\b\b\b\b\b\b\b\b\b", p - buf, filesize);

    /* x: fault handlers */
    dotrap = 1;
    if (setjmp(errtrap) != 0) {
      wbyte(NAK);
      dprintf((stderr, "*********retry*********\n"));
    }

  retry:;
    /* 1: obtain sector size */
    if ((s = rbyte()) != STX) {
      dprintf((stderr, "NG sector size(%02x)\n", s));
      flushtty(QVfd);
      wbyte(NAK);
      retrycount--;
      if (retrycount)
        goto retry;
      return -1; /*ng*/
    }
    s = rbyte();
    sum = s;
    t = rbyte();
    sum = sum + t;
    sectorsize = ((u_int)s << 8) | t;

    /* 2: drain it */
    rstr(p, sectorsize);
    sum = sum + calcsum(p, sectorsize);
    p += sectorsize;

    /* 3: finalize sector */
    s = rbyte();           /*sector type?*/
    t = 0xff & (~rbyte()); /*checksum?*/
    sum = 0xff & (sum + s);
    if (sum != t) {
      flushtty(QVfd);
      wbyte(NAK);
      goto retry;
    }

    dotrap = 0;

    if (s == ETX) {
      /* final sector... terminate transfer */
      wbyte(ACK);
      break;
    } else if (s == ETB) {
      /* block cleanup */
      wbyte(ACK);
    } else {
      /* strange condition... retry this sector */
      flushtty(QVfd);
      wbyte(NAK);
      goto retry;
    }
  }

  if (qvverbose)
    fprintf(stderr, "\n");

  return p - buf;
}

int QVbattery(void) {
  uint8_t s;

  if (!QVok())
    return -1; /*ng*/

  wstr("RB", 2);
  wbyte(ENQ);
  wbyte(0xFF);
  wbyte(0xFE);
  wbyte(0xE6);
  s = rbyte(); /* check sum 0x83 */
  if (checksum(s) == -1)
    return (-1);
  wbyte(ACK);
  s = rbyte(); /* battery */

#ifdef AAAAAAA
  if (qv7xxprotocol == 0) {
    wstr("RB", 2);
    wbyte(ENQ);
    wbyte(0xFF);
    wbyte(0xFE);
    wbyte(0xE6);
    s = rbyte();
    if (checksum(s) == -1)
      return (-1);
    wbyte(ACK);
    s = rbyte();
  } else {
    wstr("BC", 2);
    s = rbyte();
    /*    fprintf(stderr,"BC cksum = %02x(%c)\n", s, s); */
    wbyte(ACK);
    s = rbyte();
    /*    fprintf(stderr, "BC value = %d %02x\n", s, s); */
  }
#endif

  return (int)s;
}

int QVdefaultpicture(int n) {
  uint8_t s;

  if (!QVok())
    return -1; /*ng*/
  wstr("DV", 2);
  wbyte(n);
  s = rbyte();
  if (checksum(s) == -1)
    return (-1);
  wbyte(ACK);
  return (int)1;
}

int QVnewprotocol(void) {
  uint8_t s;
  if (!QVok())
    return -1; /*ng*/
  wstr("NP", 2);
  wbyte(0x01);
  s = rbyte();
  if (checksum(s) == -1)
    return (-1);
  wbyte(ACK);
  return (int)1;
}

int QVdisableAutoPowerOff(void) {
  uint8_t s;
  if (!QVok())
    return -1; /*ng*/
  wstr("DU", 2);
  s = rbyte();
  if (checksum(s) == -1)
    return (-1);
  wbyte(ACK);
  QVreset(0); /* QE */
  return (int)1;
}
