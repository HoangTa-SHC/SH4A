//=============================================================================
/**
 *	@brief ＳＨ-４ ＣＰＵモジュール モニタプログラム
 *	
 *	@file Mon_CNS.c
 *	
 *	@date	2007/12/10
 *	@version 1.00 送信先IPアドレス、ポート番号設定コマンドモジュール
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
 * ＣＮＳコマンドの実行
 */
//=============================================================================
void CmdCNS(void)
{
	BYTE	code;
	
	Ctrl.Proc = ReadCmd;						// コマンド入力に戻る
	
	if (code = ParsParm())						// コマンドパラメータ解釈
	{
		SendMsg(code);							// パラメータエラー
		return;
	}
//	dly_tsk( 1000/MSEC);
//	reset();									// リセット
}

//-----------------------------------------------------------------------------
// コマンドパラメータの解釈
//-----------------------------------------------------------------------------
static UB ParsParm(void)
{
	char*	Parm;
	LONG	tmp;
	UB		ip_addr[4];
	UH		portno;
	int		i;
	
	for(i=0;i<sizeof ip_addr;i++) {
		if (Parm = strtok(NULL," .\x0D\t"))		// パラメータ指定あり
		{
			if (!(Asc2Dec(Parm,&tmp,0))) return MSG_BADPARM;
			ip_addr[i] = (UB)tmp;
		}
		else return MSG_BADPARM;
	}

	if (Parm = strtok(NULL," \x0D\t"))			// パラメータ指定あり
	{
		if (!(Asc2Dec(Parm,&tmp,0))) return MSG_BADPARM;
		portno = (UH)tmp;
	}
	else return MSG_BADPARM;
	
	lan_set_ctrlip( ip_addr );			// 制御盤のIPアドレス保存
	lan_set_ctrlpt( portno );			// 制御盤の通信ポート番号保存

	SendText("OK\r\n");

	return MSG_NONE;
}
