#include "common.h"
#include "config.h"
#include "getuint.h"
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include "cam2jpgtab.h"
#include "jpegtab_f.h"

int write_file(const void *data, size_t len, FILE *outfp) {
  const uint8_t *buf = data;
  size_t i, l;

  i = 0;
  while (len > i) {
    l = ((len - i) < BUFSIZ) ? (len - i) : BUFSIZ;
    if (fwrite(&buf[i], sizeof(uint8_t), l, outfp) != l) {
      perror("write_file");
      return (-1);
    }
    i = i + l;
  }
  return i <= INT_MAX ? (int)i : -1;
}

#ifdef USEWORKFILE
int write_file_file(const char *filename, long len, long skip, FILE *outfp) {
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

int write_jpeg(const uint8_t *buf, size_t len, FILE *outfp) {
  int i = 0;
  int areaNum;
  int ysize;
  int usize;
  int vsize;

  if (buf == NULL || len < 136) {
    fprintf(stderr, "invalid JPEG source: header is truncated.\n");
    return -1;
  }

  areaNum = get_uint16_t(buf); /* areaNum == 0x03 */
  ysize = get_uint16_t(buf + 2);
  usize = get_uint16_t(buf + 4);
  vsize = get_uint16_t(buf + 6);
  i = i + 8;

  if (areaNum != 3 || (size_t)ysize + (size_t)usize + (size_t)vsize > len - 136) {
    fprintf(stderr, "invalid JPEG source: component sizes exceed input.\n");
    return -1;
  }

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

#define FINE_SOURCE_HEADER_SIZE 136

size_t jpeg_fine_output_size(size_t source_size) {
  const size_t fixed_size =
      sizeof(soi) + sizeof(app0) + sizeof(dqt_f) + 64 + 1 + 64 +
      sizeof(sof_f) + sizeof(dht_f) + sizeof(sos_f) + sizeof(eoi);
  const size_t overhead = fixed_size - FINE_SOURCE_HEADER_SIZE;

  if (source_size < FINE_SOURCE_HEADER_SIZE || source_size > SIZE_MAX - overhead)
    return 0;
  return source_size + overhead;
}

static int write_jpeg_fine_header(const uint8_t *buf, FILE *outfp) {
  static const uint8_t table_id = 0x01;

  if (write_file(soi, sizeof(soi), outfp) == -1 ||
      write_file(app0, sizeof(app0), outfp) == -1 ||
      write_file(dqt_f, sizeof(dqt_f), outfp) == -1 ||
      write_file(buf + 8, 64, outfp) == -1 ||
      write_file(&table_id, sizeof(table_id), outfp) == -1 ||
      write_file(buf + 72, 64, outfp) == -1 ||
      write_file(sof_f, sizeof(sof_f), outfp) == -1 ||
      write_file(dht_f, sizeof(dht_f), outfp) == -1 ||
      write_file(sos_f, sizeof(sos_f), outfp) == -1)
    return -1;
  return 0;
}

#ifdef USEWORKFILE
int write_jpeg_fine(const char *filename, FILE *outfp) {
  uint32_t size;
  size_t output_size;
  long file_size;
  FILE *fp;
  uint8_t buf[FINE_SOURCE_HEADER_SIZE];

  fp = fopen(filename, RMODE);
  if (fp == NULL) {
    fprintf(stderr, "can't read workfile(%s).\n", filename);
    return (-1);
  }
  if (fread(buf, 1, sizeof(buf), fp) != sizeof(buf)) {
    fprintf(stderr, "invalid fine JPEG source: header is truncated.\n");
    fclose(fp);
    return -1;
  }
  size = get_u_int(buf + 4);
  if (fseek(fp, 0, SEEK_END) != 0 || (file_size = ftell(fp)) < 0 ||
      (uint64_t)file_size != size) {
    fprintf(stderr, "invalid fine JPEG source: size does not match input.\n");
    fclose(fp);
    return -1;
  }
  fclose(fp);

  output_size = jpeg_fine_output_size(size);
  if (output_size == 0 || output_size > INT_MAX ||
      write_jpeg_fine_header(buf, outfp) == -1)
    return -1;

  if (write_file_file(filename, size, FINE_SOURCE_HEADER_SIZE, outfp) == -1)
    return (-1);

  if (write_file(eoi, sizeof(eoi), outfp) == -1)
    return (-1);

  return (int)output_size;
}
#else
int write_jpeg_fine(const uint8_t *buf, size_t len, FILE *outfp) {
  uint32_t size;
  size_t output_size;

  if (buf == NULL || len < FINE_SOURCE_HEADER_SIZE) {
    fprintf(stderr, "invalid fine JPEG source: header is truncated.\n");
    return -1;
  }

  size = get_u_int(buf + 4);
  output_size = jpeg_fine_output_size(size);
  if (output_size == 0 || output_size > INT_MAX || (size_t)size != len) {
    fprintf(stderr, "invalid fine JPEG source: size does not match input.\n");
    return -1;
  }
  if (write_jpeg_fine_header(buf, outfp) == -1 ||
      write_file(buf + FINE_SOURCE_HEADER_SIZE,
                 size - FINE_SOURCE_HEADER_SIZE, outfp) == -1)
    return (-1);

  if (write_file(eoi, sizeof(eoi), outfp) == -1)
    return (-1);

  return (int)output_size;
}

#endif
