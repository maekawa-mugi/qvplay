#include "config.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>

#define NORM(x)                                                                \
  {                                                                            \
    if (x < 0)                                                                 \
      x = 0;                                                                   \
    else if (x > 255)                                                          \
      x = 255;                                                                 \
  }

int write_ppm(uint8_t *buf, FILE *outfp, int PPM_WIDTH, int PPM_HEIGHT,
              int rateW, int rateH, int withheader, int order) {
  int x, y;
  int Sy;
  long cr, cb;
  long L;
  long r, g, b;
  uint8_t *Y;
  uint8_t *Cr;
  uint8_t *Cb;
  int i;
  i = 0;

  if (withheader)
    fprintf(outfp, "P6\n%d %d\n255\n", PPM_WIDTH, PPM_HEIGHT);

  Y = buf;

  Cb = Y + (PPM_HEIGHT * PPM_WIDTH);
  Cr = Cb + (PPM_HEIGHT / rateH) * (PPM_WIDTH / rateW);

  for (Sy = 0; Sy < PPM_HEIGHT; Sy++) {
    if (order)
      y = PPM_HEIGHT - Sy - 1;
    else
      y = Sy;
    for (x = 0; x < PPM_WIDTH; x++) {
      L = Y[y * PPM_WIDTH + x] * 100000;
      cb = Cb[(y / rateH) * PPM_WIDTH / rateW + (x / rateW)];
      if (cb > 127)
        cb = cb - 256;
      cr = Cr[(y / rateH) * PPM_WIDTH / rateW + (x / rateW)];
      if (cr > 127)
        cr = cr - 256;

      r = L + 140200 * cr;
      g = L - 34414 * cb - 71414 * cr;
      b = L + 177200 * cb;

      r = r / 100000;
      g = g / 100000;
      b = b / 100000;

      NORM(r);
      NORM(g);
      NORM(b);
      if (order) {
        if (fputc(b, outfp) == EOF) {
          perror("write_ppm");
          return (-1);
        }
        i++;
        if (fputc(g, outfp) == EOF) {
          perror("write_ppm");
          return (-1);
        }
        i++;
        if (fputc(r, outfp) == EOF) {
          perror("write_ppm");
          return (-1);
        }
        i++;
      } else {
        if (fputc(r, outfp) == EOF) {
          perror("write_ppm");
          return (-1);
        }
        i++;
        if (fputc(g, outfp) == EOF) {
          perror("write_ppm");
          return (-1);
        }
        i++;
        if (fputc(b, outfp) == EOF) {
          perror("write_ppm");
          return (-1);
        }
        i++;
      }
    }
  }
  return (i);
}
