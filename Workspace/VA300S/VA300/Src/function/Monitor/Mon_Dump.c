//=============================================================================
//					ＳＨ-４ ＣＰＵモジュール モニタプログラム
//					<<< メモリダンプコマンドモジュール >>>
//
// Version 1.00 1999.10.21 S.Nakano	新規
// Version 1.10 2001.12.07 S.Nakano	MCM(HJ945010BP)対応追加
//=============================================================================
#include "kernel.h"
#include "nonet.h"
#include "nonteln.h"
#include "mon.h"

//=============================================================================
// [機能] Ｄコマンドの実行
// [引数] なし
// [戻値] なし
//=============================================================================
void CmdDump(void)
{
	switch (Ctrl.DumpUnit)						// 直前のダンプ単位検査
	{
	case 'B': Ctrl.Proc = CmdDumpB; break;		// バイト単位ダンプ
	case 'W': Ctrl.Proc = CmdDumpW; break;		// ワード単位ダンプ
	case 'L': Ctrl.Proc = CmdDumpL; break;		// ロング単位ダンプ
	case 'A': Ctrl.Proc = CmdDumpA; break;		// アセンブルダンプ
	}
}

//=============================================================================
// [機能] ＤＢコマンドの実行
// [引数] なし
// [戻値] なし
//=============================================================================
void CmdDumpB(void)
{
	BYTE	code;
	long	rest;
	int		i;

	if (code = ParsParmD())						// コマンドパラメータ解釈
	{
		SendMsg(code);							// パラメータエラー
		Ctrl.Proc = ReadCmd;					// コマンド入力に戻る
		return;
	}

	for (rest=Ctrl.DumpSize; rest>0;)
	{
		memset(Ctrl.TxText,' ',sizeof Ctrl.TxText);
		LongAsc((LONG)Ctrl.DumpAddr,Ctrl.TxText);
		for (i=0; i<16 && rest>0; i++,rest--)	// １行分のテキスト編集
		{
			code = *(Ctrl.DumpAddr++);			// メモリデータ取得
			ByteAsc(code,Ctrl.TxText+9+3*i);	// 16進テキストに変換
			Ctrl.TxText[58+i] = HexChr[code];	// 対応ASCII文字セット
		}
		strcpy(Ctrl.TxText+58+i,CRLF);
		SendText(Ctrl.TxText);
	}
	Ctrl.DumpUnit = 'B';						// バイト単位ダンプ保存
	Ctrl.Proc = ReadCmd;						// コマンド入力に戻る
}

//=============================================================================
// [機能] ＤＷコマンドの実行
// [引数] なし
// [戻値] なし
//=============================================================================
void CmdDumpW(void)
{
	WORD*	addr;
	WORD	data;
	BYTE	code;
	long	rest;
	int		i;

	if (code = ParsParmD())						// コマンドパラメータ解釈
	{
		SendMsg(code);							// パラメータエラー
		Ctrl.Proc = ReadCmd;					// コマンド入力に戻る
		return;
	}

	addr = (WORD*)((LONG)Ctrl.DumpAddr & 0xFFFFFFFE);
	for (rest=Ctrl.DumpSize; rest>0;)
	{
		memset(Ctrl.TxText,' ',sizeof Ctrl.TxText);
		LongAsc((LONG)addr,Ctrl.TxText);
		for (i=0; i<8 && rest>0; i++,rest-=2)	// １行分のテキスト編集
		{
			data = *(addr++);					// メモリデータ取得
			WordAsc(data,Ctrl.TxText+9+5*i);	// 16進テキストに変換
		}
		strcpy(Ctrl.TxText+9+5*i,CRLF);
		SendText(Ctrl.TxText);
	}
	Ctrl.DumpAddr = (BYTE*)addr;				// 次ダンプアドレス保存
	Ctrl.DumpUnit = 'W';						// ワード単位ダンプ保存
	Ctrl.Proc = ReadCmd;						// コマンド入力に戻る
}

//=============================================================================
// [機能] ＤＬコマンドの実行
// [引数] なし
// [戻値] なし
//=============================================================================
void CmdDumpL(void)
{
	LONG*	addr;
	LONG	data;
	BYTE	code;
	long	rest;
	int		i;

	if (code = ParsParmD())						// コマンドパラメータ解釈
	{
		SendMsg(code);							// パラメータエラー
		Ctrl.Proc = ReadCmd;					// コマンド入力に戻る
		return;
	}

	addr = (LONG*)((LONG)Ctrl.DumpAddr & 0xFFFFFFFC);
	for (rest=Ctrl.DumpSize; rest>0;)
	{
		memset(Ctrl.TxText,' ',sizeof Ctrl.TxText);
		LongAsc((LONG)addr,Ctrl.TxText);
		for (i=0; i<4 && rest>0; i++,rest-=4)	// １行分のテキスト編集
		{
			data = *(addr++);					// メモリデータ取得
			LongAsc(data,Ctrl.TxText+9+9*i);	// 16進テキストに変換
		}
		strcpy(Ctrl.TxText+9+9*i,CRLF);
		SendText(Ctrl.TxText);
	}
	Ctrl.DumpAddr = (BYTE*)addr;				// 次ダンプアドレス保存
	Ctrl.DumpUnit = 'L';						// ロング単位ダンプ保存
	Ctrl.Proc = ReadCmd;						// コマンド入力に戻る
}

//-----------------------------------------------------------------------------
// コマンドパラメータの解釈
//-----------------------------------------------------------------------------
BYTE ParsParmD(void)
{
	char*	Parm;
	LONG	dTop,dEnd,dSize;
	BOOL	fTop,fEnd,fSize;

	Ctrl.DumpSize = 256;						// バイト数省略値セット
	dTop = dEnd = dSize = 0;					// パラメータ初期化
	fTop = fEnd = fSize = FALSE;				// パラメータフラグクリア

	if (Parm = strtok(NULL," \x0D\t"))			// パラメータ指定あり
	{
		if (!(fTop = Asc2Hex(Parm,&dTop,0))) return MSG_BADADDR;

		if (Parm = strtok(NULL," \x0D\t"))
		{
			if (Parm[0] == '@')
			{
				if (!(fSize = Asc2Hex(Parm+1,&dSize,0))) return MSG_BADHEX;
				if (dSize == 0)							 return MSG_BADPARM;
			}
			else if (!(fEnd = Asc2Hex(Parm,&dEnd,0)))	 return MSG_BADADDR;
			else if (dTop > dEnd)						 return MSG_BADADDR;
		}
	}
	if (fTop)	Ctrl.DumpAddr = (BYTE*)dTop;	// 先頭アドレス指定あり
	if (fEnd)	Ctrl.DumpSize = dEnd-dTop+1;	// 終端アドレス指定あり
	if (fSize)	Ctrl.DumpSize = dSize;			// バイト数指定あり
	return MSG_NONE;
}
