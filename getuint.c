#include "config.h"
#include <stdint.h>
#include <sys/types.h>
uint16_t get_uint16_t(uint8_t *buf) { return ((uint16_t)buf[0] << 8) | buf[1]; }

uint32_t get_u_int(uint8_t *buf) {
  uint32_t t;

  t = (((uint32_t)buf[0] << 8) | buf[1]) << 16;
  t |= ((uint32_t)buf[2] << 8) | buf[3];
  return t;
}
