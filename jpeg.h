#include <stdio.h>
#include <stdint.h>
int write_jpeg(uint8_t *, FILE *);
int write_file(uint8_t *, int, FILE *);

#ifdef USEWORKFILE
int write_jpeg_fine(char *, FILE *);
int write_file_file(uint8_t *, int, int, FILE *);
#else
int write_jpeg_fine(uint8_t *, FILE *);
#endif
