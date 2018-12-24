/**
*	VA-300プログラム
*
*	@file drv_tpl.h
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/09/12
*	@brief  タッチパネル定義情報
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
#ifndef	_DRV_TPL_H_
#define	_DRV_TPL_H_
#include "drv_tsc2004.h"						///< デバイスはTSC2004を使用

// マクロ定義(デバイスの関数にあわせる)
#define	tpl_reset()			Tsc2004Init()		///< リセットON送信
#define	tpl_pos_get(x,y)	Tsc2004PosGet(x,y)	///< 座標取得
#define	tpl_com_int() 		Tsc2004ComInt()		///< 通信割込み関数

// 型宣言
typedef struct {						///< タッチパネルの補正値
	float fXx;							///< X座標用 X座標値にかける値
	float fXy;							///<         Y座標地にかける値
	float fXofs;						///<         オフセット
	float fYx;							///< Y座標用 X座標値にかける値
	float fYy;							///<         Y座標地にかける値
	float fYofs;						///<         オフセット
} ST_TPL_REV;

typedef struct {						///< ポジション定義
	int iX;								///< X座標
	int iY;								///< Y座標
} ST_POS;

typedef struct {						///< 補正用ポジション格納構造体定義
	ST_POS	tPos[ 3 ];					///< ポジション3点
} ST_POS_REV;

// EXTERN宣言の定義
#ifdef EXTERN
#undef EXTERN
#endif

#if defined(_DRV_TPL_C_)
#define	EXTERN
#else
#define	EXTERN extern
#endif

// プロトタイプ宣言
EXTERN ER TplInit(ID idSem);						///< タッチパネル初期化
EXTERN ER TplPosGet(int *piPosX, int *piPosY, TMO tmout);		///< 座標取得
EXTERN void TplRevSet(float fXx, float fXy, float fXofs, float fYx, float fYy, float fYofs);				///< 補正値の設定
EXTERN void TplRevGet(float *pfXx, float *pfXy, float *pfXofs, float *pfYx, float *pfYy, float *pfYofs);	///< 現在の補正値の取得
EXTERN void TplRevInit(void);						///< 補正値の初期化
EXTERN void TplRevCalc(ST_POS_REV *pBase, ST_POS_REV *pInput, ST_TPL_REV *pRev);	///< 補正値の計算
#endif										/* end of _DRV_TPL_H_				*/
