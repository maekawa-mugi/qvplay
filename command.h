#ifndef QVPLAY_COMMAND_H
#define QVPLAY_COMMAND_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

typedef enum {
  QV_DATA_JPEG,
  QV_DATA_THUMBNAIL
} QVdataType;

void QVsetfd(int);
int QVgetfd(void);
int QVreset(int);
int QVhowmany(void);
void wbyte(uint8_t);
uint8_t rbyte(void);
void wstr(const void *, int);
void rstr(uint8_t *, int);
int checksum(uint8_t);
int QVok(void);
int QVshowpicture(int);
int QVchangespeed(int);
#ifdef USEWORKFILE
int QVblockrecv_file(FILE *, int);
#endif
int QVblockrecv(uint8_t *, size_t, size_t);
int QVbattery(void);
int QVremain(int);
int QVswstat(void);
long QVrevision(void);
int QVsectorsize(int);
int QVdefaultpicture(int);
int QVnewprotocol(void);
int QVdisableAutoPowerOff(void);
int QVdeletepicture(int);
int QVtakepicture(void);
int QVgetpicture(int, uint8_t *, size_t, QVdataType, int, FILE *);
int QVmovepicture(int, int);
int QV4split(int *);
int QV9split(int *);
int QVprotect(int, int);
int QVhidepicnum(void);
int QVpoweroff(void);
int QVcolorpattern(void);
int QVpicattr(int);
int QVgetsize2(int);
int QVgetextdata(int, uint8_t *, size_t);
int QValldelete(void);

#endif
