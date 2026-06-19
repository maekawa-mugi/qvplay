#include "config.h"
#include "tty_w32.h"

#include <stdio.h>

#define TTYTIMEOUT 15

static HANDLE hCom = INVALID_HANDLE_VALUE;

static void report_error(const char *message) {
  fprintf(stderr, "%s (%lu)\n", message, (unsigned long)GetLastError());
}

int changespeed(int fd, int baud) {
  DCB dcb = {0};
  (void)fd;

  dcb.DCBlength = sizeof(dcb);
  if (!GetCommState(hCom, &dcb)) {
    report_error("Can't get tty attributes");
    return -1;
  }
  dcb.BaudRate = (DWORD)baud;
  if (!SetCommState(hCom, &dcb)) {
    report_error("Can't set tty attributes");
    return -1;
  }
  PurgeComm(hCom, PURGE_RXABORT | PURGE_TXABORT | PURGE_RXCLEAR |
                      PURGE_TXCLEAR);
  EscapeCommFunction(hCom, CLRRTS);
  FlushFileBuffers(hCom);
  return 1;
}

int opentty(char *path) {
  COMMTIMEOUTS timeouts = {0};
  DCB dcb = {0};

  hCom = CreateFileA(path, GENERIC_READ | GENERIC_WRITE, 0, NULL,
                     OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (hCom == INVALID_HANDLE_VALUE) {
    report_error("Can't open tty");
    return -1;
  }

  dcb.DCBlength = sizeof(dcb);
  if (!GetCommState(hCom, &dcb)) {
    report_error("Can't get tty attributes");
    goto fail;
  }
  timeouts.ReadTotalTimeoutConstant = 1000 * TTYTIMEOUT;
  timeouts.WriteTotalTimeoutConstant = 1000 * TTYTIMEOUT;
  if (!SetCommTimeouts(hCom, &timeouts)) {
    report_error("Can't set tty timeout");
    goto fail;
  }

  dcb.BaudRate = B9600;
  dcb.ByteSize = 8;
  dcb.Parity = NOPARITY;
  dcb.StopBits = ONESTOPBIT;
  dcb.fOutxCtsFlow = FALSE;
  dcb.fOutxDsrFlow = FALSE;
  dcb.fDtrControl = DTR_CONTROL_ENABLE;
  dcb.fDsrSensitivity = FALSE;
  dcb.fOutX = FALSE;
  dcb.fInX = FALSE;
  dcb.fNull = FALSE;
  dcb.fRtsControl = RTS_CONTROL_ENABLE;
  if (!SetCommState(hCom, &dcb)) {
    report_error("Can't set tty attributes");
    goto fail;
  }

  PurgeComm(hCom, PURGE_RXABORT | PURGE_TXABORT | PURGE_RXCLEAR |
                      PURGE_TXCLEAR);
  FlushFileBuffers(hCom);
  EscapeCommFunction(hCom, CLRRTS);
  return 1;

fail:
  CloseHandle(hCom);
  hCom = INVALID_HANDLE_VALUE;
  return -1;
}

int readtty(int fd, uint8_t *buf, int length) {
  DWORD received = 0;
  (void)fd;

  if (length < 0)
    return -1;
  if (!ReadFile(hCom, buf, (DWORD)length, &received, NULL)) {
    report_error("Can't read tty");
    return -1;
  }
  if (length != 0 && received == 0)
    fprintf(stderr, "tty did not respond.\n");
  return (int)received;
}

void flushtty(int fd) {
  (void)fd;
  PurgeComm(hCom, PURGE_RXABORT | PURGE_TXABORT | PURGE_RXCLEAR |
                      PURGE_TXCLEAR);
  FlushFileBuffers(hCom);
}

int writetty(int fd, uint8_t *buf, int length) {
  DWORD written = 0;
  (void)fd;

  if (length < 0)
    return -1;
  if (!WriteFile(hCom, buf, (DWORD)length, &written, NULL)) {
    report_error("Can't write tty");
    return -1;
  }
  return (int)written;
}

int closetty(int fd) {
  BOOL result;
  (void)fd;

  if (hCom == INVALID_HANDLE_VALUE)
    return 0;
  result = CloseHandle(hCom);
  hCom = INVALID_HANDLE_VALUE;
  return result ? 1 : 0;
}

void sleep(int seconds) {
  if (seconds > 0)
    Sleep((DWORD)seconds * 1000);
}
