//=============================================================================
//					ＳＨ-４ ＣＰＵモジュール モニタプログラム
//				<<< メモリチェックコマンドマンドモジュール >>>
//
// Version 1.00 1999.10.22 S.Nakano	新規
// Version 1.10 2001.12.07 S.Nakano	MCM(HJ945010BP)対応追加
//=============================================================================
#include "kernel.h"
#include "nonet.h"
#include "nonteln.h"
#include "mon.h"

static BYTE ParsParm(void);		// コマンドパラメータの解釈

//=============================================================================
// [機能] Ｃコマンドの実行
// [引数] なし
// [戻値] なし
//=============================================================================
void CmdCheck(void)
{
	BYTE	wCode,rCode;
	BYTE*	addr;

	Ctrl.Proc = ReadCmd;
	if (wCode = ParsParm())						// コマンドパラメータ解釈
	{
		SendMsg(wCode); return;					// パラメータエラー
	}

	for (addr=Ctrl.topAddr; addr<=Ctrl.endAddr; addr++)
	{
		*addr = (BYTE)(LONG)addr;				// アドレス下位１バイト書き込み
	}
	for (addr=Ctrl.topAddr; addr<=Ctrl.endAddr; addr++)
	{
		wCode = (BYTE)(LONG)addr;				// ベリファイ
		rCode = *addr;
		if (wCode != rCode) { SendVfy(addr,wCode,rCode); return; }
	}
	for (addr=Ctrl.topAddr; addr<=Ctrl.endAddr; addr++)
	{
		*addr = ~(BYTE)(LONG)addr;				// ビット反転データ書き込み
	}
	for (addr=Ctrl.topAddr; addr<=Ctrl.endAddr; addr++)
	{
		wCode = ~(BYTE)(LONG)addr;				// ベリファイ
		rCode = *addr;
		if (wCode != rCode) { SendVfy(addr,wCode,rCode); return; }
	}
	SendText("Complete!" CRLF);					// メモリチェック完了
}

//-----------------------------------------------------------------------------
// コマンドパラメータの解釈
//-----------------------------------------------------------------------------
static BYTE ParsParm(void)
{
	char*	Parm;
	LONG	nByte;

	if (!(Parm = strtok(NULL," \x0D\t")))		return MSG_BADADDR;
	if (!Asc2Hex(Parm,(LONG*)&Ctrl.topAddr,0))	return MSG_BADADDR;
	if (!(Parm = strtok(NULL," \x0D\t")))		return MSG_BADPARM;
	if (Parm[0] == '@')
	{
		if (!Asc2Hex(Parm+1,&nByte,0))	return MSG_BADHEX;
		if (nByte == 0)					return MSG_BADPARM;
		Ctrl.endAddr = Ctrl.topAddr + nByte - 1;
	}
	else
	{
		if (!Asc2Hex(Parm,(LONG*)&Ctrl.endAddr,0))		return MSG_BADADDR;
		if ((LONG)Ctrl.topAddr > (LONG)Ctrl.endAddr)	return MSG_BADADDR;
	}
	return MSG_NONE;
}
