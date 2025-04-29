/* config.h.  Generated automatically by configure.  */
/* config.h.in.  Generated automatically from configure.in by autoheader.  */

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef gid_t */

/* Define as __inline if that's what the C compiler calls it.  */
/* #undef inline */

/* Define to `unsigned' if <sys/types.h> doesn't define.  */
/* #undef size_t */

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

/* Define if you can safely include both <sys/time.h> and <time.h>.  */
#define TIME_WITH_SYS_TIME 1

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef uid_t */

/* Define if you have the select function.  */
#define HAVE_SELECT 1

/* Define if you have the setreuid function.  */
#define HAVE_SETREUID 0

/* Define if you have the <fcntl.h> header file.  */
#define HAVE_FCNTL_H 1

/* Define if you have the <sgtty.h> header file.  */
/* #undef HAVE_SGTTY_H */

/* Define if you have the <sys/ioctl.h> header file.  */
#define HAVE_SYS_IOCTL_H 1

/* Define if you have the <sys/param.h> header file.  */
#define HAVE_SYS_PARAM_H 0

/* Define if you have the <sys/time.h> header file.  */
#define HAVE_SYS_TIME_H 0

/* Define if you have the <termio.h> header file.  */
#define HAVE_TERMIO_H 0

/* Define if you have the <termios.h> header file.  */
#define HAVE_TERMIOS_H 0

/* Define if you have the <unistd.h> header file.  */
#define HAVE_UNISTD_H 0


#define inline
#define DONTCAREUID	 1
#define BINARYFILEMODE 1

#define USEWORKFILE 1

/* for VC++1.5 */
#include <stdlib.h>
#include <io.h>


/*
	Win32用のconfig.hから
	#include <windows.h>を抜いて、
	それに相当したものを追加しました。
*/
typedef unsigned short		u_short;
typedef unsigned long		u_long;
typedef unsigned int		u_int;
typedef unsigned char		u_char;

#define DOS
