#include "command.h"
#include "command3.h"
#include "common.h"
#include "config.h"
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#ifdef _WIN32
#include "win32/tty_w32.h"
#else
#include "tty.h"
#include <termios.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

int QValldelete(void) {
  uint8_t s;
  if (!QVok())
    return -1;
  wstr("DD", 2);
  s = rbyte();
  if (checksum(s) == -1)
    return (-1);
  wbyte(ACK);
  return 1;
}
