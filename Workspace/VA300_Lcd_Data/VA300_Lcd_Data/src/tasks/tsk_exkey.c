/**
*	VA-300プログラム
*
*	@file tsk_exkey.c
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/08/30
*	@brief  外部キー制御タスク
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "kernel.h"
#include "sh7750.h"
#include "id.h"
#include "drv_fpga.h"
#include "drv_7seg.h"
#include "err_ctrl.h"
#include "command.h"
#include "drv_exkey.h"

#include "va300.h"

#define	KEY_FIG		8					///< 桁数

// 変数定義
static ID s_idFlg;						///< フラグID
static ID s_idTsk;						///< タスクID
static B s_cId[ KEY_FIG + 1 ];			///< ID番号
static int s_iFig;						///< 入力桁数

// プロトタイプ宣言
static void idNumberClr(void);			///< ID番号をクリア
static void dispSegOff(void);			///< 7SEG表示OFF
static void dispSegNumber(B *p);		///< 7SEGに番号表示

// 
const T_CTSK ctsk_key = { TA_HLNG, NULL, ExKeyTask, 5, 2048, NULL, (B *)"ExKey task" };//

/*==========================================================================*/
/**
 * 外部キータスク初期化
 */
/*==========================================================================*/
ER ExKeyTaskInit(ID tskid)
{
	ER ercd;
	UW psw;
	
	// ID番号クリア
	idNumberClr();
	
	// タスクの生成
	if (tskid > 0) {
		ercd = cre_tsk(tskid, &ctsk_key);
		if (ercd == E_OK) {
			s_idTsk = tskid;
		}
	} else {
		ercd = acre_tsk(&ctsk_key);
		if (ercd > 0) {
			s_idTsk = ercd;
		}
	}
	if (ercd < 0) {
		return ercd;
	}
	
	// タスクの起動
	ercd = sta_tsk(s_idTsk, 0);
	
	// キーの初期化
	if (ercd == E_OK) {
		ercd = ExKeyInit(TRUE, s_idTsk);
	}
	return ercd;
}

/*==========================================================================*/
/**
 * 外部キー制御タスク
 */
/*==========================================================================*/
TASK ExKeyTask(void)
{
	ER		ercd;
	enum KEY_CODE eCode;
	
	// 処理開始
	for(;;) {
		ercd = ExtKeyGet(&eCode, TMO_FEVR);
		if (ercd == E_OK) {
			if ((eCode >= KEY_0 && eCode <= KEY_9) || (eCode == KEY_ASTERISK) || (eCode == KEY_UNDEF)) {
				if (s_iFig < KEY_FIG) {		// 桁数がいっぱいでないとき
					s_cId[ s_iFig ] = eCode;
					s_iFig++;				// 桁数インクリメント
				}
				dispSegNumber( s_cId );		// 7SEGに表示
			} else {
				switch (eCode) {
				case KEY_ENTER:
//					MdSendMsg(s_idTsk, MD_CMD_KEY_ON, 0, s_cId, s_iFig);	// ID番号の送信
					idNumberClr();
					dispSegOff();			// ７SEG表示を消灯
					break;
				case KEY_DEL:				// [取消]キー
					if (s_iFig) {			// 桁数デクリメント
						s_iFig--;
						s_cId[ s_iFig ] = 0;	// 1文字削除
					}
					dispSegNumber( s_cId );	// 7SEGに表示
					break;
//				case KEY_ASTERISK:			// 何もしない
//
//					break;
//				case KEY_UNDEF:				// 何もしない
//
//					break;
				default:					// 実装エラー
				
					break;
				}
			}
		} else {
			// ここにくるのは実装エラー
			PrgErrSet();
			slp_tsk();							// エラーのときはタスク終了
		}
	}
}

/*==========================================================================*/
/**
 * ID番号をクリア
 */
/*==========================================================================*/
static void idNumberClr(void)
{
	memset(s_cId, 0, (KEY_FIG + 1));
	s_iFig = 0;
}

/*==========================================================================*/
/**
 * 表示を消去
 */
/*==========================================================================*/
static void dispSegOff(void)
{
	const char seg[] = { 0, 0, 0, 0, 0, 0, 0, 0};
	
	dispSegNumber( seg );
}

/*==========================================================================*/
/**
 * 7SEG表示
 * @param p 表示データ
 */
/*==========================================================================*/
static void dispSegNumber(B *p)
{
	int i, iVal;
	
	for(i = 0;i < KEY_FIG;i++, p++) {
		iVal = 0;
		if (isdigit(*p)) {
			iVal = *p - '0';
		} else if (*p == KEY_ASTERISK) {	// テスト用に'A'表示
			iVal = 10;
		} else if (*p == KEY_UNDEF) {		// テスト用に'B'表示
			iVal = 11;
		} else if (*p == 0){
			iVal = SEG_PTN_OFF;
		}
		Disp7Seg((LED_7SEG_1 + i), iVal, CTRL7SEG_NORMAL);
	}
}
