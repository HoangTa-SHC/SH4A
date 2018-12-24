/**
*	VA-300プログラム
*
*	@file drv_tim.c
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/07/31
*	@brief  タイマモジュール(他案件から流用)
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
#include "kernel.h"
#include "sh7750.h"
#include "drv_tim.h"

// プロトタイプ宣言
static void TimerCyc(VP_INT exinf);					// タイマー処理

// 変数宣言
static const T_CCYC ccyc_tmo    = { TA_HLNG, NULL, TimerCyc, 10,    0};
static long s_lTmupTimer[ TIM_MAX ];				///< タイマカウンタ

//=============================================================================
/**
 * 周期ハンドラ初期化
 * @param idCyc ID
 * @return エラーコード
 */
//=============================================================================
ER TmInit(ID idCyc)
{
	ER  ercd;
	UB  i;
	
	ercd = E_PAR;
	
	// カウンタクリア
	for(i = 0;i < TIM_MAX;i++) {
		s_lTmupTimer[ i ] = 0L;
	}
	
	// サイクリックタイマー作成、起動
	if (idCyc) {
		ercd = cre_cyc(idCyc, &ccyc_tmo);		/* Create cyclic handler */
		if (ercd == E_OK) {
			ercd = sta_cyc( idCyc );
		}
	} else {
		ercd = acre_cyc(&ccyc_tmo);
		if (ercd > 0) {
			ercd = sta_cyc( ercd );
		}
	}
	
	return ercd;
}

//=============================================================================
/**
 * タイマー設定(タイマーカウンタを設定する)
 * @param eName 使用するタイマー
 * @param lTime 時間
 */
//=============================================================================
void TmSet(enum TIM_NAME eName, const long lTime)
{
	if( eName < TIM_MAX) {
		s_lTmupTimer[ eName ] = lTime;
	}
}

//=============================================================================
/**
 * タイマーポーリング
 * @param eName 使用するタイマー
 * @return 時間(IDがおかしいときは-1を返す)
 */
//=============================================================================
long TmPol(enum TIM_NAME eName)
{
	if( eName < TIM_MAX) {
		return s_lTmupTimer[ eName ];
	} else {
		return (-1);
	}
}

//=============================================================================
/**
 * 周期ハンドラ処理(タイマカウンタを減らす)
 * @param exinf 未使用
 */
//=============================================================================
static void TimerCyc(VP_INT exinf)
{
	enum TIM_NAME eName;
	
	// タイマーカウンタ減算
	for (eName = 0;eName < TIM_MAX;eName++) {
		if(s_lTmupTimer[ eName ]) s_lTmupTimer[ eName ]--;		// カウンタ減算
	}
}
