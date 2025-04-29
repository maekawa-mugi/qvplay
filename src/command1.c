#include "config.h"
#include <stdio.h>
#include <sys/types.h>
#include <setjmp.h>
#include "common.h"
#ifdef X68
#include "tty_x68.h"
#else
#ifdef WIN32
#include "tty_w32.h"
#else
#ifdef OS2
#include "tty_os2.h"
#else
#ifdef DOS
#include "tty_dos.h"
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

#include "command.h"

#if 0
# define dprintf(x)	fprintf x
#else
# define dprintf(x)
#endif

extern int qvverbose;


/*------------------------------------------------------------*/

int
QVdeletepicture(n)
     int	n;
{
  u_char	s;

  if (!QVok())
    return -1;			/*ng*/
  wstr("DF", 2);
  wbyte((u_char) n);
  wbyte(0xff);
  s = rbyte();			/*supposed to be 0x76 - n*/
  if(checksum(s) == -1) return(-1);
  wbyte(ACK);
  return 1;
}

int
QVtakepicture()
{
  u_char	s;
  u_char buf[3];
  buf[0] = 'D';
  buf[1] = 'R';
  buf[2] = 0x06;

  QVsectorsize(0x0340);
  if (!QVok())
    return -1;		/*ng*/
  /*  wstr("DR", 2); */
  wstr(buf, 3);
  /* fprintf(stderr,"DR\n", s); */
  s = rbyte();			/* checksum */
  /*   fprintf(stderr,"rbyte\n"); */
  /*  if(checksum(s) == -1) return(-1); */
  /* wbyte(ACK); */
  /* fprintf(stderr,"ACK\n"); */
  s = rbyte();			/* SW status */
  /* fprintf(stderr,"SW status %02x\n", s); */
  sleep(10);

  return 1;
}

int
QVmovepicture(from, to)
     int from;
     int to;
{
  u_char	s;

  if (!QVok())
    return -1;			/*ng*/
  wstr("DI", 2);
  wbyte((u_char) from);
  s = rbyte();			/*  0x72 - from ?*/
  if(checksum(s) == -1) return(-1);
  wbyte(ACK);

  if (!QVok())
    return -1;			/*ng*/
  wstr("DY", 2);
  wbyte(0x02);
  wbyte((u_char) from);
  s = rbyte();			/* ?*/
  if(checksum(s) == -1) return(-1);
  wbyte(ACK);

  if (!QVok())
    return -1;			/*ng*/
  wstr("DF", 2);
  wbyte((u_char) from);
  wbyte(0xff);
  s = rbyte();			/*  0x76 - from ?*/
  if(checksum(s) == -1) return(-1);
  wbyte(ACK);

  if (!QVok())
    return -1;			/*ng*/
  wstr("Dj", 2);
  wbyte((u_char) to);
  s = rbyte();			/* 0x51 - to ? */
  if(checksum(s) == -1) return(-1);
  wbyte(ACK);

  return 1;
}

int
QVpicturesize(n)
     int n;
{
  u_char s;
  if (!QVok())
    return -1;			/*ng*/
  if(n)
    wstr("Mq", 2);
  else
    wstr("MQ", 2);
  s = rbyte();
  if(checksum(s) == -1) return(-1);
  wbyte(ACK);
  if(n){
    s = rbyte();fprintf(stderr,"0x%02x", s);
    s = rbyte();fprintf(stderr," %02x\n", s);
  } else {
    s = rbyte();fprintf(stderr,"0x%02x", s);
    s = rbyte();fprintf(stderr," %02x", s);
    s = rbyte();fprintf(stderr," %02x", s);
    s = rbyte();fprintf(stderr," %02x", s);
    s = rbyte();fprintf(stderr," %02x", s);
    s = rbyte();fprintf(stderr," %02x", s);

    s = rbyte();fprintf(stderr," %02x", s);
    s = rbyte();fprintf(stderr," %02x\n", s);
  }
  return 1;
}


int
QVgetsize2(n)
{
  /* QV700/770 only */
  u_char	s;
  long filesize = 0;

  if (QVshowpicture(n) < 0)
    return -1;			/*ng*/

  if (!QVok())
    return -1;			/*ng*/

  wstr("DL", 2);
  s = rbyte();		/*supposed to be 0x6f*/
  
  if(checksum(s) == -1) return(-1);
  wbyte(ACK);

  if (!QVok())
    return -1;			/*ng*/
  wstr("EM", 2);
  s = rbyte();
  if(checksum(s) == -1) return(-1);
  wbyte(ACK);
  s = rbyte();  /* file size ? */
  filesize = s;
  s = rbyte();  
  filesize = filesize << 8;
  filesize = filesize | s;
  s = rbyte();
  filesize = filesize << 8;
  filesize = filesize | s;
  s = rbyte();  
  filesize = filesize << 8;
  filesize = filesize | s;
  return(filesize);
}

int
QVgetextdata(n, buf)
     int n;
     u_char	*buf;
{
  /* QV700/770 only */
  u_char	s;
  int len;

  if (QVshowpicture(n) < 0)
    return -1;			/*ng*/

  if (!QVok())
    return -1;			/*ng*/

  wstr("DL", 2);
  s = rbyte();		/*supposed to be 0x6f*/
  
  if(checksum(s) == -1) return(-1);
  wbyte(ACK);

  if (!QVok())
    return -1;			/*ng*/
  wstr("CV", 2);
  s = rbyte();			
  if(checksum(s) == -1) return(-1);
  wbyte(ACK);


  len = QVblockrecv(buf, 0);
  if (!QVok())
    return -1;			/*ng*/

  return len;
}


int
QVgetpicture(n, buf, format, vga, fp)
     int	n;
     u_char	*buf;
     int format;
     int vga;
     FILE *fp;
{
  u_char	s;
  int	len;
  long filesize = 0;

  if(vga == 2){
    if((format == JPEG) || (format == CAM)){
      filesize = QVgetsize2(n);
      if( filesize < 0)
	return -1;
    }
  }

  if (QVshowpicture(n) < 0)
    return -1;			/*ng*/

  if (!QVok())
    return -1;			/*ng*/

  wstr("DL", 2);
  s = rbyte();		/*supposed to be 0x6f*/
  
  if(checksum(s) == -1) return -1;
  wbyte(ACK);

  if (!QVok())
    return -1;			/*ng*/
  switch(format){
  case PPM_T:
  case RGB_T:
  case BMP_T:
    wstr("MK", 2);
    break;
  case PPM_P:
  case RGB_P:
  case BMP_P:
    if(vga)
      wstr("Ml", 2);
    else
      wstr("ML", 2);
    break;
  case JPEG:
  default:
    if(vga == 1)
      wstr("Mg", 2);
    else if(vga == 2){
      wstr("EG", 2);
    }
    else
      wstr("MG", 2);
    break;
  }
  s = rbyte();
  if(checksum(s) == -1) return(-1);
  wbyte(ACK);
  if (qvverbose){
    switch(format){
    case PPM_T:
    case RGB_T:
    case BMP_T:
      fprintf(stderr, "Thumbnail %3d: ", n);
      break;
    case CAM:
    case JPEG:
    case PPM_P:
    case RGB_P:
    case BMP_P:
    default:
      fprintf(stderr, "Picture   %3d: ", n);
      break;
    }
  }

#ifdef USEWORKFILE
  if(fp != NULL)
	  len = QVblockrecv_file(fp, filesize);
  else
#endif	  
	  len = QVblockrecv(buf, filesize);

  if (!QVok())
    return -1;			/*ng*/

  return len;
}


int
QV4split(n)
     int *n;
{
  u_char	s;
  int i;

  if (!QVok())
    return -1;			/*ng*/
  wstr("DB", 2);
  for( i = 0 ; i < 4 ; i++)
    wbyte((u_char) n[i]);
  s = rbyte();			/* checksum */
  if(checksum(s) == -1) return(-1);
  wbyte(ACK);
  return 1;
}


int
QV9split(n)
     int *n;
{
  u_char	s;
  int i;

  if (!QVok())
    return -1;			/*ng*/
  wstr("DC", 2);
  for( i = 0 ; i < 9 ; i++)
    wbyte((u_char) n[i]);
  s = rbyte();			/* checksum */
  if(checksum(s) == -1) return(-1);
  wbyte(ACK);
  return 1;
}

int
QVprotect(n, on)
     int n;
     int on;
{
  u_char	s;

  if (!QVok())
    return -1;			/*ng*/
  wstr("DY", 2);
  if(on)
    wbyte((u_char) 0x01);
  else
    wbyte((u_char) 0x00);
  wbyte((u_char) n);
  s = rbyte();			/* checksum */
  if(checksum(s) == -1) return(-1);
  wbyte(ACK);
  return 1;
}


int
QVhidepicnum(n)
     int n;
{
  u_char	s;

  if (!QVok())
    return -1;			/*ng*/
  wstr("DM", 2);
  s = rbyte();			/* checksum */
  if(checksum(s) == -1) return(-1);
  wbyte(ACK);
  return 1;
}

int
QVpoweroff()
{
  u_char	s;

  if (!QVok())
    return -1;			/*ng*/
  wstr("QX", 2);
  s = rbyte();			/* checksum */
  if(checksum(s) == -1) return(-1);
  wbyte(ACK);
  return 1;
}

int
QVcolorpattern()
{
  u_char	s;

  if (!QVok())
    return -1;			/*ng*/
  wstr("DP", 2);
  s = rbyte();			/* checksum */
  if(checksum(s) == -1) return(-1);
  wbyte(ACK);
  return 1;
}

int
QVpicattr(n)
     int n;
{
  u_char	s;

  if (!QVok())
    return -1;			/*ng*/
  wstr("DY", 2);
  wbyte((u_char) 0x02);
  wbyte((u_char) n);
  s = rbyte();			/* checksum */
  if(checksum(s) == -1) return(-1);
  wbyte(ACK);
  s = rbyte();
  return s;  /* picture attribute */
  /* 00 protect off semi qvga  */
  /* 01 protect on  semi qvga */
  /* 02 protect off vga */
  /* 03 protect on  vga */
}

