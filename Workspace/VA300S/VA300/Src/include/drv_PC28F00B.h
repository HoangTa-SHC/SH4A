/**
*	VA-300プログラム
*
*	@file drv_PC28F00B.h
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/09/13
*	@brief  Micron製PC28F00BM29EWHA定義情報
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
#ifndef	_DRV_PC28F00B_H_
#define	_DRV_PC28F00B_H_

// 定義
#define	ERS_TMOUT	(5000 * 1000)
#define	PRG_TMOUT	(1000 * 1000)
#ifndef FL_ADDR
	#define	FL_ADDR		0xA2000000L				// フラッシュロム開始アドレス
#endif
#define	FL_AREA_SZ	0x02000000					// バンク切替でアクセスできる領域サイズ

// EXTERN宣言の定義
#ifdef EXTERN
#undef EXTERN
#endif

#if defined(_DRV_PC28F00B_C_)
#define	EXTERN
#else
#define	EXTERN extern
#endif

// プロトタイプ宣言
EXTERN ER FdErase(UW uwAddr);						///< フラッシュメモリ消去
EXTERN UW FdWrite(UW uwFp, UH *puhBp, UW n);		///< フラッシュメモリ書込み
EXTERN ER FdRead(UW uwFp, UH *puhBp, UW n);			///< フラッシュメモリ読込み
EXTERN UW FdFlAllSize(void);						///< フラッシュメモリサイズを返す
#endif										/* end of _DRV_FLASH_H_				*/
