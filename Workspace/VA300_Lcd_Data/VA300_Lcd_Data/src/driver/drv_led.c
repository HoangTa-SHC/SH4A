/**
*	VA-300プログラム
*
*	@file drv_led.c
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/09/07
*	@brief  LEDモジュール(他案件から流用)
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
#define	_DRV_LED_C_
#include "kernel.h"
#include "drv_fpga.h"
#include "drv_led.h"
#include "sh7750.h"

// マクロ定義
#define	led_out(n)	fpga_outw(LED_CRL, n)	///< LED出力(SIL)
#define	led_out2(n)	sfr_outw(BSC_PDTRA, ((sfr_inw(BSC_PDTRA) & 0xF0FF) | (n << 5)))	///< LED出力(SIL)

// ローカル変数
static UH s_uhLedStatus;				// 状態保持用変数
static const UH s_uhLedList[] = {
	LED_POW,							///< 通電LED(緑)
	LED_OK,								///< 認証OK LED(オレンジ)
	LED_ERR,							///< エラーLED(赤)
	
	LED_DBG1,							///< デバッグ用LED
	LED_DBG2,							///< デバッグ用LED
	LED_DBG3,							///< デバッグ用LED
	LED_DBG4,							///< デバッグ用LED
};
static const UH s_nLedList = sizeof s_uhLedList / sizeof s_uhLedList[ 0 ];
static ID s_idSem;						///< セマフォID

const T_CSEM csem_led = { TA_TFIFO, 1, 1, (B *)"sem_led" };

// プロトタイプ宣言
static void ledOut(int SelectLed, int OnOff);	///< LED出力制御(本体)

/*==========================================================================*/
/**
 * LED初期化
 *
 *	@param idSem LED用セマフォID
 *	@return エラーコード
 */
/*==========================================================================*/
ER LedInit(ID idSem)
{
	ER ercd;

	sfr_setl(BSC_PCTRA, 0x00550000);	// PA8-11設定

	s_uhLedStatus = 0;
	s_idSem = 0;
	ercd = cre_sem(idSem, &csem_led);	// Create semaphore
	
	if (ercd == E_OK) {
		s_idSem = idSem;
		ercd = LedOut(LED_ALL, LED_OFF);// LED OFF
	}
	
	return ercd;
}

/*==========================================================================*/
/**
 * LEDのON/OFF(GDIC)
 *
 * @param  SelectLed LED_POW(01)/LED_OK(02)/LED_ERR(04)を指定
 * @param  OnOff LED_OFF(0)/LED_ON(1)/LED_REVERSE(2)を指定
 * @retval E_OK 設定OK
 */
/*==========================================================================*/
ER LedOut(int SelectLed, int OnOff)
{
	ER	ercd;
	
	ercd = twai_sem(s_idSem, (10/MSEC));
	if (ercd == E_OK) {
		ledOut(SelectLed, OnOff);
	} else {								// セマフォ獲得失敗のとき
		return ercd;
	}

	if (sig_sem(s_idSem) != E_OK) {			// ここでエラーがでるのは設計ミス
		return E_SYS;						// エラー
	}
	
	return ercd;
}

/*==========================================================================*/
/**
 * LEDのON/OFF(PDIC)
 *
 * @param  SelectLed LED_POW(01)/LED_OK(02)/LED_ERR(04)を指定
 * @param  OnOff LED_OFF(0)/LED_ON(1)/LED_REVERSE(2)を指定
 * @retval E_OK 設定OK
 */
/*==========================================================================*/
static void ledOut(int SelectLed, int OnOff)
{
	int i;
	UH	uhLed;
	
	for (i = 0;i < s_nLedList;i++) {
		uhLed = s_uhLedList[ i ];
		if (SelectLed & uhLed) {
			if (OnOff == LED_ON) {
				s_uhLedStatus |= uhLed;	// Highで点灯
			} else if (OnOff == LED_REVERSE){
				s_uhLedStatus ^= uhLed;
			} else {
				s_uhLedStatus &= ~uhLed;// Lowで消灯
			}
		}
	}
	s_uhLedStatus &= LED_ALL;
	led_out((s_uhLedStatus & (LED_POW | LED_OK | LED_ERR)));	// LED出力
	// LED出力
	led_out2((s_uhLedStatus & (LED_DBG1 | LED_DBG2 | LED_DBG3 | LED_DBG4)));
}

/*==========================================================================*/
/**
 * LEDの状態取得
 *
 * @param  SelectLed LED_POW(01)/LED_OK(02)/LED_ERR(04)を指定
 * @retval LED_ON(1)
 * @retval LED_OFF(0)
 */
/*==========================================================================*/
int LedStatusGet(int SelectLed)
{
	int	iStatus;
	int	i;
	UH	uhLed;

	iStatus = -1;						// パラメータエラーにしておく
	for (i = 0;i < s_nLedList;i++) {
		uhLed = s_uhLedList[ i ];
		if (SelectLed == uhLed) {
			iStatus = LED_OFF;
			if (s_uhLedStatus & uhLed) {
				iStatus = LED_ON;
			}
			break;
		}
	}
	
	return iStatus;
}
