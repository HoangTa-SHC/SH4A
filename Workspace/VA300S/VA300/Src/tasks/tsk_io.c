/**
*	VA-300プログラム
*
*	@file tsk_io.c
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/8/2
*	@brief  I/O検知タスク
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
#include <stdio.h>

#include "kernel.h"
#include "sh7750.h"
#include "id.h"
#include "drv_fpga.h"
#include "err_ctrl.h"

#include "va300.h"

// 

// プロトタイプ宣言
static BOOL ioVecInit(void);				// IOベクタ初期化
static void dma_stop_int(void);				// DMA転送終了割込み

// 変数定義
const T_DINH dinh_stop_dma  = { TA_HLNG, DmaStopInt,   5};

/*==========================================================================*/
/*	I/Oベクタ初期化															*/
/*==========================================================================*/
static BOOL ioVecInit(void)
{
	ER ercd;

	return (ercd == E_OK);
}

/*==========================================================================*/
/**
 * I/O検知タスク
 */
/*==========================================================================*/
TASK IoTask(void)
{
	ER		ercd;
	FLGPTN	flgptn;
	static const FLGPTN	waiptn = FPTN_INIT;
	UB		ubErrCode;
	
	// 初期化

	
	ioVecInit();
	
	// 処理開始
	for(;;) {
		ercd = wai_flg(ID_FLG_IO, waiptn, TWF_ORW, &flgptn);
		if (ercd == E_OK) {
			if (flgptn & FPTN_INIT) {
				
				// フラグのクリア
				clr_flg(ID_FLG_IO, ~waiptn);
			}
			if (flgptn & FPTN_CAP_TR_END) {
				
				// フラグのクリア
				clr_flg(ID_FLG_IO, ~FPTN_CAP_TR_END);
			}
		} else {
			// ここにくるのは実装エラー
			PrgErrSet();
			slp_tsk();							// エラーのときはタスク終了
		}
	}
}

#pragma interrupt(DmaStopInt)
//=============================================================================
/**
 *	DMA転送完了割込み
 */
//=============================================================================
INTHDR DmaStopInt(void)
{
	ent_int();							// 割込みハンドラの開始
	dma_stop_int();						// 割込みハンドラ本体
	ret_int();							// 割込みハンドラから復帰する
}

//=============================================================================
/**
 *	DMA転送完了割込み
 */
//=============================================================================
static void dma_stop_int(void)
{
	
	iset_flg(ID_FLG_IO, FPTN_CAP_TR_END);
}
