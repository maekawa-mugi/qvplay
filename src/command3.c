#include "config.h"
#include <stdio.h>
#include <sys/types.h>
#include <setjmp.h>
#include "common.h"
#include "command.h"
#ifdef X68
#include "tty_x68.h"
#else
#ifdef WIN32
#include "tty_w32.h"
#else
#ifdef OS2
#include "tty_os2.h"
#else
#ifdef DOS
#include "tty_dos.h"
#else
#include "tty.h"
#include <termios.h>
#endif /* DOS */
#endif /* OS2 */
#endif /* WIN32 */
#endif /* X68 */
#if HAVE_UNISTD_H
#include <unistd.h>
#endif

int
QValldelete()
{
  u_char	s;
  if (!QVok())
    return -1;
  wstr("DD", 2);
  s = rbyte();
  if(checksum(s) == -1) return(-1);
  wbyte(ACK);
}



