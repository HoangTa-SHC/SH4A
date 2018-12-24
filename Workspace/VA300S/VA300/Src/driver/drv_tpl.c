//=============================================================================
/**
 *
 * VA-300プログラム
 * <<タッチパネル制御関連モジュール>>
 *
 *	@brief タッチパネル制御機能。デバイスはTSC2004()
 *	
 *	@file drv_tpl.c
 *	
 *	@date	2012/09/12
 *	@version 1.00 新規作成
 *	@author	F.Saeki
 *
 *	Copyright (C) 2012, OYO Electric Corporation
 */
//=============================================================================
/*
	<<割込みについて>>
	本ドライバではタッチパネルが押されたときの割込み時、
	初期化関数で設定したフラグIDとビットパターンを使用してフラグが
	セットされます。
	タスク側でwai_flg()で待って処理してください。
	
	<<補正値について>>
	初期化関数内で1倍に初期化されます。
	システム起動時にTplRevSet()関数で値を設定してください。
	補正値については補正値を求めるための調整モードを作成
	してください。
 */
#define	_DRV_TPL_C_
#include "kernel.h"
#include "sh7750.h"
#include "drv_tpl.h"

// マクロ定義
#define	TPL_IP			12				///< タッチパネル割込みレベル(12)
#define	TPL_INT			INT_IRL12		///< タッチパネル割込み番号
#define	TPL_COM_IP		7				///< タッチパネル通信割込みレベル(7)
#define	TPL_COM_INT		INT_IRL7		///< タッチパネル通信割込み番号

// プロトタイプ宣言
static INTHDR tplInt(void);				///< タッチパネル割込み
static void tpl_int(void);				///< タッチパネル割込み(本体)
static INTHDR tplComInt(void);			///< タッチパネル通信割込み
static ER tplPosGet(int *piPosX, int *piPosY);	///< 座標読込み

// 変数定義
static ID s_idTsk;						///< 待ちタスク
static ID s_idSem;						///< セマフォID
const T_DINH dinh_tpl     = { TA_HLNG, tplInt,   TPL_IP};		// 割込み定義
const T_DINH dinh_tpl_com = { TA_HLNG, tplComInt,TPL_COM_IP};	// 割込み定義
const T_CSEM csem_tpl     = { TA_TFIFO, 1, 1, (B *)"sem_tpl" };	// タッチパネルのセマフォ
static ST_TPL_REV s_stTplParam;			///< タッチパネル用補正値
static const ST_TPL_REV s_stTplParamDef = {	///< タッチパネル用補正値のデフォルト値
	1.0, 0.0, 0.0, 0.0, 1.0, 0.0
};

//=============================================================================
/**
 * タッチパネルコントローラ初期化
 *
 * @param idSem セマフォID
 * @retval E_OK 成功
 */
//=============================================================================
ER TplInit(ID idSem)
{
	ER ercd;
	UW psw;
	
	//
	// 変数初期化
	//
	s_idTsk = 0;
	TplRevInit();								// 補正値を初期化

	// セマフォIDを設定
	if (idSem) {
		ercd = cre_sem(idSem, &csem_tpl);		// セマフォ生成
		if (ercd == E_OK) {
			s_idSem = idSem;
		} else {
			return ercd;
		}
	} else {
		return E_PAR;
	}

	//
	// ポートの設定(FPGAなので不要と思われる)
	//
	
	
	//
	// 割込み設定
	//
	psw = vdis_psw();									// 割込み禁止
	
	if (ercd == E_OK) {
		ercd = def_inh(TPL_INT, &dinh_tpl);				// タッチパネルI/F割込み設定
	}
	if (ercd == E_OK) {
		ercd = def_inh(TPL_COM_INT, &dinh_tpl_com);		// タッチパネル通信I/F割込み設定
	}
	// 割込み許可
	if (ercd == E_OK) {
		enable_tpl_int();
		enable_tplcom_int();
	}
	vset_psw(psw);										// 割込み許可

	//
	// タッチパネルのデバイスをリセット
	//
	if (ercd == E_OK) {
		ercd = tpl_reset();
	}
	return ercd;
}

#pragma interrupt(tplInt)
//=============================================================================
/**
 * タッチパネル割込み
 */
//=============================================================================
static INTHDR tplInt(void)
{
	ent_int();
	tpl_int();						// タッチパネル割込み処理(本体)
	ret_int();
}

//=============================================================================
/**
 * タッチパネル割込み処理(本体)
 */
//=============================================================================
static void tpl_int(void)
{
	disable_tpl_int();				// タッチパネル割込みフラグ初期化
	if (s_idTsk) {
		iwup_tsk(s_idTsk);
	}
	enable_tpl_int();				// タッチパネル割込み許可
}

#pragma interrupt(tplComInt)
//=============================================================================
/**
 * タッチパネル通信割込み
 */
//=============================================================================
static INTHDR tplComInt(void)
{
	ent_int();
	tpl_com_int();					// タッチパネル通信割込み処理(本体)
	ret_int();
}

//=============================================================================
/**
 * 座標読出し
 *
 * @param piPosX X座標
 * @param piPosY Y座標
 * @param tmout タイムアウト時間
 */
//=============================================================================
ER TplPosGet(int *piPosX, int *piPosY, TMO tmout)
{
	ER ercd;
	
	ercd = twai_sem(s_idSem, (100/MSEC));		// セマフォ取得
	
	if (ercd == E_OK) {
		s_idTsk = vget_tid();					// 自タスクIDの取得
		vcan_wup();								// 起床要求のクリア
		ercd = tslp_tsk( tmout );
		s_idTsk = 0;
	} else {
		return ercd;
	}
	
	// ポーリング時でないときは正常なとき、ポーリング時のみタイムアウト許可
	if ((ercd == E_OK) || ((tmout == TMO_POL) && ercd == E_TMOUT)) {
		dly_tsk( 50/MSEC );					// 
		ercd = tplPosGet(piPosX, piPosY);		// 座標取得
	}
	
	sig_sem(s_idSem);							// セマフォ返却
	
	return ercd;
}


//=============================================================================
/**
 * 座標読出し(PDIC)
 *
 * @param piPosX X座標
 * @param piPosY Y座標
 */
//=============================================================================
static ER tplPosGet(int *piPosX, int *piPosY)
{
	ER ercd;
	int iPosX, iPosY;
	ST_TPL_REV *p;
	
	p = &s_stTplParam;
	
	*piPosX = 0;
	*piPosY = 0;
	
	ercd = tpl_pos_get(&iPosX, &iPosY);
	if (ercd == E_OK) {
		// 画面サイズに合わせて補正
		iPosX = (int)((float)iPosX * 489.0 / 1023.0);
		iPosY = (int)((float)iPosY * 271.0 / 1023.0);
		// 読み込まれた座標を補正
		*piPosX = (int)(iPosX * p->fXx + iPosY * p->fXy + p->fXofs);
		*piPosY = (int)(iPosX * p->fYx + iPosY * p->fYy + p->fYofs);
	}
	return ercd;
}

//=============================================================================
/**
 * 補正値の設定
 *
 *	補正後X = X * fXx + Y * fXy + fXofs
 *	補正後Y = Y * fYx + Y * fYy + fYofs
 * で計算される
 *
 * @param fXx   X座標用 X座標値にかける値
 * @param fXy   X座標用 Y座標値にかける値
 * @param fXofs X座標用 オフセット
 * @param fYx   Y座標用 X座標値にかける値
 * @param fYy   Y座標用 Y座標値にかける値
 * @param fYofs Y座標用 オフセット
 */
//=============================================================================
void TplRevSet(float fXx, float fXy, float fXofs, float fYx, float fYy, float fYofs)
{
	ST_TPL_REV *p;
	
	p = &s_stTplParam;
	p->fXx   = fXx;
	p->fXy   = fXy;
	p->fXofs = fXofs;
	p->fYx   = fYx;
	p->fYy   = fYy;
	p->fYofs = fYofs;
}

//=============================================================================
/**
 * 設定されている補正値の取得
 *
 * @param pfXx   X座標用 X座標値にかける値
 * @param pfXy   X座標用 Y座標値にかける値
 * @param pfXofs X座標用 オフセット
 * @param pfYx   Y座標用 X座標値にかける値
 * @param pfYy   Y座標用 Y座標値にかける値
 * @param pfYofs Y座標用 オフセット
 */
//=============================================================================
void TplRevGet(float *pfXx, float *pfXy, float *pfXofs, float *pfYx, float *pfYy, float *pfYofs)
{
	ST_TPL_REV *p;
	
	p = &s_stTplParam;
	*pfXx   = p->fXx;
	*pfXy   = p->fXy;
	*pfXofs = p->fXofs;
	*pfYx   = p->fYx;
	*pfYy   = p->fYy;
	*pfYofs = p->fYofs;
}

//=============================================================================
/**
 * 補正値の初期化
 */
//=============================================================================
void TplRevInit(void)
{
	s_stTplParam = s_stTplParamDef;
}

//=============================================================================
/**
 * 補正値の計算
 *
 * @param pBase  調整用の座標位置
 * @param pInput タッチパネル上のポジション
 * @param pRev   格納先補正値構造体へのポインタ
 */
//=============================================================================
void TplRevCalc(ST_POS_REV *pBase, ST_POS_REV *pInput, ST_TPL_REV *pRev)
{
//	float lcd_x1 = 10.0;	// タッチパネル調整用の座標位置X1
//	float lcd_y1 = 10.0;	// タッチパネル調整用の座標位置Y1
//	float lcd_x2 = 300.0;	// タッチパネル調整用の座標位置X2
//	float lcd_y2 = 50.0;	// タッチパネル調整用の座標位置Y2
//	float lcd_x3 = 450.0;	// タッチパネル調整用の座標位置X3
//	float lcd_y3 = 260.0;	// タッチパネル調整用の座標位置Y3
	float lcd_x1;						// タッチパネル調整用の座標位置X1
	float lcd_y1;						// タッチパネル調整用の座標位置Y1
	float lcd_x2;						// タッチパネル調整用の座標位置X2
	float lcd_y2;						// タッチパネル調整用の座標位置Y2
	float lcd_x3;						// タッチパネル調整用の座標位置X3
	float lcd_y3;						// タッチパネル調整用の座標位置Y3
	float aa;
	int tp_x1, tp_x2, tp_x3;
	int tp_y1, tp_y2, tp_y3;

	tp_x1 = pInput->tPos[ 0 ].iX;
	tp_x2 = pInput->tPos[ 1 ].iX;
	tp_x3 = pInput->tPos[ 2 ].iX;
	tp_y1 = pInput->tPos[ 0 ].iY;
	tp_y2 = pInput->tPos[ 1 ].iY;
	tp_y3 = pInput->tPos[ 2 ].iY;
	lcd_x1 = (float)pBase->tPos[ 0 ].iX;
	lcd_x2 = (float)pBase->tPos[ 1 ].iX;
	lcd_x3 = (float)pBase->tPos[ 2 ].iX;
	lcd_y1 = (float)pBase->tPos[ 0 ].iY;
	lcd_y2 = (float)pBase->tPos[ 1 ].iY;
	lcd_y3 = (float)pBase->tPos[ 2 ].iY;
	
	aa = (float)tp_x1*(tp_y2 - tp_y3) - (float)tp_x2*(tp_y1 - tp_y3) + (float)tp_x3*(tp_y1 - tp_y2);
	
	pRev->fXx   = ( (tp_y2 - tp_y3)*lcd_x1 + -(tp_y1 - tp_y3)*lcd_x2 + (tp_y1 - tp_y2)*lcd_x3 ) / aa;
	pRev->fXy   = ( -(tp_x2 - tp_x3)*lcd_x1 + (tp_x1 - tp_x3)*lcd_x2 + -(tp_x1 - tp_x2)*lcd_x3 )/ aa;
	pRev->fXofs = ( (tp_x2*tp_y3 - tp_y2*tp_x3)*lcd_x1 + -(tp_x1*tp_y3 - tp_y1*tp_x3)*lcd_x2 + (tp_x1*tp_y2 - tp_y1*tp_x2)*lcd_x3 )/ aa;
	
	pRev->fYx   = ( (tp_y2 - tp_y3)*lcd_y1 + -(tp_y1 - tp_y3)*lcd_y2 + (tp_y1 - tp_y2)*lcd_y3 )/ aa;
	pRev->fYy   = ( -(tp_x2 - tp_x3)*lcd_y1 + (tp_x1 - tp_x3)*lcd_y2 + -(tp_x1 - tp_x2)*lcd_y3 )/ aa;
	pRev->fYofs = ( (tp_x2*tp_y3 - tp_y2*tp_x3)*lcd_y1 + -(tp_x1*tp_y3- tp_y1*tp_x3)*lcd_y2 + (tp_x1*tp_y2 - tp_y1*tp_x2)*lcd_y3 )/ aa;
}

