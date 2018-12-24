/**
*	VA-300プログラム
*
*	@file drv_io.h
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2013/03/08
*	@brief  I/O定義情報
*
*	Copyright (C) 2013, OYO Electric Corporation
*/
#ifndef	_DRV_IO_H_
#define	_DRV_IO_H_
#include "drv_fpga.h"

// 定義
#define	AuxOut1On()				fpga_setw(AUX_OUT, AUX_OUT_1)	///< AUX OUT1 出力ON
#define	AuxOut1Off()			fpga_clrw(AUX_OUT, AUX_OUT_1)	///< AUX OUT1 出力OFF
#define	AuxOut2On()				fpga_setw(AUX_OUT, AUX_OUT_2)	///< AUX OUT2 出力ON
#define	AuxOut2Off()			fpga_clrw(AUX_OUT, AUX_OUT_2)	///< AUX OUT2 出力OFF

#define	IsAuxInA0()				((fpga_inw(AUX_IN) & AUX_IN_A0) == AUX_IN_A0)	///< AUX IN A0入力
#define	IsAuxInA1()				((fpga_inw(AUX_IN) & AUX_IN_A1) == AUX_IN_A1)	///< AUX IN A1入力
#define	IsAuxInB0()				((fpga_inw(AUX_IN) & AUX_IN_B0) == AUX_IN_B0)	///< AUX IN B0入力
#define	IsAuxInB1()				((fpga_inw(AUX_IN) & AUX_IN_B1) == AUX_IN_B1)	///< AUX IN B1入力

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
EXTERN ER IoInit(ID idTsk);					// I/O割込み初期化

#endif										/* end of _DRV_IO_H_				*/
