#include "config.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#define TTYTIMEOUT 15
static HFILE hCom;

int changespeed(int fd, int baud) {
  struct {
    unsigned long rate;
    char fraction;
  } param = {0, 0};
  ULONG i;
  APIRET Error;

  i = ((param.rate = baud) > 19200) ? sizeof(param) : sizeof(USHORT);

  if ((Error = DosDevIOCtl(
           hCom, IOCTL_ASYNC,
           param.rate > 19200 ? ASYNC_EXTSETBAUDRATE : ASYNC_SETBAUDRATE,
           (PVOID)&param, i, NULL, NULL, 0L, NULL)) != NO_ERROR) {
    fprintf(stderr, "Can't set tty baud (%d)\n", Error);
    return -1;
  }

  return 1;
}

int opentty(char *tty) {
  ULONG ulAct;
  BYTE Cmd = 0;
  USHORT usCommError, usBaud = 9600; /* 9600bps */
  DCBINFO DCBInfo;
  LINECONTROL LINECtrl = {
      8, 0, 0, 1}; /* 8 data bit, no parity, 1 stop bit, transmit break */
  MODEMSTATUS MODEMStat = {DTR_ON, RTS_OFF};
  APIRET Error;

  /* open com device */
  if ((Error = DosOpen(tty, &hCom, &ulAct, 0L, FILE_NORMAL, FILE_OPEN,
                       OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYNONE, 0L)) !=
      NO_ERROR) {
    fprintf(stderr, "opentty fail(%d)\n", Error);
    return -1;
  }

  /* get com device characteristics */
  if ((Error = DosDevIOCtl(hCom, IOCTL_ASYNC, ASYNC_GETDCBINFO, NULL, 0L, NULL,
                           (PVOID)&DCBInfo, sizeof(DCBInfo), NULL)) !=
      NO_ERROR) {
    fprintf(stderr, "get tty attribute fail(%d)\n", Error);
    return -1;
  }

  DCBInfo.usReadTimeout = 100 * TTYTIMEOUT;
  DCBInfo.usWriteTimeout = 0; /* set write timeout as soon as possible */
  DCBInfo.fbCtlHndShake = MODE_DTR_CONTROL;
  DCBInfo.fbFlowReplace = MODE_RTS_CONTROL;
  DCBInfo.fbTimeout = MODE_READ_TIMEOUT;

  /* update com device characteristics */
  if ((Error = DosDevIOCtl(hCom, IOCTL_ASYNC, ASYNC_SETDCBINFO, (PVOID)&DCBInfo,
                           sizeof(DCBInfo), NULL, NULL, 0L, NULL)) !=
      NO_ERROR) {
    fprintf(stderr, "set tty attribute fail(%d)\n", Error);
    return -1;
  }

  /* set 9600bps */
  if ((Error = DosDevIOCtl(hCom, IOCTL_ASYNC, ASYNC_SETBAUDRATE, (PVOID)&usBaud,
                           sizeof(usBaud), NULL, NULL, 0L, NULL)) != NO_ERROR) {
    fprintf(stderr, "set tty baud fail(%d)\n", Error);
    return -1;
  }

  /* set 8 data bit, no parity, 1 stop bit */
  if ((Error =
           DosDevIOCtl(hCom, IOCTL_ASYNC, ASYNC_SETLINECTRL, (PVOID)&LINECtrl,
                       sizeof(LINECtrl), NULL, NULL, 0L, NULL)) != NO_ERROR) {
    fprintf(stderr, "set line attribute fail(%d)\n", Error);
    return -1;
  }

  /* flush the input and output buffers */
  DosDevIOCtl(hCom, IOCTL_GENERAL, DEV_FLUSHOUTPUT, (PVOID)&Cmd, sizeof(Cmd),
              NULL, NULL, 0L, NULL);
  DosDevIOCtl(hCom, IOCTL_GENERAL, DEV_FLUSHINPUT, (PVOID)&Cmd, sizeof(Cmd),
              NULL, NULL, 0L, NULL);

  /* set DTR on / RTS off */
  if ((Error = DosDevIOCtl(hCom, IOCTL_ASYNC, ASYNC_SETMODEMCTRL,
                           (PVOID)&MODEMStat, sizeof(MODEMStat), NULL,
                           (PVOID)&usCommError, sizeof(usCommError), NULL)) !=
      NO_ERROR) {
    fprintf(stderr, "set DTR/RTS control fail(%d)\n", Error);
    return -1;
  }

  return 1;
}

int readtty(int fd, unsigned char *p, int c) {
  APIRET Error;
  ULONG readed = 0;

  if ((Error = DosRead(hCom, (PVOID)p, (ULONG)c, &readed)) != NO_ERROR) {
    fprintf(stderr, "readtty fail(%d)\n", Error);
    return -1;
  }
  if (readed != c)
    fprintf(stderr, "tty not respond.\n");

  return (int)readed;
}

void flushtty(int fd) {
  BYTE Cmd = 0;

  /* flush the input and output buffers */
  DosDevIOCtl(hCom, IOCTL_GENERAL, DEV_FLUSHOUTPUT, (PVOID)&Cmd, sizeof(Cmd),
              NULL, NULL, 0L, NULL);
  DosDevIOCtl(hCom, IOCTL_GENERAL, DEV_FLUSHINPUT, (PVOID)&Cmd, sizeof(Cmd),
              NULL, NULL, 0L, NULL);
}

int writetty(int fd, unsigned char *p, int c) {
  ULONG writed = 0;
  APIRET Error;

  if ((Error = DosWrite(hCom, (PVOID)p, (ULONG)c, &writed)) != NO_ERROR) {
    fprintf(stderr, "write tty fail(%d)\n", Error);
    return -1;
  }

  return (int)writed;
}

int closetty(int fd) { return (DosClose(hCom) == NO_ERROR) ? 0 : 1; }
