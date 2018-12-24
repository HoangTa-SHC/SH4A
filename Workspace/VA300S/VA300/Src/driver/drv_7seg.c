/**
*	VA-300プログラム
*
*	@file drv_7seg.c
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/09/07
*	@brief  7SegLEDモジュール(他案件から流用)
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
#define	_DRV_7SEG_C_
#include <string.h>
#include "kernel.h"
#include "drv_7seg.h"

// マクロ定義
#define	seg_out(n,v)	fpga_outw((SEG_CRL + (n << 1)), v)	///< 7SEGへ出力(SIL)
#define	seg_in(n)		fpga_inw((SEG_CRL + (n << 1)))		///< 7SEGの状態を読出し(SIL)

// ローカル変数
static unsigned char SegStatus[ LED_7SEG_MAX ];				///< 状態保持用変数
static ID s_idSem;											///< セマフォID

const T_CSEM csem_seg = { TA_TFIFO, 1, 1, (B *)"sem_7seg" };

// プロトタイプ宣言
static BOOL SegOut(enum LED_7SEG_SEL SelectSeg, int Value);	///< 7SegLEDへの実際の出力関数

/*==========================================================================*/
/**
 * 7SegLED初期化
 *
 *	@param idSem LED用セマフォID
 *	@return エラーコード
 */
/*==========================================================================*/
ER Led7SegInit(ID idSem)
{
	ER ercd;

	memset(SegStatus, 0, sizeof(SegStatus));
	s_idSem = 0;

	ercd = cre_sem(idSem, &csem_seg);		// Create semaphore
	if (ercd == E_OK) {
		s_idSem = idSem;
		ercd = Led7SegOut(LED_7SEG_ALL, 0);	// 7Seg初期化
	}
	
	return ercd;
}

/*==========================================================================*/
/**
 * 7SegLEDのON/OFF
 *
 * @param  eSelectSeg LED_7SEG_1(1)～LED_7SEG_10(10)を指定
 * @param  Value 0～H'0Fを指定
 * @retval E_OK 設定OK
 */
/*==========================================================================*/
ER Led7SegOut(enum LED_7SEG_SEL eSelectSeg, int Value)
{
	ER	ercd;
	
	ercd = twai_sem(s_idSem, (10/MSEC));
	if (ercd == E_OK) {
		if ( SegOut(eSelectSeg, Value) == FALSE) {
			ercd = E_PAR;				// パラメータエラー
		}
	} else {							// セマフォ獲得失敗のとき
		return ercd;
	}
	if (sig_sem(s_idSem) != E_OK) {	// ここでエラーがでるのは設計ミス
		return E_SYS;					// エラー
	}
	return ercd;
}

/*==========================================================================*/
/* 7SegLEDの出力															*/
/*==========================================================================*/
static BOOL SegOut(enum LED_7SEG_SEL eSelectSeg, int Value)
{
	int i, iSelSeg;
	unsigned short usOut;
	unsigned short usReg;

	if (eSelectSeg == LED_7SEG_ALL) {
		for( i = LED_7SEG_MIN;i <= LED_7SEG_MAX;i++) {
			SegOut( i, Value);
		}
	} else if (eSelectSeg >= LED_7SEG_MIN && eSelectSeg <= LED_7SEG_MAX) {
		if ((Value & ~SEG_PTN_DOT) >= s_nSegPtn) {
			return FALSE;
		}
		iSelSeg = (int)(eSelectSeg - LED_7SEG_1);
		SegStatus[ iSelSeg ] = Value;						// 保存変数に値を格納
		if ((SegStatus[ iSelSeg ] & ~SEG_PTN_DOT) < s_nSegPtn) {
			usOut = s_SegPtn[ (SegStatus[ iSelSeg ]) ];		// 出力パターンがあれば設定
			if (SegStatus[ iSelSeg ] & SEG_PTN_DOT) {		// '.'表示あれば設定
				usOut |= SEG_PTN_DOT;
			}
			// 7SEG出力
			seg_out(iSelSeg, usOut);
		}
	} else {
		return FALSE;
	}
	return TRUE;
}