#include "command.h"
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
#endif /* X68 */
#if HAVE_UNISTD_H
#include <unistd.h>
#endif

extern int qvverbose;

/*------------------------------------------------------------*/
int QVputcam(n, size, buf)
int n;
int size;
uint8_t *buf;
{
  uint8_t s;
  int len;

#ifdef FINEMODE
  if (!QVok())
    return -1; /*ng*/
  wstr("Eb", 2);
  s = rbyte();
  if (checksum(s) == -1)
    return (-1);
  wbyte(ACK);
  s = rbyte(); /* remain */
#endif

  /* drain */
  if (!QVok())
    return -1; /*ng*/

  wstr("MH", 2);
  s = rbyte();
  if (checksum(s) == -1)
    return (-1);
  wbyte(ACK);

  s = rbyte(); /* may be 0x12 ?*/

  if (qvverbose)
    fprintf(stderr, "Picture   %3d: ", n);

  len = QVblocksend(buf, size);
  if (len == -1)
    return -1;

  if (!QVok())
    return -1; /*ng*/

  wstr("DJ", 2);
  s = rbyte(); /* checksum */
  if (checksum(s) == -1)
    return (-1);
  wbyte(ACK);

#ifdef FINEMODE
  if (!QVok())
    return -1; /*ng*/
  len = QVhowmany();
  len = QVhowmany();
  QVshowpicture(len);
  s = len;
  if (!QVok())
    return -1; /*ng*/
  wstr("DV", 2);
  wbyte(s);
  s = rbyte();
  wbyte(ACK);
#endif
  return len;
}

/*------------------------------------------------------------*/
int QVputpicture(n, size, buf)
int n;
int size;
uint8_t *buf;
{
  uint8_t s;
  int len;

  /* drain */

  if (!QVok())
    return -1; /*ng*/
  wstr("MM", 2);
  s = rbyte();
  if (checksum(s) == -1)
    return (-1);

  wbyte(ACK);
  s = rbyte(); /* may be 0x12 */

  if (qvverbose)
    fprintf(stderr, "Picture   %3d: ", n);

  len = QVblocksend(buf, size);
  if (len == -1)
    return -1;

  if (!QVok())
    return -1; /*ng*/
  wstr("DN", 2);
  s = rbyte(); /* checksum */
  if (checksum(s) == -1)
    return (-1);
  wbyte(ACK);

  if (!QVok())
    return -1; /*ng*/
  wstr("DJ", 2);
  s = rbyte(); /* checksum */
  if (checksum(s) == -1)
    return (-1);
  wbyte(ACK);

  return len;
}

/*------------------------------------------------------------*/
int QVputJpegQv7xx(n, size, buf)
int n;
int size;
uint8_t *buf;
{
  uint8_t s;
  int len;

  if (!QVok())
    return -1; /*ng*/

  wstr("CZ", 2);
  wbyte(0x01); /* 01 eco 02 normal 03 fine */
  s = rbyte();
  fprintf(stderr, "CZ 0 %02x\n", s);
  wbyte(ACK);
  s = rbyte();
  fprintf(stderr, "CZ 1 %02x\n", s);
  s = rbyte();
  fprintf(stderr, "CZ 2 %02x\n", s);

#ifdef AAAAAAAA
  if (!QVok())
    return -1; /*ng*/

  wstr("ES", 2);
  s = rbyte();
  if (checksum(s) == -1)
    return (-1);
  wbyte(ACK);
  s = rbyte(); /* 0x00 */
  fprintf(stderr, "ES 1 %02x\n", s);
  s = rbyte(); /* 0x00 */
  fprintf(stderr, "ES 2 %02x\n", s);
  s = rbyte(); /* 0x00 */
  fprintf(stderr, "ES 3 %02x\n", s);
  s = rbyte(); /* 0x0b */
  fprintf(stderr, "ES 4 %02x\n", s);
#endif

  /* drain */
  if (!QVok())
    return -1; /*ng*/

  wstr("MW", 2);
  s = rbyte();
  if (checksum(s) == -1)
    return (-1);
  wbyte(ACK);

  s = rbyte(); /* may be 0x12 ?*/

  if (qvverbose)
    fprintf(stderr, "Picture   %3d: ", n);

  len = QVblocksend(buf, size);
  if (len == -1)
    return -1;

  if (!QVok())
    return -1; /*ng*/

  wstr("DJ", 2);
  s = rbyte(); /* checksum */
  if (checksum(s) == -1)
    return (-1);
  wbyte(ACK);
  fprintf(stderr, "DJ\n");

  if (!QVok())
    return -1; /*ng*/
  wstr("CF", 2);
  s = rbyte(); /* checksum */
  if (checksum(s) == -1)
    return (-1);
  wbyte(ACK);
  s = rbyte(); /* 0x00 */
  fprintf(stderr, "CF 1 %02x\n", s);
  s = rbyte(); /* 0x0b */
  fprintf(stderr, "CF 2 %02x\n", s);

#ifdef HOGE1
  if (!QVok())
    return -1; /*ng*/
  len = QVhowmany();
  QVshowpicture(len);
  s = len;
  if (!QVok())
    return -1; /*ng*/
  wstr("DV", 2);
  wbyte(s);
  s = rbyte();
  wbyte(ACK);
#endif
  return len;
}
