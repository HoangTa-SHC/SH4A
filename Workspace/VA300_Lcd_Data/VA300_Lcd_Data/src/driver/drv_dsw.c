/**
*	VA-300プログラム
*
*	@file drv_dsw.c
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/08/30
*	@brief  DSWモジュール(他案件から流用)
*
*	Copyright (C) 2010, OYO Electric Corporation
*/
#define	_DRV_DSW_C_
#include "kernel.h"
#include "sh7750.h"
#include "drv_dsw.h"
#include "drv_fpga.h"

// 定義
#define	dsw_in()	((~sfr_inw(BSC_PDTRA)) & DSW1_ALL)	///< DSWのポートから読込み(SIL)

/*==========================================================================*/
/**
 * DSWの取得
 *
 * @return DSWの状態(ON:bitがON、上位8bitがDSW1、下位8bitがDSW2)
 */
/*==========================================================================*/
UH DswGet(void)
{
	UH uhDsw;
	
	// ポートに接続されているDSW
	uhDsw = dsw_in();
	
	return uhDsw;
}
