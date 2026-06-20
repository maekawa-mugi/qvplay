#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
int write_jpeg(const uint8_t *, size_t, FILE *);
int write_file(const void *, size_t, FILE *);
size_t jpeg_fine_output_size(size_t);

#ifdef USEWORKFILE
int write_jpeg_fine(const char *, FILE *);
int write_file_file(const char *, long, long, FILE *);
#else
int write_jpeg_fine(const uint8_t *, size_t, FILE *);
#endif
