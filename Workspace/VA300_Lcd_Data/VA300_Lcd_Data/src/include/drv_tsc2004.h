/**
*	VA-300プログラム
*
*	@file drv_tsc2004.h
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/09/10
*	@brief  TSC2004定義情報
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
#ifndef	_DRV_TSC2004_H_
#define	_DRV_TSC2004_H_
#include "drv_fpga.h"

// マクロ定義
#define	tscSetCtrlByte(n)	fpga_outw(TPL_CBYTE, n)			///< CONTROL BYTE送信
#define	tscSetDataByte(n)	fpga_outw(TPL_DBYTE, n)			///< DATA BYTE送信
#define	tscGetDataByte()	fpga_inw(TPL_DBYTE)				///< DATA BYTEを読出し
#define	tscSendCtrlByte()	fpga_setw(TPL_CRL, TPL_CRL_WRC)	///< CONTROL BYTE送信のみのシーケンス
#define	tscSendDataByte()	fpga_setw(TPL_CRL, TPL_CRL_WRD)	///< DATA BYTE送信シーケンス
#define	tscReadReg()		fpga_setw(TPL_CRL, TPL_CRL_RD)	///< 読出しシーケンス

#define	tscSendResetOn()	tsc2004Send(s_cCmdResetOn, sizeof s_cCmdResetOn)	///< リセットON送信
#define	tscSendResetOff()	tsc2004Send(s_cCmdResetOff, sizeof s_cCmdResetOff)	///< リセットOFF送信
#define	tscSetCFR0()		tsc2004Send(s_cCmdCFR0Set, sizeof s_cCmdCFR0Set)	///< CFR0レジスタに設定送信
#define	tscSetCFR1()		tsc2004Send(s_cCmdCFR1Set, sizeof s_cCmdCFR1Set)	///< CFR1レジスタに設定送信
#define	tscSetCFR2()		tsc2004Send(s_cCmdCFR2Set, sizeof s_cCmdCFR2Set)	///< CFR2レジスタに設定送信

#define	tscSendPosXGet()	tsc2004Send(s_cCmdPosXGet, sizeof s_cCmdPosXGet)	///< X座標取得要求
#define	tscSendPosYGet()	tsc2004Send(s_cCmdPosYGet, sizeof s_cCmdPosYGet)	///< Y座標取得要求

#define	enable_tpl_int()		fpga_setw(INT_CRL, INT_CRL_TPL);				///< タッチパネル割込み許可
#define	enable_tplcom_int()		fpga_setw(INT_CRL, INT_CRL_TPL_CMD);			///< タッチパネル通信割込み許可
#define	disable_tpl_int()		fpga_clrw(INT_CRL, INT_CRL_TPL);				///< タッチパネル割込み許可
#define	disable_tplcom_int()	fpga_clrw(INT_CRL, INT_CRL_TPL_CMD);			///< タッチパネル通信割込み許可

// EXTERN宣言の定義
#ifdef EXTERN
#undef EXTERN
#endif

#if defined(_DRV_TSC2004_C_)
#define	EXTERN
#else
#define	EXTERN extern
#endif

// プロトタイプ宣言
EXTERN ER Tsc2004Init(void);							///< デバイス初期化
EXTERN ER Tsc2004PosGet(int *piPosX, int *piPosY);		///< 座標取得
EXTERN void Tsc2004ComInt(void);						///< 通信完了割込み
#endif										/* end of _DRV_TSC2004_H_				*/
