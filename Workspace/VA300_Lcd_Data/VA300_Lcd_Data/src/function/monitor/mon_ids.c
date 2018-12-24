//=============================================================================
/**
 *	@brief ＳＨ-４ ＣＰＵモジュール モニタプログラム
 *	
 *	@file Mon_IDS.c
 *	
 *	@date	2007/12/10
 *	@version 1.00 機器番号設定コマンドモジュール
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
#include "va300.h"
#include "drv_eep.h"

static UB ParsParm(void);

//=============================================================================
/**
 * ＩＤＳコマンドの実行
 */
//=============================================================================
void CmdIDS(void)
{
	BYTE	code;
	
	Ctrl.Proc = ReadCmd;						// コマンド入力に戻る
	
	if (code = ParsParm())						// コマンドパラメータ解釈
	{
		SendMsg(code);							// パラメータエラー
		return;
	}
}

//-----------------------------------------------------------------------------
// コマンドパラメータの解釈
//-----------------------------------------------------------------------------
static UB ParsParm(void)
{
	char*	Parm;
	LONG	tmp;
	UH		idno;
	
	if (Parm = strtok(NULL," \x0D\t"))			// パラメータ指定あり
	{
		if (!(Asc2Dec(Parm,&tmp,0))) return MSG_BADPARM;
		idno = (UH)tmp;
	}
	else return MSG_BADPARM;
	
	lan_set_id( idno );					// 制御盤のIPアドレス保存

	SendText("OK\r\n");

	return MSG_NONE;
}
