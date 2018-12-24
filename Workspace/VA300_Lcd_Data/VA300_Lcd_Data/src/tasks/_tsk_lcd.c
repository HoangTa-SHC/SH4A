/**
*	VA-300テストプログラム
*
*	@file tsk_lcd.c
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/09/28
*	@brief  LCD表示タスク
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
#include "err_ctrl.h"

#include "va300.h"

//#define LCDPROCON	//LCD制御用宣言　LCD制御のテストをしない場合は、本宣言をコメントアウトしてください
#define LCDX	480
#define LCDY	272

// 変数定義
static ID s_idTsk;

// プロトタイプ宣言
static TASK LcdTask( void );		///< LCD表示タスク

// タスクの定義
const T_CTSK ctsk_lcd = { TA_HLNG, NULL, LcdTask, 7, 2048, NULL, (B *)"lcd task" };//

UB WaitKeyProc(int sousa, int tflg, UB *msg);
void SetGamen01(int buf, UB perm, UB perm_r, UB perm_g, UB perm_b, UH stx, UH sty, UH edx, UH edy);
void SetGamen02(int buf, UB perm, UB perm_r, UB perm_g, UB perm_b, UH stx, UH sty, UH edx, UH edy);
UH GetPixDataFromGamen01(int buf, unsigned long cnt);
UH GetPixDataFromGamen02(int buf, unsigned long cnt);
UINT DbgSendMessageToMain(int num, int sousa, UB *msg);

//extern const unsigned short LcdGmn01[];
//extern const unsigned short LcdGmn02[];
extern const unsigned short btn_NEXT[];
extern const unsigned short dbgmn001[];
extern const unsigned short dbgmn002[];
extern const unsigned short dbgmn003[];
extern const unsigned short dbgmn004[];
extern const unsigned short dbgmn005[];
extern const unsigned short dbgmn006[];
extern const unsigned short dbgmn007[];
extern const unsigned short dbgmn008[];
extern const unsigned short dbgmn009[];
extern const unsigned short dbgmn010[];
extern const unsigned short dbgmn011[];
extern const unsigned short dbgmn012[];
extern const unsigned short dbgmn101[];
extern const unsigned short dbgmn102[];
extern const unsigned short dbgmn103[];
extern const unsigned short dbgmn104[];
extern const unsigned short dbgmn121[];
extern const unsigned short dbgmn122[];
extern const unsigned short dbgmn123[];
extern const unsigned short dbgmn124[];
extern const unsigned short dbgmn125[];
extern const unsigned short dbgmn126[];
extern const unsigned short dbgmn127[];
extern const unsigned short dbgmn128[];
extern const unsigned short dbgmn129[];
extern const unsigned short dbgmn130[];
extern const unsigned short dbgmn131[];
extern const unsigned short dbgmn132[];
extern const unsigned short dbgmn141[];
extern const unsigned short dbgmn142[];
extern const unsigned short dbgmn143[];
extern const unsigned short dbgmn144[];
extern const unsigned short dbgmn145[];
extern const unsigned short dbgmn146[];
extern const unsigned short dbgmn201[];
extern const unsigned short dbgmn202[];
extern const unsigned short dbgmn203[];
extern const unsigned short dbgmn204[];

//extern const UINT g_LcdMsgSize;
//extern const UB g_LcdMsgBuff[1024];
extern struct{
	unsigned int LcdMsgSize;
	UB LcdMsgBuff[1024];
}g_LcdmsgData;


/*==========================================================================*/
/**
 * LCD表示タスク初期化
 *
 * @param idTsk タスクID
 * @retval E_OK 正常起動
 */
/*==========================================================================*/
ER LcdTaskInit(ID idTsk)
{
	ER ercd;
	
	// タスクの生成
	if (idTsk > 0) {
		ercd = cre_tsk(idTsk, &ctsk_lcd);
		if (ercd == E_OK) {
			s_idTsk = idTsk;
		}
	} else {
		ercd = acre_tsk(&ctsk_lcd);
		if (ercd > 0) {
			s_idTsk = ercd;
		}
	}
	if (ercd < 0) {
		return ercd;
	}
	
	// タスクの起動
	ercd = sta_tsk(s_idTsk, 0);
	
	return ercd;
}

/*==========================================================================*/
/**
 * LCD表示タスク
 * 
 */
/*==========================================================================*/
static TASK LcdTask( void )
{
	FLGPTN	flgptn;
	ER		ercd;
	UB	sts_key;
	UB	buf_num, perm_enb, perm_r, perm_g, perm_b;
	UH	st_x, st_y, ed_x, ed_y;
	
	int cnt, num;
	int posx, posy;
	
	/** メッセージ・バッファ用の宣言  **/
	UB *msg; 		// <-- サイズは120くらいまでの範囲で適宜変えて下さい。
	UINT MsgSize;


	// LCDの機器Initialize は、ここに記入。
	buf_num = 0;	//画面バッファ(0～15)
	perm_enb = 0;	//透過色設定 0:未使用 1:使用
	perm_r = 0;		//R透過色
	perm_g = 0;		//G透過色
	perm_b = 0;		//B透過色
	st_x = 0;
	st_y = 0;
	ed_x = 479;
	ed_y = 271;
	
	msg = &g_LcdmsgData.LcdMsgBuff[0];
	
	// 通常処理開始
	for(;;) {
		// メインタスクからのの受信待ち
		ercd = wai_flg( ID_FLG_LCD, ( FPTN_LCD_INIT				// LCD初期画面表示要求(メイン→LCD、ノーマルモード移行の時)
									| FPTN_LCD_SCREEN1			// 画面１表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN2			// 画面２表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN3			// 画面３表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN4			// 画面４表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN5			// 画面５表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN6			// 画面６表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN7			// 画面７表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN8			// 画面８表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN9			// 画面９表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN10			// 画面１０表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN11			// 画面１１表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN12			// 画面１２表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN100 		// 画面１００表示要求(メイン→LCD)　通常モード
									| FPTN_LCD_SCREEN101 		// 画面１０１表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN102 		// 画面１０２表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN103 		// 画面１０３表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN104 		// 画面１０４表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN120		// 画面１２０表示要求(メイン→LCD)　通常モード（登録）
									| FPTN_LCD_SCREEN121		// 画面１２１表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN122		// 画面１２２表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN123		// 画面１２３表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN124		// 画面１２４表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN125		// 画面１２５表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN126		// 画面１２６表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN127		// 画面１２７表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN128		// 画面１２８表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN129		// 画面１２９表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN130		// 画面１３０表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN131		// 画面１３１表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN132		// 画面１３２表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN140		// 画面１４０表示要求(メイン→LCD)　通常モード（削除）
									| FPTN_LCD_SCREEN141		// 画面１４１表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN142		// 画面１４２表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN143		// 画面１４３表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN144		// 画面１４４表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN145		// 画面１４５表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN146		// 画面１４６表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN200		// 画面２００表示要求(メイン→LCD)　メンテナンス・モード
									| FPTN_LCD_SCREEN201		// 画面２０１表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN202		// 画面２０２表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN203		// 画面２０３表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN204		// 画面２０４表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN205 ), TWF_ORW, &flgptn );	// 画面２０５表示要求(メイン→LCD)
		if ( ercd != E_OK ){
			break;
		}
														
		// 以下に、Switch文などで受信内容に沿った処理を記述
		// この時、処理内容の最初に、
		//		ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_INIT );		// フラグのクリア
		// 		if ( ercd != E_OK ){
		//			break;
		//		}
		//  などで、受信したフラグ毎に、クリア処理を行う。
		//  wai_flg() で使用するフラグパターンは、id.h　を参照のこと。
		// 

		memset( msg, 0x20, sizeof(msg) );

		switch(flgptn)
		{
// イニシャライズ ------------------------------------------------------------------------------------//
			case FPTN_LCD_INIT:
				num = 0;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_INIT );
				if ( ercd != E_OK ){
					break;
				}

				LcdcBackLightOff();
				for(cnt = 0 ; cnt < 2 ; cnt++){	//画面を画面バッファに転送する
					SetGamen01(cnt, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				}
				SetGamen02(0, perm_enb, perm_r, perm_g, perm_b, 208, 104, 271, 167);
				LcdcDisplayModeSet(0, 0);
				LcdcBackLightOn();
				break;
// 初期モード ------------------------------------------------------------------------------------//
			case FPTN_LCD_SCREEN1:
				num = 1;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN1 );
				if ( ercd != E_OK ){
					break;
				}

				//////// 画面表示 ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);

				///////// キー入力待ち ////////
				sts_key = WaitKeyProc(99, 0, msg);
				if(sts_key == 1){	//キー押下あり

					//////// メッセージ送信 ////////
					g_LcdmsgData.LcdMsgSize = DbgSendMessageToMain(num, 0, msg);
					//g_LcdmsgData.LcdMsgSize = MsgSize;
					//ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
					//if(ercd != E_OK){
					//	//エラ－処理要
					//}					
					///////// フラグセット ////////
					ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
					if(ercd != E_OK){
						//エラ－処理要
					}
				}
				break;
			case FPTN_LCD_SCREEN2:
				num = 2;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN2 );
				if ( ercd != E_OK ){
					break;
				}


				//////// 画面表示 ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				///////// キー入力待ち ////////
				sts_key = WaitKeyProc(99, 0, msg);
				if(sts_key == 1){	//キー押下あり

					//////// メッセージ送信 ////////
					g_LcdmsgData.LcdMsgSize = DbgSendMessageToMain(num, 0, msg);
					//ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
					//if(ercd != E_OK){
						//エラ－処理要
					//}					
					///////// フラグセット ////////
					ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
					if(ercd != E_OK){
						//エラ－処理要
					}
				}
				break;
			case FPTN_LCD_SCREEN3:
				num = 3;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN3 );
				if ( ercd != E_OK ){
					break;
				}

				//////// 画面表示 ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				///////// キー入力待ち ////////
				sts_key = WaitKeyProc(99, 0, msg);
				if(sts_key == 1){	//キー押下あり

					//////// メッセージ送信 ////////
					g_LcdmsgData.LcdMsgSize = DbgSendMessageToMain(num, 0, msg);
					//ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
					//if(ercd != E_OK){
						//エラ－処理要
					//}					
					///////// フラグセット ////////
					ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
					if(ercd != E_OK){
						//エラ－処理要
					}
				}
				break;
			case FPTN_LCD_SCREEN4:
				num = 4;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN4 );
				if ( ercd != E_OK ){
					break;
				}

				//////// 画面表示 ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				///////// キー入力待ち ////////
				sts_key = WaitKeyProc(99, 0, msg);
				if(sts_key == 1){	//キー押下あり

					//////// メッセージ送信 ////////
					g_LcdmsgData.LcdMsgSize = DbgSendMessageToMain(num, 0, msg);
					//ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
					//if(ercd != E_OK){
						//エラ－処理要
					//}					
					///////// フラグセット ////////
					ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
					if(ercd != E_OK){
						//エラ－処理要
					}
				}
				break;
			case FPTN_LCD_SCREEN5:
				num = 5;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN5 );
				if ( ercd != E_OK ){
					break;
				}

				//////// 画面表示 ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				///////// キー入力待ち ////////
				sts_key = WaitKeyProc(99, 0, msg);
				if(sts_key == 1){	//キー押下あり

					//////// メッセージ送信 ////////
					g_LcdmsgData.LcdMsgSize = DbgSendMessageToMain(num, 0, msg);
					//ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
					//if(ercd != E_OK){
						//エラ－処理要
					//}
					///////// フラグセット ////////
					ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
					if(ercd != E_OK){
						//エラ－処理要
					}
				}				
				break;
			case FPTN_LCD_SCREEN6:
				num = 6;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN6 );
				if ( ercd != E_OK ){
					break;
				}

				//////// 画面表示 ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				break;
			case FPTN_LCD_SCREEN7:
				num = 7;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN7 );
				if ( ercd != E_OK ){
					break;
				}

				//////// 画面表示 ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				break;
			case FPTN_LCD_SCREEN8:
				num = 8;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN8 );
				if ( ercd != E_OK ){
					break;
				}

				//////// 画面表示 ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				break;
			case FPTN_LCD_SCREEN9:
				num = 9;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN9 );
				if ( ercd != E_OK ){
					break;
				}

				//////// 画面表示 ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);

				dly_tsk((3000/MSEC));	//タイマー3sec

				//////// メッセージ送信 ////////
				g_LcdmsgData.LcdMsgSize = DbgSendMessageToMain(num, 0, msg);
				//ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
				//if(ercd != E_OK){
					//エラ－処理要
				//}					

				///////// フラグセット ////////
				ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
				if(ercd != E_OK){
					//エラ－処理要
				}
				break;
			case FPTN_LCD_SCREEN10:
				num = 10;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN10 );
				if ( ercd != E_OK ){
					break;
				}
			
				//////// 画面表示 ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);

				dly_tsk((3000/MSEC));	//タイマー3sec

				//////// メッセージ送信 ////////
				g_LcdmsgData.LcdMsgSize = DbgSendMessageToMain(num, 0, msg);
				//ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
				//if(ercd != E_OK){
					//エラ－処理要
				//}					

				///////// フラグセット ////////
				ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
				if(ercd != E_OK){
					//エラ－処理要
				}
				break;
			case FPTN_LCD_SCREEN11:
				num = 11;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN11 );
				if ( ercd != E_OK ){
					break;
				}

				//////// 画面表示 ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				///////// キー入力待ち ////////
				sts_key = WaitKeyProc(99, 0, msg);
				if(sts_key == 1){	//キー押下あり
					//////// メッセージ送信 ////////
					g_LcdmsgData.LcdMsgSize = DbgSendMessageToMain(num, 0, msg);
					//ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
					//if(ercd != E_OK){
						//エラ－処理要
					//}	

					///////// フラグセット ////////
					ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
					if(ercd != E_OK){
						//エラ－処理要
					}

					//////// メッセージ送信 ////////
					//MsgSize = DbgSendMessageToMain(num, 0, msg);
					//ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
					//if(ercd != E_OK){
						//エラ－処理要
					//}	
				}				
				break;
			case FPTN_LCD_SCREEN12:
				num = 12;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN12 );
				if ( ercd != E_OK ){
					break;
				}

				//////// 画面表示 ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				///////// キー入力待ち ////////
				sts_key = WaitKeyProc(99, 0, msg);
				if(sts_key == 1){	//キー押下あり
					///////// フラグセット ////////
					ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
					if(ercd != E_OK){
						//エラ－処理要
					}

					//////// メッセージ送信 ////////
					MsgSize = DbgSendMessageToMain(num, 0, msg);
					ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
					if(ercd != E_OK){
						//エラ－処理要
					}
				}					
				break;
// 通常モード ------------------------------------------------------------------------------------//
			case FPTN_LCD_SCREEN100:
				num = 100;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN100 );
				if ( ercd != E_OK ){
					break;
				}

				//////// 画面表示 ////////
				SetGamen01(1, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				LcdcDisplayModeSet(1, 0);
				
				//////// 画面バッファに転送 ////////
				for(cnt = 0 ; cnt < 2 ; cnt++){	//画面を画面バッファに転送する
					SetGamen01(cnt, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				}
				SetGamen02(0, perm_enb, perm_r, perm_g, perm_b, 208, 104, 271, 167);	//NEXTボタン

				///////// フラグセット ////////
				ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
				if(ercd != E_OK){
					//エラ－処理要
				}

				//////// メッセージ送信 ////////
				MsgSize = DbgSendMessageToMain(num, 0, msg);
				ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
				if(ercd != E_OK){
					//エラ－処理要
				}
				break;
			case FPTN_LCD_SCREEN101:
				num = 101;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN101 );
				if ( ercd != E_OK ){
					break;
				}

				//////// 画面表示 ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				///////// キー入力待ち ////////
				sts_key = WaitKeyProc(99, 1, msg);
				if(sts_key == 1){	//キー押下あり
					///////// フラグセット ////////
					ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
					if(ercd != E_OK){
						//エラ－処理要
					}

					//////// メッセージ送信 ////////
					MsgSize = DbgSendMessageToMain(num, 0, msg);
					ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
					if(ercd != E_OK){
						//エラ－処理要
					}
				}					
				break;
			case FPTN_LCD_SCREEN102:
				num = 101;	//画面Sleep状態 (画面101の処理をする)
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN102 );
				if ( ercd != E_OK ){
					break;
				}

				//////// 画面表示 ////////
				LcdcBackLightOff();	//画面Sleep状態
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				///////// キー入力待ち ////////
				sts_key = WaitKeyProc(99, 2, msg);	//画面Sleep状態
				if(sts_key == 1){	//キー押下あり
					///////// フラグセット ////////
					ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
					if(ercd != E_OK){
						//エラ－処理要
					}

					//////// メッセージ送信 ////////
					MsgSize = DbgSendMessageToMain(num, 0, msg);
					ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
					if(ercd != E_OK){
						//エラ－処理要
					}
				}					
				break;
			case FPTN_LCD_SCREEN103:
				num = 103;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN103 );
				if ( ercd != E_OK ){
					break;
				}

				//////// 画面表示 ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);

				dly_tsk((3000/MSEC));	//タイマー3sec

				///////// フラグセット ////////
				ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
				if(ercd != E_OK){
					//エラ－処理要
				}

				//////// メッセージ送信 ////////
				MsgSize = DbgSendMessageToMain(num, 0, msg);
				ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
				if(ercd != E_OK){
					//エラ－処理要
				}					
				break;
			case FPTN_LCD_SCREEN104:
				num = 104;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN104 );
				if ( ercd != E_OK ){
					break;
				}
			
				//////// 画面表示 ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);

				dly_tsk((3000/MSEC));	//タイマー3sec

				///////// フラグセット ////////
				ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
				if(ercd != E_OK){
					//エラ－処理要
				}

				//////// メッセージ送信 ////////
				MsgSize = DbgSendMessageToMain(num, 0, msg);
				ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
				if(ercd != E_OK){
					//エラ－処理要
				}					
				break;
// 通常モード(登録) ------------------------------------------------------------------------------------//
			case FPTN_LCD_SCREEN120:
				num = 120;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN120 );
				if ( ercd != E_OK ){
					break;
				}

				//////// 画面表示 ////////
				SetGamen01(1, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				LcdcDisplayModeSet(1, 0);
				
				//////// 画面バッファに転送 ////////
				for(cnt = 0 ; cnt < 2 ; cnt++){	//画面を画面バッファに転送する
					SetGamen01(cnt, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				}
				SetGamen02(0, perm_enb, perm_r, perm_g, perm_b, 208, 104, 271, 167);	//NEXTボタン

				///////// フラグセット ////////
				ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
				if(ercd != E_OK){
					//エラ－処理要
				}

				//////// メッセージ送信 ////////
				MsgSize = DbgSendMessageToMain(num, 0, msg);
				ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
				if(ercd != E_OK){
					//エラ－処理要
				}
				break;
			case FPTN_LCD_SCREEN121:
				num = 121;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN121 );
				if ( ercd != E_OK ){
					break;
				}

				//////// 画面表示 ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				break;
			case FPTN_LCD_SCREEN122:
				num = 122;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN122 );
				if ( ercd != E_OK ){
					break;
				}

				//////// 画面表示 ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);

				dly_tsk((3000/MSEC));	//タイマー3sec

				///////// フラグセット ////////
				ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
				if(ercd != E_OK){
					//エラ－処理要
				}

				//////// メッセージ送信 ////////
				MsgSize = DbgSendMessageToMain(num, 0, msg);
				ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
				if(ercd != E_OK){
					//エラ－処理要
				}					
				break;
			case FPTN_LCD_SCREEN123:
				num = 123;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN123 );
				if ( ercd != E_OK ){
					break;
				}
			
				//////// 画面表示 ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);

				dly_tsk((3000/MSEC));	//タイマー3sec

				///////// フラグセット ////////
				ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
				if(ercd != E_OK){
					//エラ－処理要
				}

				//////// メッセージ送信 ////////
				MsgSize = DbgSendMessageToMain(num, 0, msg);
				ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
				if(ercd != E_OK){
					//エラ－処理要
				}					
				break;
			case FPTN_LCD_SCREEN124:
				num = 124;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN124 );
				if ( ercd != E_OK ){
					break;
				}

				//////// 画面表示 ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				///////// キー入力待ち ////////
				sts_key = WaitKeyProc(99, 0, msg);
				if(sts_key == 1){	//キー押下あり
					///////// フラグセット ////////
					ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
					if(ercd != E_OK){
						//エラ－処理要
					}

					//////// メッセージ送信 ////////
					MsgSize = DbgSendMessageToMain(num, 0, msg);
					ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
					if(ercd != E_OK){
						//エラ－処理要
					}					
				}
				break;
			case FPTN_LCD_SCREEN125:
				num = 125;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN125 );
				if ( ercd != E_OK ){
					break;
				}

				//////// 画面表示 ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				///////// キー入力待ち ////////
				sts_key = WaitKeyProc(99, 0, msg);
				if(sts_key == 1){	//キー押下あり
					///////// フラグセット ////////
					ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
					if(ercd != E_OK){
						//エラ－処理要
					}

					//////// メッセージ送信 ////////
					MsgSize = DbgSendMessageToMain(num, 0, msg);
					ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
					if(ercd != E_OK){
						//エラ－処理要
					}					
				}
				break;
			case FPTN_LCD_SCREEN126:
				num = 126;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN126 );
				if ( ercd != E_OK ){
					break;
				}

				//////// 画面表示 ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				///////// キー入力待ち ////////
				sts_key = WaitKeyProc(99, 0, msg);
				if(sts_key == 1){	//キー押下あり
					///////// フラグセット ////////
					ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
					if(ercd != E_OK){
						//エラ－処理要
					}

					//////// メッセージ送信 ////////
					MsgSize = DbgSendMessageToMain(num, 0, msg);
					ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
					if(ercd != E_OK){
						//エラ－処理要
					}
				}				
				break;
			case FPTN_LCD_SCREEN127:
				num = 127;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN127 );
				if ( ercd != E_OK ){
					break;
				}

				//////// 画面表示 ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				break;
			case FPTN_LCD_SCREEN128:
				num = 128;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN128 );
				if ( ercd != E_OK ){
					break;
				}

				//////// 画面表示 ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				break;
			case FPTN_LCD_SCREEN129:
				num = 129;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN129 );
				if ( ercd != E_OK ){
					break;
				}

				//////// 画面表示 ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				break;
			case FPTN_LCD_SCREEN130:
				num = 130;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN130 );
				if ( ercd != E_OK ){
					break;
				}

				//////// 画面表示 ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);

				dly_tsk((3000/MSEC));	//タイマー3sec

				///////// フラグセット ////////
				ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
				if(ercd != E_OK){
					//エラ－処理要
				}

				//////// メッセージ送信 ////////
				MsgSize = DbgSendMessageToMain(num, 0, msg);
				ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
				if(ercd != E_OK){
					//エラ－処理要
				}					
				break;
			case FPTN_LCD_SCREEN131:
				num = 131;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN131 );
				if ( ercd != E_OK ){
					break;
				}
			
				//////// 画面表示 ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);

				dly_tsk((3000/MSEC));	//タイマー3sec

				///////// フラグセット ////////
				ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
				if(ercd != E_OK){
					//エラ－処理要
				}

				//////// メッセージ送信 ////////
				MsgSize = DbgSendMessageToMain(num, 0, msg);
				ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
				if(ercd != E_OK){
					//エラ－処理要
				}					
				break;
			case FPTN_LCD_SCREEN132:
				num = 132;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN132 );
				if ( ercd != E_OK ){
					break;
				}

				//////// 画面表示 ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				///////// キー入力待ち ////////
				sts_key = WaitKeyProc(99, 0, msg);
				if(sts_key == 1){	//キー押下あり
					///////// フラグセット ////////
					ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
					if(ercd != E_OK){
						//エラ－処理要
					}

					//////// メッセージ送信 ////////
					MsgSize = DbgSendMessageToMain(num, 0, msg);
					ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
					if(ercd != E_OK){
						//エラ－処理要
					}
				}					
				break;
// 通常モード(削除) ------------------------------------------------------------------------------------//
			case FPTN_LCD_SCREEN140:
				num = 140;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN140 );
				if ( ercd != E_OK ){
					break;
				}

				//////// 画面表示 ////////
				SetGamen01(1, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				LcdcDisplayModeSet(1, 0);
				
				//////// 画面バッファに転送 ////////
				for(cnt = 0 ; cnt < 2 ; cnt++){	//画面を画面バッファに転送する
					SetGamen01(cnt, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				}
				SetGamen02(0, perm_enb, perm_r, perm_g, perm_b, 208, 104, 271, 167);	//NEXTボタン

				///////// フラグセット ////////
				ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
				if(ercd != E_OK){
					//エラ－処理要
				}

				//////// メッセージ送信 ////////
				MsgSize = DbgSendMessageToMain(num, 0, msg);
				ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
				if(ercd != E_OK){
					//エラ－処理要
				}
				break;
			case FPTN_LCD_SCREEN141:
				num = 141;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN141 );
				if ( ercd != E_OK ){
					break;
				}

				//////// 画面表示 ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				break;
			case FPTN_LCD_SCREEN142:
				num = 142;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN142 );
				if ( ercd != E_OK ){
					break;
				}

				//////// 画面表示 ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);

				dly_tsk((3000/MSEC));	//タイマー3sec

				///////// フラグセット ////////
				ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
				if(ercd != E_OK){
					//エラ－処理要
				}

				//////// メッセージ送信 ////////
				MsgSize = DbgSendMessageToMain(num, 0, msg);
				ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
				if(ercd != E_OK){
					//エラ－処理要
				}					
				break;
			case FPTN_LCD_SCREEN143:
				num = 143;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN143 );
				if ( ercd != E_OK ){
					break;
				}
			
				//////// 画面表示 ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);

				dly_tsk((3000/MSEC));	//タイマー3sec

				///////// フラグセット ////////
				ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
				if(ercd != E_OK){
					//エラ－処理要
				}

				//////// メッセージ送信 ////////
				MsgSize = DbgSendMessageToMain(num, 0, msg);
				ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
				if(ercd != E_OK){
					//エラ－処理要
				}					
				break;
			case FPTN_LCD_SCREEN144:
				num = 144;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN144 );
				if ( ercd != E_OK ){
					break;
				}

				//////// 画面表示 ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				///////// キー入力待ち ////////
				sts_key = WaitKeyProc(99, 0, msg);
				if(sts_key == 1){	//キー押下あり
					///////// フラグセット ////////
					ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
					if(ercd != E_OK){
						//エラ－処理要
					}

					//////// メッセージ送信 ////////
					MsgSize = DbgSendMessageToMain(num, 0, msg);
					ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
					if(ercd != E_OK){
						//エラ－処理要
					}					
				}
				break;
			case FPTN_LCD_SCREEN145:
				num = 145;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN145 );
				if ( ercd != E_OK ){
					break;
				}

				//////// 画面表示 ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				///////// キー入力待ち ////////
				sts_key = WaitKeyProc(99, 0, msg);
				if(sts_key == 1){	//キー押下あり
					///////// フラグセット ////////
					ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
					if(ercd != E_OK){
						//エラ－処理要
					}

					//////// メッセージ送信 ////////
					MsgSize = DbgSendMessageToMain(num, 1, msg);
					ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
					if(ercd != E_OK){
						//エラ－処理要
					}
				}					
				break;
			case FPTN_LCD_SCREEN146:
				num = 146;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN146 );
				if ( ercd != E_OK ){
					break;
				}

				//////// 画面表示 ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				///////// キー入力待ち ////////
				sts_key = WaitKeyProc(99, 0, msg);
				if(sts_key == 1){	//キー押下あり
					///////// フラグセット ////////
					ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
					if(ercd != E_OK){
						//エラ－処理要
					}

					//////// メッセージ送信 ////////
					MsgSize = DbgSendMessageToMain(num, 0, msg);
					ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
					if(ercd != E_OK){
						//エラ－処理要
					}
				}					
				break;
// メンテモード ------------------------------------------------------------------------------------//
			case FPTN_LCD_SCREEN200:
				num = 200;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN200 );
				if ( ercd != E_OK ){
					break;
				}

				//////// 画面表示 ////////
				SetGamen01(1, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				LcdcDisplayModeSet(1, 0);
				
				//////// 画面バッファに転送 ////////
				for(cnt = 0 ; cnt < 2 ; cnt++){	//画面を画面バッファに転送する
					SetGamen01(cnt, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				}
				SetGamen02(0, perm_enb, perm_r, perm_g, perm_b, 208, 104, 271, 167);	//NEXTボタン

				///////// フラグセット ////////
				ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
				if(ercd != E_OK){
					//エラ－処理要
				}

				//////// メッセージ送信 ////////
				MsgSize = DbgSendMessageToMain(num, 0, msg);
				ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
				if(ercd != E_OK){
					//エラ－処理要
				}
				break;
			case FPTN_LCD_SCREEN201:
				num = 201;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN201 );
				if ( ercd != E_OK ){
					break;
				}


//画面表示テスト
				SetGamen02(201, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				dly_tsk((3000/MSEC));	//タイマー3sec
				SetGamen02(202, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				dly_tsk((3000/MSEC));	//タイマー3sec
				SetGamen02(203, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				dly_tsk((3000/MSEC));	//タイマー3sec
				SetGamen02(204, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				dly_tsk((3000/MSEC));	//タイマー3sec
				SetGamen02(205, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				dly_tsk((3000/MSEC));	//タイマー3sec
//画面表示テスト

				//////// 画面表示 ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				///////// キー入力待ち ////////
				sts_key = WaitKeyProc(99, 0, msg);
				if(sts_key == 1){	//キー押下あり
					///////// フラグセット ////////
					ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
					if(ercd != E_OK){
						//エラ－処理要
					}

					//////// メッセージ送信 ////////
					MsgSize = DbgSendMessageToMain(num, 0, msg);
					ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
					if(ercd != E_OK){
						//エラ－処理要
					}					
				}
				break;
			case FPTN_LCD_SCREEN202:
				num = 202;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN202 );
				if ( ercd != E_OK ){
					break;
				}

				//////// 画面表示 ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				///////// キー入力待ち ////////
				sts_key = WaitKeyProc(99, 0, msg);
				if(sts_key == 1){	//キー押下あり
					///////// フラグセット ////////
					ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
					if(ercd != E_OK){
						//エラ－処理要
					}

					//////// メッセージ送信 ////////
					MsgSize = DbgSendMessageToMain(num, 0, msg);
					ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
					if(ercd != E_OK){
						//エラ－処理要
					}					
				}
				break;
			case FPTN_LCD_SCREEN203:
				num = 203;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN203 );
				if ( ercd != E_OK ){
					break;
				}

				//////// 画面表示 ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				break;
			case FPTN_LCD_SCREEN204:
				num = 204;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN204 );
				if ( ercd != E_OK ){
					break;
				}

				//////// 画面表示 ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);

				dly_tsk((3000/MSEC));	//タイマー3sec

				///////// フラグセット ////////
				ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
				if(ercd != E_OK){
					//エラ－処理要
				}

				//////// メッセージ送信 ////////
				MsgSize = DbgSendMessageToMain(num, 0, msg);
				ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
				if(ercd != E_OK){
					//エラ－処理要
				}					
				break;
			case FPTN_LCD_SCREEN205:
				num = 205;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN205 );
				if ( ercd != E_OK ){
					break;
				}
			
				//////// 画面表示 ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);

				dly_tsk((3000/MSEC));	//タイマー3sec

				///////// フラグセット ////////
				ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
				if(ercd != E_OK){
					//エラ－処理要
				}

				//////// メッセージ送信 ////////
				MsgSize = DbgSendMessageToMain(num, 0, msg);
				ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
				if(ercd != E_OK){
					//エラ－処理要
				}					
				break;

			default:
				break;
		}
	}

	PrgErrSet();
	slp_tsk();					//ここに来るのは、システム・エラー
}


UB WaitKeyProc(int sousa, int tflg, UB *msg)
{
	UB	rtn=0;
	UB	rtnsts;
	ER	ercd;
	int posx, posy;
	int x1, x2, y1, y2;
	int tcnt;
	int dbg_cnt;
	UB *buf;

	buf = msg;
	tcnt = 0;
	dbg_cnt = 0;
	while(1){
		ercd = TplPosGet(&posx, &posy, (500/MSEC));	//キー待ち500msec
		if(	ercd == E_OK ){
			dly_tsk((500/MSEC));	//500msecのウエイト(タッチキーの反応解除用)
			rtn = 1;	//CHG_REQ要求

			if(tflg == 2){	//画面Sleep状態の場合(LCDバックライトをOFFにしている)
				LcdcBackLightOn();
				tflg = 1;
				tcnt = 0;
			}else{
				if( sousa == 99 ){
					x1 = 208;
					x2 = 272;
					y1 = 104;
					y2 = 167;
					if( (posx > x1 && posx < x2) && (posy > y1 && posy < y2) ){
						break;
					}
				}else{
					break;
				}
			}
			
		}

		if(tflg == 1){
			++tcnt;
			if(tcnt >= 120){	//画面Sleep状態にする
				LcdcBackLightOff();
				tflg = 2;
				tcnt = 0;
			}
		}

		rtnsts = MdGetMode();
		if( rtnsts == MD_CAP ){
			rtn = 2;	//CHG_REQなし
			LcdcBackLightOn();
			break;
		}
	}

	return(rtn);
}


void SetGamen01(int buf, UB perm, UB perm_r, UB perm_g, UB perm_b, UH stx, UH sty, UH edx, UH edy)
{
	ER rtn;
	int x, y, x1, x2, y1, y2, size;
	UH val1, num;
	//UH val2;
	unsigned long cnt;

	num = buf;
	LcdcDisplayWindowSet(num, perm, perm_r, perm_g, perm_b, stx, sty, edx, edy);
	
	x1 = stx;
	x2 = edx;
	y1 = sty;
	y2 = edy;

	size = (x2 - x1 + 1) * (y2 - y1 + 1) * 2;
	for(cnt = 0 ; cnt < size ; cnt++ ){
		val1 = GetPixDataFromGamen01(num, cnt);
		rtn = LcdImageWrite(val1);	
		if(rtn != E_OK ){
			break;
		}
	}
	
	LcdcDisplayWindowFinish();
}


UH GetPixDataFromGamen01(int buf, unsigned long cnt)
{
	UH val;

	switch(buf){
		case 0:
			//val = LcdGmn01[cnt];
			val = 0x00;
			break;
		case 1:
			val = 0x00;
			break;
		default:
			val = 0x00;
			break;
	}
	return(val);
}



void SetGamen02(int buf, UB perm, UB perm_r, UB perm_g, UB perm_b, UH stx, UH sty, UH edx, UH edy)
{
	ER rtn;
	int x, y, x1, x2, y1, y2, size;
	UH val1, num;
	//UH val2;
	unsigned long cnt;

	num = buf;
	LcdcDisplayWindowSet(1, perm, perm_r, perm_g, perm_b, stx, sty, edx, edy);
	
	x1 = stx;
	x2 = edx;
	y1 = sty;
	y2 = edy;

	size = (x2 - x1 + 1) * (y2 - y1 + 1) * 2;
	for(cnt = 0 ; cnt < size ; cnt++ ){
		val1 = GetPixDataFromGamen02(num, cnt);
		rtn = LcdImageWrite(val1);	
		if(rtn != E_OK ){
			break;
		}
	}
	
	LcdcDisplayWindowFinish();
}



UH GetPixDataFromGamen02(int buf, unsigned long cnt)
{
	UH val;
	
	switch(buf){
		case 0:
			val = btn_NEXT[cnt];
			break;
		case 1:
			val = dbgmn001[cnt];
			break;
		case 2:
			val = dbgmn002[cnt];
			break;
		case 3:
			val = dbgmn003[cnt];
			break;
		case 4:
			val = dbgmn004[cnt];
			break;
		case 5:
			val = dbgmn005[cnt];
			break;
		case 6:
			val = dbgmn006[cnt];
			break;
		case 7:
			val = dbgmn007[cnt];
			break;
		case 8:
			val = dbgmn008[cnt];
			break;
		case 9:
			val = dbgmn009[cnt];
			break;
		case 10:
			val = dbgmn010[cnt];
			break;
		case 11:
			val = dbgmn011[cnt];
			break;
		case 12:
			val = dbgmn012[cnt];
			break;
		case 101:
			val = dbgmn101[cnt];
			break;
		case 102:
			val = dbgmn102[cnt];
			break;
		case 103:
			val = dbgmn103[cnt];
			break;
		case 104:
			val = dbgmn104[cnt];
			break;
		case 121:
			val = dbgmn121[cnt];
			break;
		case 122:
			val = dbgmn122[cnt];
			break;
		case 123:
			val = dbgmn123[cnt];
			break;
		case 124:
			val = dbgmn124[cnt];
			break;
		case 125:
			val = dbgmn125[cnt];
			break;
		case 126:
			val = dbgmn126[cnt];
			break;
		case 127:
			val = dbgmn127[cnt];
			break;
		case 128:
			val = dbgmn128[cnt];
			break;
		case 129:
			val = dbgmn129[cnt];
			break;
		case 130:
			val = dbgmn130[cnt];
			break;
		case 131:
			val = dbgmn131[cnt];
			break;
		case 132:
			val = dbgmn132[cnt];
			break;
		case 141:
			val = dbgmn141[cnt];
			break;
		case 142:
			val = dbgmn142[cnt];
			break;
		case 143:
			val = dbgmn143[cnt];
			break;
		case 144:
			val = dbgmn144[cnt];
			break;
		case 145:
			val = dbgmn145[cnt];
			break;
		case 146:
			val = dbgmn146[cnt];
			break;
		case 201:
			val = dbgmn201[cnt];
			break;
		case 202:
			val = dbgmn202[cnt];
			break;
		case 203:
			val = dbgmn203[cnt];
			break;
		case 204:
			val = dbgmn204[cnt];
			break;
		default:
			val = 0x00;
			break;
	}
	
	return(val);
}


UINT DbgSendMessageToMain(int num, int sousa, UB *msg)
{
	UB *buf;
	int cnt, i;
	UINT msize;

	msize = 0;	
	buf = msg;

	cnt = 0;
	switch(num)
	{
		case 1:
			*(buf + cnt++) = ( UB )LCD_USER_ID;
			*(buf + cnt++) = '0';
			*(buf + cnt++) = '9';
			*(buf + cnt++) = '8';
			*(buf + cnt++) = '7';
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 2:
			*(buf + cnt++) = ( UB )LCD_INIT_INPUT;
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 3:
			*(buf + cnt++) = ( UB )LCD_YUBI_ID;
			*(buf + cnt++) = '0';
			*(buf + cnt++) = '0';
			*(buf + cnt++) = '1';
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 4:
			for(i = 0 ; i < 24 ; i++ ){
				*(buf + i) = 0x20;
			}
			*(buf + 24) = 0;
				
			*(buf + cnt++) = ( UB )LCD_NAME;
			*(buf + cnt++) = 'ﾀ';
			*(buf + cnt++) = 'ﾛ';
			*(buf + cnt++) = 'ｳ';
			msize = 26;
			break;
		case 5:
			*(buf + cnt++) = ( UB )LCD_YUBI_SHUBETU;
			*(buf + cnt++) = '1';
			*(buf + cnt++) = '2';
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 6:
			//*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 7:
			//*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 8:
			//*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 9:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 10:
			*(buf + cnt++) = ( UB )LCD_BACK;
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 11:
			if(sousa == 0){
				*(buf + cnt++) = ( UB )LCD_YES;
				*(buf + cnt++) = 0;
				msize = cnt;
			}else{
				*(buf + cnt++) = ( UB )LCD_NO;
				*(buf + cnt++) = 0;
				msize = cnt;
			}
			break;
		case 12:
			if(sousa == 0){
				*(buf + cnt++) = ( UB )LCD_YES;
				*(buf + cnt++) = 0;
				msize = cnt;
			}else{
				*(buf + cnt++) = ( UB )LCD_NO;
				*(buf + cnt++) = 0;
				msize = cnt;
			}
			break;
		case 100:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 101:
		case 102:
			if(sousa == 0){
				*(buf + cnt++) = ( UB )LCD_TOUROKU;
				*(buf + cnt++) = 0;
				msize = cnt;
			}else if(sousa == 1){
				*(buf + cnt++) = ( UB )LCD_SAKUJYO;
				*(buf + cnt++) = 0;
				msize = cnt;
			}else{
				*(buf + cnt++) = ( UB )LCD_MAINTE;
				*(buf + cnt++) = 0;
				msize = cnt;
			}
			break;
		case 103:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 104:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 120:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 121:
			//*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 122:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 123:
			*(buf + cnt++) = ( UB )LCD_BACK;
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 124:
			*(buf + cnt++) = ( UB )LCD_YUBI_ID;
			*(buf + cnt++) = '0';
			*(buf + cnt++) = '0';
			*(buf + cnt++) = '1';
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 125:
			for(i = 0 ; i < 24 ; i++ ){
				*(buf + i) = 0x20;
			}
			*(buf + 24) = 0;
				
			*(buf + cnt++) = ( UB )LCD_NAME;
			*(buf + cnt++) = 'ﾀ';
			*(buf + cnt++) = 'ﾛ';
			*(buf + cnt++) = 'ｳ';
			msize = 26;
			break;
		case 126:
			*(buf + cnt++) = ( UB )LCD_YUBI_SHUBETU;
			*(buf + cnt++) = '1';
			*(buf + cnt++) = '2';
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 127:
			//*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 128:
			//*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 129:
			//*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 130:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 131:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 132:
			if(sousa == 0){
				*(buf + cnt++) = ( UB )LCD_YES;
				*(buf + cnt++) = 0;
				msize = cnt;
			}else{
				*(buf + cnt++) = ( UB )LCD_NO;
				*(buf + cnt++) = 0;
				msize = cnt;
			}
			break;
		case 140:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 141:
			//*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 142:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 143:
			*(buf + cnt++) = ( UB )LCD_BACK;
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 144:
			*(buf + cnt++) = ( UB )LCD_YUBI_ID;
			*(buf + cnt++) = '0';
			*(buf + cnt++) = '0';
			*(buf + cnt++) = '1';
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 145:
			if(sousa == 0){
				*(buf + cnt++) = ( UB )LCD_YES;
				*(buf + cnt++) = 0;
				msize = cnt;
			}else{
				*(buf + cnt++) = ( UB )LCD_NO;
				*(buf + cnt++) = 0;
				msize = cnt;
			}
			break;
		case 146:
			if(sousa == 0){
				*(buf + cnt++) = ( UB )LCD_YES;
				*(buf + cnt++) = 0;
				msize = cnt;
			}else{
				*(buf + cnt++) = ( UB )LCD_NO;
				*(buf + cnt++) = 0;
				msize = cnt;
			}
			break;
		case 200:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 201:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = '9';
			*(buf + cnt++) = '9';
			*(buf + cnt++) = '9';
			*(buf + cnt++) = '9';
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 202:
			*(buf + cnt++) = ( UB )LCD_FULL_PIC_SEND_REQ;
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 203:
			//*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 204:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 205:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		default:
			break;
	}


	return(msize);
}
