/**
*	VA-300プログラム
*
*	@file drv_buz.c
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/07/10
*	@brief  Buzzerモジュール(他案件から流用)
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
#define	_DRV_BUZ_C_
#include <mathf.h>
#include "kernel.h"
#include "sh7750.h"
#include "drv_buz.h"

// 定義
#define	FREQ_MIN	50.0						///< 最小周波数
#define	FREQ_MAX	48000.0						///< 最大周波数

// ローカル変数
static ID s_idSem;						// セマフォID

const T_CSEM csem_buz  = { TA_TFIFO, 1, 1, (B *)"sem_buz" };
	// 音階と周波数リスト(デモ機から引用)
const float	buzfrq[] = { 
	261.5,	///< ド
	277.0,	///< ド#
	293.5,	///< レ
	311.0,	///< レ#
	329.5,	///< ミ
	349.0,	///< ファ
	369.5,	///< ファ#
	391.5,	///< ソ
	415.0,	///< ソ#
	440.0,	///< ラ
	466.0,	///< ラ#
	494.0,	///< シ
	523.0,
	554.0,
	587.0,
	662.0
};

// プロトタイプ宣言
static void buzTplSet(int iScale, enum BUZ_CTRL eCtrl);	///< ブザー設定
static float freqCalc(int iScale);						///< 周波数の計算

/*==========================================================================*/
/**
 * ブザー初期化
 * @param id ブザー用セマフォID
 */
/*==========================================================================*/
void BuzInit(ID id)
{
	ER ercd; 

	s_idSem = 0;
	
	// セマフォの生成
	if (id > 0) {
		ercd = cre_sem(id, &csem_buz);	
		if (ercd == E_OK) {
			s_idSem = id;
		}
	} else {
		ercd = acre_sem(&csem_buz);
		if (ercd > 0) {
			s_idSem = ercd;
		}
	}
	
	BuzTplSet(0, BUZ_OFF);				// タッチパネル用ブザーOFF
	BuzEmgOff();						// 警報用ブザーOFF
}

/*==========================================================================*/
/**
 * タッチパネル用ブザー設定(GDIC)
 *
 * @param  iScale 音階
 * @param  eCtrl BUZ_ON ブザーON/BUZ_OFF ブザーOFF
 * @retval E_OK 設定OK
 */
/*==========================================================================*/
ER BuzTplSet(int iScale, enum BUZ_CTRL eCtrl)
{
	ER ercd;
	
	ercd = twai_sem(s_idSem, (100/MSEC));
	if (ercd != E_OK) {
		return ercd;
	}
	buzTplSet(iScale, eCtrl);
		
	ercd = sig_sem(s_idSem);
	
	return ercd;
}

/*==========================================================================*/
/**
 * ブザー設定(PDIC)
 *
 * @param  iScale 音階
 * @param  eCtrl BUZ_ON ブザーON/BUZ_OFF ブザーOFF
 */
/*==========================================================================*/
static void buzTplSet(int iScale, enum BUZ_CTRL eCtrl)
{
	float fFreq;
	
	fFreq = 0.0;
	
	if (eCtrl == BUZ_ON) {
		// 設定周波数算出
		fFreq = freqCalc(iScale);
		BuzCycSet((UH)fFreq);			// 周波数設定
		BuzTplOn();						// ブザーON
	
	} else {
		// 停止設定
		BuzTplOff();					// ブザーOFF
	}
}

/**
 * 周波数の計算(デモ機のソースコードを流用)
 */
static float freqCalc(int iScale)
{
	float fFreq;
	
	fFreq = ldexpf(buzfrq[ iScale & 0xF ] , ((iScale >> 4) & 0xF) - 7); // The new period or frequency
		
	// 周波数範囲を一定の範囲でカット
	if (fFreq < FREQ_MIN) {
		fFreq = FREQ_MIN;
	}
	if (fFreq > FREQ_MAX) {
		fFreq = FREQ_MAX;
	}

	return fFreq;
}
