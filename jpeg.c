#include "common.h"
#include "config.h"
#include "getuint.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#ifdef DOS
#include "cam2jpg.h"
#include "jpegtabf.h"
#else
#include "cam2jpgtab.h"
#include "jpegtab_f.h"
#endif

int write_file(buf, len, outfp)
uint8_t *buf;
int len;
FILE *outfp;
{
  int i, l;

  i = 0;
  while (len > i) {
    l = ((len - i) < BUFSIZ) ? (len - i) : BUFSIZ;
    if (fwrite(&buf[i], sizeof(uint8_t), l, outfp) != l) {
      perror("write_file");
      return (-1);
    };
    i = i + l;
  }
  return (i);
}

#ifdef USEWORKFILE
int write_file_file(filename, len, skip, outfp)
char *filename;
long len;
long skip;
FILE *outfp;
{
  long i, l;
  FILE *fp;
  uint8_t buf[BUFSIZ];

  fp = fopen(filename, RMODE);
  if (fp == NULL) {
    fprintf(stderr, "can't read workfile(%s).\n", filename);
    return (-1);
  }

  for (i = 0; i < skip; i++)
    (void)fgetc(fp);

  len = len - skip;
  i = 0;
  while (len > i) {
    l = ((len - i) < BUFSIZ) ? (len - i) : BUFSIZ;
    fread(buf, sizeof(uint8_t), l, fp);
    if (fwrite(buf, sizeof(uint8_t), l, outfp) != l) {
      perror("write_file_file");
      fclose(fp);
      return (-1);
    }
    i = i + l;
  }
  fclose(fp);
  return (i);
}
#endif

int write_jpeg(buf, outfp)
uint8_t *buf;
FILE *outfp;
{
  int i = 0;
  int areaNum;
  int ysize;
  int usize;
  int vsize;

  areaNum = get_uint16_t(buf); /* areaNum == 0x03 */
  ysize = get_uint16_t(buf + 2);
  usize = get_uint16_t(buf + 4);
  vsize = get_uint16_t(buf + 6);
  i = i + 8;

  if (write_file(soi, sizeof(soi), outfp) == -1)
    return (-1);
  if (write_file(app0, sizeof(app0), outfp) == -1)
    return (-1);
  if (write_file(dqt0, sizeof(dqt0), outfp) == -1)
    return (-1);
  if (write_file(&buf[i], 64, outfp) == -1)
    return (-1);
  i = i + 64;

  if (write_file(dqt1, sizeof(dqt1), outfp) == -1)
    return (-1);
  if (write_file(&buf[i], 64, outfp) == -1)
    return (-1);
  i = i + 64;

  if (write_file(sof, sizeof(sof), outfp) == -1)
    return (-1);
  if (write_file(dht, sizeof(dht), outfp) == -1)
    return (-1);

  if (write_file(sos_y, sizeof(sos_y), outfp) == -1)
    return (-1);
  if (write_file(&buf[i], ysize, outfp) == -1)
    return (-1);
  i = i + ysize;

  if (write_file(sos_u, sizeof(sos_u), outfp) == -1)
    return (-1);
  if (write_file(&buf[i], usize, outfp) == -1)
    return (-1);
  i = i + usize;

  if (write_file(sos_v, sizeof(sos_v), outfp) == -1)
    return (-1);
  if (write_file(&buf[i], vsize, outfp) == -1)
    return (-1);
  i = i + vsize;

  if (write_file(eoi, sizeof(eoi), outfp) == -1)
    return (-1);

  return (i);
}

#ifdef USEWORKFILE
int write_jpeg_fine(filename, outfp)
char *filename;
FILE *outfp;
{
  int i = 0;
  int size;
  uint8_t c = 0x01;
  FILE *fp;
  uint8_t buf[136];

  fp = fopen(filename, RMODE);
  if (fp == NULL) {
    fprintf(stderr, "can't read workfile(%s).\n", filename);
    return (-1);
  }
  fread(buf, sizeof(uint8_t), 136, fp);
  fclose(fp);

  size = get_u_int(buf + 4);

  i = i + 8;
  if (write_file(soi, sizeof(soi), outfp) == -1)
    return (-1);
  if (write_file(app_f, sizeof(app_f), outfp) == -1)
    return (-1);
  if (write_file(dqt_f, sizeof(dqt_f), outfp) == -1)
    return (-1);

  if (write_file(&buf[i], 64, outfp) == -1)
    return (-1);
  i = i + 64;
  if (write_file(&c, 1, outfp) == -1)
    return (-1);
  if (write_file(&buf[i], 64, outfp) == -1)
    return (-1);
  i = i + 64;
  if (write_file(sof_f, sizeof(sof_f), outfp) == -1)
    return (-1);

  if (write_file(dht_f, sizeof(dht_f), outfp) == -1)
    return (-1);

  if (write_file(sos_f, sizeof(sos_f), outfp) == -1)
    return (-1);

  /* skip 136 byte */
  if (write_file_file(filename, size, 136, outfp) == -1)
    return (-1);

  if (write_file(eoi, sizeof(eoi), outfp) == -1)
    return (-1);

  return (i);
}
#else
int write_jpeg_fine(buf, outfp)
uint8_t *buf;
FILE *outfp;
{
  int i = 0;
  int size;
  uint8_t c = 0x01;

  size = get_u_int(buf + 4);
  i = i + 8;
  if (write_file(soi, sizeof(soi), outfp) == -1)
    return (-1);
  if (write_file(app_f, sizeof(app_f), outfp) == -1)
    return (-1);
  if (write_file(dqt_f, sizeof(dqt_f), outfp) == -1)
    return (-1);

  if (write_file(&buf[i], 64, outfp) == -1)
    return (-1);
  i = i + 64;
  if (write_file(&c, 1, outfp) == -1)
    return (-1);
  if (write_file(&buf[i], 64, outfp) == -1)
    return (-1);
  i = i + 64;
  if (write_file(sof_f, sizeof(sof_f), outfp) == -1)
    return (-1);

  if (write_file(dht_f, sizeof(dht_f), outfp) == -1)
    return (-1);

  if (write_file(sos_f, sizeof(sos_f), outfp) == -1)
    return (-1);
  if (write_file(&buf[i], size, outfp) == -1)
    return (-1);

  if (write_file(eoi, sizeof(eoi), outfp) == -1)
    return (-1);

  return (i);
}

#endif
