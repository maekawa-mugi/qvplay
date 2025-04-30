#include "command.h"
#include "common.h"
#include "config.h"
#include <setjmp.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#ifdef X68K
#include "x68k/tty_x68.h"
#else
#ifdef _WIN32
#include "win32/tty_w32.h"
#else
#ifdef OS2
#include "os2/tty_os2.h"
#else
#ifdef DOS
#include "dos/tty_dos.h"
#else
#include "tty.h"
#include <termios.h>
#endif /* DOS */
#endif /* OS2 */
#endif /* WIN32 */
#endif /* X68 */
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
}
