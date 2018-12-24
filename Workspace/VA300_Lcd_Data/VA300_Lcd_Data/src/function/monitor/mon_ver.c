//=============================================================================
/**
 *	ＳＨ-２ ＣＰＵモジュール モニタプログラム
 *	
 *	@file Mon_ver.c
 *	@version 1.00
 *	
 *	@author	F.Saeki
 *	@date	2001/12/07
 *	@brief	バージョン表示コマンドモジュール
 *
 *	Copyright (C) 2007, OYO Electric Corporation
 */
//=============================================================================
#include "kernel.h"
#include "nonet.h"
#include "nonteln.h"
#include "mon.h"

//=============================================================================
/**
 * ＶＥＲコマンドの実行
 */
//=============================================================================
void CmdVer(void)
{
	Ctrl.Proc = ReadCmd;						// コマンド入力に戻る

	SendText("*** VA-300 Driver test Program Version 0.91       ***" CRLF);
	SendText("*** Copyright (C)2012 OYO Electric Software Group ***" CRLF);
}

