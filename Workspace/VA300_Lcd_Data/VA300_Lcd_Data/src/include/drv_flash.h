/**
*	VA-300プログラム
*
*	@file drv_flash.h
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/08/01
*	@brief  フラッシュメモリ定義情報
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
#ifndef	_DRV_FLASH_H_
#define	_DRV_FLASH_H_
#include "drv_fpga.h"

// 定義
#ifndef FL_ADDR
	#define	FL_ADDR		0xA2000000L				// フラッシュロム開始アドレス
#endif
#ifndef FL_AREA_SZ
	#define	FL_AREA_SZ	0x02000000				// バンク切替でアクセスできる領域サイズ
#endif

#define	FlBankCalc(n)	(n / FL_AREA_SZ)			///< フラッシュメモリのアクセスウィンドウ番号を求める
#define	FlBankSet(n)	fpga_outw(FROM_BANK, n)		///< フラッシュメモリのアクセスウィンドウ切替
#define	FlBankGet()		fpga_inw(FROM_BANK)			///< フラッシュメモリのアクセスウィンドウ番号読込み
#define	IsFlBank(n)		(fpga_inw(FROM_BANK) == n)	///< 現在のアクセスウィンドウ番号と比較

// EXTERN宣言の定義
#ifdef EXTERN
#undef EXTERN
#endif

#if defined(_DRV_FLASH_C_)
#define	EXTERN
#else
#define	EXTERN extern
#endif

// プロトタイプ宣言
EXTERN void FlInit(ID id);							///< フラッシュメモリドライバ初期化
EXTERN ER FlErase(UW uwAddr);						///< フラッシュメモリ消去
EXTERN UW FlWrite(UW uwFp, UH *puhBp, UW n);		///< フラッシュメモリ書込み
EXTERN ER FlRead(UW uwFp, UH *puhBp, UW n);			///< フラッシュメモリ読込み

#endif										/* end of _DRV_FLASH_H_				*/
