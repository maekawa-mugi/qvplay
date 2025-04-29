#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <iocslib.h>

#define NORM(x) { if(x<0) x=0; else if (x>255) x=255;}

void
write_x68k(buf, PPM_WIDTH, PPM_HEIGHT, rateW, rateH)
     uint8_t	*buf;
     int PPM_WIDTH;
     int PPM_HEIGHT;
     int rateW;
     int rateH;
{
  int x, y;
  long cr, cb;
  long L;
  long r,g,b;
  uint8_t *Y;
  uint8_t *Cr;
  uint8_t *Cb;

  struct PSETPTR psetptr;

  CRTMOD(12);
  G_CLR_ON();

  Y = buf;

  Cb = Y + (PPM_HEIGHT * PPM_WIDTH);
  Cr = Cb + (PPM_HEIGHT / rateH) * (PPM_WIDTH / rateW);

  for( y = 0 ; y < PPM_HEIGHT; y++){
    for( x = 0 ; x < PPM_WIDTH ; x++){
      L = Y[y * PPM_WIDTH + x] *  100000; 
      cb = Cb[(y / rateH) * PPM_WIDTH /rateW + (x /rateW)];
      if(cb > 127) cb = cb - 256;
      cr = Cr[(y / rateH) * PPM_WIDTH /rateW + (x /rateW)];
      if(cr > 127) cr = cr - 256;

      r = L + 140200 * cr;
      g = L - 34414 * cb - 71414 * cr;
      b = L + 177200 * cb;
      
      r = r / 100000;
      g = g / 100000;
      b = b / 100000;

      NORM(r);
      NORM(g);
      NORM(b);

      psetptr.x = x;
      psetptr.y = y*2;
	  psetptr.color = ((g & 0xf8) << 8) | 
					 ((r & 0xf8) << 3) | 
					 ((b & 0xf8) >> 2 ) ;
	  PSET(&psetptr);
	  psetptr.y ++;
	  PSET(&psetptr);
    }
  }
}

void
write_x68k_fine(buf, PPM_WIDTH, PPM_HEIGHT, rateW, rateH)
     uint8_t	*buf;
     int PPM_WIDTH;
     int PPM_HEIGHT;
     int rateW;
     int rateH;
{
	int x, y;
	long cr, cb;
	long L;
	long r[4],g[4],b[4];
	uint8_t *Y;
	uint8_t *Cr;
	uint8_t *Cb;
	int i;

	struct PSETPTR psetptr;

	CRTMOD(12);
	G_CLR_ON();

	Y = buf;

	Cb = Y + (PPM_HEIGHT * PPM_WIDTH);
	Cr = Cb + (PPM_HEIGHT / rateH) * (PPM_WIDTH / rateW);

	for( y = 0 ; y < PPM_HEIGHT; y++){
		psetptr.x = 0;
		for( x = 0 ; x < PPM_WIDTH ; x = x + 4 ){
			for(i = 0 ; i < 4 ; i++){
				L = Y[y * PPM_WIDTH + x + i] *  100000; 
				cb = Cb[(y / rateH) * PPM_WIDTH /rateW + ((x + i)/rateW)];
				if(cb > 127) cb = cb - 256;
				cr = Cr[(y / rateH) * PPM_WIDTH /rateW + ((x + i)/rateW)];
				if(cr > 127) cr = cr - 256;

				r[i] = L + 140200 * cr;
				g[i] = L - 34414 * cb - 71414 * cr;
				b[i] = L + 177200 * cb;
      
				r[i] = r[i] / 100000;
				g[i] = g[i] / 100000;
				b[i] = b[i] / 100000;

			}

			for(i = 0 ; i < 3 ; i++){
				r[i] = (r[i] + r[i+1]) / 2;
				g[i] = (g[i] + g[i+1]) / 2;
				b[i] = (b[i] + b[i+1]) / 2;
				NORM(r[i]);
				NORM(g[i]);
				NORM(b[i]);
				psetptr.x ++;
				psetptr.y = y;
				psetptr.color = ((g[i] & 0xf8) << 8) | 
					((r[i] & 0xf8) << 3) | 
						((b[i] & 0xf8) >> 2 ) ;
				PSET(&psetptr);
			}
		}
	}
}






