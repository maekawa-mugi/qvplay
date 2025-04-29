/*-------------------------------------------------------------

MCD-IF.H	By Jun Suda  (1996.12.14)

 	JM1PQQ/Yassさんの作られた
 	Multi-Port Communication Driver (MCD) を利用するために
 	必要なヘッダーファイルです。
 	
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

#ifdef __TURBOC__
	#define _far	far
	#define _near	near
	#define _pascal	pascal
#endif

/*--------------------------------------------------------
 型宣言と定数宣言
--------------------------------------------------------*/

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned long dword;
typedef int bool;
typedef void _far *fptr;

typedef struct {
	word baud;		/* 通信速度       1..65535, 0のときは別関数で取得する */
	word length;	/* キャラクタ長   5～8 */
	word stop;		/* ストップビット 1bit:1, 1.5bit:2, 2bit:3 */
	word parity;	/* パリティ       PARITY_NONE, PARITY_ODD, PARITY_EVEN */
	word flow;		/* フロー制御     FLOW_xxx (bitmap) */
	word limit;		/* ソフトフローのタイムリミット（秒） */
} mcd_lprm_t;

/* パリティ */
#define PARITY_NONE		0
#define PARITY_ODD		1
#define PARITY_EVEN		2

/* フロー制御 */
#define TXRX_FLOW		0x8000	/* このビットが1のときはTXとRXをそれぞれ指定 */
#define FLOW_HALF		0x4000	/* CTS/RTSによる半二重制御、他のビットは無視 */
#define FLOW_NONE		0x0000

/* TXRX_FLOWビットが0のとき */
#define FLOW_XONOFF		0x0001
#define FLOW_RTSCTS		0x0002
#define FLOW_DTRDSR		0x0004

/* TXRX_FLOWビットが1のとき */
#define FLOW_RX_XONOFF	0x0001
#define FLOW_RX_RTS		0x0002
#define FLOW_RX_DTR		0x0004
#define FLOW_TX_XONOFF	0x0100
#define FLOW_TX_CTS		0x0200
#define FLOW_TX_DSR		0x0400

#define MCD_DTR		0x0002		/* DTR					(Get/Set) */
#define MCD_RTS		0x0020		/* RTS					(Get/Set) */
#define MCD_DBK		0x0040		/* Break Signal			(Get) */
#define MCD_DSR		0x0080		/* DSR					(Get) */
#define MCD_DCD		0x2000		/* DCD					(Get) */
#define MCD_CTS		0x4000		/* CTS			 		(Get) */
#define MCD_RI		0x8000		/* Ring Indicator		(Get) */
#define MCD_PERR	0x0100		/* パリティーエラー     (Get) */
#define MCD_FERR	0x0200		/* フレーミングエラー   (Get) */
#define MCD_OERR	0x0400		/* オーバーランエラー   (Get) */
#define MCD_RXFULL	0x0800		/* MCDバッファフル      (Get) */

#define MCD_CTRLON	0x0001
#define MCD_DTROFF	(MCD_DTR)				/* DTR OFF */
#define MCD_DTRON	(MCD_DTR | MCD_CTRLON)	/* DTR ON */
#define MCD_RTSOFF	(MCD_RTS)				/* RTS OFF */
#define MCD_RTSON	(MCD_RTS | MCD_CTRLON)	/* RTS ON */

typedef struct {
	word txnum;			/* 送信バッファ内の未送信バイト数 */
	word rxnum;			/* 受信バッファ内の受信バイト数 */
	word rx_xoff;		/* 0以外：フロー制御で送信停止状態状態 */
	word tx_xoff;		/* 0以外：相手に送信停止をリクエストした */
} mcd_bufstat_t;

#define CLR_TX_BUF	0x0001		/* 送信バッファクリア */
#define CLR_RX_BUF	0x0002		/* 受信バッファクリア */
#define REQ_TXSTOP	0x0100		/* 相手に送信停止をリクエストする */
#define REQ_TXSTART	0x0200		/* 相手に送信再開をリクエスとする */
#define R_XOFF		0x1000		/* XOFF文字を受信したことにする */
#define R_XON		0x2000		/* XON文字を受信したことにする */
#define FLWSIG_OFF	0x0010		/* ハードウェアフロー制御信号OFF */
#define FLWSIG_ON	0x0020		/* ハードウェアフロー制御信号ON */
#define FLWSIG_ON2	0x0040		/* ハードウェアフロー制御信号ON(可能なら) */

typedef struct {
	word tx_buflen;				/* 送信バッファの大きさ */
	byte _far *tx_buffer;		/* 送信バッファのアドレス */
	word rx_buflen;				/* 受信バッファの大きさ */
	byte _far *rx_buffer;		/* 受信バッファのアドレス */
} mcd_buffer_t;



/*--------------------------------------------------------
 プロトタイプ宣言
--------------------------------------------------------*/

extern void (_far _pascal *mcd_getlinepara)(mcd_lprm_t _far *param);
extern void (_far _pascal *mcd_setlinepara)(mcd_lprm_t _far *param);
extern word (_far _pascal *mcd_getlinestat)(void);
extern void (_far _pascal *mcd_setlinestat)(word stat);
extern void (_far _pascal *mcd_getbufstat)(mcd_bufstat_t _far *stat);
extern void (_far _pascal *mcd_setbufstat)(word stat);
extern word (_far _pascal *mcd_blockread)(byte _far *buf, word size);
extern word (_far _pascal *mcd_blockwrite)(byte _far *buf, word size);
extern void (_far _pascal *mcd_getbuffer)(mcd_buffer_t _far *buf);
extern void (_far _pascal *mcd_setbuffer)(mcd_buffer_t _far *buf);
extern void (_far _pascal *mcd_rstbuffer)(void);
extern int (_far _pascal *mcd_getc)(void);
extern int (_far _pascal *mcd_putc)(int c);
extern word (_far _pascal *mcd_sbufchars)(void);
extern word (_far _pascal *mcd_sbuffree)(void);
extern word (_far _pascal *mcd_rbufchars)(void);
extern word (_far _pascal *mcd_rbuffree)(void);
extern int (_far _pascal *mcd_ndread)(void);
extern dword (_far _pascal *mcd_getspeed)(void);
extern void (_far _pascal *mcd_setspeed)(dword speed);
extern void (_far _pascal *mcd_getflwsize)(word _far *stop, word _far *start);
extern void (_far _pascal *mcd_setflwsize)(word stop, word start);
extern void (_far _pascal *mcd_getdevopen)(int _far *ignore, int _far *count);
extern void (_far _pascal *mcd_setdevopen)(int ignore, int count);
extern word (_far _pascal *mcd_linectrl)(word ctrl);
extern void (_far _pascal *mcd_setflow)(word word);

extern int mcd_open(char *name, int *maj, int *min);
extern bool mcd_entry(int fd);
