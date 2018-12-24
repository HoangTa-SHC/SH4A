/**
*	VA-300 ブザー制御プログラム
*
*	@file drv_buz.h
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/07/10
*	@brief  ブザー定義情報(新規作成)
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
#ifndef	_DRV_BUZ_H_
#define	_DRV_BUZ_H_
#include "drv_fpga.h"

// 定義
enum BUZ_CTRL {
	BUZ_OFF,							///< ブザーOFF
	BUZ_ON,								///< ブザーON
};
// マクロ定義
#define	BuzTplOn()		fpga_setw(BUZ_CRL, BUZ_CRL_BZ1)	///< タッチパネルブザーON
#define	BuzTplOff()		fpga_clrw(BUZ_CRL, BUZ_CRL_BZ1)	///< タッチパネルブザーOFF
#define	BuzCycSet(n)	fpga_outw(BUZ1_CYC, n)			///< ブザー周波数設定(SIL)
#define	BuzEmgOn()		fpga_setw(BUZ_CRL, BUZ_CRL_BZ2)	///< 警報ブザーON
#define	BuzEmgOff()		fpga_clrw(BUZ_CRL, BUZ_CRL_BZ2)	///< 警報ブザーOFF

// EXTERN宣言の定義
#if defined(EXTERN)
#undef EXTERN
#endif

#if defined(_DRV_BUZ_C_)
#define	EXTERN
#else
#define	EXTERN	extern
#endif

// プロトタイプ宣言
EXTERN void BuzInit(ID id);								///< ブザー初期化
EXTERN ER BuzTplSet(int iScale, enum BUZ_CTRL eCtrl);	///< ブザーON
#endif										/* end of _DRV_LED_H_				*/
