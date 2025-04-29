/*-------------------------------------------------------------

MCDCALL.H	By Jun Suda  (1996.12.14)

 	JM1PQQ/Yassさんの作られた
 	Multi-Port Communication Driver (MCD) を利用するための
 	関数群をコンパイルする際に必要なヘッダーファイルです。
 	
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

#define FALSE	0
#define TRUE	(!FALSE)

typedef struct {
	byte minver;			/* マイナーバージョン */
	byte majver;			/* メジャーバージョン */
	char _far *id_str;		/* ID文字列へのポインタ */
} mcd_getid_t;

typedef struct {
	void (_far *func)(void);
} mcd_entry_t;

typedef struct {
	word _far *offs;	/* 各関数のnearポインタの配列の先頭アドレス */
	word cseg;			/* 関数のセグメント */
	word tblnum;		/* 配列の大きさ */
} mcd_c_entry_t;

typedef struct {
	word _far *offs;	/* 各関数のnearポインタの配列の先頭アドレス */
	word cseg;			/* 関数のセグメント */
	word tblnum;		/* 配列の大きさ */
	byte reserved[6];	/* mcd_c_entry_tとサイズを変えるためのダミーです */
} mcd_p_entry_t;

typedef struct {
	byte volatile _far *inmcd_addr;		/* IN-MCDフラグのアドレス */
	word tint_times;					/* タイマ割り込み回数（１０秒間） */
	dword volatile _far *timer_addr;	/* フリーランタイムカウンタアドレス */
	dword volatile _far *iocnt_addr;	/* 受信カウンタのアドレス */
										/* *(iocnt_addr+1)は送信カウンタ */
	byte reserved[16];					/* 予約 */
} mcd_mcdinfo_t;

typedef struct MCD_TMSRV {
	struct MCD_TMSRV _far * (_far _pascal *func)(word);
							/* 登録する（されている）関数 */
	word ch;				/* funcの引き数 */
	word dseg;				/* funcをコールするときのDS */
	word _far *stack;		/* funcをコールするときのSS:SP */
} mcd_timer_t;

#ifdef NETB_OLD
typedef struct {
	char machine[8];
	char device[9];
} net_remote_t;
#endif

typedef struct {
	word func;					/* 機能番号 */
	word retcode;				/* 戻り値 */
	void _far *data;			/* 受渡しするデータへのfarポインタ */
} mcd_machineio_t;


typedef struct {
	word func;					/* IOCTL-INPUTの機能番号 */
	union {
		mcd_getid_t		id;
		mcd_entry_t		entry;
		mcd_c_entry_t	c_entry;
		mcd_p_entry_t	p_entry;
		mcd_mcdinfo_t	mcdinfo;
		mcd_timer_t		timer;
#ifdef NETB_OLD
		net_remote_t _far *call_remote;
		net_remote_t _far *get_remote;
#endif
		mcd_machineio_t	machine_io;
	} body;
} mcd_ioctl_t;

/* 
 * IOCTL-INPUTの機能番号
 */
#define MCD_IOCTL_GETID			9
#define MCD_IOCTL_GETENT		20
#define MCD_IOCTL_GETCENT		21
#define MCD_IOCTL_GETPENT		22
#define MCD_IOCTL_MCDINFO		23
#define MCD_IOCTL_GETTIMER		24
#define MCD_IOCTL_SETTIMER		25
#ifdef NETB_OLD
#define F_CALL_REMOTE		22		/* WRITE */
#define F_GET_REMOTE		22		/* READ */
#endif
#define MCD_IOCTL_MACHINEIO		30

#ifdef NETB_OLD
#define L_CALL_REMOTE		sizeof (net_remote_t _far *)
#define L_GET_REMOTE		sizeof (net_remote_t _far *)
#endif

/*
 * ダイレクトPascalコールファンクション番号（テーブル番号）
 */
#define MCD_DPC_GETID			0
#define MCD_DPC_GET_LINEPARA	1
#define MCD_DPC_SET_LINEPARA	2
#define MCD_DPC_GET_LINESTAT	3
#define MCD_DPC_SET_LINESTAT	4
#define MCD_DPC_GET_BUFSTAT		5
#define MCD_DPC_SET_BUFSTAT		6
#define MCD_DPC_BLOCKREAD		7
#define MCD_DPC_BLOCKWRITE		8
#define MCD_DPC_CHK_BREAK		9
#define MCD_DPC_SND_BREAK		10
#define MCD_DPC_GET_BUFFER		11
#define MCD_DPC_SET_BUFFER		12
#define MCD_DPC_RST_BUFFER		13
#define MCD_DPC_GETC			14
#define MCD_DPC_PUTC			15
#define MCD_DPC_SBUF_CHARS		16
#define MCD_DPC_SBUF_FREE		17
#define MCD_DPC_RBUF_CHARS		18
#define MCD_DPC_RBUF_FREE		19
#define MCD_DPC_NDREAD			20
#define MCD_DPC_GET_SPEED		21
#define MCD_DPC_SET_SPEED		22
#define MCD_DPC_GET_RXFLOWSIZE	23
#define MCD_DPC_SET_RXFLOWSIZE	24
#define MCD_DPC_GET_DEVOPEN		25
#define MCD_DPC_SET_DEVOPEN		26
#define MCD_DPC_LINECTRL		27
#define MCD_DPC_SET_FLOW		28
