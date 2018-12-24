/**
*	VA-300プログラム
*
*	@file err_ctrl.h
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/08/01
*	@brief  エラーステータス定義情報(他案件から流用)
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
#ifndef	_ERR_CTRL_H_
#define	_ERR_CTRL_H_

// 定義
typedef struct {
	FP	task;							///< タスクアドレス
	B	*byTaskName;					///< タスク
	UB	ubErrType;						///< エラータイプ
	int	ercd;							///< エラーコード
}ST_ERR;

enum MAIN_ERR_CODE {					///< メインエラーコード定義
	MAIN_ERR_NONE,						///< エラーコード無
	
	MAIN_ERR_LAN = 7,					///< 通信異常
	MAIN_ERR_PROGRAM,					///< プログラムエラー
	MAIN_ERR_RTOS,						///< リアルタイムOSエラー
	MAIN_ERR_FPGA,						///< FPGAエラー
	MAIN_ERR_PARM,						///< パラメータエラー
};

enum SUB_ERR_PRGERR {					///< プログラムエラーのサブコード
	SUB_ERR_PRG_CODE,					///< プログラムコードエラー
	
};

enum SUB_ERR_PARMERR {
	SUB_ERR_PARM_CHG = 1,				///< 印加プロパティエラー
};

// FPGAエラー定義
#define	FPGA_ERR_EEP			0x04	///< EEPROMエラー
#define	FPGA_ERR_DMA			0x02	///< DMA転送エラー

#define	ErrCodeSet(ercd)	ErrStatusSet(MAIN_ERR_RTOS, (int)ercd, __FILE__, __LINE__)
#define	PrgErrSet()			ErrStatusSet(MAIN_ERR_PROGRAM, SUB_ERR_PRG_CODE, __FILE__, __LINE__)

// プロトタイプ宣言
#if defined(_ERR_CTRL_C_)
void ErrStatusInit(void);				// エラー初期化
ER ErrStatusSet(UB, int, char*, int);	// エラーステータス設定
void ErrStatusGet(UB*, UB*);			// 最新のエラーコード読出し
void ErrLog(char *p);					// エラーログ書込み
#else
extern void ErrStatusInit(void);		// エラー初期化
extern ER ErrStatusSet(UB, int, char*, int);	// エラーステータス設定
extern void ErrStatusGet(UB*, UB*);		// 最新のエラーコード読出し
extern void ErrLog(char *p);			// エラーログ書込み
#endif
#endif									/* end of _ERR_CTRL_H_				*/
