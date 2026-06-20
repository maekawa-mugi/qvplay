/* tty device */
/* linux	"/dev/ttyS0" */
/* NEWS-OS 4.2  "/dev/tty00" */
/* NEWS-OS 6.X	"/dev/term/00" */
/* EWS4800	"/dev/term/00" */
/* SunOS 4.1.4	"/dev/ttya" */
/* Solaris2.3	"/dev/ttya" */
/* BSD/OS 2.0	"/dev/tty00" */
/* FreeBSD      "/dev/cuaa0" */
/* NEXTSTEP "/dev/ttya" */
/* Windows NT "COM1" */
#if defined(_WIN32)
#define RSPORT "COM1"
#else
/* for unix variant */
#define RSPORT "/dev/qvtty"
#endif

/* NEXTSTEP and (Old) FreeBSD cannot hold RTS to off
   so you short link cable's RTS and GND */
/* FreeBSD 2.2 Release can hold RTS to off. */
#if defined(NeXT)
#define NO_RTS
#endif

#define STX 0x02
#define ETX 0x03
#define ENQ 0x05
#define ACK 0x06
#define DC2 0x12
#define NAK 0x15
#define ETB 0x17

#define DEFAULTBAUD B9600
#define LIGHT 5
#define TOP 4
#define HIGH 3
#define MID 2
#define DEFAULT 1

/* for function prototypes */
#ifdef STDC_HEADERS
#define P__(x) x
#else
#define P__(x) ()
#endif

void Exit (int);

#define RETRY 100
#define LOW_BATT 0x3b
#define SECTOR 0x0600

#define THUMBNAIL_WIDTH 52
#define THUMBNAIL_HEIGHT 36

#define THUMBNAIL_MAXSIZ 4 * 1024
#define JPEG_MAXSIZ 32 * 1024
#define JPEG_MAXSIZ_VGA 70 * 1024 /* not for QV770/QV700(may be QV5000SX) */

#ifdef BINARYFILEMODE
#define WMODE "wb"
#define RMODE "rb"
#else
#define WMODE "w"
#define RMODE "r"
#endif

#define MAX_PICTURE_NUM_QV10 96
