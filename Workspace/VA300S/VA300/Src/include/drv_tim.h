/**
*	VA-300プログラム
*
*	@file drv_tim.h
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/07/31
*	@brief  タイマ定義情報
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
#ifndef	_DRV_TIM_H_
#define	_DRV_TIM_H_

// タイマー定義
enum TIM_NAME {								///< タイマ定義
	TIM_FLASH,								///< フラッシュメモリドライバ用タイマ
	
	
	TIM_MAX									///< タイマ最大数
};

// EXTERN宣言の定義
#ifdef EXTERN
#undef EXTERN
#endif

#if defined(_DRV_TIM_C_)
#define	EXTERN
#else
#define	EXTERN extern
#endif

// プロトタイプ宣言
EXTERN ER TmInit(ID idCyc);									///< タイマー初期化
EXTERN void TmSet(enum TIM_NAME eName, const long lTime);	///< タイマー設定
EXTERN long TmPol(enum TIM_NAME eName);						///< タイマーポーリング
#endif										/* end of _DRV_TIM_H_				*/
