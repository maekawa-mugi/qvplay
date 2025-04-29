#include "config.h"
#include "version.h"
#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#ifdef BINARYFILEMODE
#include <fcntl.h> /* for setmode() */
#endif
#include <stdlib.h>
#include <time.h>
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
#include "command2.h"
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
#include "getuint.h"
#define MAX_PICTURE_NUM_QV10 96

extern int optind, opterr;
extern char *optarg;

static int speed = 0;
static int format = PPM_P;
extern int qvverbose;
extern int qvhasvgamode;
extern int qv7xxprotocol;

static int errflg = 0;
static int all_pic_num = -1;
#ifndef DONTCAREUID
static uid_t uid, euid;
static gid_t gid, egid;
static int uidswapped = 0;
#endif

static uint8_t buf[YCC_MAXSIZ_VGA];

#define HOGE 9

void version() {
  static char *usagestr[] = {
      QVREC_USAGE_HEADER,
      (char *)NULL,
  };
  char **p;

  p = usagestr;
  while (*p)
    fprintf(stdout, *p++);
}

void usage() {
  static char *usagestr[] = {
      QVREC_USAGE_HEADER,
      "qvrec [options] filename1.cam filename2.cam ...\n",
      "\t -h           : show this usage.\n",
#if defined(__linux__) || defined(WIN32) || defined(OS2) ||                    \
    defined(__FreeBSD__) || defined(DOS)
      "\t -S speed     : serial speed. [normal middle high top light]\n",
#else
      "\t -S speed     : serial speed. [normal middle high]\n",
#endif
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
  if (!(QVgetfd() < 0))
    QVchangespeed(DEFAULT);
  QVreset(1);
  closetty(QVgetfd());
  exit(code);
}

void put_cam(n, infilename) int n;
char *infilename;
{
  int len;
  FILE *infp;
  int i;
  int j;
  uint16_t s;
  uint16_t id;
  u_int length = 0;
  int skip = 0;
  uint8_t *u;
  uint8_t lbuf[16];
  uint8_t magic[4] = {0x07, 0x20, 0x4d, 0x4d};

  if (QVbattery() <= LOW_BATT) {
    fprintf(stderr, "LOW BATTERY, change battery or connect AC adapter.\n");
    Exit(3);
  }

  infp = stdin;
  if (infilename) {
    infp = fopen(infilename, RMODE);
    if (infp == NULL) {
      fprintf(stderr, "can't open infile(%s).\n", infilename);
      errflg++;
      goto cleanup;
    }
  }

  if (fread(lbuf, sizeof(uint8_t), 4, infp) < 0) {
    fprintf(stderr, "can't read header.\n");
    errflg++;
    goto cleanup;
  }
  if (memcmp(lbuf, magic, sizeof(magic)) != 0) {
    fprintf(stderr, "this file(%s) is not CAM format.\n", infilename);
    errflg++;
    goto cleanup;
  }
  if (fread(lbuf, sizeof(uint8_t), 2, infp) < 0) {
    fprintf(stderr, "can't read area count.\n");
    errflg++;
    goto cleanup;
  }

  s = get_uint16_t(lbuf);
  for (i = 0; i < s; i++) {
    if (fread(lbuf, sizeof(uint8_t), 16, infp) < 0) {
      fprintf(stderr, "can't read area id and length.\n");
      errflg++;
      goto cleanup;
    }

    id = get_uint16_t(lbuf);
    if (id == 3) {
      length = get_u_int(lbuf + 2);
      break;
    }
    skip = skip + (int)get_u_int(lbuf + 2);
  }

  while (skip > 0) {
    i = fread(buf, sizeof(uint8_t), ((skip > 256) ? 256 : skip), infp);
    if (i < 0) {
      fprintf(stderr, "can't skip.\n");
      errflg++;
      goto cleanup;
    }
    skip = skip - i;
  }

  u = buf;
  for (i = 0; i < (int)length; i++) {
    j = fgetc(infp);
    if (j < 0) {
      perror("fgetc");
      errflg++;
      goto cleanup;
    }
    buf[i] = (uint8_t)j;
  }

  len = QVputcam(n, length, buf);

  if (len < 0) {
    errflg++;
  }

cleanup:;
  if (infp != stdin)
    fclose(infp);

  return;
}

void put_hoge(n, infilename) int n;
char *infilename;
{
  int len;
  FILE *infp;
  int i;
  int j;
  uint16_t s;
  uint16_t id;
  u_int length = 0;
  int skip = 0;
  uint8_t *u;
  uint8_t magic[4] = {0xff, 0xd8, 0xff, 0xe0};

  if (QVbattery() <= LOW_BATT) {
    fprintf(stderr, "LOW BATTERY, change battery or connect AC adapter.\n");
    Exit(3);
  }

  infp = stdin;
  if (infilename) {
    infp = fopen(infilename, RMODE);
    if (infp == NULL) {
      fprintf(stderr, "can't open infile(%s).\n", infilename);
      errflg++;
      goto cleanup;
    }
  }

  u = buf;
  length = 0;
  while (1) {
    j = fgetc(infp);
    if (j < 0) {
      fprintf(stderr, "length = %d\n", length);
      /*      perror("fgetc"); */
      goto fooo;
    }
    buf[i] = (uint8_t)j;
    length++;
  }
fooo:
  len = QVputJpegQv7xx(n, length, buf);
  fprintf(stderr, "jpeg7xx = %d\n", length);
  fprintf(stderr, "len = %d\n", len);

  if (len < 0) {
    errflg++;
  }

cleanup:;
  if (infp != stdin)
    fclose(infp);

  return;
}

void skip_space(infp) FILE *infp;
{
  int c;
  c = ' ';
  while (isspace(c)) {
    c = fgetc(infp);
  }
  if (c == EOF)
    return;
  ungetc(c, infp);
}

int fgetline(s, size, infp)
char *s;
int size;
FILE *infp;
{
  int i;
  int c;
fgetretry:
  skip_space(infp);
  for (i = 0; i < size; i++) {
    c = fgetc(infp);
    if (c == EOF)
      return (EOF);
    if (isspace(c)) {
      s[i] = '\0';
      if (c != '\n')
        skip_space(infp);
      return (i);
    } else if (c == '#') {
      while (fgetc(infp) != '\n')
        ;
      s[i] = '\0';
      if (i == 0)
        goto fgetretry;
      return (i);
    } else
      s[i] = c;
  }
  return (i);
}

#define NORM(x)                                                                \
  {                                                                            \
    if (x < 0)                                                                 \
      x = 0;                                                                   \
    else if (x > 255)                                                          \
      x = 255;                                                                 \
  }
void put_ppm(n, infilename, skiphead) int n;
char *infilename;
int skiphead;
{
  int len;
  FILE *infp;

  int x, y;
  uint8_t *Y;
  uint8_t *Cr;
  uint8_t *Cb;
  int r, g, b;
  long L;
  long cr, cb;
  int i, j;

  uint8_t lbuf[256];
  uint8_t magic[2] = {'P', '6'};

  int R[PICTURE_WIDTH / 3];
  int G[PICTURE_WIDTH / 3];
  int B[PICTURE_WIDTH / 3];

  if (QVbattery() <= LOW_BATT) {
    fprintf(stderr, "LOW BATTERY, change battery or connect AC adapter.\n");
    Exit(3);
  }

  infp = stdin;
  if (infilename) {
    infp = fopen(infilename, RMODE);
    if (infp == NULL) {
      fprintf(stderr, "can't open infile(%s).\n", infilename);
      errflg++;
      goto cleanup;
    }
  }

  if (!skiphead) {
    if (fgetline(lbuf, 3, infp) < 0) {
      fprintf(stderr, "can't read header.\n");
      errflg++;
      goto cleanup;
    }

    if (memcmp(lbuf, magic, sizeof(magic)) != 0) {
      fprintf(stderr, "this file(%s) is not PPM(P6) format.\n", infilename);
      errflg++;
      goto cleanup;
    }

    if (fgetline(lbuf, sizeof(lbuf), infp) < 0) {
      fprintf(stderr, "can't read width data.\n");
      errflg++;
      goto cleanup;
    }

    if (strcmp((char *)lbuf, "480") != 0) {
      fprintf(stderr, "this picture's width is not 480.\n");
      errflg++;
      goto cleanup;
    }

    if (fgetline(lbuf, sizeof(lbuf), infp) < 0) {
      fprintf(stderr, "can't read height data.\n");
      errflg++;
      goto cleanup;
    }

    if (strcmp((char *)lbuf, "240") != 0) {
      fprintf(stderr, "this picture's width is not 240.\n");
      errflg++;
      goto cleanup;
    }

    if (fgetline(lbuf, sizeof(lbuf), infp) < 0) {
      fprintf(stderr, "can't read color depth data.\n");
      errflg++;
      goto cleanup;
    }

    if (strcmp((char *)lbuf, "255") != 0) {
      fprintf(stderr, "this picture's depth is not 255.\n");
      errflg++;
      goto cleanup;
    }
  }

  Y = buf;
  Cb = Y + (PICTURE_WIDTH * PICTURE_HEIGHT);
  Cr = Cb + (PICTURE_WIDTH / 3 * PICTURE_HEIGHT / 2);

  memset(R, 0, sizeof(R));
  memset(G, 0, sizeof(G));
  memset(B, 0, sizeof(B));

  j = 0;
  for (y = 0; y < PICTURE_HEIGHT; y++) {
    for (x = 0; x < PICTURE_WIDTH; x++) {
      r = fgetc(infp);
      g = fgetc(infp);
      b = fgetc(infp);
      if ((r < 0) || (g < 0) || (b < 0)) {
        perror("fgetc");
        errflg++;
        goto cleanup;
      }

      L = (r * 29900 + g * 58700 + b * 11400) / 100000;
      NORM(L);
      Y[PICTURE_WIDTH * y + x] = (unsigned char)L;

      R[x / 3] = R[x / 3] + r;
      G[x / 3] = G[x / 3] + g;
      B[x / 3] = B[x / 3] + b;
    }
    if (y % 2 == 1) {
      for (i = 0; i < PICTURE_WIDTH / 3; i++) {
        r = R[i] / 6;
        g = G[i] / 6;
        b = B[i] / 6;

        cb = (50000 * b - 16874 * r - 33126 * g) / 100000;
        if (cb < 0)
          cb = cb + 256;
        NORM(cb);
        cr = (50000 * r - 41869 * g - 8131 * b) / 100000;
        if (cr < 0)
          cr = cr + 256;
        NORM(cr);
        Cb[PICTURE_WIDTH / 3 * j + i] = (unsigned char)cb;
        Cr[PICTURE_WIDTH / 3 * j + i] = (unsigned char)cr;
        R[i] = 0;
        G[i] = 0;
        B[i] = 0;
      }
      j++;
    }
  }

  len = QVputpicture(n,
                     PICTURE_WIDTH * PICTURE_HEIGHT +
                         PICTURE_WIDTH / 3 * PICTURE_HEIGHT / 2 * 2,
                     buf);

  if (len < 0) {
    errflg++;
  }

cleanup:;
  if (infp != stdin)
    fclose(infp);

  return;
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
  char *outfilename = NULL;
  char c;
  qvverbose = 0;
  qvhasvgamode = 0;
  qv7xxprotocol = 0;

#ifndef DONTCAREUID
  uid = getuid();
  euid = geteuid();
  gid = getgid();
  egid = getegid();
  useruid();
#endif

  devpath = getenv("QVPLAYTTY");

  while ((c = getopt(argc, argv, "D:S:F:hvV")) != -1) {
    switch (c) {
    case 'V':
      usage();
      exit(0);
    case 'h':
    case '?':
      usage();
      exit(0);
    case 'v':
      qvverbose = 1;
      break;
    case 'D':
      devpath = optarg;
      break;
    case 'S':
      switch (optarg[0]) {
#if defined(__linux__) || defined(WIN32) || defined(OS2) ||                    \
    defined(__FreeBSD__) || defined(DOS)
      case 'l':
      case '5':
        speed = LIGHT;
        break;
      case 't':
      case '4':
        speed = TOP;
        break;
#endif
      case 'h':
      case '3':
        speed = HIGH;
        break;
      case 'm':
      case '2':
        speed = MID;
        break;
      default:
        speed = DEFAULT;
        break;
      }
      break;
    case 'F':
      switch (optarg[0]) {

      case 'h':
        format = HOGE;
        break;

      case 'C':
      case 'c':
        format = CAM;
        break;
#ifndef USEWORKFILE
      case 'R':
      case 'r':
        format = RGB_P;
        break;
      default:
        format = PPM_P;
        break;
#else
      default:
        format = CAM;
#endif
      }
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

  qvhasvgamode = QVrevision() & 0x01000000;
  if ((QVrevision() & 0x00a00000) == 0x00a00000)
    qv7xxprotocol = 1;
  if (qv7xxprotocol != 0) {
    QVnewprotocol();
    fprintf(stderr, "NEW protocol \n");
  }
  QVchangespeed(speed);
  if (optind == argc) {
    if (all_pic_num < 0)
      Exit(1);
    if (qvhasvgamode) {
      if (QVremain(1) < 0) {
        fprintf(stderr, "picture full.\n");
        errflg++;
        return 1;
      }
      if (QVremain(0) < 0) {
        fprintf(stderr, "picture full.\n");
        errflg++;
        return 1;
      }
    } else {
      if (all_pic_num >= MAX_PICTURE_NUM_QV10) {
        fprintf(stderr, "MEMORY FULL, QV-10 has %d pictures.\n",
                MAX_PICTURE_NUM_QV10);
        Exit(2);
      }
    }
#ifdef BINARYFILEMODE
#ifdef _WIN32
    _setmode(_fileno(stdin), _O_BINARY);
#else
    setmode(fileno(stdin), O_BINARY);
#endif
#endif
    switch (format) {
    case CAM:
      put_cam(all_pic_num + 1, NULL);
      break;
    case RGB_P:
      put_ppm(all_pic_num + 1, NULL, 1);
      break;
    case PPM_P:
    default:
      put_ppm(all_pic_num + 1, NULL, 0);
      break;
    }
  }
  if (optind < argc) {
    while (optind < argc) {
      all_pic_num = QVhowmany();
      if (all_pic_num < 0)
        Exit(1);
      if (qvhasvgamode) {
        if (QVremain(1) < 0) {
          fprintf(stderr, "picture full.\n");
          errflg++;
          return 1;
        }
        if (QVremain(0) < 0) {
          fprintf(stderr, "picture full.\n");
          errflg++;
          return 1;
        }
      } else {
        if (all_pic_num >= MAX_PICTURE_NUM_QV10) {
          fprintf(stderr, "NO MEMORY, QV-10 has %d pictures.\n",
                  MAX_PICTURE_NUM_QV10);
          Exit(2);
        }
      }
      switch (format) {

      case HOGE:
        put_hoge(all_pic_num + 1, argv[optind++]);
        break;

      case CAM:
        put_cam(all_pic_num + 1, argv[optind++]);
        break;
      case RGB_P:
        put_ppm(all_pic_num + 1, argv[optind++], 1);
        break;
      case PPM_P:
      default:
        put_ppm(all_pic_num + 1, argv[optind++], 0);
        break;
      }
      QVshowpicture(all_pic_num + 1);
    }
  }

  Exit(errflg ? 1 : 0);
}
