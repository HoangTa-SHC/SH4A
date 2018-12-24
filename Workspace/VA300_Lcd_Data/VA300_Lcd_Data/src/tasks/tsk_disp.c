/**
*	VA-300プログラム
*
*	@file tsk_disp.c
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/08/31
*	@brief  表示タスク(他案件から流用)
*
*	Copyright (C) 2010, OYO Electric Corporation
*/
#include "kernel.h"
#include "drv_led.h"
#include "drv_7seg.h"
#include "id.h"
#include "command.h"
#include "va300.h"
#include "err_ctrl.h"

// 定義
#define	LED_BLINK		2				///< LED点滅

// 変数宣言
static int iLedBlink;					// 反転するLED保存領域
static BOOL bSegSlide;					// スライド

// プロトタイプ宣言
static void LedCtrl(int, int);

/*==========================================================================*/
/**
 * LED表示
 */
/*==========================================================================*/
void DispLed(int iSelectLed, int iCtrl)
{
	ER			ercd;
	ST_DISPMSG	*msg;
	
	ercd = tget_mpf(MPF_DISP, (T_MSG**)&msg, (10/MSEC));
	if (ercd != E_OK) {
		ErrCodeSet(ercd);
		return;
	}
	// メッセージ作成
	msg->idOrg   = TSK_DISP;
	msg->eDispSelect = DISP_LED;
	msg->Dev.Led.iSelect = iSelectLed;
	msg->Dev.Led.iCtrl   = iCtrl;
	// メッセージ送信
	ercd = snd_mbx(MBX_DISP, msg);
	if (ercd != E_OK) {
		ErrCodeSet(ercd);
		rel_mpf(MPF_DISP, msg);
	}
}

/*==========================================================================*/
/**
 * 7SEG表示
 */
/*==========================================================================*/
void Disp7Seg(enum LED_7SEG_SEL eSelect7Seg, int iValue, enum CTRL7SEG eCtrl)
{
	ER			ercd;
	ST_DISPMSG	*msg;
	
	ercd = tget_mpf(MPF_DISP, &msg, (10/MSEC));
	if (ercd != E_OK) {
		ErrCodeSet(ercd);
		return;
	}
	// メッセージ作成
	msg->idOrg   = TSK_DISP;
	msg->eDispSelect = DISP_7SEG;
	msg->Dev.Seg.eSelect = eSelect7Seg;
	msg->Dev.Seg.iValue  = iValue;
	msg->Dev.Seg.eCtrl   = eCtrl;
	// メッセージ送信
	ercd = snd_mbx(MBX_DISP, msg);
	if (ercd != E_OK) {
		ErrCodeSet(ercd);
		rel_mpf(MPF_DISP, msg);
	}
}

/*==========================================================================*/
/**
 * 表示タスク
 */
/*==========================================================================*/
TASK DispTask(void)
{
	ER			ercd;
	ST_DISPMSG	*msg;
	enum CTRL7SEG e7SegCtrl;
	
//	LedInit(SEM_LED);					// LED初期化
	Led7SegInit(SEM_7SEG);				// 7SEG初期化
	
	e7SegCtrl = CTRL7SEG_NORMAL;
	
	for(;;) {
		ercd = trcv_mbx(MBX_DISP, &msg, (1000/MSEC));
		if (ercd == E_OK) {
			if (msg->eDispSelect == DISP_LED) {			// LEDの制御選択時
				LedCtrl(msg->Dev.Led.iSelect, msg->Dev.Led.iCtrl);
			} else if (msg->eDispSelect == DISP_7SEG) {	// 7SEGの制御選択時
				e7SegCtrl = msg->Dev.Seg.eCtrl;
				if (e7SegCtrl == CTRL7SEG_NORMAL) {
					Led7SegOut(msg->Dev.Seg.eSelect, msg->Dev.Seg.iValue);
				}
			}
			ercd = rel_mpf(MPF_DISP, msg);
			if (ercd != E_OK) {
				ErrCodeSet(ercd);			// エラー処理
			}
		} else if (ercd == E_TMOUT) {
			LedOut(iLedBlink, LED_REVERSE);
			
			
		} else {
			ErrCodeSet(ercd);				// エラー処理
		}
	}
}

/*==========================================================================*/
/* LED表示																	*/
/*==========================================================================*/
static void LedCtrl(int iSelect, int iCtrl)
{
	switch (iCtrl) {
	case LED_BLINK:
		iLedBlink |= iSelect;
		LedOut(iSelect, iCtrl);
		break;
	case LED_ON:
	case LED_OFF:
		iLedBlink &= ~iSelect;
		LedOut(iSelect, iCtrl);
		break;
	default:
		ErrCodeSet(E_PAR);					// エラー処理
		break;
	}
}
