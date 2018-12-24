/**
*	VA-300 赤外線LEDプログラム
*
*	@file drv_irled.c
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/09/25
*	@brief  赤外線LEDモジュール(新規作成)
*			たぶん不要と思うのでセマフォは使用しないようにした。
*			ただし、必要になったら入れれるような構造にしている。
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
#define	_DRV_IRLED_C_
#include "kernel.h"
#include "drv_fpga.h"
#include "drv_irled.h"

// マクロ定義
#define	irled_set(n,v)	FpgaIrLedSet(n,v)	///< 赤外線LED設定(SIL)

// プロトタイプ宣言
static void irLedSet(UB ubLed, UB ubVal);	///< 赤外線LEDにDuty比を設定する

/*==========================================================================*/
/**
 * 赤外線LED関連初期化
 *
 *	@param idSem LED用セマフォID
 *	@return エラーコード
 */
/*==========================================================================*/
ER IrLedInit(ID idSem)
{
	return E_OK;
}

/*==========================================================================*/
/**
 * 赤外線LEDにDutyを設定する(GDIC)
 *
 * @param  ubLed 赤外線LEDを指定(IR_LED_1(01)/IR_LED_2(02)/IR_LED_3(04)...)
 * @param  ubVal Duty(0～255)
 * @retval E_OK 設定OK
 */
/*==========================================================================*/
ER IrLedSet(UB ubLed, UB ubVal)
{
	ER	ercd;
	
	ercd = E_OK;
	
	irLedSet(ubLed, ubVal);

	return ercd;
}

/*==========================================================================*/
/**
 * 赤外線LEDにDutyを設定する(PDIC)
 *
 * @param  ubLed 赤外線LEDを指定(IR_LED_1(01)/IR_LED_2(02)/IR_LED_3(04)...)
 * @param  ubVal Duty(0～255)
 * @retval E_OK 設定OK
 */
/*==========================================================================*/
static void irLedSet(UB ubLed, UB ubVal)
{
	irled_set(ubLed, ubVal);				// 赤外線LEDに値を設定
}
