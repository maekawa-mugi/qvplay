#include "common.h"
#include "config.h"
#include "ppm.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

static int put_uint32_t(uint32_t l, FILE *outfp) {
  int i;
  for (i = 0; i < 4; i++) {
    if (fputc(0xff & (l >> (8 * i)), outfp) == EOF) {
      perror("put_uint32_t");
      return (-1);
    }
  }
  return (i);
}

static int put_uint16_t(uint16_t s, FILE *outfp) {
  if ((fputc(0xff & s, outfp) == EOF) ||
      (fputc(0xff & (s >> 8), outfp) == EOF)) {
    perror("put_uint16_t");
    return (-1);
  }
  return (2);
}

int write_bmp(uint8_t *buf, FILE *outfp, int width, int height, int rateW,
              int rateH)

{
  long size;
  size = width * height * 3;

  if (fputc(0x42, outfp) == EOF)
    return (-1); /* B */
  if (fputc(0x4d, outfp) == EOF)
    return (-1); /* M */
  if (put_uint32_t(size + 54, outfp) == -1)
    return (-1);
  if (put_uint16_t(0, outfp) == -1)
    return (-1);
  if (put_uint16_t(0, outfp) == -1)
    return (-1);
  if (put_uint32_t(54, outfp) == -1)
    return (-1);

  if (put_uint32_t(40, outfp) == -1)
    return (-1); /* 40  Windows 3.x style */
  if (put_uint32_t(width, outfp) == -1)
    return (-1);
  if (put_uint32_t(height, outfp) == -1)
    return (-1);
  if (put_uint16_t(1, outfp) == -1)
    return (-1);
  if (put_uint16_t(24, outfp) == -1)
    return (-1);
  if (put_uint32_t(0, outfp) == -1)
    return (-1);
  if (put_uint32_t(size, outfp) == -1)
    return (-1);
  if (put_uint32_t(2925, outfp) == -1)
    return (-1);
  if (put_uint32_t(2925, outfp) == -1)
    return (-1);
  if (put_uint32_t(0, outfp) == -1)
    return (-1);
  if (put_uint32_t(0, outfp) == -1)
    return (-1);

  return (write_ppm(buf, outfp, width, height, rateW, rateH, 0, 1));
}
