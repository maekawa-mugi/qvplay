#include "config.h"
#include <stdio.h>
#include <string.h>
#ifdef BINARYFILEMODE
#include <fcntl.h>  /* for setmode() */
#endif
#include <stdlib.h>
#include <time.h>
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
#include "command1.h"
#ifdef X68KK
#include "x68k/tty_x68.h"
#else
#ifdef _WIN32
#include "win32/tty_w32.h"
#include "win32/getopt/getopt.h"
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
#include "jpeg.h"
#include "ppm.h"
#include "bmp.h"
#include "common.h"


extern int	optind, opterr;
extern char	*optarg;

#define OUTFILENAME_JPG "%s_%03d.jpg"
#define OUTFILENAME_JPG0 "qv_%03d.jpg"

#define OUTFILENAME_PPM "%s_%03d.ppm"
#define OUTFILENAME_PPM0 "qv_%03d.ppm"

#define OUTFILENAME_CAM "%s_%03d.cam"
#define OUTFILENAME_CAM0 "qv_%03d.cam"

#define OUTFILENAME_RGB "%s_%03d.rgb"
#define OUTFILENAME_RGB0 "qv_%03d.rgb"

#define OUTFILENAME_BMP "%s_%03d.bmp"
#define OUTFILENAME_BMP0 "qv_%03d.bmp"

static int     format = JPEG;

static int	raw_data = 0;
static int     speed = 0;
extern int	qvverbose;
extern int qvhasvgamode;
extern int qv7xxprotocol;

static	int	errflg = 0;
static	int	all_pic_num = -1;
#ifndef DONTCAREUID
static	uid_t	uid, euid;
static	gid_t	gid, egid;
static	int	uidswapped = 0;
#endif

void usage()
{
  static	char	*usagestr[] =  {
    "qvplay (Ver 0.95) (c)1996-2000 ken-ichi HAYASHI, itojun\n",
    "\t -h             : show this usage.\n",
    "\t -n             : print how many pictures in QV.\n",
    "\t -p num         : show picture on LCD.\n",
    "\t -i num         : show information about a picture\n",
    "\t -I             : show informations about all picture\n",
    "\t -4 n1,n2,n3,n4 : show 4 picures on LCD.\n",
    "\t -9 n1,n2,...,n9: show 9 picures on LCD.\n",
    "\t -7             : show color bar on LCD.\n",
    "\t -o filename    : set output filename.\n",
    "\t -g num         : get a picture data in QV.\n",
    "\t -a             : get all picture data in QV.\n",
#ifdef USEWORKFILE
    "\t -F format      : picture format.[jpeg ppm rgb bmp cam]\n",
#else
    "\t -F format      : picture format.[jpeg ppm PPM rgb RGB bmp BMP cam]\n",
#endif
    "                    (use with -a or -g).\n",
    "\t -s num         : start picture number.(use with -a or -I)\n",
    "\t -e num         : end picture number.(use with -a or -I)\n",
    "\t -v             : verbose mode(use with -a or -g)\n",
    "\t -r             : reset QV.\n",
    "\t -d num         : delete picture in QV.\n",
    "\t -t             : take a picture.(QV-700/770 cannot use)\n",
    "\t -P num         : protect a picture.\n",
    "\t -U num         : unprotect a picture.\n",
    "\t -V             : report battery status. (roughly)(QV-700/770 not work)\n",
    "\t -0             : power off QV.\n",
    "\t -1             : disable auto power off function.\n",
    "\t -z             : report QV status\n",
    "\t -Z             : report QV revision?\n",
#if defined(__linux__) || defined(WIN32) || defined(OS2) || defined(__FreeBSD__) || defined(DOS)
    "\t -S speed       : serial speed. [normal middle high top light]\n",
#else
    "\t -S speed       : serial speed. [normal middle high]\n",
#endif
#ifndef X68
    "\t -D ttydevice   : set tty(cua) device.\n",
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

  if (!(QVgetfd() < 0))
    QVchangespeed(DEFAULT);
  closetty(QVgetfd());
  exit(code);
} 

#ifdef USEWORKFILE
#define WORKFILE "qvwork.dat"

void
get_picture(n, outfilename, format)
     int	n;
     char *outfilename;
     int  format;
{
  int	len;
  u_char	*buf = NULL;
  FILE	*outfp;
  FILE *fp;
  int vga = 0;

  if (all_pic_num < n) {
    fprintf(stderr, "picture number is too large.\n");
    errflg ++;
    return;
  }
  
  if(QVpicattr(n) & 0x02){
    vga = 1;
    if(qv7xxprotocol != 0)
      vga = 2;
  }

  switch(format){
  case JPEG:
    if(vga == 0)
      buf = (u_char *)malloc(JPEG_MAXSIZ);
    break;
  case PPM_T:
  case RGB_T:
  case BMP_T:
    buf = (u_char *)malloc(THUMBNAIL_MAXSIZ);
    break;
  case PPM_P:
  case RGB_P:
  case BMP_P:
  default:
    buf = NULL;
    break;
  }

  outfp = stdout;
  if (outfilename) {
    outfp = fopen(outfilename, WMODE);
    if (outfp == NULL){
      fprintf(stderr, "can't open outfile(%s).\n", outfilename);
      errflg ++;
      goto cleanup0;
    }
  }
#ifdef BINARYFILEMODE
  if(outfp == stdout){
#ifdef _WIN32
    _setmode(_fileno(stdout), _O_BINARY);
#else
    setmode(fileno(stdout), O_BINARY); /* X680x0 DOS */
#endif
  }
#endif  

  if(buf != NULL)
    len = QVgetpicture(n, buf, format, vga, NULL);
  else{
    fp = fopen(WORKFILE, WMODE);
    if(fp == NULL){
      fprintf(stderr, "can't open workfile(%s).\n", WORKFILE);
      errflg ++;
      goto cleanup0;
    }
    len = QVgetpicture(n, buf, format, vga, fp);
    fclose(fp);
  }

  if (len < 0) {
    errflg ++;
    goto cleanup;
  }

  if (raw_data){
    if(buf != NULL){
      if(write_file(buf, len, outfp) == -1){
	errflg ++;
	goto cleanup;
      }
    }else{
      if(write_file_file(WORKFILE, len, 0, outfp) == -1){
	errflg ++;
	goto cleanup;
      }
      unlink(WORKFILE);
    }
  }else 
    switch(format){
    case PPM_T:
      if(write_ppm(buf, outfp,
		THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT,
		2, 2,
		1, 0) == -1){
	errflg ++;
	goto cleanup;
      }
      break;
    case PPM_P:
      fprintf(stderr, "sorry. PPM format is not supported.\n");
      break;
    case RGB_T:
      if(write_ppm(buf, outfp,
		THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT,
		2, 2,
		0, 0) == -1){
	errflg ++;
	goto cleanup;
      }
      break;
    case RGB_P:
      fprintf(stderr, "sorry. RGB format is not supported.\n");
      break;
    case BMP_T:
      if(write_bmp(buf, outfp,
		THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT,
		2, 2) == -1){
	errflg ++;
	goto cleanup;
      }
      break;
    case BMP_P:
      fprintf(stderr, "sorry. RGB format is not supported.\n");
      break;
    case JPEG:
    default:
      if(vga == 1){
	if(write_jpeg_fine(WORKFILE, outfp) == -1){
	  errflg ++;
	  goto cleanup;
	}
	unlink(WORKFILE);
      }else if(vga == 2){
	if(write_file_file(WORKFILE, len, 0, outfp) == -1){
	  errflg ++;
	  goto cleanup;
	}
	unlink(WORKFILE);
      }else
	if(write_jpeg(buf, outfp) == -1){
	  errflg ++;
	  goto cleanup;
	}
      break;
    }
cleanup:;
  if (outfp != stdout)
    fclose(outfp);
cleanup0:;
  if(buf)
    free(buf);
}
#else  /* not defined USEWORLFILE */
void
get_picture(n, outfilename, format)
     int	n;
     char *outfilename;
     int  format;
{
  int	len;
  u_char	*buf;
  FILE	*outfp;
  int vga = 0;

  buf = NULL;

  if (all_pic_num < n) {
    fprintf(stderr, "picture number is too large.\n");
    errflg ++;
    return;
  }

  if(QVpicattr(n) & 0x02){
    vga = 1;
    if(qv7xxprotocol != 0)
      vga =2;
  }

  switch(format){
  case JPEG:
    if(vga == 1)
      buf = (u_char *)malloc(JPEG_MAXSIZ_VGA);
    else if (vga == 2){
      int filesize;
      filesize = QVgetsize2(n);
      if( filesize < 0)
	return;
      buf = (u_char *)malloc(filesize);
    }else
      buf = (u_char *)malloc(JPEG_MAXSIZ);
    break;
  case PPM_T:
  case RGB_T:
  case BMP_T:
    buf = (u_char *)malloc(THUMBNAIL_MAXSIZ);
    break;
  case PPM_P:
  case RGB_P:
  case BMP_P:
  default:
    if(vga)
      buf = (u_char *)malloc(YCC_MAXSIZ_VGA);
    else
      buf = (u_char *)malloc(YCC_MAXSIZ);
    break;
  }

  if (buf == (u_char *)NULL) {
    fprintf(stderr, "can't alloc\n");
    errflg ++;
    return;
  }
  outfp = stdout;
  if (outfilename) {
    outfp = fopen(outfilename, WMODE);
    if (outfp == NULL){
      fprintf(stderr, "can't open outfile(%s).\n", outfilename);
      errflg ++;
      goto cleanup0;
    }
  }
#ifdef BINARYFILEMODE
  if(outfp == stdout){
#ifdef _WIN32
    _setmode(_fileno(stdout), _O_BINARY);
#else
    setmode(fileno(stdout), O_BINARY); /* X680x0 */
#endif
  }
#endif  


  len = QVgetpicture(n, buf, format, vga, NULL);
  if (len < 0) {
    errflg ++;
    goto cleanup;
  }

  if (raw_data){
    if(write_file(buf, len, outfp) == -1){
      errflg ++;
      goto cleanup;
    }
  }else {
    switch(format){
    case PPM_T:
      if(write_ppm(buf, outfp,
		THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT,
		2, 2,
		1, 0) == -1){
	errflg ++;
	goto cleanup;
      }
      break;
    case PPM_P:
      if(vga){
	if(write_ppm(buf, outfp, 
		  PICTURE_WIDTH_FINE, PICTURE_HEIGHT_FINE,
		  2, 2,
		  1, 0) == -1){
	  errflg ++;
	  goto cleanup;
	}
      }else{
	if(write_ppm(buf, outfp, 
		  PICTURE_WIDTH, PICTURE_HEIGHT,
		  3, 2,
		  1, 0) == -1){
	  errflg ++;
	  goto cleanup;
	}
      }
      break;
    case RGB_T:
      if(write_ppm(buf, outfp,
		THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT,
		2, 2,
		   0, 0) == -1){
	errflg ++;
	goto cleanup;
      }
      break;
    case RGB_P:
      if(vga){
	if(write_ppm(buf, outfp, 
		  PICTURE_WIDTH_FINE, PICTURE_HEIGHT_FINE,
		  2, 2,
		  0, 0) == -1){
	  errflg ++;
	  goto cleanup;
	}
      }else{
	if(write_ppm(buf, outfp, 
		  PICTURE_WIDTH, PICTURE_HEIGHT,
		  3, 2,
		  0, 0) == -1){
	  errflg ++;
	  goto cleanup;
	}
      }
      break;
    case BMP_T:
      if(write_bmp(buf, outfp,
		THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT,
		2, 2) == -1){
	errflg ++;
	goto cleanup;
      }
      break;
    case BMP_P:
      if(vga){
	if(write_bmp(buf, outfp, 
		  PICTURE_WIDTH_FINE, PICTURE_HEIGHT_FINE,
		     2, 2) == -1){
	  errflg ++;
	  goto cleanup;
	}
      }else{
	if(write_bmp(buf, outfp, 
		  PICTURE_WIDTH, PICTURE_HEIGHT,
		  3, 2) == -1){
	  errflg ++;
	  goto cleanup;
	}
      }
      break;
    case JPEG:
    default:
      if(vga == 1){
	if(write_jpeg_fine(buf, outfp) == -1){
	  errflg ++;
	  goto cleanup;
	}
      }else if(vga == 2){
	if(write_file(buf, len, outfp) == -1){
	  errflg ++;
	  goto cleanup;
	}
      }else{
	if(write_jpeg(buf, outfp) == -1){
	  errflg ++;
	  goto cleanup;
	}
      }

      break;
    }
  }
cleanup:;
  if (outfp != stdout)
    fclose(outfp);
cleanup0:;
  if(buf != NULL)
    free(buf);
}
#endif

void
get_camfile(n, outfilename)
     int	n;
     char	*outfilename;
{
  long	len;
  long    lenj;
  char buf1[64];
  char buf5[64];
  u_char	*bufj;
  u_char	*bufp;
  FILE	*outfp;
  int i;
  time_t tt;
  int vga = 0;
#ifdef USEWORKFILE
  FILE *fp;
#endif

  if (all_pic_num < n) {
    fprintf(stderr, "picture number is too large.\n");
    errflg ++;
    return;
  }

  if(QVpicattr(n) & 0x02){
    vga = 1;
    if(qv7xxprotocol != 0)
      vga = 2;
  }

  bufp = (u_char *)malloc(THUMBNAIL_MAXSIZ);
  if (bufp == (u_char *)NULL) {
    fprintf(stderr, "can't alloc\n");
    errflg ++;
    return;
  }

#ifdef USEWORKFILE
  if(vga){
    fp = fopen(WORKFILE, WMODE);
    if(fp == NULL){
      fprintf(stderr, "can't open workfile(%s).\n", WORKFILE);
      errflg ++;
      return;
    }
  }else{
    bufj = (u_char *)malloc(JPEG_MAXSIZ);
    if (bufj == (u_char *)NULL) {
      fprintf(stderr, "can't alloc\n");
      errflg ++;
      return;
    }
  }
#else
  if(vga == 1)
    bufj = (u_char *)malloc(JPEG_MAXSIZ_VGA);
  else if(vga == 2){
    int filesize;
    filesize = QVgetsize2(n);
    if( filesize < 0)
      return;
    bufj = (u_char *)malloc(filesize);
  }else
    bufj = (u_char *)malloc(JPEG_MAXSIZ);
  if (bufj == (u_char *)NULL) {
    fprintf(stderr, "can't alloc\n");
    errflg ++;
    return;
  }
#endif

  outfp = stdout;
  if (outfilename) {
    outfp = fopen(outfilename, WMODE);
    if (outfp == NULL){
      fprintf(stderr, "can't open outfile(%s).\n, outfilename");
      errflg ++;
      goto cleanup0;
    }
  }

  fputc(0x07, outfp);		/* CAM header */
  fputc(0x20, outfp);
  fputc(0x4d, outfp);
  fputc(0x4d, outfp);

  fputc(0x00, outfp);
  fputc(0x03, outfp);

  fputc(0x00, outfp);		/* comment area  size */
  fputc(0x01, outfp);
  sprintf(buf1,"Generated by qvplay-0.93");
  tt = time(0);
  sprintf(buf5,"%s", asctime(localtime(&tt)));
  len = strlen(buf5) - 1;
  buf5[len] = '\0';
  len = len + strlen(buf1);

  len = len + 14 +1 ;		/* (ID + 00) * 7  + 00 */
  fputc((len >> 24) & 0xff , outfp);
  fputc((len >> 16) & 0xff , outfp);
  fputc((len >> 8) & 0xff , outfp);
  fputc(len & 0xff , outfp);
  for(i = 0 ; i < 10 ; i ++)
    fputc(0x00, outfp);		/* dummy */

  fputc(0x00, outfp);		/* thumbnail area  size */
  fputc(0x02, outfp);
  fputc(0x00, outfp);
  fputc(0x00, outfp);
  fputc(0x15, outfp);
  fputc(0xf0, outfp);		/* 5616 = 3 * 52 * 36 */
  for(i = 0 ; i < 10 ; i ++)
    fputc(0x00, outfp);		/* dummy */
	
  fputc(0x00, outfp);		/* jpeg area  size */
  if(vga)
    fputc(0x04, outfp);
  else
    fputc(0x03, outfp);

#ifdef USEWORKFILE
  if(vga){
    lenj = QVgetpicture(n, NULL, JPEG, vga, fp);
    fclose(fp);
  }else
    lenj = QVgetpicture(n, bufj, JPEG, vga, NULL);
#else
  lenj = QVgetpicture(n, bufj, JPEG, vga, NULL);
#endif
  if (lenj < 0) {
    errflg ++;
    goto cleanup;
  }
  if(vga)
    lenj = lenj + 473;
  fputc((lenj >> 24) & 0xff , outfp);
  fputc((lenj >> 16) & 0xff , outfp);
  fputc((lenj >> 8) & 0xff , outfp);
  fputc(lenj & 0xff , outfp);

  if(vga){
    fputc(0xf0, outfp); fputc(0x77, outfp);
    fputc(0xff, outfp); fputc(0xff, outfp);
    fputc(0xff, outfp); fputc(0xff, outfp);
    fputc(0x10, outfp); fputc(0xf8, outfp);
    fputc(0x03, outfp); fputc(0x00, outfp);
  }else
    for(i = 0 ; i < 10 ; i ++)
      fputc(0x00, outfp);		/* dummy */
	
  /* comment  */
  fputc(0x01, outfp);
  if(write_file((u_char *)buf1, strlen(buf1) +1 , outfp) == -1){
    errflg ++;
    goto cleanup;
  }
  fputc(0x02, outfp); fputc(0x00, outfp);
  fputc(0x03, outfp); fputc(0x00, outfp);
  fputc(0x04, outfp); fputc(0x00, outfp);
  fputc(0x05, outfp);
  if(write_file((u_char *)buf5, strlen(buf5) +1 , outfp) == -1){
    errflg ++;
    goto cleanup;
  }

  fputc(0x06, outfp); fputc(0x00, outfp);
  fputc(0x07, outfp); fputc(0x00, outfp);

  fputc(0x00, outfp);

  len = QVgetpicture(n, bufp, PPM_T, vga, NULL);
  if (len < 0) {
    errflg ++;
    goto cleanup;
  }
  if(write_ppm(bufp, outfp,
	    THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT,
	    2, 2,
	    0, 0) == -1){
    errflg ++;
    goto cleanup;
  }
  if(vga == 1){
#ifdef USEWORKFILE
    if(write_jpeg_fine(WORKFILE, outfp) == -1){
      errflg ++;
      goto cleanup;
    };
    unlink(WORKFILE);
#else
    if(write_jpeg_fine(bufj, outfp) == -1){
      errflg ++;
      goto cleanup;
    }
#endif
  } else if(vga == 2){
    if(write_file(bufj, lenj, outfp) == -1){
      errflg ++;
      goto cleanup;
    }
  }else{
    if(write_file(bufj, lenj, outfp) == -1){
      errflg ++;
      goto cleanup;
    }
  }
cleanup:;
  if (outfp != stdout)
    fclose(outfp);
cleanup0:;
  free(bufp);
  if(bufj)
    free(bufj);
}

void
show_picture(n)
     int	n;
{
  int m;
  if (n < 1) 
    m = 1;
  if (all_pic_num < n) 
    m = all_pic_num;

  if (QVshowpicture(m) < 0)
    errflg ++;
}

void
default_picture(n)
     int	n;
{
  int m;
  if (n < 1) 
    m = 1;
  if (all_pic_num < n) 
    m = all_pic_num;

  if (QVdefaultpicture(m) < 0)
    errflg ++;
}

void
get_all_pictures(start, end, outfilename, format)
     int	start;
     int	end;
     char	*outfilename;
     int format;
{
  int	i;
  char	fname[MAXPATHLEN];


  if (all_pic_num < start || all_pic_num < end) {
    fprintf(stderr, "picture number is too large.\n");
    errflg ++;
    return;
  }
  if (start > end) {
    int	tmp;
    tmp = end;
    end = start;
    start = tmp;
  }

  for (i = start; i <= end; i++) {
    switch(format){
    case PPM_P:
    case PPM_T:
      if (outfilename)
	sprintf(fname, OUTFILENAME_PPM, outfilename, i);
      else
	sprintf(fname, OUTFILENAME_PPM0, i);
      break;
    case RGB_P:
    case RGB_T:
      if (outfilename)
	sprintf(fname, OUTFILENAME_RGB, outfilename, i);
      else
	sprintf(fname, OUTFILENAME_RGB0, i);
      break;
    case BMP_P:
    case BMP_T:
      if (outfilename)
	sprintf(fname, OUTFILENAME_BMP, outfilename, i);
      else
	sprintf(fname, OUTFILENAME_BMP0, i);
      break;
    case CAM:
      if (outfilename)
	sprintf(fname, OUTFILENAME_CAM, outfilename, i);
      else
	sprintf(fname, OUTFILENAME_CAM0, i);
      break;
    case JPEG:
    default:
      if (outfilename)
	sprintf(fname, OUTFILENAME_JPG, outfilename, i);
      else
	sprintf(fname, OUTFILENAME_JPG0, i);
      break;
    }
    if(format == CAM)
      get_camfile(i, fname);
    else
      get_picture(i, fname, format);
  }
}

void
delete_picture(n)
     int	n;
{
  if (all_pic_num < n) {
    fprintf(stderr, "picture number is too large.\n");
    errflg ++;
    return;
  }

  if (QVdeletepicture(n) < 0)
    errflg ++;
  all_pic_num = -1;		/*need update*/
}

void
show_4_pictures(str)
     char * str;
{
  int pictureNo[4];
  int i;
  for(i = 0 ; i < 4 ; i++)
    pictureNo[i] = 0;
  if(( i = sscanf(str, "%d,%d,%d,%d", 
		  &pictureNo[0],
		  &pictureNo[1],
		  &pictureNo[2],
		  &pictureNo[3])) < 1) return;;
  for( ; i < 4 ; i++)
    pictureNo[i] = pictureNo[i -1] + 1;
  if(pictureNo[0] > all_pic_num)
    pictureNo[0] = all_pic_num;
  for(i = 0;  i < 4 ; i++)
    if(pictureNo[i] > all_pic_num) 
      pictureNo[i] = pictureNo[i -1];
  QV4split(pictureNo);
}

void
show_9_pictures(str)
     char * str;
{
  int pictureNo[9];
  int i;
  for(i = 0 ; i < 9 ; i++)
    pictureNo[i] = 0;
  if(( i = sscanf(str, "%d,%d,%d,%d,%d,%d,%d,%d,%d", 
		  &pictureNo[0],
		  &pictureNo[1],
		  &pictureNo[2],
		  &pictureNo[3],
		  &pictureNo[4],
		  &pictureNo[5],
		  &pictureNo[6],
		  &pictureNo[7],
		  &pictureNo[8])) < 1) return;
  for( ; i < 9 ; i++)
    pictureNo[i] = pictureNo[i -1] + 1;
  if(pictureNo[0] > all_pic_num)
    pictureNo[0] = all_pic_num;
  for(i = 0;  i < 9 ; i++)
    if(pictureNo[i] > all_pic_num) 
      pictureNo[i] = pictureNo[i -1];
  QV9split(pictureNo);
}

void
protect_picture(n)
     int n;
{
  if(n < 1 ) return;
  if(n > all_pic_num){
    fprintf(stderr, "picture number is too large.\n");
    errflg ++;
    return;
  }
  QVprotect(n, 1);
}

void
unprotect_picture(n)
     int n;
{
  if(n < 1 ) return;
  if(n > all_pic_num){
    fprintf(stderr, "picture number is too large.\n");
    errflg ++;
    return;
  }
  QVprotect(n, 0);
}

void
picture_information(n)
     int n;
{
  int i;
  if(n < 1 ) return;
  if(n > all_pic_num){
    fprintf(stderr, "picture number is too large.\n");
    errflg ++;
    return;
  }
  i = QVpicattr(n);
  printf("Picture %3d = ", n);
  if(0x01 & i)
    printf("protect on");
  else
    printf("protect off");
  if(0x02 & i)
    printf(" 640x480 %02x\n", i);
  else
    printf(" 480x240\n");
}


void
take_picture()
{
  if(qv7xxprotocol !=0){
    fprintf(stderr, "QV-700/770 cannot take picture by remote command.\n");
    errflg ++;
    return;
  }
  if(QVswstat() & 0x0040){
    fprintf(stderr, "set mode SW to REC position.\n");
    errflg ++;
    return;
  }
		
  if(qvhasvgamode) {
    if(QVremain(1) < 0){
      fprintf(stderr, "picture full.\n");
      errflg ++;
      return;
    }
    if(QVremain(0) < 0){
      fprintf(stderr, "picture full.\n");
      errflg ++;
      return;
    }
  } else
    if (MAX_PICTURE_NUM_QV10 <= all_pic_num) {
      fprintf(stderr, "picture full.\n");
      errflg ++;
      return;
    }

  if (QVtakepicture() < 0)
    errflg ++;
  all_pic_num = QVhowmany();
}

#ifdef X68K
void
show_on_X68k(n)
     int n;
{
  int	len;
  u_char	*buf;
  int fine = 0;
  
  if (all_pic_num < n) {
    fprintf(stderr, "picture number is too large.\n");
    errflg ++;
    return;
  }
  if(QVpicattr(n) & 0x02)
    fine = 1;

  if(fine)
    buf = (u_char *)malloc(YCC_MAXSIZ_VGA);
  else
    buf = (u_char *)malloc(YCC_MAXSIZ);

  if (buf == (u_char *)NULL) {
    fprintf(stderr, "can't alloc\n");
    errflg ++;
    return;
  }
  len = QVgetpicture(n, buf, PPM_P, fine, NULL);
  if (len < 0) {
    errflg ++;
    goto cleanup0;
  }
  if(fine)
    write_x68k_fine(buf, PICTURE_WIDTH_FINE, PICTURE_HEIGHT_FINE, 2, 2);
  else
    write_x68k(buf, PICTURE_WIDTH, PICTURE_HEIGHT, 3, 2);
  
cleanup0:;
  free(buf);

}
#endif
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

void print_swstat(stat)
     int stat;
{
  if(qv7xxprotocol !=0){
    fprintf(stderr, "QV-700/770 cannot report status.\n");
    return;
  }
  if(qvhasvgamode)
    printf("QV-100/300/200 status is...\n");
  else
    printf("QV-10/10A/30/11/70 status is...\n");
  if(0x0080 & stat)
    printf("CCD unit is reverse angle.\n");
  else
    printf("CCD unit is normal angle.\n");
  if(0x0040 & stat)
    printf("REC/PLAY switch is PLAY position.\n");
  else
    printf("REC/PLAY switch is REC position.\n");
  if(0x8000 & stat)
    printf("too bright sign is shown on LCD.\n");
  if(0x4000 & stat)
    printf("too dark sign is shown on LCD.\n");
  if(0x0800 & stat)
    printf("[-] button is pressed.\n");
  if(0x0400 & stat)
    printf("[+] button is pressed.\n");
  if(0x0020 & stat)
    printf("PROTECT button is pressed.\n");
  if(0x0010 & stat)
    printf("DEL button is pressed.\n");
  if(0x0008 & stat)
    printf("DISP button is pressed.\n");
  if(0x0004 & stat)
    printf("MODE button is pressed.\n");
  if(0x0002 & stat)
    printf("ZOOM button is pressed.\n");
  if(0x0001 & stat)
    printf("Shutter button is pressed.\n");
}

int
main(argc, argv)
     int	argc;
     char	**argv;
{
  char	*devpath = NULL;
  char	*outfilename = NULL;
  int	start_picture = 0;
  int	end_picture = 0;
  int     move_from = 0 ;
  int     move_to = 0 ;
  char	c;
  int i, j;

  u_char hoge[12];  /* extdatatest */

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

  if(devpath == NULL){
    devpath = malloc(sizeof(char) * (strlen(RSPORT) +1));
    if(devpath == NULL) {
      fprintf(stderr, "can't malloc\n");
      exit(1);
    }
    strcpy(devpath, RSPORT);
  }

  for(i = 0 ; i < argc; i++){
    if(strcmp("-D", argv[i]) == 0){
      devpath = argv[i+1];
      break;
    }
  }

  if (devpath) {
#ifndef DONTCAREUID
    daemonuid();
#endif
    QVsetfd(opentty(devpath));
#ifndef DONTCAREUID
    useruid();
#endif
  }
  if (QVgetfd() < 0)
    Exit(1);

  while ((c = getopt(argc, argv, "D:p:o:g:rRnNas:e:d:tvF:S:X:4:9:P:U:10V7i:IzZy:Y:h")) != EOF) {
    if(c == 'h'){
      usage();
      Exit(-1);
    }
    if(!(QVgetfd() < 0)){
      all_pic_num = QVhowmany();
      if((end_picture == 0) || (end_picture > all_pic_num))
	end_picture = all_pic_num;
      if(start_picture == 0)
	start_picture = 1;
      qvhasvgamode = QVrevision() & 0x01000000;
      if((QVrevision() & 0x00a00000) == 0x00a00000)
	qv7xxprotocol = 1;
      if(qv7xxprotocol != 0){
	QVnewprotocol();
      }
    }
    if(QVbattery() <= LOW_BATT){
      fprintf(stderr,"LOW BATTERY, change battery or connect AC adapter.\n");
      Exit(3);
    }

    QVsectorsize(SECTOR);

    switch(c) {
    case 'p':
      show_picture(atoi(optarg));
      break;
    case 'o':
      outfilename = optarg;
      break;
    case 'g':
      if(format == CAM)
	get_camfile(atoi(optarg), outfilename);
      else
	get_picture(atoi(optarg), outfilename, format);
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
      if(qvhasvgamode == 0)
	printf("remain = %d\n", QVremain(0));
      else{
	printf("remain(normal) = %d\n", QVremain(0));
	printf("remain(fine) = %d\n", QVremain(1));
      }
      break;
    case 'a':
      get_all_pictures(start_picture, end_picture, outfilename, format);
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
    case 'V':
      printf("battery = %.2f V\n", (float) QVbattery() / 16);
      break;
    case 'H': 		/* not used */
      QVhidepicnum();
      break;
    case '7': 		
      QVcolorpattern();
      break;
    case 'i': 		
      picture_information(atoi(optarg));
      break;
    case 'I': 		
      for(i = start_picture ; i <= end_picture ; i++)
	picture_information(i);
      break;
    case 'z': 		
      print_swstat(QVswstat());
      break;
    case 'Z': 		
      i = QVrevision();
      fprintf(stderr,"revision = 0x%08x\n", i);
      break;
    case 'y': 		
      default_picture(atoi(optarg));
      break;
    case 'Y': 		
      j = QVgetextdata(atoi(optarg), hoge);
      fprintf(stderr,"ext ");
      for( i = 0 ; i < j; i++){
	fprintf(stderr, "%02x ", hoge[i]);
      }
      fprintf(stderr,"\n");
      break;
    case 'M':			/* don't use */
      /* QVmovepicture is very dangerous */
      if(sscanf(optarg, "%d,%d", &move_from, &move_to)
	 != 2)
	break;
      if(move_from > all_pic_num) break;
      if(move_from < 1) break;
      if(move_to > all_pic_num) break;
      if(move_to < 1) break;
      QVmovepicture(move_from, move_to);
      break;
    case 'F':
      {
	char *p;
	if(optarg[0] == '+'){
	  raw_data = 1;
	  p = &optarg[1];
	} else
	  p = &optarg[0];
	switch(*p){
	case 'j':
	  format = JPEG;
	  break;
	case 'p':
	  format = PPM_T;
	  break;
	case 'r':
	  format = RGB_T;
	  break;
	case 'b':
	  format = BMP_T;
	  break;
#ifndef USEWORKFILE
	case 'P':
	  format = PPM_P;
	  break;
	case 'R':
	  format = RGB_P;
	  break;
	case 'B':
	  format = BMP_P;
	  break;
#endif
	case 'c':
	  format = CAM;
	  break;
	default:
	  format = JPEG;
	  break;
	}
      }
    break;
#ifdef X68K
    case 'X':
      show_on_X68k(atoi(optarg));
      break;
#endif
    case 'S':
      switch(optarg[0]){
#if defined(__linux__) || defined(WIN32) || defined(OS2) || defined(__FreeBSD__) || defined(DOS)
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
      break; /* do nothing */
    default:
      usage();
      Exit(-1);
      return 0; /* dummy */
    }
  }

  Exit (errflg ? 1 : 0);
}
