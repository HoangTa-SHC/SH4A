/**
*	VA-300プログラム
*
*	@file drv_exkey.c
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/08/30
*	@brief  外部キー制御ドライバ
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
#define	_DRV_EXKEY_C_
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "kernel.h"
#include "sh7750.h"
#include "id.h"
#include "drv_fpga.h"
#include "drv_exkey.h"
#include "va300.h"


#define	EXKEY_IP	9					///< 外部キーの割込みレベル
#define	EXKEY_INT	INT_IRL9			///< 外部キーの割込み番号

#define	enable_key_int()	fpga_setw(INT_CRL, INT_CRL_KEY);	///< 割込み許可設定
#define	clear_key_int()		fpga_clrw(INT_CRL, INT_CRL_KEY);	///< 割込みフラグクリア
#define	ex_key_in()			fpga_inw(KEY_IN)					///< キー入力定義

// 変数定義
static ID s_idTsk;						///< タスクID

// プロトタイプ宣言
static INTHDR exKeyInt(void);			///< 10キー入力割込み
static void exkey_int(void);			///< 10キー割込み本体
static enum KEY_CODE key2Code(UH uhKey);///< キーコード
// 
const T_DINH dinh_key = { TA_HLNG, exKeyInt, EXKEY_IP};

/*==========================================================================*/
/**
 * 外部キー初期化
 * 
 * @param bIntEnable 割込み許可/不許可
 * @param idTsk 割込み時に起床するタスクID
 */
/*==========================================================================*/
ER ExKeyInit(BOOL bIntEnable, ID idTsk)
{
	ER ercd;
	UW psw;
	
	ercd = E_OK;
	
	// 割込み時の起動タスクを設定
	if (idTsk > 0) {
		s_idTsk = idTsk;
	} else {
		return E_PAR;					// パラメータエラー
	}
	
	if (bIntEnable == TRUE) {
		psw = vdis_psw();
	
		// ベクタ登録
		ercd = def_inh(EXKEY_INT, &dinh_key);		// 割込み設定
		if (ercd == E_OK) {
			// 割込み設定(ハードウェア側)
			enable_key_int();
		}
		vset_psw(psw);
	}
	
	return ercd;
}

#pragma interrupt(exKeyInt)
/*==========================================================================*/
/**
 * 割込みハンドラ
 *
 */
/*==========================================================================*/
static INTHDR exKeyInt(void)
{
	ent_int();
	exkey_int();
	ret_int();
}

/*==========================================================================*/
/**
 * 割込みハンドラ(本体)
 *
 */
/*==========================================================================*/
static void exkey_int(void)
{
	clear_key_int();							/* 割込み要因をクリア&不許可 */
	
	if (s_idTsk) {
		iwup_tsk(s_idTsk);
	}
	enable_key_int();							/* 割込み許可 */
}

/*==========================================================================*/
/**
 * キー入力取得待ち
 *
 *	@param peKey キーコード格納先アドレス
 *	@param tmout タイムアウト時間
 *	@return OSのエラーコード
 */
/*==========================================================================*/
ER ExtKeyGet(enum KEY_CODE *peKey, TMO tmout)
{
	ER ercd;

	ercd = E_OK;
	
	// キーを２回取得し、照合する
	if (tmout != TMO_POL) {
		
		ercd = tslp_tsk( tmout );
	}
	if (ercd == E_OK) {
		*peKey = ExtKeyPol();
	}
	return ercd;
}

/*==========================================================================*/
/**
 * キー入力を取得
 *
 *	@retval KEY_ENTER 呼出
 *	@retval KEY_DEL 取消
 *	@retval KEY_ASTERISK *
 *	@retval KEY_UNDEF 未定義
 *	@retval 0～9 値
 */
/*==========================================================================*/
enum KEY_CODE ExtKeyPol(void)
{
	enum KEY_CODE eKey;
	UH uhKey;

	uhKey = ex_key_in();				// キーの取り込み
	eKey = key2Code(uhKey);

	return eKey;
}

/*==========================================================================*/
/**
 * キー入力をコード変換
 *
 *	@param uhKey 押されたキー
 *	@retval KEY_ENTER 呼出
 *	@retval KEY_DEL 取消
 *	@retval KEY_ASTERISK *
 *	@retval KEY_UNDEF 未定義
 *	@retval 0～9 値
 */
/*==========================================================================*/
static enum KEY_CODE key2Code(UH uhKey)
{
	static const struct {
		UH uhInkey;						// 入力キー
		enum KEY_CODE eCode;			// キーコード
	} stCodeList[] = {
		{ KEY_IN_0, KEY_0},
		{ KEY_IN_1, KEY_1},
		{ KEY_IN_2, KEY_2},
		{ KEY_IN_3, KEY_3},
		{ KEY_IN_4, KEY_4},
		{ KEY_IN_5, KEY_5},
		{ KEY_IN_6, KEY_6},
		{ KEY_IN_7, KEY_7},
		{ KEY_IN_8, KEY_8},
		{ KEY_IN_9, KEY_9},
		{ KEY_IN_ENTER, KEY_ENTER},
		{ KEY_IN_ASTERISK, KEY_ASTERISK},
		{ KEY_IN_CLR,  KEY_DEL},
		{ KEY_IN_NONE, KEY_NONE},
		
	};
	static const int iCodeListCount = sizeof stCodeList / sizeof stCodeList[ 0 ];
	int i;
	
	// キーコード検索
	for (i = 0;i < iCodeListCount;i++) {
		if (uhKey == stCodeList[ i ].uhInkey) {
			return stCodeList[ i ].eCode;
		}
	}
	return KEY_UNDEF;
}

