/*-------------------------------------------------------------
TTY_DOS.C	By Jun Suda  (1996.12.14)
-------------------------------------------------------------*/


#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include <time.h>

#include "mcd_if.h"

#define	TTYTIMEOUT	15
#define	SENDBUF		1024
#define	RECVBUF		1024

char	send_buf[SENDBUF];		/* 送信バッファ */
char	recv_buf[RECVBUF];		/* 受信バッファ */

int changespeed(fd, baud)
     int fd;
     int baud;
{ 
	mcd_lprm_t lprm;
	
	lprm.baud 	= baud;
	lprm.length	= 8;
	lprm.stop	= 1;
	lprm.parity	= PARITY_NONE;
	lprm.flow	= FLOW_NONE;
	lprm.limit	= 0;
	(*mcd_setlinepara)(&lprm);
	
	(*mcd_linectrl)(MCD_DTRON);
	(*mcd_linectrl)(MCD_RTSOFF);

	(*mcd_setbufstat)(CLR_TX_BUF);
	(*mcd_setbufstat)(CLR_RX_BUF);
	
	return(1);
}

int opentty(tty)
char *tty;
{
	int		fd;
	mcd_lprm_t lprm;
	mcd_buffer_t t;

	if((fd = mcd_open(tty,NULL,NULL)) < 0) {
	 	fprintf(stderr, "mcd_open error\n");
		return(-1);
	}
	if (!mcd_entry(fd)) {
	 	fprintf(stderr, "mcd_entry error\n");
		close(fd);
		return(-1);
	}


	lprm.baud 	= 9600;
	lprm.length	= 8;
	lprm.stop	= 1;
	lprm.parity	= PARITY_NONE;
	lprm.flow	= FLOW_NONE;
	lprm.limit	= 0;
	(*mcd_setlinepara)(&lprm);

	t.tx_buflen = SENDBUF;		/* 送信バッファの大きさ */
	t.tx_buffer = send_buf;		/* 送信バッファのアドレス */
	t.rx_buflen = RECVBUF;		/* 受信バッファの大きさ */
	t.rx_buffer = recv_buf;		/* 受信バッファのアドレス */
	(*mcd_setbuffer)(&t);

	(*mcd_linectrl)(MCD_DTRON);
	(*mcd_linectrl)(MCD_RTSOFF);

	(*mcd_setbufstat)(CLR_TX_BUF);
	(*mcd_setbufstat)(CLR_RX_BUF);

	return(fd);
}


int readtty(fd, p, c)
     int fd;
     unsigned char *p;
     int c;
{
	int i, pp;
	time_t lt;
	time_t lt2;
	time(&lt);

	for (i = 0; i < c; i++) {
		while ((pp = (*mcd_getc)()) == -1) {
			time(&lt2);		
			if((lt2 - lt) >= TTYTIMEOUT) {
				fprintf(stderr, "tty not respond.\n");
				return(i);
			}
		}
		*p++ = pp;
	}
	return(i);
}


void flushtty(fd)
     int fd;
{
	(*mcd_setbufstat)(CLR_TX_BUF);
	(*mcd_setbufstat)(CLR_RX_BUF);

	return;
}

int writetty(fd, p, c)
     int fd;
     unsigned char *p;
     int c;
{
	int i;

	for (i = 0; i < c; i++) {
		(*mcd_putc)(*p++);
	}

	return(i);
}

int closetty(fd)
     int fd;
{
	close(fd);
	return(1);
}
