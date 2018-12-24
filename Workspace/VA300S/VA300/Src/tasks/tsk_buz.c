/**
*	VA-300プログラム
*
*	@file tsk_buz.c
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/07/10
*	@brief  ブザー制御タスク
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
#include <stdio.h>
#include <string.h>

#include "kernel.h"
#include "sh7750.h"
#include "sh7750R.h"
#include "id.h"
#include "command.h"
#include "drv_fpga.h"
#include "err_ctrl.h"

#include "va300.h"
#include "drv_buz.h"

// マクロ定義
#ifndef	CLK
#define	CLK		50000000
#endif

#ifndef	CH
#define	CH		2
#endif

#if (CH==1)
#define	TMU_TCOR	TMU_TCOR1
#define	TMU_TCNT	TMU_TCNT1
#define	TMU_TCR		TMU_TCR1
#define	INT_TUNI	INT_TUNI1
#else
#define	TMU_TCOR	TMU_TCOR2
#define	TMU_TCNT	TMU_TCNT2
#define	TMU_TCR		TMU_TCR2
#define	INT_TUNI	INT_TUNI2
#endif

#define	INT_LVL		10					///< 割込みレベル
#define	TCR_UNF		0x0100

// 変数定義
static ID s_idFlg;						///< フラグID
static ID s_idTsk;						///< タスクID
static UW s_uwCount;					///< カウント用

// プロトタイプ宣言
ER SoundInit(ID tskid, ID flgid);		// 初期化
static ER tmInit(void);					///< タイマー初期化
static INTHDR tmInt(void);				///< タイマー割込み
static void tm_int(void);				///< タイマー割込み本体

// 初期設定用
const T_CTSK ctsk_buz = { TA_HLNG, NULL, SoundTask, 5, 2048, NULL, (B *)"Sound task" };//
const T_CFLG cflg_buz = { TA_WMUL, 0, (B *)"buz_flag" };
const T_DINH dinh_tim = { TA_HLNG, tmInt, INT_LVL};
const T_CSEM csem_tim   = { TA_TFIFO, 1, 1, (B *)"sem_tim" };

/*==========================================================================*/
/**
 * サウンドタスク初期化
 * 
 * @param tskid タスクID
 * @param flgid フラグID
 */
/*==========================================================================*/
ER SoundInit(ID tskid, ID flgid)
{
	ER ercd;
	
	// タスクの生成
	if (tskid > 0) {
		ercd = cre_tsk(tskid, &ctsk_buz);
		if (ercd == E_OK) {
			s_idTsk = tskid;
		}
	} else {
		ercd = acre_tsk(&ctsk_buz);
		if (ercd > 0) {
			s_idTsk = ercd;
		}
	}
	if (ercd < 0) {
		return ercd;
	}
	
	// フラグの生成
	if (flgid > 0) {
		ercd = cre_flg(flgid, &cflg_buz);
		if (ercd == E_OK) {
			s_idFlg = flgid;
		}
	} else {
		ercd = acre_flg(&cflg_buz);
		if (ercd > 0) {
			s_idFlg = ercd;
		}
	}
	if (ercd < 0) {
		return ercd;
	}
	
	// タイマ設定
	ercd = tmInit();
	
	if (ercd == E_OK) {
		ercd = sta_tsk(s_idTsk, 0);
	}
	return ercd;
}

/*==========================================================================*/
/**
 * タイマ初期化
 */
/*==========================================================================*/
static ER tmInit(void)
{
	ER ercd;
	unsigned long  tc;
	unsigned short tpsc;
	UW psw;
	const UH uhUsec = 62500;					// 62.5ms固定
	
	psw = vdis_psw();
	 
	// ベクタ登録
	ercd = def_inh(INT_TUNI, &dinh_tim);		// タイマ割込み設定
	if (ercd == E_OK) {
		
		// 時定数を計算
		if (((CLK) / (256 * 1000 * 1000)) * uhUsec >= 0x0fffffff) {
			tc = ((CLK) / (256 * 1000 * 1000)) * uhUsec;
			tpsc = 3;
		} else if (((CLK) / (64 * 1000 * 1000)) * uhUsec >= 0x0fffffff) {
			tc = ((CLK) / (64 * 1000 * 1000)) * uhUsec;
			tpsc = 2;
		} else if (((CLK) / (16 * 1000 * 1000)) * uhUsec >= 0x0fffffff) {
			tc = ((CLK) / (16 * 1000 * 1000)) * uhUsec;
			tpsc = 1;
		} else {
			tc = ((CLK) / (4 * 1000 * 1000)) * uhUsec;
			tpsc = 0;
		}

		// タイマユニット初期化
		sfrr_clr(TMU_TSTR, 0x01 << CH);		// タイマ停止
		sfrr_outl(TMU_TCOR, (tc-1));		// タイマコンスタント設定
		sfrr_outl(TMU_TCNT, (tc-1));		// タイマカウンタ初期値
		sfrr_outw(TMU_TCR, (tpsc | 0x20));	// タイマプリスケーラ選択、アンダーフロー割り込みを許可

#if (CH == 1)
		sfr_outw(INTC_IPRA, sfr_inw(INTC_IPRA) | (INT_LVL <<  8));
#elif (CH == 2)
		sfr_outw(INTC_IPRA, sfr_inw(INTC_IPRA) | (INT_LVL <<  4));
#elif (CH == 3)
    	sfrr_outw(INTC_INTPRI00, sfr_inw(INTC_INTPRI00) | (INT_LVL <<  8));
#elif (CH == 4)
    	sfrr_outw(INTC_INTPRI00, sfr_inw(INTC_INTPRI00) | (INT_LVL <<  12));
#else

#endif
	
		ercd = cre_sem(SEM_TIM, &csem_tim);	// セマフォ作成
	}
	// 変数を初期化
	s_uwCount = 0;

	vset_psw(psw);
	
	return ercd;
}

/*==========================================================================*/
/**
 * サウンド制御タスク
 */
/*==========================================================================*/
TASK SoundTask(void)
{
	ER		ercd;
	FLGPTN	flgptn;

	BuzTplSet(0x17, BUZ_OFF);

	// 処理開始
	for(;;) {
		ercd = wai_flg(s_idFlg, (FPTN_EMG_ON | FPTN_EMG_OFF | FPTN_TPL_OK | FPTN_TPL_NG), TWF_ORW, &flgptn);
		if (ercd == E_OK) {
			//--- 開始 ---
			if (flgptn & FPTN_EMG_ON) {
				BuzEmgOn();
				// フラグのクリア
				clr_flg(s_idFlg, ~FPTN_EMG_ON);
			}
			//--- 終了 ---
			if (flgptn & FPTN_EMG_OFF) {
				BuzEmgOff();
				// フラグのクリア
				clr_flg(s_idFlg, ~FPTN_EMG_OFF);
			}
			//--- タッチパネル用OKブザー ---
			if (flgptn & FPTN_TPL_OK) {
				BuzTplOn();
				dly_tsk(50 / MSEC);
				BuzTplOff();
				// フラグのクリア
				clr_flg(s_idFlg, ~FPTN_TPL_OK);
			}
			//--- タッチパネル用NGブザー ---
			if (flgptn & FPTN_TPL_NG) {
				BuzTplOn();
				dly_tsk(50 / MSEC);
				BuzTplOff();
				dly_tsk(50 / MSEC);
				BuzTplOn();
				dly_tsk(50 / MSEC);
				BuzTplOff();
				// フラグのクリア
				clr_flg(s_idFlg, ~FPTN_TPL_NG);
			}
			//--- タッチパネル用ブザーOFF ---
			if (flgptn & FPTN_TPL_OFF) {
				BuzTplOff();
				// フラグのクリア
				clr_flg(s_idFlg, ~FPTN_TPL_NG);
			}			
		} else {
			// ここにくるのは実装エラー
			PrgErrSet();
			slp_tsk();							// エラーのときはタスク終了
		}
	}
}

#pragma interrupt(tmInt)
/*==========================================================================*/
/**
 * タイマ割込みハンドラ
 *
 */
/*==========================================================================*/
static INTHDR tmInt(void)
{
	ent_int();
	tm_int();
	ret_int();
}

/*==========================================================================*/
/**
 * タイマ割込みハンドラ(本体)
 *
 */
/*==========================================================================*/
static void tm_int(void)
{
	// 割込みクリア
	sfrr_clrw(TMU_TCR, TCR_UNF);		// アンダーフローフラグクリア
	
	if (s_uwCount) {
		s_uwCount--;
	} else {
		// タイマ停止
#if (CH < 3)
		sfr_clr(TMU_TSTR, 0x01 << CH);	// カウント動作停止
#else
		sfrr_clr(TMU_TSTR, 0x01 << (CH - 3));	// カウント動作停止
#endif
		if (s_idFlg) {
			iset_flg(s_idFlg, FPTN_TPL_OFF);	// タッチパネル用ブザーOFF
		}
	}
}

/*==========================================================================*/
/**
 * サウンド制御
 *
 * @param eCtrl 制御内容
 */
/*==========================================================================*/
void SoundCtrl(enum SOUND_CTRL eCtrl)
{
	switch (eCtrl) {
	case SOUND_CTRL_EMG_ON:
		set_flg(s_idFlg, FPTN_EMG_ON);
		break;
	case SOUND_CTRL_EMG_OFF:
		set_flg(s_idFlg, FPTN_EMG_OFF);
		break;
	case SOUND_CTRL_TPL_OK:
		set_flg(s_idFlg, FPTN_TPL_OK);
		break;
	case SOUND_CTRL_TPL_NG:
		set_flg(s_idFlg, FPTN_TPL_NG);
		break;
	}
}
