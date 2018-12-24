//=============================================================================
/**
 *
 * VA-300プログラム
 * <<生態検知センサ制御関連モジュール>>
 *
 *	@brief 生態検知センサ制御機能。
 *	
 *	@file drv_ts.c
 *	
 *	@date	2012/09/26
 *	@version 1.00 新規作成
 *	@author	F.Saeki
 *
 *	Copyright (C) 2012, OYO Electric Corporation
 */
//=============================================================================
#define	_DRV_S_C_
#include "kernel.h"
#include "sh7750.h"
#include "drv_fpga.h"

// マクロ定義
#define	TS_IP			13				///< タッチパネル割込みレベル(13)
#define	TS_INT			INT_IRL13		///< タッチパネル割込み番号
#define	enable_ts_int()	fpga_setw(INT_CRL, (INT_CRL_SENS));	///< タッチパネル割込み許可
#define	clear_ts_int()	fpga_clrw(INT_CRL, (INT_CRL_SENS));	///< タッチパネル割込みクリア

// プロトタイプ宣言
static INTHDR tsInt(void);				///< 生態検知センサ割込み
static void ts_int(void);				///< 生態検知センサ割込み(本体)


// 変数定義
static ID s_idTsk;						///< 待ちタスク
const T_DINH dinh_ts = { TA_HLNG, tsInt,   TS_IP};		// 割込み定義

//=============================================================================
/**
 * 生態検知センサ初期化
 *
 * @param idTsk 割込み時に起床するタスク
 * @retval E_OK 成功
 */
//=============================================================================
ER TsInit(ID idTsk)
{
	ER ercd;
	UW psw;
	
	// タスクIDを設定
	if (idTsk) {
		s_idTsk = idTsk;
	} else {
		return E_PAR;
	}

	//
	// ポートの設定(FPGAなので不要と思われる)
	//
	
	
	//
	// 割込み設定
	//
	psw = vdis_psw();
	
	ercd = def_inh(TS_INT, &dinh_ts);				// 生態検知センサ割込み設定

	// 割込み許可
	if (ercd == E_OK) {
		enable_ts_int();
	}
	vset_psw(psw);

	return ercd;
}

#pragma interrupt(tsInt)
//=============================================================================
/**
 * 生態検知センサ割込み
 */
//=============================================================================
static INTHDR tsInt(void)
{
	ent_int();
	ts_int();						// 生態検知センサ割込み処理(本体)
	ret_int();
}

//=============================================================================
/**
 * 生態検知センサ割込み(本体)
 */
//=============================================================================
static void ts_int(void)
{
	clear_ts_int();					// タッチセンサ割込みクリア&不許可
	if (s_idTsk) {
		iwup_tsk( s_idTsk );
	}
// By T.Nagai	enable_ts_int();				// タッチセンサ割込み許可
}

