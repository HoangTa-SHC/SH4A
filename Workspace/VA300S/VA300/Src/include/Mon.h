//=============================================================================
/**
 *	@file Mon.h
 *
 *	@brief	ＳＨ-４ ＣＰＵモジュール モニタプログラム
 *	
 *	@date		1999/10/20
 *	@version	1.00 新規作成
 *	@author		S.Nakano
 *
 *	@date		2001/12/7
 *	@version	1.10 MCM(HJ945010BP)対応追加
 *	@author		S.Nalano
 *
 *	@date		2007/11/27
 *	@version	1.11 サンプル用に不要部分を削除、kernel.hをinclude
 *	@author		F.Saeki
 *
 *	@date		2011/4/7
 *	@version	1.20 includeファイルを変更
 *	@author		F.Saeki
 *
 *	Copyright (C) 2007, OYO Electric Corporation
 */
//=============================================================================

#include <ctype.h>
#include <machine.h>
#include <string.h>
#include <stdarg.h>
#if defined(NORTI_USE)
 #include "kernel.h"
 #define	BYTE	UB
 #define	WORD	UH
 #define	LONG	UW
#else
 #if defined(SH4)
//  #include "sh4def.h"
 #else
//  #include "sh2def.h"
 #endif
#endif

//=== レスポンスコード ===
#define	RX_NORM		("0000" CRLF)		///< 正常
#define	RX_DATA		("0001" CRLF)		///< データ送信要求
#define	RX_SAME		("0002" CRLF)		///< データ再送要求
#define	RX_ERAS		("0101" CRLF)		///< ＲＯＭイレーズ失敗
#define	RX_WERR		("0102" CRLF)		///< ＲＯＭ書き込み失敗

//=== エラーメッセージコード ===
#define	MSG_NONE		0
#define	MSG_BADCMD		1
#define MSG_BADHEX		2
#define	MSG_BADPARM		3
#define	MSG_BADADDR		4
#define	MSG_ODDADDR		5
#define	MSG_VERIFY		6

//=== 制御フラグ ===
#define	FLAG_VERIFY		0x01

#define	CRLF			"\x0D\x0A"

#define	MON_BUF_SIZE		128
#define	SEC_SIZE			512
#define	SEC_MUL_CNT			3

#ifndef	TRUE
	#define	TRUE			1
#endif
#ifndef	FALSE
	#define	FALSE			0
#endif

//#define	reset()				(**(void (**)(void))0)()

enum COMM_CH {							///< 通信チャンネル定義
	SCI_CH,								///< シリアル
	LAN_CH								///< LAN
};

//=== デバッグ用 ===
#if defined( _DEBUG_)
	#define DPRINT	SendText
#else
	#define DPRINT
#endif

#if defined(EXTERN)
#undef EXTERN
#endif

#if defined(_MON_MAIN_)
	#define	EXTERN
	const BYTE Hexasc[ 16]	= "0123456789abcdef";
	const BYTE HexAsc[ 16]	= "0123456789ABCDEF";
	const BYTE HexChr[256]	=
		"................"	"................"
		" !\"#$%&'()*+,-./"	"0123456789:;<=>?"
		"@ABCDEFGHIJKLMNO"	"PQRSTUVWXYZ[\\]^_"
		"`abcdefghijklmno"	"pqrstuvwxyz{|}~."
		"................"	"................"
		"................"	"................"
		"................"	"................"
		"................"	"................"
	;
#else
	#define	EXTERN	extern
	extern const BYTE Hexasc[];
	extern const BYTE HexAsc[];
	extern const BYTE HexChr[];
#endif

typedef struct						//=== 実行制御情報 ========================
{
	void	(*Proc)(void);			///< シーケンス制御ポインタ
	void	(*AppStart)(void);		///< アプリケーションポインタ
	BYTE	Flag;					///< 制御フラグ
	BYTE*	DumpAddr;				///< Dump先頭アドレス
	LONG	DumpSize;				///< Dump表示バイト数
	BYTE	DumpUnit;				///< Dump単位	('B'/'W'/'L')
	BYTE	EditUnit;				///< Edit単位	('B'/'W'/'L')
	BYTE*	topAddr;				///< コマンド開始アドレス
	BYTE*	endAddr;				///< コマンド終了アドレス
	WORD	Retry;					///< リトライカウンタ
									//=== ターミナル通信情報 ==================
	char	TxText[1024];			///< シリアル送信データ
	char	RxText[128];			///< シリアル受信データ
	BYTE	RxSetP;					///< シリアル受信ポインタ(格納)
	BYTE	RxGetP;					///< シリアル受信ポインタ(取出)
	char	RxBuff[128];			///< シリアル受信バッファ
	
	BYTE	sel;		//0：SCI、1：telnet 2003.01.10追加
	
	LONG	DlyTimer;				///< ディレイ用タイマー
	LONG	LedTimer;				///< LED用タイマー
	LONG	SlpTimer;				///< スリープ用タイマー
	int		ComCh;					///< 通信チャンネル
} CTRL;

typedef struct t_monmsg{
	struct t_monmsg *next;			///< 先頭4ﾊﾞｲﾄは OS で使用(4)	*/
	INT			ch;					///< チャンネル番号
	enum COMM_CH eCom;				///< 通信チャンネル種別
	UH			cnt;				///< データ数
	BYTE		buf[ MON_BUF_SIZE ];
} T_MONMSG;


EXTERN	CTRL	Ctrl;				///< 実行制御情報

//=== MON_MAIN.C ===
EXTERN ER Mon_ini(INT ch);				///< モニタ起動
EXTERN BOOL GetCmd(char*);				///< モニタコマンド受信
EXTERN void CmdExec(void);				///< Ｇ  コマンド
EXTERN void CmdHelp(void);				///< HELPコマンド
EXTERN void CmdVer(void);				///< Ｖｅｒコマンド
EXTERN void CmdMon(void);				///< Monコマンド
EXTERN BOOL Asc2Hex(char*,UW*,UH);		///< ASCII化16進テキストからバイナリへの変換
EXTERN void LongAsc(LONG,char*);		///< ロングデータをASCII化16進テキストに変換
EXTERN void WordAsc(WORD,char*);		///< ワードデータをASCII化16進テキストに変換
EXTERN void ByteAsc(BYTE,char*);		///< バイトデータをASCII化16進テキストに変換
EXTERN BOOL Asc2Dec(const char*,UW*,UH);///< ASCII化10進テキストからバイナリへの変換
EXTERN void SendMsg(BYTE);				///< エラーメッセージ送信
EXTERN void SendVfy(BYTE*,BYTE,BYTE);	///< 書き込みチェックＮＧデータ送信
EXTERN void SendText(char*);			///< ターミナルへの文字列データ送信
EXTERN void SendBin(long, const char*);	///< ターミナルへのデータ送信
EXTERN void ReadText(void);				///< ターミナルから文字列データ受信
EXTERN void ReadCmd(void);				///< コマンドの読出し
EXTERN void cyc_tim(VP_INT);			///< 周期起動ハンドラ
EXTERN TASK MonRcvTask(INT);			///< モニタ受信タスク
EXTERN TASK MonTask(INT);				///< モニタタスク

//=== MON_CHCK.C ===
void CmdCheck(void);				///< Ｃコマンド

//=== MON_DUMP.C ===
void CmdDump (void);				///< Ｄ  コマンド
void CmdDumpB(void);				///< ＤＢコマンド
void CmdDumpW(void);				///< ＤＷコマンド
void CmdDumpL(void);				///< ＤＬコマンド
BYTE ParsParmD(void);				///< コマンドパラメータの解釈

//=== MON_EDIT.C ===
void CmdEdit (void);				///< Ｍコマンド
void CmdEdit1(void);				///< ＭＢコマンド
void CmdEdit2(void);				///< ＭＷコマンド
void CmdEdit3(void);				///< ＭＬコマンド

//=== MON_DASM.C ===
void CmdDumpA(void);				///< Dump with Assembler instruction
int i_sprintf(char*,const char*,...);	///< カーネル用簡易sprintf関数

//=== MON_MAC.C ===
void CmdMS(void);					///< MACアドレス編集

//=== MON_NS.C ===
void CmdNS(void);					///< ネットワークパラメータ編集

//=== MON_NR.C ===
void CmdNR(void);					///< ネットワークパラメータ参照

//=== TIMER.C ===
INTHDR IntTMU1(void);				///< 割込みタイマーハンドラ
INTHDR IntTMU2(void);				///< 割込みタイマーハンドラ

//=== MON_TEST.C ===
void CmdHardTest(void);				///< ハードウェアのチェックモード
void CmdSoftTest(void);				///< ソフトウェアのチェックモード

//=== MON_CNR.C ===
void CmdCNR(void);					///< 制御盤のネットワークパラメータ参照

//=== MON_IDS.C ===
void CmdIDS(void);					///< 機器番号編集

//=== MON_CNS.C ===
void CmdCNS(void);					///< 制御盤のネットワークパラメータ設定

#if defined(NORTI_USE)
extern char* _strtok(char *s1, const char *s2);	///< NORTi用strtok
#endif