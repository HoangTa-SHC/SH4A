//=============================================================================
/**
 *
 * VA-300プログラム
 * <<機能モジュール>>
 *
 *	@brief 一般的な機能。
 *	
 *	@file sub_func.c
 *	
 *	@date	2012/05/20
 *	@version 1.00 新規作成
 *	@author	F.Saeki
 *
 *	Copyright (C) 2011, OYO Electric Corporation
 */
//=============================================================================
#include <machine.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
//#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include "kernel.h"
#include "sh7750.h"
#include "id.h"
#include "err_ctrl.h"

//==========================================================================
/**
 * sprintfのセマフォ版
 */
//==========================================================================
int _sprintf(char *s, const char *control, ...)
{
	ER ercd;
	int n;
	va_list	ap;
	
	ercd = twai_sem(SEM_SPF, (1000/MSEC));
	if (ercd != E_OK) {
		return 0;
	}
	
	va_start(ap, control);
	n = vsprintf(s, control, ap);
	va_end(ap);
	
	sig_sem(SEM_SPF);
	
	return n;
}

//=============================================================================
/**
 *	メモリプール使用量表示データ取得
 *	@param iLine 行番号
 *	@param pBuf 格納先
 *	@retval TRUE 取得成功
 */
//=============================================================================
BOOL MpfUseDisplayLine(int iLine, char *pBuf)
{
	T_RMPF rmpf;
	const char* ccMpfName[] = {
		"NONE",
		"MPF_COM",
		"MPF_MON",
		"MPF_DISP",
		"MPF_LRES",
	};

	if ((iLine < 1) && (iLine >= MPF_MAX)) {
		return FALSE;
	}
	if (ref_mpf(iLine, &rmpf) != E_OK) {
		return FALSE;
	}
	_sprintf(pBuf, "MPF : %s  COUNT : %d\r\n", ccMpfName[ iLine ], rmpf.fblkcnt);
	
	return TRUE;
}
