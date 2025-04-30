#include <stdint.h>
void QVsetfd(int);
int QVgetfd(void);
int QVreset(int);
int QVhowmany(void);
void wbyte(uint8_t);
uint8_t rbyte(void);
void wstr(uint8_t *, int);
void rstr(uint8_t *, int);
int checksum(uint8_t);
int QVok(void);
int QVshowpicture(int);
int QVchangespeed(int);
#ifdef USEWORKFILE
int QVblockrecv_file(FILE *, int);
#endif
int QVblockrecv(uint8_t *, int);
int QVblocksend(uint8_t *, int);
int QVbattery(void);
int QVremain(int);
int QVswstat(void);
long QVrevision(void);
int QVsectorsize(int);
int QVdefaultpicture(int);
int QVnewprotocol(void);
int QVdisableAutoPowerOff(void);
