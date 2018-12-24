//=============================================================================
/**
 *	ＳＨ-４ ＣＰＵモジュール モニタプログラム
 *	
 *	@file Mon_MAC.c
 *	@version 1.00
 *	
 *	@author	F.Saeki
 *	@date	2007/12/10
 *	@brief	MACアドレス設定コマンドモジュール
 *
 *	Copyright (C) 2007, OYO Electric Corporation
 */
//=============================================================================
#include "kernel.h"
#include "nonet.h"
#include "nonteln.h"
#include "mon.h"
#include "lan9118.h"

static UB ParsParm(void);

//=============================================================================
/**
 * ＭＳコマンドの実行
 */
//=============================================================================
void CmdMS(void)
{
	BYTE	code;

	Ctrl.Proc = ReadCmd;						// コマンド入力に戻る
	
	if (code = ParsParm())						// コマンドパラメータ解釈
	{
		SendMsg(code);							// パラメータエラー
		return;
	}
//	dly_tsk( 1000/MSEC);
//	reset();
}

//-----------------------------------------------------------------------------
// コマンドパラメータの解釈
//-----------------------------------------------------------------------------
static UB ParsParm(void)
{
	char*	Parm;
	LONG	tmp;
	UB		mac_addr[6];
	int		i;
	
	for(i=0;i<sizeof mac_addr;i++) {
		if (Parm = strtok(NULL," -\x0D\t"))			// パラメータ指定あり
		{
			if (!(Asc2Hex(Parm,&tmp,0))) return MSG_BADPARM;
			mac_addr[i] = (UB)tmp;
		}
		else return MSG_BADPARM;
	}
	lan_set_mac( mac_addr );
	
	SendText("OK\r\n");
	
	return MSG_NONE;
}
