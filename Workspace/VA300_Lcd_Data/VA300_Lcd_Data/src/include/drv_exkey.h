/**
*	VA-300 センサー部プログラム
*
*	@file drv_exkey.h
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/09/24
*	@brief  10キー定義情報
*
*	Copyright (C) 2010, OYO Electric Corporation
*/
#ifndef	_DRV_EXKEY_H_
#define	_DRV_EXKEY_H_

// 定義(DSW2はFPGAの定義ファイルで定義)
enum KEY_CODE {							///< キーコード
	KEY_NONE     = 0,					///< キーを押していない
	KEY_ENTER    = 0x0D,				///< [呼出]
	KEY_UNDEF    = 0x15,				///< 未定義キー
	KEY_DEL      = 0x18,				///< [取消]
	KEY_0        = '0',					///< 0
	KEY_1,								///< 1
	KEY_2,								///< 2
	KEY_3,								///< 3
	KEY_4,								///< 4
	KEY_5,								///< 5
	KEY_6,								///< 6
	KEY_7,								///< 7
	KEY_8,								///< 8
	KEY_9,								///< 9
	KEY_ASTERISK = '*',					///< [*]
};

// EXTERN宣言の定義
#ifdef EXTERN
#undef EXTERN
#endif

#if defined(_DRV_EXKEY_C_)
#define	EXTERN
#else
#define	EXTERN extern
#endif

// プロトタイプ宣言
EXTERN ER ExKeyInit(BOOL bIntEnable, ID idTsk);			///< 外部キー初期化
EXTERN enum KEY_CODE ExtKeyPol(void);					///< キー入力ポーリング
EXTERN ER ExtKeyGet(enum KEY_CODE *peKey, TMO tmout);	///< キー入力取得
#endif										/* end of _DRV_EXKEY_H_				*/
