#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <iocslib.h>
#include "common.h"
#include "x68k/tty_x68.h"

#define TTYTIMEOUT (10 * 100)

static int oldttymode ;
static void check_key()
{
	if(BITSNS(0xc) & 0x02)  /* BREAK key */
		exit(2);
	if(BITSNS(0x0) & 0x02)  /* ESC key */
		exit(2);
	if((BITSNS(0xe) & 0x02) && (BITSNS(0x5) & 0x10))
		exit(2);    /* CTRL C */

	return;
}
int changespeed(fd, baud)
     int fd;
     int baud;
{
	volatile unsigned char *scc_cmd = (unsigned char *) 0xe98005;
	volatile unsigned char *scc_data = (unsigned char *) 0xe98007;
	int brate = 0x7; /* B9600 */
	unsigned char u;
	int stack;

	if (baud > B9600 && siochk() == 0) {
		fprintf(stderr, "sio driver(c.f. tmsio, bsio, psxio ..) does not exist.\n");
		closetty(fd);
		exit(1);
	}

	switch(baud){
		case B38400:
		brate = 0x9;
		break;
		case B19200:
		brate = 0x8;
		break;
		case B9600:
		default:
		brate = 0x7;
		break;
	}
	oldttymode = SET232C(0x4c00 | brate);  /* 8 bit non parity stop 1 no flow control */
	stack = B_SUPER(0);
	u = *scc_cmd; /* dummy read */
	*scc_cmd = 0x05;
	*scc_cmd = 0xe8;  /* set RTS off  DSR on */
	B_SUPER(stack);
	flushtty(fd);
	return(1);
}

int opentty(path)
     char *path;
{
	changespeed(0, DEFAULTBAUD);
	return(1);
}

int readtty(fd, p, c)
     int fd;
     unsigned char *p;
     int c;
{
	int i =0;
	int j;

	j = ONTIME();

	while(ISNS232C() == 0){
		check_key();
		if( (ONTIME() - j) > TTYTIMEOUT){
			fprintf(stderr,"tty not respond.\n");
			return(0);
		}
	}

	for(i = 0 ; i < c; i++){
		check_key();
		p[i]=INP232C();
	}
	return(c);
}

void flushtty(fd)
     int fd;
{

	if(ISNS232C() == 0)
		return;

	while(ISNS232C()){
		check_key();
		INP232C();
	}
	return;
}

int writetty(fd, p, c)
     int fd;
     unsigned char *p;
     int c;
{
	int i;

	while(OSNS232C() == 0)
		check_key();
		
	for(i = 0 ; i < c; i++){
		OUT232C(p[i]);
	}
	return(c);
}

int closetty(fd)
     int fd;
{
	SET232C(oldttymode);
	return(1);
}
