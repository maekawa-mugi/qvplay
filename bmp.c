#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include "common.h"
#include "ppm.h"

static int
put_u_long(l, outfp)
     u_long l;
     FILE	*outfp;
{
  int i;
  for(i = 0 ; i < 4 ; i++){
    if(fputc( 0xff & (l >> (8 * i)), outfp) == EOF){
      perror("put_u_long");
      return(-1);
    }
  }
  return(i);
}

static int
put_u_short(s, outfp)
     u_short s;
     FILE	*outfp;
{
  if((fputc( 0xff & s, outfp) == EOF) ||
     (fputc( 0xff & (s >> 8), outfp) == EOF)){
    perror("put_u_short");
    return(-1);
  }
  return(2);
}

int
write_bmp(buf, outfp, width, height, rateW, rateH)
     u_char	*buf;
     FILE	*outfp;
     int width;
     int height;
     int rateW;
     int rateH;
{
  long size;
  size = width * height * 3;

  if(fputc(0x42, outfp) == EOF) return(-1);		/* B */
  if(fputc(0x4d, outfp) == EOF) return(-1);		/* M */
  if(put_u_long(size + 54 , outfp) == -1) return(-1);
  if(put_u_short(0, outfp) == -1) return(-1);
  if(put_u_short(0, outfp) == -1) return(-1);
  if(put_u_long(54, outfp) == -1) return(-1);

  if(put_u_long(40, outfp) == -1) return(-1); /* 40  Windows 3.x style */
  if(put_u_long(width, outfp) == -1) return(-1);
  if(put_u_long(height, outfp) == -1) return(-1);
  if(put_u_short(1, outfp) == -1) return(-1);
  if(put_u_short(24, outfp) == -1) return(-1);
  if(put_u_long(0, outfp) == -1) return(-1);
  if(put_u_long(size, outfp) == -1) return(-1);
  if(put_u_long(2925, outfp) == -1) return(-1);
  if(put_u_long(2925, outfp) == -1) return(-1);
  if(put_u_long(0, outfp) == -1) return(-1);
  if(put_u_long(0, outfp) == -1) return(-1);

  return(write_ppm(buf, outfp, width, height, rateW, rateH, 0, 1));
}
