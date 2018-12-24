/**
*	VA-300プログラム
*
*	@file drv_irled.h
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/09/25
*	@brief  赤外線LED定義情報
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
#ifndef	_DRV_IRLED_H_
#define	_DRV_IRLED_H_
#include "drv_fpga.h"						///< FPGAで制御

// マクロ定義(デバイスの関数にあわせる)
#define	IrLedCrlOn(n)	fpga_setw(IRLED_CRL, n)	///< 赤外線LEDをONする
#define	IrLedCrlOff(n)	fpga_clrw(IRLED_CRL, n)	///< 赤外線LEDをOFFする

#define	IR_LED_1	IRLED_CRL_LED1			///< 赤外線LED1
#define	IR_LED_2	IRLED_CRL_LED2			///< 赤外線LED2
#define	IR_LED_3	IRLED_CRL_LED3			///< 赤外線LED3
#define	IR_LED_4	IRLED_CRL_LED4			///< 赤外線LED4
#define	IR_LED_5	IRLED_CRL_LED5			///< 赤外線LED5

// 型宣言


// EXTERN宣言の定義
#ifdef EXTERN
#undef EXTERN
#endif

#if defined(_DRV_IRLED_C_)
#define	EXTERN
#else
#define	EXTERN extern
#endif

// プロトタイプ宣言
EXTERN ER IrLedInit(ID idSem);			///< 赤外線LED初期化
EXTERN ER IrLedSet(UB ubLed, UB ubVal);	///< 赤外線LED Duty設定
#endif									/* end of _DRV_IRLED_H_				*/
