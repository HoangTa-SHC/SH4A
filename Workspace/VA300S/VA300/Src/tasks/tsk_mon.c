//=============================================================================
/**
 *	@brief	ＳＨ-４ ＣＰＵモジュール モニタプログラム
 *	
 *	@file tsk_mon.c
 *	
 *	@date		2011/4/7
 *	@version	1.20 OYO-105A用ブートプログラムから流用
 *	@author		F.Saeki
 *
 *	Copyright (C) 2007, OYO Electric Corporation
 */
//=============================================================================
#include "itron.h"
#include "nonet.h"
#include "kernel.h"

#define	_MON_MAIN_
#include "mon.h"
#include "id.h"
#include "nosio.h"
#include "va300.h"
#include "drv_led.h"

// 定義
#define	LED_DCMD	LED_DBG1	// モニタコマンドLED

static const struct				//=== モニタコマンド一覧 ===
{
	char*	CmdName;			// コマンド名
	void	(*CmdProc)(void);	// コマンド処理関数
} CmdList[]	=
{
	{"C"	,CmdCheck},			// Check RAM memory access
	{"D"	,CmdDump },			// Dump
	{"DA"	,CmdDumpA},			// Dump with Assembler instruction
	{"DB"	,CmdDumpB},			// Dump with Byte unit
	{"DW"	,CmdDumpW},			// Dump with 16bit Word unit
	{"DL"	,CmdDumpL},			// Dump with 32bit Longword unit
	{"G"	,CmdExec },			// Go to user program
	{"M"	,CmdEdit },			// Memory edit
	{"MB"	,CmdEdit1},			// Memory edit with Byte
	{"MW"	,CmdEdit2},			// Memory edit with Word
	{"ML"	,CmdEdit3},			// Memory edit with Longword
	{"VER"	,CmdVer },			// Version information
//	{"HWT"	,CmdHardTest},		// ハードウェアテスト
//	{"SWT"	,CmdSoftTest},		// ソフトウェアテスト
	{"?"	,CmdHelp },			// Show Help Message
	{"MS"	,CmdMS   },			// MAC Address Set
	{"NS"	,CmdNS   },			// Network parameter Set
	{"CNS"	,CmdCNS  },			// Control box Network parameter Set
	{"IDS"	,CmdIDS  },			// ID number Set
	{"NR"	,CmdNR   },			// Network parameter Read
	{"MON"	,CmdMon  },			// モニタコマンド
};

static void InitMon(INT ch);			// プログラムの初期化

// コマンド読みだしシーケンス
static void ReadCmd01(void);
static void ReadCmd02(void);
static void PasswdCheck(void);			///< パスワードチェック

static	int	nRead;

#if 1
extern void DebugOut(char*);			///< デバッグ用出力関数
#endif

const T_CTSK cMonTask  = { TA_HLNG, NULL, MonTask, 7, 2048, NULL, "MonTask" };
const T_CTSK cMonRcvTask  = { TA_HLNG, NULL, MonRcvTask, 3, 2048, NULL, "MonRcvTask" };
const T_CMBX cMonMbx   = { TA_TFIFO|TA_MFIFO, 0, NULL, (B *)"mon_mbx" };
const T_CMPF cMonMpf   = { TA_TFIFO, 4, sizeof (T_MONMSG), NULL, (B *)"mon_mpf" };

const unsigned long MonAddr = 0x80001000;	///< モニタモード時のアドレス

static const char cMonPasswd[] = "20011";	///< モニタ用パスワード(仮)

//=============================================================================
/**
 * モニタプログラム起動
 *
 * @param ch	チャンネル番号
 *
 * @retval E_OK モニタ起動成功
 */
//=============================================================================
ER Mon_ini(INT ch)
{
	ER ercd;
	
//	ercd = cre_tsk(TSK_CMD_MON, &cMonTask);
	if (ercd != E_OK) {
		return ercd;
	}
	
//	ercd = cre_tsk(TSK_RCV_MON, &cMonRcvTask);
	if (ercd != E_OK) {
		return ercd;
	}
	
	ercd = cre_mbx(MBX_MON, &cMonMbx);
	if (ercd != E_OK) {
		return ercd;
	}
	
	ercd = cre_mpf(MPF_MON, &cMonMpf);
	if (ercd != E_OK) {
		return ercd;
	}

//	ercd = sta_tsk(TSK_CMD_MON, ch);
	if (ercd != E_OK) {
		return ercd;
	}

//	ercd = sta_tsk(TSK_RCV_MON, ch);

	return ercd;
}

//-----------------------------------------------------------------------------
// プログラムの初期化
//-----------------------------------------------------------------------------
static void InitMon(INT ch)
{
	
	memset(&Ctrl,0,sizeof Ctrl);				// 実行制御情報クリア
	Ctrl.DumpUnit = 'B';						// Dump単位省略値
	Ctrl.EditUnit = 'B';						// Edit単位省略値
	Ctrl.Proc	  = PasswdCheck;				// 初期シーケンスセット
	Ctrl.DumpSize = 256;						// バイト数省略値セット
	Ctrl.ComCh = ch;							// 通信チャンネルの設定
}

//=============================================================================
/**
 * モニタコマンド受信
 */
//=============================================================================
void ReadCmd(void)
{
	char*	Cmd;
	int		i;

	SendText(">>");								// プロンプト送信
	ReadText();									// コマンドテキスト受信

	if (Cmd = strtok(Ctrl.RxText," \x0D\t"))
	{
		for (i=0; i<(sizeof CmdList / sizeof CmdList[0]); i++)
		{
			if (!strcmp(Cmd,CmdList[i].CmdName))
			{
				LedOut(LED_DCMD, LED_ON);
				CmdList[i].CmdProc();			// コマンド処理関数に移行
				LedOut(LED_DCMD, LED_OFF);
				return;
			}
		}
		SendMsg(MSG_BADCMD);					// 未定義コマンド
	}
}

//=============================================================================
/**
 * Ｇコマンドの実行
 */
//=============================================================================
void CmdExec(void)
{
	char*	Parm;
	LONG	Addr;

	Ctrl.Proc = ReadCmd;

	if (!(Parm = strtok(NULL," \x0D\t"))) 	{ SendMsg(MSG_BADADDR); return; }
	if (!Asc2Hex(Parm,&Addr,0))				{ SendMsg(MSG_BADADDR); return; }

	(**(void (**)(void))Addr)();			/* プログラムに分岐			*/
}

//=============================================================================
/**
 * HELPコマンドの実行
 */
//=============================================================================
void CmdHelp(void)
{
	const char *HelpText[] = {
		"C :メモリチェック             C <Addr> {<addr>|@<size>}",
		"D :メモリダンプ               {D|DB|DW|DL} [<addr> [<addr>|@<size>}]",
		"G :ユーザプログラム実行       G <addr>",
		"M :メモリデータ変更           M <addr> [{B|W|L}] [N]",
		"DA:逆アセンブル               {D|DA} [<addr> [<addr>|@<size>}]",
		"MS:MACアドレス変更            MS **-**-**-**-**-**",
		"NS:ネットワークパラメータ変更 NS <IP Addr> <Sub net Mask> <Port>",
		"NR:ネットワークパラメータ参照 NR",
		"CNS:送信先設定                CNS <IP Addr> <Port>",
		"IDS:ID番号設定                IDS <ID>",
		"HWT:ハードウェアテスト        HWT [<number>}",
		"SWT:ソフトウェアテスト        SWT [<number>}",
		"MON:レベル０モニタ起動        MON",
		0,
	};

	int	i=0;

	while (HelpText[i])
	{
		SendText(HelpText[i++]);
		SendText(CRLF);
	}
	Ctrl.Proc = ReadCmd;						// コマンド入力に戻る
}

//=============================================================================
/**
 * MONコマンドの実行
 */
//=============================================================================
void CmdMon(void)
{
	Ctrl.Proc = ReadCmd;

	(*(void (*)(void))MonAddr)();			/* モニタプログラムに分岐		*/
}

//=============================================================================
/**
 * ASCII化16進テキストからバイナリへの変換
 *
 * @param Text	ASCII化16進テキスト
 * @param Bin	バイナリ変換結果の格納先
 * @param Len	変換対象テキストのバイト数
 *
 * @retval TRUE 変換成功
 * @retval FALSE Text内に変換できない文字がある
 *
 * @note Len==0の場合は, Textの終端(NULL)までが変換対象になる
 */
//=============================================================================
BOOL Asc2Hex(char *Text, UW *Bin, UH Len)
{
	BYTE	ch;
	WORD	i;

	if (Len == 0) Len = strlen(Text);
	for (*Bin=0,i=0; i<Len; i++)
	{
		ch = (BYTE)toupper(Text[i]);
		if		(ch >= '0' && ch <= '9') ch -= '0';
		else if (ch >= 'A' && ch <= 'F') ch -= 'A' - 10;
		else return FALSE;
		*Bin = (*Bin << 4) | (LONG)ch;
	}
	return TRUE;
}

//=============================================================================
/**
 * ASCII化10進テキストからバイナリへの変換
 *
 * @param Text	ASCII化10進テキスト
 * @param Bin	バイナリ変換結果の格納先
 * @param Len	変換対象テキストのバイト数
 *
 * @retval TRUE 変換成功
 * @retval FALSE Text内に変換できない文字がある
 *
 * @note Len==0の場合は, Textの終端(NULL)までが変換対象になる
 */
//=============================================================================
BOOL Asc2Dec(const char *Text, UW *Bin, UH Len)
{
	BYTE	ch;
	WORD	i;
	LONG	v;
	
	if (Len == 0) Len = strlen(Text);
	for (v=0,i=0; i<Len; i++)
	{
		ch = (BYTE)Text[i];
		if (ch >= '0' && ch <= '9') ch -= '0';
		else return FALSE;
		v = (v * 10) + (LONG)ch;
	}
	*Bin = v;
	return TRUE;
}

//=============================================================================
/**
 * ロングデータをASCII化16進テキストに変換
 *
 * @param hex	変換対象のロングデータ
 * @param Text	変換結果の格納先
 *
 * @return なし
 */
//=============================================================================
void LongAsc(LONG hex, char *Text)
{
	Text[7] = HexAsc[hex & 0x0000000F]; hex >>= 4;
	Text[6] = HexAsc[hex & 0x0000000F]; hex >>= 4;
	Text[5] = HexAsc[hex & 0x0000000F]; hex >>= 4;
	Text[4] = HexAsc[hex & 0x0000000F]; hex >>= 4;
	Text[3] = HexAsc[hex & 0x0000000F]; hex >>= 4;
	Text[2] = HexAsc[hex & 0x0000000F]; hex >>= 4;
	Text[1] = HexAsc[hex & 0x0000000F]; hex >>= 4;
	Text[0] = HexAsc[hex & 0x0000000F];
}

//=============================================================================
/**
 * ワードデータをASCII化16進テキストに変換
 *
 * @param hex	変換対象のワードデータ
 * @param Text	変換結果の格納先
 *
 * @return なし
 */
//=============================================================================
void WordAsc(WORD hex, char *Text)
{
	Text[3] = HexAsc[hex & 0x000F];	hex >>= 4;
	Text[2] = HexAsc[hex & 0x000F];	hex >>= 4;
	Text[1] = HexAsc[hex & 0x000F];	hex >>= 4;
	Text[0] = HexAsc[hex & 0x000F];
}

//=============================================================================
/**
 * バイトデータをASCII化16進テキストに変換
 *
 * @param hex	変換対象のバイトデータ
 * @param Text	変換結果の格納先
 *
 * @return なし
 */
//=============================================================================
void ByteAsc(BYTE hex, char *Text)
{
	Text[1] = HexAsc[hex & 0x0F]; hex >>= 4;
	Text[0] = HexAsc[hex & 0x0F];
}

//=============================================================================
/**
 * エラーメッセージ送信
 *
 * @param Code	メッセージコード
 *
 * @return なし
 */
//=============================================================================
void SendMsg(BYTE Code)
{
	const char *MsgTable[] =
	{
		"",
		"*** Invalid Command",
		"*** Hex Value Error",
		"*** Invalid Parameter",
		"*** Invalid Address",
		"*** Odd Address",
		"*** Data Verify Error",
	};
	SendText(MsgTable[Code]);
	SendText(CRLF);
}

//=============================================================================
/**
 * 書き込みチェックＮＧデータ送信
 * 
 * @param addr	書き込み異常を検出したメモリアドレス
 * @param wd	メモリに書き込んだデータ値
 * @param rd	メモリから得られたデータ値
 *
 * @return なし
 */
//=============================================================================
void SendVfy(BYTE *addr, BYTE wd, BYTE rd)
{
	memset(Ctrl.TxText,' ',sizeof Ctrl.TxText);
	LongAsc((LONG)addr,Ctrl.TxText);
	Ctrl.TxText[ 9] = 'W'; Ctrl.TxText[10] = ':'; ByteAsc(wd,Ctrl.TxText+11);
	Ctrl.TxText[14] = 'R'; Ctrl.TxText[15] = ':'; ByteAsc(rd,Ctrl.TxText+16);
	strcpy(Ctrl.TxText+18,CRLF);
	SendText(Ctrl.TxText);
	SendMsg(MSG_VERIFY);
}

//=============================================================================
/**
 * ターミナルへの文字列データ送信
 *
 * @param Text	送信文字列(ASCIIZ)
 *
 * @return なし
 */
//=============================================================================
void SendText(char *Text)
{
	switch (Ctrl.ComCh) {
	case SCI_CH:DebugOut(Text);
				break;
	case LAN_CH:telnetd_send(Text);
				break;
	}
}

//=============================================================================
/**
 * ターミナルからの文字列データ受信
 *
 * @param なし
 *
 * @return なし
 */
//=============================================================================
void ReadText(void)
{
	ER			ercd;
	T_MONMSG	*msg;

	memset(Ctrl.RxText,0,sizeof Ctrl.RxText);	// 受信データ域クリア
	ercd = trcv_mbx( MBX_MON, (T_MSG**)&msg, TMO_FEVR);
	if( ercd == E_OK) {
		memcpy( &Ctrl.RxText, &msg->buf, msg->cnt);
		Ctrl.ComCh = msg->eCom;
		rel_mpf( MPF_MON, (VP)msg);
	}
}

//==========================================================================
// [機能] コマンド受信
// [引数] コマンド
// [戻値] 成否
// [補足] 
//==========================================================================
BOOL GetCmd(char *s)
{
	int			nRead = 0;
	char		c;
	ER			ercd;
	T_MONMSG	*msg;
	
	ercd = tget_mpf( MPF_MON, &msg, TMO_FEVR);
	if( ercd == E_OK) {
		memset( &msg->buf, 0, sizeof MON_BUF_SIZE);	// 受信データ域クリア
	
		for(;;)
		{
			c = *s;
			s++;
			if( c < 0x00) break;
			if (c == 0x08)							// BackSpace
			{
				if (nRead) msg->buf[--nRead] = 0;
				continue;
			}
			if( nRead < MON_BUF_SIZE) {
				msg->buf[ nRead ] = c;				// 受信データ保存
				nRead++;
			}
			if( c == 0x00) {
				if( strcmp((B*)msg->buf, "BYE")) {	// 切断要求？
					msg->cnt = nRead;
					msg->eCom = LAN_CH;
					snd_mbx( MBX_MON, (T_MSG*)msg);
					break;
				}
				else {
					Ctrl.Proc = ReadCmd;			// コマンド入力に戻る
					return FALSE;
				}
			}
		}
	}
	return TRUE;
}

//==========================================================================
/**
 * コマンド受信
 * 
 * @param ch チャンネル番号
 * 
 * @return なし
 */
//==========================================================================
TASK MonRcvTask(INT ch)
{
	T_MONMSG *msg;
	ER ercd;
	UINT i;
	UB c;

    /* initialize */

	ini_sio(ch, "115200 B8 PN S1 XON");
	ctl_sio(ch, TSIO_RXE|TSIO_TXE|TSIO_RTSON);

	for (;;) {
		/* Get memory block */

		ercd = tget_mpf(MPF_MON, &msg, TMO_FEVR);

		/* Received one line */

		for (i = 0;;) {
//			ercd = get_sio(ch, &c, 5000/MSEC);
			ercd = get_sio(ch, &c, TMO_FEVR);
			if (ercd == E_TMOUT) {
				if ( i ) {
					break;
				}
				continue;
			}
			if (c == 0x08) {
				if ( i ) {
					msg->buf[--i] = 0;
				}
				continue;
			}
			if (c == '\n') {
				continue;
			}
			msg->buf[i] = c;
			if ((++i >= MON_BUF_SIZE - 1) || (c == '\r')) {
				break;
			}
		}
		msg->buf[i] = '\0';
		msg->ch  = ch;					// チャンネル設定
		msg->eCom = SCI_CH;
		msg->cnt = i;

		/* Send message */

		snd_mbx(MBX_MON, msg);
	}
}

//=============================================================================
/**
 * メインタスク
 *
 * @param ch チャンネル番号
 * 
 * @return なし
 */
//=============================================================================
TASK MonTask(INT ch)
{
	InitMon(ch);								// プログラムの初期化
	
	while(1) Ctrl.Proc();						// 制御シーケンス駆動
}

//=============================================================================
/**
 * ターミナルからの文字列データ受信
 *
 * @param なし
 *
 * @return なし
 */
//=============================================================================
static void PasswdCheck(void)
{
	ER			ercd;
	T_MONMSG	*msg;
	int iLen;

	ercd = trcv_mbx( MBX_MON, (T_MSG**)&msg, TMO_FEVR);
	if( ercd == E_OK) {
		iLen = strlen(cMonPasswd);
		if (!memcmp(&msg->buf, cMonPasswd, iLen)) {
			Ctrl.Proc = ReadCmd;
		}
		rel_mpf( MPF_MON, (VP)msg);
	}
}
