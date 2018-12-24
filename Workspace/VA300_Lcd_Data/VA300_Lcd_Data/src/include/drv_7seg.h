/**
*	VA-300プログラム
*
*	@file drv_7seg.h
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/08/31
*	@brief  7Seg定義情報(他案件から流用)
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
#ifndef	_DRV_7SEG_H_
#define	_DRV_7SEG_H_
#include "drv_fpga.h"

enum LED_7SEG_SEL {						///< 7SEGの定義
	LED_7SEG_ALL,						///< ALL
	LED_7SEG_1,							///< 7Seg LED(最上位桁?)
	LED_7SEG_2,							///< 7Seg LED
	LED_7SEG_3,							///< 7Seg LED
	LED_7SEG_4,							///< 7Seg LED
	LED_7SEG_5,							///< 7Seg LED
	LED_7SEG_6,							///< 7Seg LED
	LED_7SEG_7,							///< 7Seg LED
	LED_7SEG_8,							///< 7Seg LED(最下位桁?)

};

// 最下位、最上位については
// FPGAの仕様書によって変更になる可能性あり
// ただし、この定義自体は範囲なので問題なし
#define	LED_7SEG_MIN	LED_7SEG_1		///< 7SEG上位桁
#define	LED_7SEG_MAX	LED_7SEG_8		///< 7SEG下位桁

enum CTRL7SEG {							//=== 7SEG 制御 ===
	CTRL7SEG_NORMAL,					///< 通常表示
};

enum DISP_SELECT {						//=== 表示選択 ===
	DISP_LED,							///< LED選択
	DISP_7SEG							///< 7SEG選択
};

typedef struct {
	ID		idOrg;						///< タスクID
	enum DISP_SELECT eDispSelect;		///< 表示選択
	union {
		struct {
			int iSelect;				///< LEDの選択
			int iCtrl;					///< 制御
			int dummy;					///< 予約
		} Led;
		struct {
			enum LED_7SEG_SEL eSelect;	///< 7SEGの選択
			int iValue;					///< 設定値
			enum CTRL7SEG eCtrl;		///< ７SEG制御
		} Seg;
	}Dev;
}ST_DISPMSG;

// 表示定義
#if defined(_DRV_7SEG_C_)
static const UH s_SegPtn[] = {
	0x3f, 0x06, 0x5b, 0x4f, 0x66, 	//0-4
	0x6d, 0x7d, 0x07, 0x7f, 0x6F,	//5-9
	0x77, 0x7c, 0x58, 0x5e, 0x79,   //A,b,c,d,E
	0x71, 0x50, 0x80, 0x00			//F,r,-,off	
};
static const s_nSegPtn = sizeof s_SegPtn / sizeof s_SegPtn[ 0 ];
#endif
#define	SEG_PTN_r		0x10		///< r
#define	SEG_PTN_MINUS	0x11		///< -
#define	SEG_PTN_OFF		0x12		///< 非表示
#define	SEG_PTN_DOT		0x80		///< .(値にORして使用)

// プロトタイプ宣言
#if defined(_DRV_7SEG_C_)
ER Led7SegInit(ID);							// 7SegLED初期化
ER Led7SegOut(enum LED_7SEG_SEL, int);		// 7SegLED出力
#else
extern ER Led7SegInit(ID);
extern ER Led7SegOut(enum LED_7SEG_SEL, int);
#endif
#endif										/* end of _DRV_7SEG_H_				*/
