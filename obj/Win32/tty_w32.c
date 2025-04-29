#include "config.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>

#define	TTYTIMEOUT	15
HANDLE hCom;

int changespeed(fd, baud)
     int fd;
     int baud;
{ 
	DCB dcb;
	DWORD Error;

	if (!GetCommState(hCom, &dcb)) {
		Error = GetLastError();
		fprintf(stderr, " Can't get tty attribute.(%d)\n", Error);
		return(-1);
	}

	dcb.BaudRate = baud;
 
	if (!SetCommState(hCom, &dcb)) {
		Error = GetLastError();
		fprintf(stderr, "Can't set tty attribute.(%d)\n",Error);
 		return(-1);
 	}
   	PurgeComm(hCom, PURGE_RXABORT|PURGE_TXABORT|PURGE_RXCLEAR|PURGE_TXCLEAR);
	EscapeCommFunction(hCom, CLRRTS); // set RTS off

	FlushFileBuffers(hCom);

	return(1);
}

int opentty(tty)
     char *tty;
{
	DCB dcb;
	DWORD Error;
	COMMTIMEOUTS  CommTimeOuts ;

	hCom = CreateFile(tty,
	   	GENERIC_READ | GENERIC_WRITE,
    	0,    
    	NULL, 
    	OPEN_EXISTING, 
    	FILE_ATTRIBUTE_NORMAL, 
    	NULL 
    );

	if (hCom == INVALID_HANDLE_VALUE) {
    	Error = GetLastError();
	 	fprintf(stderr, "opentty fail(%d)\n", Error);
		return(-1);
	}

	if (!GetCommState(hCom, &dcb)) {
	   	Error = GetLastError();
	 	fprintf(stderr, "get tty attribute fail(%d)\n", Error);
		return(-1);
 	}


      CommTimeOuts.ReadIntervalTimeout = 0;
      CommTimeOuts.ReadTotalTimeoutMultiplier = 0 ;
      CommTimeOuts.ReadTotalTimeoutConstant = 1000 * TTYTIMEOUT;
      CommTimeOuts.WriteTotalTimeoutMultiplier = 0 ;
      CommTimeOuts.WriteTotalTimeoutConstant = 1000 * TTYTIMEOUT;
   if(!SetCommTimeouts( hCom, &CommTimeOuts )){
   	   	Error = GetLastError();
	 	fprintf(stderr, "set tty timeout fail(%d)\n", Error);
		return(-1);
  }
  
   /*  9600 baud, 8 data bits, no parity, 1 stop bit. */

	dcb.BaudRate = 9600;
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
	dcb.fRtsControl = RTS_CONTROL_ENABLE;    // RTS flow control
	if(!SetCommState(hCom, &dcb)){
	  	   	Error = GetLastError();
	 	fprintf(stderr, "set tty attribute fail(%d)\n", Error);
		return(-1);
 }

   	PurgeComm(hCom, PURGE_RXABORT|PURGE_TXABORT|PURGE_RXCLEAR|PURGE_TXCLEAR);
	FlushFileBuffers(hCom);
	EscapeCommFunction(hCom, CLRRTS); // set RTS off

	return(1);
}

int readtty(fd, p, c)
     int fd;
     unsigned char *p;
     int c;
{
	int readed = -1;
	DWORD	Error;
//	time_t lt;
//	time_t lt2;
//	time(&lt);

	if(ReadFile(hCom, p, c, &readed, NULL)){
//		time(&lt2);		
//		if((lt2 - lt) >= TTYTIMEOUT)
//			fprintf(stderr, "tty not respond.\n");
	 //   Error = GetLastError();
	//	fprintf(stderr, "readtty ok(%d) %d bytes\n", Error, readed);
		if(c == 0)
			return(0);
		if(readed == 0)
			fprintf(stderr, "tty not respond.\n",readed , c);
		return(readed);
	}else{
	    Error = GetLastError();
		fprintf(stderr, "readtty fail(%d)\n", Error);
		return(-1);
	}
}

void flushtty(fd)
     int fd;
{
	PurgeComm(hCom, PURGE_RXABORT|PURGE_TXABORT|PURGE_RXCLEAR|PURGE_TXCLEAR);
	FlushFileBuffers(hCom);
	
	return;
}

int writetty(fd, p, c)
     int fd;
     unsigned char *p;
     int c;
{
	int written = 0;
	DWORD Error;

	if(WriteFile(hCom, p, c, &written, NULL))
		return(written);
	else{
	    Error = GetLastError();
		fprintf(stderr, "write tty fail(%d)\n", Error);
		return(-1);
	}
}

int closetty(fd)
     int fd;
{
	if(CloseHandle(hCom))
  		return(1);
	else
		return(0);
}

void sleep(sec)
	int sec;
{
   clock_t goal;
   goal = (clock_t) sec * CLOCKS_PER_SEC + clock();
   while( goal > clock() )
      ;	
}
