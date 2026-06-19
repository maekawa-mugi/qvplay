#include "command.h"
#include "common.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint8_t input[256];
static size_t input_length;
static size_t input_offset;
static uint8_t output[256];
static size_t output_length;

static void reset_transport(void) {
  input_length = 0;
  input_offset = 0;
  output_length = 0;
}

static void append_block(const uint8_t *data, size_t length, uint8_t type,
                         int valid_checksum) {
  int sum = (int)((length >> 8) & 0xff) + (int)(length & 0xff) + type;
  size_t i;

  input[input_length++] = STX;
  input[input_length++] = (uint8_t)(length >> 8);
  input[input_length++] = (uint8_t)length;
  for (i = 0; i < length; i++) {
    input[input_length++] = data[i];
    sum += data[i];
  }
  input[input_length++] = type;
  input[input_length++] = valid_checksum ? (uint8_t)~sum : 0;
}

static void require(int condition, const char *message) {
  if (!condition) {
    fprintf(stderr, "FAIL: %s\n", message);
    exit(1);
  }
}

int readtty(int fd, uint8_t *buf, int length) {
  (void)fd;
  if (length <= 0 || input_offset == input_length)
    return 0;
  *buf = input[input_offset++];
  return 1; /* Deliberately short to exercise read_exact(). */
}

int writetty(int fd, uint8_t *buf, int length) {
  int count = length > 1 ? 1 : length;
  (void)fd;
  if (count <= 0)
    return 0;
  memcpy(output + output_length, buf, (size_t)count);
  output_length += (size_t)count;
  return count; /* Deliberately short to exercise write_all(). */
}

void flushtty(int fd) { (void)fd; }
int changespeed(int fd, int baud) {
  (void)fd;
  (void)baud;
  return 1;
}
void sleep(int seconds) { (void)seconds; }
void Exit(int code) {
  fprintf(stderr, "unexpected Exit(%d)\n", code);
  exit(2);
}

static void test_checksum_retry_does_not_advance(void) {
  const uint8_t first[] = {'A', 'B'};
  const uint8_t last[] = {'C'};
  const uint8_t expected_output[] = {DC2, NAK, ACK, ACK};
  uint8_t received[3] = {0};

  reset_transport();
  append_block(first, sizeof(first), ETB, 0);
  append_block(first, sizeof(first), ETB, 1);
  append_block(last, sizeof(last), ETX, 1);

  require(QVblockrecv(received, sizeof(received), sizeof(received)) == 3,
          "retried transfer length");
  require(memcmp(received, "ABC", 3) == 0,
          "retried block overwrites the same destination");
  require(output_length == sizeof(expected_output) &&
              memcmp(output, expected_output, sizeof(expected_output)) == 0,
          "retry control sequence");
}

static void test_oversized_block_is_rejected(void) {
  const uint8_t data[] = {1, 2, 3};
  uint8_t received[2] = {0};

  reset_transport();
  append_block(data, sizeof(data), ETX, 1);
  require(QVblockrecv(received, sizeof(received), 0) == -1,
          "oversized block rejection");
  require(output_length == 2 && output[0] == DC2 && output[1] == NAK,
          "oversized block NAK");
}

static void test_unexpected_final_size_is_rejected(void) {
  const uint8_t data[] = {1, 2};
  uint8_t received[3] = {0};

  reset_transport();
  append_block(data, sizeof(data), ETX, 1);
  require(QVblockrecv(received, sizeof(received), sizeof(received)) == -1,
          "unexpected final size rejection");
  require(output_length == 2 && output[0] == DC2 && output[1] == NAK,
          "unexpected final size NAK");
}

static void test_short_writes_are_completed(void) {
  reset_transport();
  wstr("TEST", 4);
  require(output_length == 4 && memcmp(output, "TEST", 4) == 0,
          "short writes are completed");
}

int main(void) {
  QVsetfd(1);
  test_checksum_retry_does_not_advance();
  test_oversized_block_is_rejected();
  test_unexpected_final_size_is_rejected();
  test_short_writes_are_completed();
  puts("protocol tests passed");
  return 0;
}
