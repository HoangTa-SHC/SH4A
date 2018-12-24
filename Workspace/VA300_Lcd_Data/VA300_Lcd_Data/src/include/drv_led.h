/**
*	VA-300プログラム
*
*	@file drv_led.h
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/07/10
*	@brief  LED定義情報(他案件から流用)
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
#ifndef	_DRV_LED_H_
#define	_DRV_LED_H_

//#define Test_Old_Version	// Added 2013.6.18 For Sanwa NewTec Products

// 定義
#if defined( Test_Old_Version )
#define	LED_OK			(0x01)				///< 緑色LED
#define	LED_POW			(0x02)				///< オレンジ色LED
#define	LED_ERR			(0x04)				///< 赤色LED
#else
#define	LED_ERR			(0x01)				///< 赤色LED
#define	LED_OK			(0x02)				///< 緑色LED
#define	LED_POW			(0x04)				///< オレンジ色LED
#endif


#define	LED_DBG1		(0x08)				///< デバッグ用LED1
#define	LED_DBG2		(0x10)				///< デバッグ用LED2
#define	LED_DBG3		(0x20)				///< デバッグ用LED3
#define	LED_DBG4		(0x40)				///< デバッグ用LED4
#define	LED_ALL			(0x7F)				///< 全LED

// LEDのON/OFF定義
#define	LED_OFF			0					///< LED OFF
#define	LED_ON			1					///< LED ON
#define	LED_REVERSE		2					///< LED 反転

// プロトタイプ宣言
#if defined(_DRV_LED_C_)
ER LedInit(ID idSem);						// LED初期化
ER LedOut(int, int);						// LED出力
int LedStatusGet(int);						// LED状態取得
#else
extern ER LedInit(ID idSem);
extern ER LedOut(int, int);
extern int LedStatusGet(int);
#endif
#endif										/* end of _DRV_LED_H_				*/
