/*-------------------------------------------------------------

MCDCALL.C	By Jun Suda  (1996.12.14)

 	JM1PQQ/Yassさんの作られた
 	Multi-Port Communication Driver (MCD) を利用するために
 	必要な関数群です。
 	
 	JM1PQQ/YassさんのMCDのパッケージにある、サンプルファイル
 	を多大に参考にしています。(というか切り貼りに近いです。)
	なお、参考にしたMCDパッケージは(MCD091.LZH)です。
	
	MCDに関しては、
　　　NIFTY-Serve FBBSS
　　　NIFTY-Serve FGALTM
　　　NIFTY-Serve FGALTLB
　　　ASCII-NET   pool msdos
	などに作者のYassさんがuploadされています。

-------------------------------------------------------------*/

#include <dos.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <string.h>

#include "mcd_if.h"
#include "mcdcall.h"


void (_far _pascal *mcd_getid)(mcd_getid_t _far *buf);
void (_far _pascal *mcd_getlinepara)(mcd_lprm_t _far *param);
void (_far _pascal *mcd_setlinepara)(mcd_lprm_t _far *param);
word (_far _pascal *mcd_getlinestat)(void);
void (_far _pascal *mcd_setlinestat)(word stat);
void (_far _pascal *mcd_getbufstat)(mcd_bufstat_t _far *stat);
void (_far _pascal *mcd_setbufstat)(word stat);
word (_far _pascal *mcd_blockread)(byte _far *buf, word size);
word (_far _pascal *mcd_blockwrite)(byte _far *buf, word size);
int  (_far _pascal *mcd_chkbrksig)(void);
void (_far _pascal *mcd_sndbrksig)(void);
void (_far _pascal *mcd_getbuffer)(mcd_buffer_t _far *buf);
void (_far _pascal *mcd_setbuffer)(mcd_buffer_t _far *buf);
void (_far _pascal *mcd_rstbuffer)(void);
int  (_far _pascal *mcd_getc)(void);
int  (_far _pascal *mcd_putc)(int c);
word (_far _pascal *mcd_sbufchars)(void);
word (_far _pascal *mcd_sbuffree)(void);
word (_far _pascal *mcd_rbufchars)(void);
word (_far _pascal *mcd_rbuffree)(void);
int  (_far _pascal *mcd_ndread)(void);
dword (_far _pascal *mcd_getspeed)(void);
void (_far _pascal *mcd_setspeed)(dword speed);
void (_far _pascal *mcd_getflwsize)(word _far *rxstop, word _far *rxstart);
void (_far _pascal *mcd_setflwsize)(word rxstop, word rxstart);
void (_far _pascal *mcd_getdevopen)(int _far *ignore, int _far *count);
void (_far _pascal *mcd_setdevopen)(int ignore, int count);
word (_far _pascal *mcd_linectrl)(word ctrl);
void (_far _pascal *mcd_setflow)(word flow);


#if defined(_MSC_VER) && _MSC_VER < 600

int _fstrcmp(char _far *s1, char _far *s2)
{
	while (*s1 && *s2 && *s1 == *s2) {
		s1++;
		s2++;
	}
	return *s1 - *s2;
}

void _fmemcpy(void _far *d, void _far *s, int n)
{
	byte _far *dp = d;
	byte _far *sp = s;

	while (n--) {
		*dp++ = *sp++;
	}
}

#endif /* _MSC_VER < 600 */



/*
 * ioread, iowriteの戻り値
 *	正常終了	転送したバイト数（buflenと等しい）
 *	エラー		4402,4403ファンクションの戻り値AXの符号を反転したもの
 */
static int ioread(int fd, int buflen, char _far *bufptr)
{
	int rc;

#if _MSC_VER < 600
	union REGS regs;
	struct SREGS sregs;

	sregs.ds = FP_SEG(bufptr);
	regs.x.dx = FP_OFF(bufptr);
	regs.x.ax = 0x4402;
	regs.x.bx = fd;
	regs.x.cx = buflen;
	rc = intdosx(&regs, &regs, &sregs);
	if (regs.x.cflag) {
		return -rc;
	} else {
		return rc;
	}
#else
	_asm {
		push	ds
		lds		dx, bufptr
		mov		ax, 4402h
		mov		bx, fd
		mov		cx, buflen
		int		21h
		pop		ds
		mov		rc, ax
		jnc		noerror
	}
	return -rc;
noerror:
	return rc;
#endif
}



bool mcd_read(int fd, int func, int size, void _far *body)
{
	static mcd_ioctl_t iopac;

	iopac.func = (word)func;
	_fmemcpy(&iopac.body, body, size);
	if (ioread(fd, size + sizeof (iopac.func), (char _far *)&iopac) < 0) {
		return FALSE;
	}
	_fmemcpy(body, &iopac.body, size);
	return TRUE;
}


bool mcd_entry(int fd)
{
	mcd_p_entry_t entry;
	#define SETENT(r, a, n)	\
			(r (_far _pascal *)a)(((dword)entry.cseg << 16) | entry.offs[n])

	if (!mcd_read(fd, MCD_IOCTL_GETPENT, sizeof (entry), &entry)) {
		return FALSE;
	}
	mcd_getid       = SETENT(void, (mcd_getid_t _far *), MCD_DPC_GETID);
	mcd_getlinepara = SETENT(void, (mcd_lprm_t _far *), MCD_DPC_GET_LINEPARA);
	mcd_setlinepara = SETENT(void, (mcd_lprm_t _far *), MCD_DPC_SET_LINEPARA);
	mcd_getlinestat = SETENT(word, (void), MCD_DPC_GET_LINESTAT);
	mcd_setlinestat = SETENT(void, (word), MCD_DPC_SET_LINESTAT);
	mcd_getbufstat  = SETENT(void, (mcd_bufstat_t _far *),
													 MCD_DPC_GET_BUFSTAT);
	mcd_setbufstat  = SETENT(void, (word), MCD_DPC_SET_BUFSTAT);
	mcd_blockread   = SETENT(word, (byte _far *, word), MCD_DPC_BLOCKREAD);
	mcd_blockwrite  = SETENT(word, (byte _far *, word), MCD_DPC_BLOCKWRITE);
	mcd_chkbrksig   = SETENT(int, (void), MCD_DPC_CHK_BREAK);
	mcd_sndbrksig   = SETENT(void, (void), MCD_DPC_SND_BREAK);
	mcd_getbuffer   = SETENT(void, (mcd_buffer_t _far *), MCD_DPC_GET_BUFFER);
	mcd_setbuffer   = SETENT(void, (mcd_buffer_t _far *), MCD_DPC_SET_BUFFER);
	mcd_rstbuffer   = SETENT(void, (void), MCD_DPC_RST_BUFFER);
	mcd_getc        = SETENT(int, (void), MCD_DPC_GETC);
	mcd_putc        = SETENT(int, (int), MCD_DPC_PUTC);
	mcd_sbufchars   = SETENT(word, (void), MCD_DPC_SBUF_CHARS);
	mcd_sbuffree    = SETENT(word, (void), MCD_DPC_SBUF_FREE);
	mcd_rbufchars   = SETENT(word, (void), MCD_DPC_RBUF_CHARS);
	mcd_rbuffree    = SETENT(word, (void), MCD_DPC_RBUF_FREE);
	mcd_ndread      = SETENT(int, (void), MCD_DPC_NDREAD);
	mcd_getspeed    = SETENT(dword, (void), MCD_DPC_GET_SPEED);
	mcd_setspeed    = SETENT(void, (dword), MCD_DPC_SET_SPEED);
	mcd_getflwsize  = SETENT(void, (word _far *, word _far *),
												 MCD_DPC_GET_RXFLOWSIZE);
	mcd_setflwsize  = SETENT(void, (word, word), MCD_DPC_SET_RXFLOWSIZE);
	mcd_getdevopen  = SETENT(void, (int _far *, int _far *),
												 MCD_DPC_GET_DEVOPEN);
	mcd_setdevopen  = SETENT(void, (int, int), MCD_DPC_SET_DEVOPEN);
	mcd_linectrl    = SETENT(word, (word), MCD_DPC_LINECTRL);
	mcd_setflow     = SETENT(void, (word), MCD_DPC_SET_FLOW);
	return TRUE;
	#undef SETENT
}


bool mcd_id(int fd, int *maj, int *min)
{
	mcd_getid_t id;
	char name[10];

	id.id_str = (char _far *)name;
	if (!mcd_read(fd, MCD_IOCTL_GETID, sizeof (id), &id)) {
		return FALSE;
	}
	if (_fstrcmp(name, "MCD") != 0) {
		return FALSE;
	}
	*maj = id.majver;
	*min = id.minver;
	return TRUE;
}


int mcd_open(char *name, int *maj, int *min)
{
	int fd;
	int majver, minver;

	if ((fd = open(name, O_BINARY | O_RDWR)) == -1) {
		fprintf(stderr, "Can't open %s\n", name);
		return -1;
	}
	if (!mcd_id(fd, &majver, &minver)) {
		close(fd);
		fprintf(stderr, "Can't get MCD-ID\n");
		return -1;
	}
	if (maj != NULL) {
		*maj = majver;
	}
	if (min != NULL) {
		*min = minver;
	}
	return fd;
}
