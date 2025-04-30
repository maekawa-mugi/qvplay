#include "config.h"
#include <stdint.h>
#include <sys/types.h>
uint16_t get_uint16_t(uint8_t *buf) { return ((uint16_t)buf[0] << 8) | buf[1]; }

uint32_t get_u_int(uint8_t *buf) {
  u_int t;

  t = (((u_int)buf[0] << 8) | buf[1]) << 16;
  ;
  t |= ((u_int)buf[2] << 8) | buf[3];
  ;
  return t;
}
