#include "common.h"
#include "command.h"
#include "config.h"
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>

#ifdef QVPLAY_TEST_TRANSPORT
#include "tests/transport.h"
#elif defined(_WIN32)
#include "win32/tty_w32.h"
#else
#include "tty.h"
#include <termios.h>
#endif /* QVPLAY_TEST_TRANSPORT */
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
static int check_sum = 0;

static int write_all(const uint8_t *buf, size_t len) {
  size_t written = 0;

  while (written < len) {
    int result = writetty(QVfd, (uint8_t *)buf + written,
                          (int)(len - written));
    if (result <= 0)
      return -1;
    written += (size_t)result;
  }
  return 0;
}

static int read_exact(uint8_t *buf, size_t len) {
  size_t received = 0;

  while (received < len) {
    int result = readtty(QVfd, buf + received, (int)(len - received));
    if (result <= 0)
      return -1;
    received += (size_t)result;
  }
  return 0;
}

void QVsetfd(int fd) {
  dprintf((stderr, "QVfd = %x\n", fd));
  QVfd = fd;
}

int QVgetfd(void) { return QVfd; }

/*------------------------------------------------------------*/
static int calcsum(const uint8_t *p, int len) {
  const uint8_t *q;
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
  if (write_all(&c, 1) < 0) {
    perror("writetty");
    Exit(1);
  }
  check_sum = check_sum + (int)c;
}

uint8_t rbyte() {
  uint8_t c = 0;

  if (read_exact(&c, 1) < 0) {
    perror("readtty");
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

void wstr(const void *p, int len) {
  dprintf((stderr, "> len=%d\n", len));
  if (write_all((const uint8_t *)p, (size_t)len) < 0) {
    perror("writetty");
    Exit(1);
  }
  check_sum = check_sum + calcsum((const uint8_t *)p, len);
}

void rstr(uint8_t *p, int len) {

  dprintf((stderr, "< len=%d\n", len));
  if (read_exact(p, (size_t)len) < 0) {
    perror("readtty");
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
    n = 11;
    baud = B38400;
    break;
  case MID: /* 19200 baud */
    n = 22;
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
  size_t received = 0;
  uint8_t buf[SECTOR];
  uint8_t control = DC2;

  if (fp == NULL || filesize < 0 || write_all(&control, 1) < 0)
    return -1;

  for (;;) {
    int retry;

    if (qvverbose)
      fprintf(stderr, "%6lu/%6d\b\b\b\b\b\b\b\b\b\b\b\b\b",
              (unsigned long)received, filesize);

    for (retry = 0; retry < RETRY; retry++) {
      uint8_t header[3];
      uint8_t footer[2];
      size_t sectorsize;
      int sum;

      if (read_exact(header, sizeof(header)) < 0 || header[0] != STX) {
        flushtty(QVfd);
        control = NAK;
        if (write_all(&control, 1) < 0)
          return -1;
        continue;
      }
      sectorsize = ((size_t)header[1] << 8) | header[2];
      if (sectorsize > sizeof(buf) ||
          (filesize != 0 && sectorsize > (size_t)filesize - received)) {
        fprintf(stderr, "received block exceeds expected file size.\n");
        control = NAK;
        (void)write_all(&control, 1);
        return -1;
      }
      if (read_exact(buf, sectorsize) < 0 ||
          read_exact(footer, sizeof(footer)) < 0) {
        flushtty(QVfd);
        control = NAK;
        if (write_all(&control, 1) < 0)
          return -1;
        continue;
      }
      sum = header[1] + header[2] + calcsum(buf, (int)sectorsize);
      sum = 0xff & (sum + footer[0]);
      if (sum != (0xff & (~footer[1])) ||
          (footer[0] != ETX && footer[0] != ETB)) {
        flushtty(QVfd);
        control = NAK;
        if (write_all(&control, 1) < 0)
          return -1;
        continue;
      }
      if (footer[0] == ETX && filesize != 0 &&
          received + sectorsize != (size_t)filesize) {
        fprintf(stderr, "received file size does not match expected size.\n");
        control = NAK;
        (void)write_all(&control, 1);
        return -1;
      }
      if (fwrite(buf, 1, sectorsize, fp) != sectorsize)
        return -1;
      received += sectorsize;
      control = ACK;
      if (write_all(&control, 1) < 0)
        return -1;
      if (footer[0] == ETX) {
        if (qvverbose)
          fprintf(stderr, "\n");
        return (int)received;
      }
      break;
    }
    if (retry == RETRY)
      return -1;
  }
}
#endif

int QVblockrecv(uint8_t *buf, size_t capacity, size_t expected_size) {
  size_t received = 0;
  uint8_t control = DC2;

  if (buf == NULL || (expected_size != 0 && expected_size > capacity))
    return -1;
  if (write_all(&control, 1) < 0)
    return -1;

  for (;;) {
    int retry;

    if (qvverbose) {
      if (expected_size == 0)
        fprintf(stderr, "%6lu\b\b\b\b\b\b", (unsigned long)received);
      else
        fprintf(stderr, "%6lu/%6lu\b\b\b\b\b\b\b\b\b\b\b\b\b",
                (unsigned long)received, (unsigned long)expected_size);
    }

    for (retry = 0; retry < RETRY; retry++) {
      uint8_t header[3];
      uint8_t footer[2];
      size_t sectorsize;
      int sum;

      if (read_exact(header, sizeof(header)) < 0 || header[0] != STX) {
        flushtty(QVfd);
        control = NAK;
        if (write_all(&control, 1) < 0)
          return -1;
        continue;
      }

      sectorsize = ((size_t)header[1] << 8) | header[2];
      if (sectorsize > capacity - received ||
          (expected_size != 0 && sectorsize > expected_size - received)) {
        fprintf(stderr, "received block exceeds image buffer (%lu bytes).\n",
                (unsigned long)sectorsize);
        flushtty(QVfd);
        control = NAK;
        (void)write_all(&control, 1);
        return -1;
      }

      if (read_exact(buf + received, sectorsize) < 0 ||
          read_exact(footer, sizeof(footer)) < 0) {
        flushtty(QVfd);
        control = NAK;
        if (write_all(&control, 1) < 0)
          return -1;
        continue;
      }

      sum = header[1] + header[2] + calcsum(buf + received, (int)sectorsize);
      sum = 0xff & (sum + footer[0]);
      if (sum != (0xff & (~footer[1])) ||
          (footer[0] != ETX && footer[0] != ETB)) {
        flushtty(QVfd);
        control = NAK;
        if (write_all(&control, 1) < 0)
          return -1;
        continue;
      }

      if (footer[0] == ETX && expected_size != 0 &&
          received + sectorsize != expected_size) {
        fprintf(stderr, "received image size does not match expected size.\n");
        control = NAK;
        (void)write_all(&control, 1);
        return -1;
      }

      received += sectorsize;
      control = ACK;
      if (write_all(&control, 1) < 0)
        return -1;
      if (footer[0] == ETX) {
        if (qvverbose)
          fprintf(stderr, "\n");
        return (int)received;
      }
      break;
    }

    if (retry == RETRY) {
      fprintf(stderr, "block receive retry limit exceeded.\n");
      return -1;
    }
  }
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

/* Camera operations used by qvplay. */
int QVdeletepicture(int n) {
  uint8_t s;

  if (!QVok())
    return -1;
  wstr("DF", 2);
  wbyte((uint8_t)n);
  wbyte(0xff);
  s = rbyte();
  if (checksum(s) == -1)
    return -1;
  wbyte(ACK);
  return 1;
}

int QVtakepicture(void) {
  const uint8_t command[] = {'D', 'R', ACK};

  QVsectorsize(0x0340);
  if (!QVok())
    return -1;
  wstr(command, sizeof(command));
  (void)rbyte();
  (void)rbyte();
  sleep(10);
  return 1;
}

int QVmovepicture(int from, int to) {
  uint8_t s;

  if (!QVok())
    return -1;
  wstr("DI", 2);
  wbyte((uint8_t)from);
  s = rbyte();
  if (checksum(s) == -1)
    return -1;
  wbyte(ACK);

  if (!QVok())
    return -1;
  wstr("DY", 2);
  wbyte(0x02);
  wbyte((uint8_t)from);
  s = rbyte();
  if (checksum(s) == -1)
    return -1;
  wbyte(ACK);

  if (!QVok())
    return -1;
  wstr("DF", 2);
  wbyte((uint8_t)from);
  wbyte(0xff);
  s = rbyte();
  if (checksum(s) == -1)
    return -1;
  wbyte(ACK);

  if (!QVok())
    return -1;
  wstr("Dj", 2);
  wbyte((uint8_t)to);
  s = rbyte();
  if (checksum(s) == -1)
    return -1;
  wbyte(ACK);
  return 1;
}

int QVgetsize2(int n) {
  uint8_t s;
  long filesize = 0;

  if (QVshowpicture(n) < 0 || !QVok())
    return -1;
  wstr("DL", 2);
  s = rbyte();
  if (checksum(s) == -1)
    return -1;
  wbyte(ACK);

  if (!QVok())
    return -1;
  wstr("EM", 2);
  s = rbyte();
  if (checksum(s) == -1)
    return -1;
  wbyte(ACK);
  s = rbyte();
  filesize = s;
  s = rbyte();
  filesize = (filesize << 8) | s;
  s = rbyte();
  filesize = (filesize << 8) | s;
  s = rbyte();
  filesize = (filesize << 8) | s;
  return (int)filesize;
}

int QVgetextdata(int n, uint8_t *buf, size_t capacity) {
  uint8_t s;
  int len;

  if (QVshowpicture(n) < 0 || !QVok())
    return -1;
  wstr("DL", 2);
  s = rbyte();
  if (checksum(s) == -1)
    return -1;
  wbyte(ACK);

  if (!QVok())
    return -1;
  wstr("CV", 2);
  s = rbyte();
  if (checksum(s) == -1)
    return -1;
  wbyte(ACK);

  len = QVblockrecv(buf, capacity, 0);
  if (!QVok())
    return -1;
  return len;
}

int QVgetpicture(int n, uint8_t *buf, size_t capacity, QVdataType data_type,
                 int vga, FILE *fp) {
  uint8_t s;
  int len;
  long filesize = 0;

#ifndef USEWORKFILE
  (void)fp;
#endif

  if (vga == 2 && data_type == QV_DATA_JPEG) {
    filesize = QVgetsize2(n);
    if (filesize < 0)
      return -1;
  }
  if (QVshowpicture(n) < 0 || !QVok())
    return -1;
  wstr("DL", 2);
  s = rbyte();
  if (checksum(s) == -1)
    return -1;
  wbyte(ACK);

  if (!QVok())
    return -1;
  switch (data_type) {
  case QV_DATA_THUMBNAIL:
    wstr("MK", 2);
    break;
  case QV_DATA_JPEG:
  default:
    if (vga == 1)
      wstr("Mg", 2);
    else if (vga == 2)
      wstr("EG", 2);
    else
      wstr("MG", 2);
    break;
  }
  s = rbyte();
  if (checksum(s) == -1)
    return -1;
  wbyte(ACK);

  if (qvverbose) {
    if (data_type == QV_DATA_THUMBNAIL)
      fprintf(stderr, "Thumbnail %3d: ", n);
    else
      fprintf(stderr, "Picture   %3d: ", n);
  }

#ifdef USEWORKFILE
  if (fp != NULL)
    len = QVblockrecv_file(fp, (int)filesize);
  else
#endif
    len = QVblockrecv(buf, capacity, (size_t)filesize);

  if (!QVok())
    return -1;
  return len;
}

int QV4split(int *n) {
  uint8_t s;
  int i;

  if (!QVok())
    return -1;
  wstr("DB", 2);
  for (i = 0; i < 4; i++)
    wbyte((uint8_t)n[i]);
  s = rbyte();
  if (checksum(s) == -1)
    return -1;
  wbyte(ACK);
  return 1;
}

int QV9split(int *n) {
  uint8_t s;
  int i;

  if (!QVok())
    return -1;
  wstr("DC", 2);
  for (i = 0; i < 9; i++)
    wbyte((uint8_t)n[i]);
  s = rbyte();
  if (checksum(s) == -1)
    return -1;
  wbyte(ACK);
  return 1;
}

int QVprotect(int n, int on) {
  uint8_t s;

  if (!QVok())
    return -1;
  wstr("DY", 2);
  wbyte(on ? 0x01 : 0x00);
  wbyte((uint8_t)n);
  s = rbyte();
  if (checksum(s) == -1)
    return -1;
  wbyte(ACK);
  return 1;
}

int QVhidepicnum(void) {
  uint8_t s;

  if (!QVok())
    return -1;
  wstr("DM", 2);
  s = rbyte();
  if (checksum(s) == -1)
    return -1;
  wbyte(ACK);
  return 1;
}

int QVpoweroff(void) {
  uint8_t s;

  if (!QVok())
    return -1;
  wstr("QX", 2);
  s = rbyte();
  if (checksum(s) == -1)
    return -1;
  wbyte(ACK);
  return 1;
}

int QVcolorpattern(void) {
  uint8_t s;

  if (!QVok())
    return -1;
  wstr("DP", 2);
  s = rbyte();
  if (checksum(s) == -1)
    return -1;
  wbyte(ACK);
  return 1;
}

int QVpicattr(int n) {
  uint8_t s;

  if (!QVok())
    return -1;
  wstr("DY", 2);
  wbyte(0x02);
  wbyte((uint8_t)n);
  s = rbyte();
  if (checksum(s) == -1)
    return -1;
  wbyte(ACK);
  return rbyte();
}

/* Recovery operation used by qvalldel. */
int QValldelete(void) {
  uint8_t s;

  if (!QVok())
    return -1;
  wstr("DD", 2);
  s = rbyte();
  if (checksum(s) == -1)
    return -1;
  wbyte(ACK);
  return 1;
}
