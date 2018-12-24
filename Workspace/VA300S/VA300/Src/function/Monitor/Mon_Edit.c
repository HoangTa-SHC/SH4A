//=============================================================================
/**
 *	@briefＳＨ-２ ＣＰＵモジュール モニタプログラム
 *	
 *	@file Mon_NS.c
 *	
 *	@date	2007/12/10
 *	@version 1.00 メモリ編集コマンドマンドモジュール
 *	@author	F.Saeki
 *
 *	@date	2011/4/7
 *	@version 1.20 OYO-105A用ブートプログラムから流用
 *	@author	F.Saeki
 *
 *	Copyright (C) 2007, OYO Electric Corporation
 */
//=============================================================================
#include "kernel.h"
#include "nonet.h"
#include "nonteln.h"
#include "nonethw.h"
#include "mon.h"

static UB*	Addr;
static UB		Flag;

static UB ParsParm(void);		// コマンドパラメータの解釈
static UB ParsParm0(void);		// コマンドパラメータの解釈

//=============================================================================
/**
 * Ｍコマンドの実行
 */
//=============================================================================
void CmdEdit(void)
{
	char*	Resp;
	UB		bCode;
	UH		wCode;
	UW		lCode;
	BOOL	Verify;
	int		offs,unit;

	if (bCode = ParsParm())							// パラメータ解釈
	{
		SendMsg(bCode);								// パラメータエラー
		Ctrl.Proc = ReadCmd;						// コマンド入力に戻る
		return;
	}

	unit = 1;										// バイト単位で初期化
	
	switch (Ctrl.EditUnit)							// アクセス単位確認
	{
	case 'B': unit = 1; break;						// バイト単位
	case 'W': unit = 2; break;						// ワード単位
	case 'L': unit = 4; break;						// ロング単位
	}
	while (1)
	{
		memset(Ctrl.TxText,' ',sizeof Ctrl.TxText);
		LongAsc((UW)Ctrl.topAddr,Ctrl.TxText);	// プロンプト編集
		offs = 9;
		if (Ctrl.Flag & FLAG_VERIFY) switch (Ctrl.EditUnit)
		{
		case 'B': ByteAsc(*(UB*)Ctrl.topAddr,Ctrl.TxText+9); offs = 12; break;
		case 'W': WordAsc(*(UH*)Ctrl.topAddr,Ctrl.TxText+9); offs = 14; break;
		case 'L': LongAsc(*(UW*)Ctrl.topAddr,Ctrl.TxText+9); offs = 18; break;
		}
		strcpy(Ctrl.TxText+offs,"? ");
		SendText(Ctrl.TxText);

		ReadText();									// 応答テキスト受信
		if (Resp = strtok(Ctrl.RxText," \x0D\t"))
		{
			if (Resp[0] == '.' && Resp[1] == 0) break;
			if (Resp[0] == '^' && Resp[1] == 0)
			{
				if (Ctrl.topAddr) Ctrl.topAddr -= unit;
				continue;
			}
			if (!Asc2Hex(Resp,&lCode,0))			// 更新データ値取得
			{
				SendMsg(MSG_BADHEX); continue;
			}
			switch (Ctrl.EditUnit)					// データ書き込み
			{
			case 'B': *(UB*)Ctrl.topAddr = (bCode = lCode); break;
			case 'W': *(UH*)Ctrl.topAddr = (wCode = lCode); break;
			case 'L': *(UW*)Ctrl.topAddr = (lCode = lCode); break;
			}
			Verify = TRUE;							// 書き込みチェック
			if (Ctrl.Flag & FLAG_VERIFY) switch (Ctrl.EditUnit)
			{
			case 'B': if (*(UB*)Ctrl.topAddr != bCode) Verify = FALSE; break;
			case 'W': if (*(UH*)Ctrl.topAddr != wCode) Verify = FALSE; break;
			case 'L': if (*(UW*)Ctrl.topAddr != lCode) Verify = FALSE; break;
			}
			if (!Verify) { SendMsg(MSG_VERIFY); continue; }
		}
		if (!(Ctrl.topAddr += unit)) break;			// 対象アドレス更新
	}
	Ctrl.Proc = ReadCmd;							// コマンド入力に戻る
}

//=============================================================================
/**
 * ＭＢコマンドの実行
 */
//=============================================================================
void CmdEdit1(void)
{
	char*	Resp;
	UB		bCode;
	UW		lCode;
	BOOL	Verify;
	int		offs;

	Ctrl.EditUnit = 'B';							// アクセス単位確認

	if (bCode = ParsParm0())						// パラメータ解釈
	{
		SendMsg(bCode);								// パラメータエラー
		Ctrl.Proc = ReadCmd;						// コマンド入力に戻る
		return;
	}

	while (1)
	{
		memset(Ctrl.TxText,' ',sizeof Ctrl.TxText);
		LongAsc((UW)Addr,Ctrl.TxText);			// プロンプト編集
		offs = 9;
		if (Flag != 'N')							// 現メモリ内容参照
		{
			ByteAsc(*(UB*)Addr,Ctrl.TxText+9);
			offs = 12;
		}
		strcpy((char*)(Ctrl.TxText+offs),"? ");
		SendText(Ctrl.TxText);

		ReadText();									// 応答テキスト受信
		if (Resp = strtok((char*)Ctrl.RxText," \x0D\t"))
		{
			if (Resp[0] == '.' && Resp[1] == 0)	break;	// Ｍコマンド終了
			if (Resp[0] == '^' && Resp[1] == 0)
			{
				if (Addr) Addr --;					// 対象アドレス後退
				continue;
			}
			if (!Asc2Hex(Resp,&lCode,0))			// 更新データ値取得
			{
				SendMsg(MSG_BADHEX); continue;
			}
			*(UB*)Addr = (bCode = lCode);
			
			Verify = TRUE;							// 書き込みチェック
			if (Flag != 'N')
			{
				if (*(UB*)Addr != bCode) Verify = FALSE;
			}
			if (!Verify) { SendMsg(MSG_VERIFY); continue; }
		}
		if (!(Addr ++)) break;						// 対象アドレス更新
	}
	Ctrl.Proc = ReadCmd;							// コマンド入力に戻る
}

//=============================================================================
/**
 * ＭＷコマンドの実行
 */
//=============================================================================
void CmdEdit2(void)
{
	char*	Resp;
	UB		bCode;
	UH		wCode;
	UW		lCode;
	BOOL	Verify;
	int		offs,unit;

	Ctrl.EditUnit = 'W';							// アクセス単位確認

	if (bCode = ParsParm0())						// パラメータ解釈
	{
		SendMsg(bCode);								// パラメータエラー
		Ctrl.Proc = ReadCmd;						// コマンド入力に戻る
		return;
	}

	unit = 2;										// ワード単位

	while (1)
	{
		memset(Ctrl.TxText,' ',sizeof Ctrl.TxText);
		LongAsc((UW)Addr,Ctrl.TxText);			// プロンプト編集
		offs = 9;
		if (Flag != 'N')							// 現メモリ内容参照
		{
			WordAsc(*(UH*)Addr,Ctrl.TxText+9); offs = 14;
		}
		strcpy((char*)(Ctrl.TxText+offs),"? ");
		SendText(Ctrl.TxText);

		ReadText();									// 応答テキスト受信
		if (Resp = strtok((char*)Ctrl.RxText," \x0D\t"))
		{
			if (Resp[0] == '.' && Resp[1] == 0)	break;	// Ｍコマンド終了
			if (Resp[0] == '^' && Resp[1] == 0)
			{
				if (Addr) Addr -= unit;				// 対象アドレス後退
				continue;
			}
			if (!Asc2Hex(Resp,&lCode,0))			// 更新データ値取得
			{
				SendMsg(MSG_BADHEX); continue;
			}
			*(UH*)Addr = (wCode = lCode);
			
			Verify = TRUE;							// 書き込みチェック
			if (Flag != 'N')
			{
				if (*(UH*)Addr != wCode) Verify = FALSE;
			}
			if (!Verify) { SendMsg(MSG_VERIFY); continue; }
		}
		if (!(Addr += unit)) break;					// 対象アドレス更新
	}
	Ctrl.Proc = ReadCmd;							// コマンド入力に戻る
}

//=============================================================================
/**
 * ＭＬコマンドの実行
 */
//=============================================================================
void CmdEdit3(void)
{
	char*	Resp;
	UB		bCode;
	UW		lCode;
	BOOL	Verify;
	int		offs,unit;

	if (bCode = ParsParm0())						// パラメータ解釈
	{
		SendMsg(bCode);								// パラメータエラー
		Ctrl.Proc = ReadCmd;						// コマンド入力に戻る
		return;
	}

	unit = 4;										// ロング単位

	while (1)
	{
		memset(Ctrl.TxText,' ',sizeof Ctrl.TxText);
		LongAsc((UW)Addr,Ctrl.TxText);			// プロンプト編集
		offs = 9;
		if (Flag != 'N')							// 現メモリ内容参照
		{
			LongAsc(*(UW*)Addr,Ctrl.TxText+9); offs = 18;
		}
		strcpy((char*)(Ctrl.TxText+offs),"? ");
		SendText(Ctrl.TxText);

		ReadText();									// 応答テキスト受信
		if (Resp = strtok((char*)Ctrl.RxText," \x0D\t"))
		{
			if (Resp[0] == '.' && Resp[1] == 0)	break;	// Ｍコマンド終了
			if (Resp[0] == '^' && Resp[1] == 0)
			{
				if (Addr) Addr -= unit;				// 対象アドレス後退
				continue;
			}
			if (!Asc2Hex(Resp,&lCode,0))			// 更新データ値取得
			{
				SendMsg(MSG_BADHEX); continue;
			}
			*(UW*)Addr = (lCode = lCode);			// データ書き込み
			Verify = TRUE;							// 書き込みチェック
			if (Flag != 'N')
			{
				if (*(UW*)Addr != lCode) Verify = FALSE;
			}
			if (!Verify) { SendMsg(MSG_VERIFY); continue; }
		}
		if (!(Addr += unit)) break;					// 対象アドレス更新
	}
	Ctrl.Proc = ReadCmd;							// コマンド入力に戻る
}

//-----------------------------------------------------------------------------
// コマンドパラメータの解釈
//-----------------------------------------------------------------------------
static UB ParsParm(void)
{
	char*	Parm;
	UB	Unit;

	Unit = Ctrl.EditUnit;						// 現在のEdit単位参照
	Ctrl.Flag |= FLAG_VERIFY;					// ベリファイあり（仮）

	if (!(Parm = strtok(NULL," \x0D\t")))		return MSG_BADADDR;
	if (!Asc2Hex(Parm,(UW*)&Ctrl.topAddr,0))	return MSG_BADADDR;

	while (Parm = strtok(NULL," \x0D\t"))		// オプションパラメータ取得
	{
		if (Parm[1]) return MSG_BADPARM;		// 無効なパラメータ
		switch (Parm[0])
		{
		case 'B': Unit = 'B'; break;				// バイトアクセス
		case 'W': Unit = 'W'; break;				// ワードアクセス
		case 'L': Unit = 'L'; break;				// ロングワードアクセス
		case 'N': Ctrl.Flag &= ~FLAG_VERIFY; break;	// ベリファイなし
		default : return MSG_BADPARM;				// 無効なパラメータ
		}
	}
	if ((Unit == 'W') && ((UW)Ctrl.topAddr & 0x00000001)) return MSG_ODDADDR;
	if ((Unit == 'L') && ((UW)Ctrl.topAddr & 0x00000003)) return MSG_ODDADDR;

	Ctrl.EditUnit = Unit;						// 指定Edit単位保存
	return MSG_NONE;
}

//-----------------------------------------------------------------------------
// コマンドパラメータの解釈
//-----------------------------------------------------------------------------
static UB ParsParm0(void)
{
	char*	Parm;
	UB	Unit;

	Unit = Ctrl.EditUnit;						// 現在のEdit単位参照
	Flag = 0x00;

	if (!(Parm = strtok(NULL," \x0D\t"))) return MSG_BADADDR;
	if (!Asc2Hex(Parm,(UW*)&Addr,0))	  return MSG_BADADDR;

	if ((Unit == 'W') && ((UW)Addr & 0x00000001)) return MSG_ODDADDR;
	if ((Unit == 'L') && ((UW)Addr & 0x00000003)) return MSG_ODDADDR;

	return MSG_NONE;
}
