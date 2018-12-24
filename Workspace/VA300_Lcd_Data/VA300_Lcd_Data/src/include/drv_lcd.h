/**
*	LCDドライバ
*
*	@file drv_lcd.c
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2013/03/09
*	@brief  LCDドライバ
*
*	Copyright (C) 2007, OYO Electric Corporation
*/

#ifndef	_DRV_LCD_H_
#define	_DRV_LCD_H_

#include <kernel.h>


// 定数定義
#define SCREEN_WIDTH	480		///< LCD画面横幅
#define SCREEN_HEIGHT	272		///< LCD画面縦幅

#define SDRAM_BUF_SIZE	0x60000	///< SDRAM ライトバッファメモリサイズ 384Kに設定
								///< 480*272*3(byte)を16KB単位に切り上げ(0x60000)

// EXTERN宣言の定義
#ifdef EXTERN
#undef EXTERN
#endif

#if defined(_DRV_LCD_C_)
#define	EXTERN
#else
#define	EXTERN extern
#endif

// プロトタイプ宣言
EXTERN void LcdTest(void);
EXTERN ER LcdcInit(ID idSem);				///< LCD初期化
EXTERN ER LcdcDisplayWindowSet( UB bufNum,
								UB colorEnb,
								UB keyR,
								UB keyG,
								UB keyB,
								UH Xstart,
								UH Ystart,
								UH Xend,
								UH Yend);	///< ディスプレイウィンドウの入力画像条件の設定
EXTERN ER LcdImageWrite(UH val);			///< ディスプレイウィンドウに16ビット画像データを書き込み
EXTERN ER LcdcDisplayWindowFinish(void);	///< ディスプレイウィンドウのための設定を完了
EXTERN ER LcdcAlphablendSet(UB mode, 
							UB alpha,
							UH width,
							UH height,
							UB src1bufnum,
							UH src1Xstart,
							UH src1Ystart,
							UB src2bufnum,
							UH src2Xstart,
							UH src2Ystart,						
							UB dstbufnum,
							UH dstXstart,
							UH dstYstart);		///< アルファブレンディング設定
EXTERN ER LcdcAlphablendEnd( UB *result );		///< アルファブレンディング終了
EXTERN ER LcdcPictureInPictureSet(UB pipnum,
							UH width_pip, 	
							UH height_pip,	
							UB srcbufnum_pip,
							UH srcXstart_pip,
							UH srcYstart_pip,
							UH dstXstart_pip,
							UH dstYstart_pip);	///< PIP設定
EXTERN ER LcdcDisplayModeSet( UB bufNum, UB pipSel);	///< メイン画面表示バッファを選択
EXTERN ER LcdcBackLightOn(void);				///< バックライト点灯
EXTERN ER LcdcBackLightOff(void);				///< バックライト消灯
EXTERN ER LcdcTestColorBarSet(void);			///< カラーバーテスト

#endif