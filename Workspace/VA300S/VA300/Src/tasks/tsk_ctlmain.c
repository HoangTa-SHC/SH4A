/**
*	VA-300テストプログラム
*
*	@file tsk_ctlmain.c
*	@version 1.00
*
*	@author Bionics Co.Ltd T.Nagai
*	@date   2013/04/20
*	@brief  VA-300 端末制御main
*
*	Copyright (C) 2013, Bionics Corporation
*/
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "kernel.h"
#include "sh7750.h"
#include "id.h"
#include "drv_fpga.h"
#include "err_ctrl.h"

#include "va300.h"

// 変数定義
static ID s_idTsk;

// プロトタイプ宣言
static TASK CtlMain( void );		///< 端末コントロール・メイン・タスク

// タスクの定義
const T_CTSK ctsk_ctlmain    = { TA_HLNG, NULL, CtlMain,    8, 4096, NULL, (B *)"ctlmain" };


/*==========================================================================*/
/**
 * 端末コントロール・メイン・タスク
 * 
 */
/*==========================================================================*/
TASK CtlMain( void )
{
	ER		ercd;
	
	// 処理開始
	for(;;) {
		// 要変更
		slp_tsk();
	}
}


/*==========================================================================*/
/**
 * 端末コントロール・メイン・タスク初期化
 *
 * @param idTsk タスクID
 * @retval E_OK 正常起動
 */
/*==========================================================================*/
ER CtlMainTaskInit(ID idTsk)
{
	ER ercd;
	
	// タスクの生成
	if (idTsk > 0) {
		ercd = cre_tsk(idTsk, &ctsk_ctlmain);
		if (ercd == E_OK) {
			s_idTsk = idTsk;
		}
	} else {
		ercd = acre_tsk(&ctsk_ctlmain);
		if (ercd > 0) {
			s_idTsk = ercd;
		}
	}
	if (ercd < 0) {
		return ercd;
	}
	
	// タスクの起動
	ercd = sta_tsk(s_idTsk, 0);
	
	return ercd;
}
