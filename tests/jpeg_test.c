#include "jpeg.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static void require(int condition, const char *message) {
  if (!condition) {
    fprintf(stderr, "FAIL: %s\n", message);
    exit(1);
  }
}

int main(void) {
  uint8_t source[140] = {0};
  size_t expected_size;
  long actual_size;
  FILE *output;
  int result;

  source[7] = (uint8_t)sizeof(source);
  expected_size = jpeg_fine_output_size(sizeof(source));
  require(expected_size != 0, "fine JPEG size is calculable");
  require(jpeg_fine_output_size(135) == 0, "truncated source is rejected");

  output = tmpfile();
  require(output != NULL, "temporary output opens");
  result = write_jpeg_fine(source, sizeof(source), output);
  require(result == (int)expected_size, "writer returns calculated size");
  require(fflush(output) == 0, "output flushes");
  actual_size = ftell(output);
  require(actual_size >= 0 && (size_t)actual_size == expected_size,
          "calculated size matches emitted bytes");
  fclose(output);

  source[7]--;
  output = tmpfile();
  require(output != NULL, "mismatch output opens");
  require(write_jpeg_fine(source, sizeof(source), output) == -1,
          "source size mismatch is rejected");
  fclose(output);

  puts("JPEG tests passed");
  return 0;
}
