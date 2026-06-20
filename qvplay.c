#include "config.h"
#include "version.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef BINARYFILEMODE
#include <fcntl.h> /* for setmode() */
#include <io.h>
#endif
#include <stdlib.h>
#include <time.h>
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
#include "common.h"

#ifdef _WIN32
#include "win32/getopt.h"
#include "win32/tty_w32.h"
#else
#include <getopt.h>
#include "tty.h"
#endif
#include "jpeg.h"

extern int optind, opterr;
extern char *optarg;

#define OUTFILENAME_CAM "%s_%03d.cam"
#define OUTFILENAME_CAM0 "qv_%03d.cam"
#define THUMBNAIL_YCC_SIZE                                                     \
  (THUMBNAIL_WIDTH * THUMBNAIL_HEIGHT * 3 / 2)

static int speed = 0;
extern int qvverbose;
extern int qvhasvgamode;
extern int qv7xxprotocol;

static int errflg = 0;
static int all_pic_num = -1;

void version(void) {
  static char *usagestr[] = {
      QVPLAY_USAGE_HEADER,
      (char *)NULL,
  };
  char **p;

  p = usagestr;
  while (*p)
    fputs(*p++, stdout);
}

void usage(void) {
  static char *usagestr[] = {
      QVPLAY_USAGE_HEADER,
      "\t -h             : show this usage.\n",
      "\t -n             : print how many pictures in QV.\n",
      "\t -p num         : show picture on LCD.\n",
      "\t -i num         : show information about a picture\n",
      "\t -I             : show informations about all picture\n",
      "\t -4 n1,n2,n3,n4 : show 4 picures on LCD.\n",
      "\t -9 n1,n2,...,n9: show 9 picures on LCD.\n",
      "\t -7             : show color bar on LCD.\n",
      "\t -o filename    : set output filename.\n",
      "\t -g num         : get a picture in CAM format.\n",
      "\t -a             : get all pictures in CAM format.\n",
      "\t -s num         : start picture number.(use with -a or -I)\n",
      "\t -e num         : end picture number.(use with -a or -I)\n",
      "\t -v             : verbose mode(use with -a or -g)\n",
      "\t -r             : reset QV.\n",
      "\t -d num         : delete picture in QV.\n",
      "\t -t             : take a picture.(QV-700/770 cannot use)\n",
      "\t -P num         : protect a picture.\n",
      "\t -U num         : unprotect a picture.\n",
      "\t -b             : report battery status. (roughly)(QV-700/770 not "
      "work)\n",
      "\t -0             : power off QV.\n",
      "\t -1             : disable auto power off function.\n",
      "\t -z             : report QV status\n",
      "\t -Z             : report QV revision?\n",
#if defined(__linux__) || defined(_WIN32) || defined(__FreeBSD__)
      "\t -S speed       : serial speed. [normal middle high top light]\n",
#else
      "\t -S speed       : serial speed. [normal middle high]\n",
#endif
      "\t -V             : show version information.\n",
      "\t -D ttydevice   : set tty(cua) device.\n",
      (char *)NULL,
  };
  char **p;

  p = usagestr;
  while (*p)
    fputs(*p++, stdout);
}

void Exit(int code) {
  if (QVgetfd() >= 0) {
    closetty(QVgetfd());
    QVsetfd(-1);
  }
  exit(code);
}

#ifdef USEWORKFILE
#define WORKFILE "qvwork.dat"
#endif

static long clamp_channel(long value) {
  if (value < 0)
    return 0;
  if (value > 255)
    return 255;
  return value;
}

static int write_cam_thumbnail(const uint8_t *buf, FILE *outfp) {
  const uint8_t *y_plane = buf;
  const uint8_t *cb_plane = y_plane + THUMBNAIL_WIDTH * THUMBNAIL_HEIGHT;
  const uint8_t *cr_plane =
      cb_plane + (THUMBNAIL_WIDTH / 2) * (THUMBNAIL_HEIGHT / 2);
  int written = 0;
  int y;

  for (y = 0; y < THUMBNAIL_HEIGHT; y++) {
    int x;
    for (x = 0; x < THUMBNAIL_WIDTH; x++) {
      const int chroma_index =
          (y / 2) * (THUMBNAIL_WIDTH / 2) + (x / 2);
      long cb = cb_plane[chroma_index];
      long cr = cr_plane[chroma_index];
      const long luminance = y_plane[y * THUMBNAIL_WIDTH + x] * 100000L;
      long red;
      long green;
      long blue;

      if (cb > 127)
        cb -= 256;
      if (cr > 127)
        cr -= 256;

      red = clamp_channel((luminance + 140200L * cr) / 100000L);
      green =
          clamp_channel((luminance - 34414L * cb - 71414L * cr) / 100000L);
      blue = clamp_channel((luminance + 177200L * cb) / 100000L);

      if (fputc((int)red, outfp) == EOF ||
          fputc((int)green, outfp) == EOF ||
          fputc((int)blue, outfp) == EOF) {
        perror("write CAM thumbnail");
        return -1;
      }
      written += 3;
    }
  }
  return written;
}

void get_camfile(int n, char *outfilename) {
  long len;
  long lenj;
  size_t jpeg_area_size;
  char buf1[64];
  char buf5[64];
  uint8_t *bufj;
  uint8_t *bufp;
  size_t bufj_capacity = 0;
  FILE *outfp;
  int i;
  time_t tt;
  int vga = 0;
#ifdef USEWORKFILE
  FILE *fp;
#endif

  if (all_pic_num < n) {
    fprintf(stderr, "picture number is too large.\n");
    errflg++;
    return;
  }

  if (QVpicattr(n) & 0x02) {
    vga = 1;
    if (qv7xxprotocol != 0)
      vga = 2;
  }

  bufj = NULL;
  bufp = (uint8_t *)malloc(THUMBNAIL_MAXSIZ);
  if (bufp == (uint8_t *)NULL) {
    fprintf(stderr, "can't alloc\n");
    errflg++;
    return;
  }

#ifdef USEWORKFILE
  if (vga) {
    fp = fopen(WORKFILE, WMODE);
    if (fp == NULL) {
      fprintf(stderr, "can't open workfile(%s).\n", WORKFILE);
      errflg++;
      free(bufp);
      return;
    }
  } else {
    bufj_capacity = JPEG_MAXSIZ;
    bufj = (uint8_t *)malloc(JPEG_MAXSIZ);
    if (bufj == (uint8_t *)NULL) {
      fprintf(stderr, "can't alloc\n");
      errflg++;
      free(bufp);
      return;
    }
  }
#else
  if (vga == 1) {
    bufj_capacity = JPEG_MAXSIZ_VGA;
    bufj = (uint8_t *)malloc(JPEG_MAXSIZ_VGA);
  } else if (vga == 2) {
    int filesize;
    filesize = QVgetsize2(n);
    if (filesize <= 0) {
      errflg++;
      free(bufp);
      return;
    }
    bufj_capacity = (size_t)filesize;
    bufj = (uint8_t *)malloc(filesize);
  } else {
    bufj_capacity = JPEG_MAXSIZ;
    bufj = (uint8_t *)malloc(JPEG_MAXSIZ);
  }
  if (bufj == (uint8_t *)NULL) {
    fprintf(stderr, "can't alloc\n");
    errflg++;
    free(bufp);
    return;
  }
#endif

  outfp = stdout;
  if (outfilename) {
    outfp = fopen(outfilename, WMODE);
    if (outfp == NULL) {
      fprintf(stderr, "can't open outfile(%s).\n", outfilename);
      errflg++;
      goto cleanup0;
    }
  }
#ifdef BINARYFILEMODE
  if (outfp == stdout)
    _setmode(_fileno(stdout), _O_BINARY);
#endif

  fputc(0x07, outfp); /* CAM header */
  fputc(0x20, outfp);
  fputc(0x4d, outfp);
  fputc(0x4d, outfp);

  fputc(0x00, outfp);
  fputc(0x03, outfp);

  fputc(0x00, outfp); /* comment area  size */
  fputc(0x01, outfp);
  sprintf(buf1, "Generated by qvplay-0.93");
  tt = time(0);
  sprintf(buf5, "%s", asctime(localtime(&tt)));
  len = strlen(buf5) - 1;
  buf5[len] = '\0';
  len = len + strlen(buf1);

  len = len + 14 + 1; /* (ID + 00) * 7  + 00 */
  fputc((len >> 24) & 0xff, outfp);
  fputc((len >> 16) & 0xff, outfp);
  fputc((len >> 8) & 0xff, outfp);
  fputc(len & 0xff, outfp);
  for (i = 0; i < 10; i++)
    fputc(0x00, outfp); /* dummy */

  fputc(0x00, outfp); /* thumbnail area  size */
  fputc(0x02, outfp);
  fputc(0x00, outfp);
  fputc(0x00, outfp);
  fputc(0x15, outfp);
  fputc(0xf0, outfp); /* 5616 = 3 * 52 * 36 */
  for (i = 0; i < 10; i++)
    fputc(0x00, outfp); /* dummy */

  fputc(0x00, outfp); /* jpeg area  size */
  if (vga)
    fputc(0x04, outfp);
  else
    fputc(0x03, outfp);

#ifdef USEWORKFILE
  if (vga) {
    lenj = QVgetpicture(n, NULL, 0, QV_DATA_JPEG, vga, fp);
    fclose(fp);
  } else
    lenj = QVgetpicture(n, bufj, bufj_capacity, QV_DATA_JPEG, vga, NULL);
#else
  lenj = QVgetpicture(n, bufj, bufj_capacity, QV_DATA_JPEG, vga, NULL);
#endif
  if (lenj < 0) {
    errflg++;
    goto cleanup;
  }
  jpeg_area_size = (size_t)lenj;
  if (vga == 1)
    jpeg_area_size = jpeg_fine_output_size(jpeg_area_size);
  if (jpeg_area_size == 0 || jpeg_area_size > UINT32_MAX) {
    fprintf(stderr, "converted JPEG size is invalid.\n");
    errflg++;
    goto cleanup;
  }
  fputc((jpeg_area_size >> 24) & 0xff, outfp);
  fputc((jpeg_area_size >> 16) & 0xff, outfp);
  fputc((jpeg_area_size >> 8) & 0xff, outfp);
  fputc(jpeg_area_size & 0xff, outfp);

  if (vga) {
    fputc(0xf0, outfp);
    fputc(0x77, outfp);
    fputc(0xff, outfp);
    fputc(0xff, outfp);
    fputc(0xff, outfp);
    fputc(0xff, outfp);
    fputc(0x10, outfp);
    fputc(0xf8, outfp);
    fputc(0x03, outfp);
    fputc(0x00, outfp);
  } else
    for (i = 0; i < 10; i++)
      fputc(0x00, outfp); /* dummy */

  /* comment  */
  fputc(0x01, outfp);
  if (write_file(buf1, strlen(buf1) + 1, outfp) == -1) {
    errflg++;
    goto cleanup;
  }
  fputc(0x02, outfp);
  fputc(0x00, outfp);
  fputc(0x03, outfp);
  fputc(0x00, outfp);
  fputc(0x04, outfp);
  fputc(0x00, outfp);
  fputc(0x05, outfp);
  if (write_file(buf5, strlen(buf5) + 1, outfp) == -1) {
    errflg++;
    goto cleanup;
  }

  fputc(0x06, outfp);
  fputc(0x00, outfp);
  fputc(0x07, outfp);
  fputc(0x00, outfp);

  fputc(0x00, outfp);

  len = QVgetpicture(n, bufp, THUMBNAIL_MAXSIZ, QV_DATA_THUMBNAIL, vga, NULL);
  if (len < 0) {
    errflg++;
    goto cleanup;
  }
  if (len < THUMBNAIL_YCC_SIZE) {
    fprintf(stderr, "thumbnail data is too short.\n");
    errflg++;
    goto cleanup;
  }
  if (write_cam_thumbnail(bufp, outfp) == -1) {
    errflg++;
    goto cleanup;
  }
  if (vga == 1) {
#ifdef USEWORKFILE
    if (write_jpeg_fine(WORKFILE, outfp) == -1) {
      errflg++;
      goto cleanup;
    };
    unlink(WORKFILE);
#else
    if (write_jpeg_fine(bufj, (size_t)lenj, outfp) == -1) {
      errflg++;
      goto cleanup;
    }
#endif
  } else if (vga == 2) {
    if (write_file(bufj, lenj, outfp) == -1) {
      errflg++;
      goto cleanup;
    }
  } else {
    if (write_file(bufj, lenj, outfp) == -1) {
      errflg++;
      goto cleanup;
    }
  }
cleanup:;
  if (outfp != stdout)
    fclose(outfp);
cleanup0:;
  free(bufp);
  if (bufj)
    free(bufj);
}

void show_picture(int n) {
  int m = n;
  if (n < 1)
    m = 1;
  if (all_pic_num < n)
    m = all_pic_num;

  if (QVshowpicture(m) < 0)
    errflg++;
}

void default_picture(int n) {
  int m = n;
  if (n < 1)
    m = 1;
  if (all_pic_num < n)
    m = all_pic_num;

  if (QVdefaultpicture(m) < 0)
    errflg++;
}

void get_all_pictures(int start, int end, char *outfilename) {
  int i;
  char fname[MAXPATHLEN];

  if (all_pic_num < start || all_pic_num < end) {
    fprintf(stderr, "picture number is too large.\n");
    errflg++;
    return;
  }
  if (start > end) {
    int tmp = end;
    end = start;
    start = tmp;
  }

  for (i = start; i <= end; i++) {
    int result;

    if (outfilename)
      result = snprintf(fname, sizeof(fname), OUTFILENAME_CAM, outfilename, i);
    else
      result = snprintf(fname, sizeof(fname), OUTFILENAME_CAM0, i);
    if (result < 0 || (size_t)result >= sizeof(fname)) {
      fprintf(stderr, "output filename is too long.\n");
      errflg++;
      return;
    }
    get_camfile(i, fname);
  }
}
void delete_picture(int n) {
  if (all_pic_num < n) {
    fprintf(stderr, "picture number is too large.\n");
    errflg++;
    return;
  }

  if (QVdeletepicture(n) < 0)
    errflg++;
  all_pic_num = -1; /*need update*/
}

void show_4_pictures(char *str) {
  int pictureNo[4];
  int i;
  for (i = 0; i < 4; i++)
    pictureNo[i] = 0;
  if ((i = sscanf(str, "%d,%d,%d,%d", &pictureNo[0], &pictureNo[1],
                  &pictureNo[2], &pictureNo[3])) < 1)
    return;
  ;
  for (; i < 4; i++)
    pictureNo[i] = pictureNo[i - 1] + 1;
  if (pictureNo[0] > all_pic_num)
    pictureNo[0] = all_pic_num;
  for (i = 0; i < 4; i++)
    if (pictureNo[i] > all_pic_num)
      pictureNo[i] = pictureNo[i - 1];
  QV4split(pictureNo);
}

void show_9_pictures(char *str) {
  int pictureNo[9];
  int i;
  for (i = 0; i < 9; i++)
    pictureNo[i] = 0;
  if ((i = sscanf(str, "%d,%d,%d,%d,%d,%d,%d,%d,%d", &pictureNo[0],
                  &pictureNo[1], &pictureNo[2], &pictureNo[3], &pictureNo[4],
                  &pictureNo[5], &pictureNo[6], &pictureNo[7], &pictureNo[8])) <
      1)
    return;
  for (; i < 9; i++)
    pictureNo[i] = pictureNo[i - 1] + 1;
  if (pictureNo[0] > all_pic_num)
    pictureNo[0] = all_pic_num;
  for (i = 0; i < 9; i++)
    if (pictureNo[i] > all_pic_num)
      pictureNo[i] = pictureNo[i - 1];
  QV9split(pictureNo);
}

void protect_picture(int n) {
  if (n < 1)
    return;
  if (n > all_pic_num) {
    fprintf(stderr, "picture number is too large.\n");
    errflg++;
    return;
  }
  QVprotect(n, 1);
}

void unprotect_picture(int n) {
  if (n < 1)
    return;
  if (n > all_pic_num) {
    fprintf(stderr, "picture number is too large.\n");
    errflg++;
    return;
  }
  QVprotect(n, 0);
}

void picture_information(int n) {
  int i;
  if (n < 1)
    return;
  if (n > all_pic_num) {
    fprintf(stderr, "picture number is too large.\n");
    errflg++;
    return;
  }
  i = QVpicattr(n);
  printf("Picture %3d = ", n);
  if (0x01 & i)
    printf("protect on");
  else
    printf("protect off");
  if (0x02 & i)
    printf(" 640x480 %02x\n", i);
  else
    printf(" 480x240\n");
}

void take_picture(void) {
  if (qv7xxprotocol != 0) {
    fprintf(stderr, "QV-700/770 cannot take picture by remote command.\n");
    errflg++;
    return;
  }
  if (QVswstat() & 0x0040) {
    fprintf(stderr, "set mode SW to REC position.\n");
    errflg++;
    return;
  }

  if (qvhasvgamode) {
    if (QVremain(1) < 0) {
      fprintf(stderr, "picture full.\n");
      errflg++;
      return;
    }
    if (QVremain(0) < 0) {
      fprintf(stderr, "picture full.\n");
      errflg++;
      return;
    }
  } else if (MAX_PICTURE_NUM_QV10 <= all_pic_num) {
    fprintf(stderr, "picture full.\n");
    errflg++;
    return;
  }

  if (QVtakepicture() < 0)
    errflg++;
  all_pic_num = QVhowmany();
}

void print_swstat(int stat) {
  if (qv7xxprotocol != 0) {
    fprintf(stderr, "QV-700/770 cannot report status.\n");
    return;
  }
  if (qvhasvgamode)
    printf("QV-100/300/200 status is...\n");
  else
    printf("QV-10/10A/30/11/70 status is...\n");
  if (0x0080 & stat)
    printf("CCD unit is reverse angle.\n");
  else
    printf("CCD unit is normal angle.\n");
  if (0x0040 & stat)
    printf("REC/PLAY switch is PLAY position.\n");
  else
    printf("REC/PLAY switch is REC position.\n");
  if (0x8000 & stat)
    printf("too bright sign is shown on LCD.\n");
  if (0x4000 & stat)
    printf("too dark sign is shown on LCD.\n");
  if (0x0800 & stat)
    printf("[-] button is pressed.\n");
  if (0x0400 & stat)
    printf("[+] button is pressed.\n");
  if (0x0020 & stat)
    printf("PROTECT button is pressed.\n");
  if (0x0010 & stat)
    printf("DEL button is pressed.\n");
  if (0x0008 & stat)
    printf("DISP button is pressed.\n");
  if (0x0004 & stat)
    printf("MODE button is pressed.\n");
  if (0x0002 & stat)
    printf("ZOOM button is pressed.\n");
  if (0x0001 & stat)
    printf("Shutter button is pressed.\n");
}

static int option_requires_camera(int option) {
  switch (option) {
  case 'D':
  case 'V':
  case '?':
  case 'e':
  case 'h':
  case 'o':
  case 's':
  case 'v':
    return 0;
  default:
    return 1;
  }
}

static int ensure_camera_connected(char *devpath, int *start_picture,
                                   int *end_picture) {
  long revision;

  if (QVgetfd() >= 0)
    return 1;
  QVsetfd(opentty(devpath));
  if (QVgetfd() < 0)
    return 0;

  all_pic_num = QVhowmany();
  if (all_pic_num < 0)
    goto fail;
  if (*end_picture == 0 || *end_picture > all_pic_num)
    *end_picture = all_pic_num;
  if (*start_picture == 0)
    *start_picture = 1;

  revision = QVrevision();
  if (revision < 0)
    goto fail;
  qvhasvgamode = revision & 0x01000000;
  qv7xxprotocol = (revision & 0x00a00000) == 0x00a00000;
  if (qv7xxprotocol != 0 && QVnewprotocol() < 0)
    goto fail;
  if (QVbattery() <= LOW_BATT) {
    fprintf(stderr, "LOW BATTERY, change battery or connect AC adapter.\n");
    goto fail;
  }
  if (QVsectorsize(SECTOR) < 0)
    goto fail;
  return 1;

fail:
  closetty(QVgetfd());
  QVsetfd(-1);
  return 0;
}

int main(int argc, char **argv)

{
  char *devpath = NULL;
  char *outfilename = NULL;
  int start_picture = 0;
  int end_picture = 0;
  int move_from = 0;
  int move_to = 0;
  int c;
  int i, j;

  uint8_t hoge[12]; /* extdatatest */

  qvverbose = 0;
  qvhasvgamode = 0;
  qv7xxprotocol = 0;

  devpath = getenv("QVPLAYTTY");
  if (devpath == NULL)
    devpath = RSPORT;

  while ((c = getopt(argc, argv,
                     "D:p:o:g:rRnNas:e:d:tvS:4:9:P:U:10b7i:IzZy:Y:hV")) !=
         EOF) {
    if (c == 'V') {
      version();
      exit(0);
    }
    if (c == 'h') {
      usage();
      exit(0);
    }
    if (option_requires_camera(c) &&
        !ensure_camera_connected(devpath, &start_picture, &end_picture))
      Exit(1);

    switch (c) {
    case 'p':
      show_picture(atoi(optarg));
      break;
    case 'o':
      outfilename = optarg;
      break;
    case 'g':
      get_camfile(atoi(optarg), outfilename);
      break;
    case 'r':
      QVchangespeed(DEFAULT);
      QVreset(1);
      break;
    case 'R':
      QVchangespeed(DEFAULT);
      QVreset(0);
      break;
    case 'n':
      printf("pictures = %d\n", all_pic_num);
      break;
    case 'N':
      if (qvhasvgamode == 0)
        printf("remain = %d\n", QVremain(0));
      else {
        printf("remain(normal) = %d\n", QVremain(0));
        printf("remain(fine) = %d\n", QVremain(1));
      }
      break;
    case 'a':
      get_all_pictures(start_picture, end_picture, outfilename);
      break;
    case 's':
      start_picture = atoi(optarg);
      break;
    case 'e':
      end_picture = atoi(optarg);
      break;
    case 'v':
      qvverbose = 1;
      break;
    case 'd':
      delete_picture(atoi(optarg));
      break;
    case 't':
      take_picture();
      break;
    case '4':
      show_4_pictures(optarg);
      break;
    case '9':
      show_9_pictures(optarg);
      break;
    case 'P':
      protect_picture(atoi(optarg));
      break;
    case 'U':
      unprotect_picture(atoi(optarg));
      break;
    case '0':
      QVpoweroff();
      /* the camera is now off; we can't reset it.  This is known to
       * work under Linux, it needs testing under other OSes.  It should
       * not cause problems under other unicies; they automatically
       * close file descriptors on process exit. */
      return 0;
      break;
    case '1':
      QVdisableAutoPowerOff();
      break;
    case 'b':
      printf("battery = %.2f V\n", (float)QVbattery() / 16);
      break;
    case 'H': /* not used */
      QVhidepicnum();
      break;
    case '7':
      QVcolorpattern();
      break;
    case 'i':
      picture_information(atoi(optarg));
      break;
    case 'I':
      for (i = start_picture; i <= end_picture; i++)
        picture_information(i);
      break;
    case 'z':
      print_swstat(QVswstat());
      break;
    case 'Z':
      i = QVrevision();
      fprintf(stderr, "revision = 0x%08x\n", i);
      break;
    case 'y':
      default_picture(atoi(optarg));
      break;
    case 'Y':
      j = QVgetextdata(atoi(optarg), hoge, sizeof(hoge));
      fprintf(stderr, "ext ");
      for (i = 0; i < j; i++) {
        fprintf(stderr, "%02x ", hoge[i]);
      }
      fprintf(stderr, "\n");
      break;
    case 'M': /* don't use */
      /* QVmovepicture is very dangerous */
      if (sscanf(optarg, "%d,%d", &move_from, &move_to) != 2)
        break;
      if (move_from > all_pic_num)
        break;
      if (move_from < 1)
        break;
      if (move_to > all_pic_num)
        break;
      if (move_to < 1)
        break;
      QVmovepicture(move_from, move_to);
      break;
    case 'S':
      switch (optarg[0]) {
#if defined(__linux__) || defined(_WIN32) || defined(__FreeBSD__)
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
      QVchangespeed(speed);
      break;
    case 'D':
      if (QVgetfd() >= 0) {
        QVchangespeed(DEFAULT);
        closetty(QVgetfd());
        QVsetfd(-1);
        all_pic_num = -1;
      }
      devpath = optarg;
      break;
    default:
      usage();
      Exit(1);
      return 1; /* dummy */
    }
  }

  if (QVgetfd() >= 0 && QVchangespeed(DEFAULT) < 0)
    errflg++;
  Exit(errflg ? 1 : 0);
}
