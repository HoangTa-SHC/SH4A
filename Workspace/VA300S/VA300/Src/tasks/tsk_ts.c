/**
*	VA-300プログラム
*
*	@file tsk_ts.c
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/09/27
*	@brief  生態検知センサタスク
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
#include "drv_led.h"

// 変数定義
static ID s_idTsk;						///< タスクID

#define	enable_ts_int()	fpga_setw(INT_CRL, (INT_CRL_SENS));	///< タッチパネル割込み許可
#define	clear_ts_int()	fpga_clrw(INT_CRL, (INT_CRL_SENS));	///< タッチパネル割込みクリア

#define CHATA_WAIT_TIME 40

static UH chata_wait_time = CHATA_WAIT_TIME;// チャタリング・チェックのWait時間
static char s_cBuf[ 256 ];				// 処理用バッファ

// プロトタイプ宣言
static ER Wait_Brink_end( void );		// 生体検知センサーのブリンクと、その終了待ち
static void ts_tsk_finish( void );		// 生体検知割込みの許可、LED OFF
static ER check_touch_off( void );		// 生体検知センサーのオフ・タッチ検出処理
static void wait_screen( void );		// 画面が「指を抜いて下さい」または、失敗画面になるのを待つ。

// タスクの定義
const T_CTSK ctsk_ts = { TA_HLNG, NULL, TsTask, 5, 2048, NULL, (B *)"Ts task" };//

/*==========================================================================*/
/**
 * 生態検知センサタスク初期化
 */
/*==========================================================================*/
ER TsTaskInit(ID tskid)
{
	ER ercd;
	
	// タスクの生成
	if (tskid > 0) {
		ercd = cre_tsk(tskid, &ctsk_ts);
		if (ercd == E_OK) {
			s_idTsk = tskid;
		}
	} else {
		ercd = acre_tsk(&ctsk_ts);
		if (ercd > 0) {
			s_idTsk = ercd;
		}
	}
	if (ercd < 0) {
		return ercd;
	}
	
	// タスクの起動
	ercd = sta_tsk(s_idTsk, 0);
	
	// 生態検知センサの初期化
	if (ercd == E_OK) {
		ercd = TsInit(s_idTsk);
	}
	return ercd;
}

/*==========================================================================*/
/**
 * 生態検知センサ制御タスク
 */
/*==========================================================================*/
TASK TsTask(void)
{
	ER		ercd, stat;
	FLGPTN	flgptn;
	
	// 処理開始
	for ( ;; ){
//		ercd = tslp_tsk( TMO_FEVR );			// 生態検知センサON待ち
		ercd = tslp_tsk( 50/MSEC );				// 生態検知センサON待ち
		
		ts_TSK_wdt = FLG_ON;					// タッチセンサ・タスク　WDTカウンタリセット・リクエスト・フラグ Added T.N 2015.3.10

		if( GetScreenNo() == LCD_SCREEN105){	//20150928Miya ｢もう1度、・・・｣で30秒反応ない場合
			if(Ninshou_wait_timer == 0){
				g_AuthCnt = 0;	//認証リトライ リセット
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN101 );	// 通常モードのメニュー画面へ。
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN101 );		// 画面番号　<-　次の画面
					if ( Pfail_mode_count == 0 ){ 
						MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ（必要無いが念のため）
					}	else	{
						MdCngMode( MD_PFAIL );			// 装置モードを	停電モードへ（必要無いが念のため）
					}
				}
				if(CmrWakFlg == 1){
					ercd = CmrCmdSleep();
				}
				
			}
		}
		//20160108Miya FinKeyS
		if( GetScreenNo() == LCD_SCREEN605){	//｢もう1度、・・・｣で30秒反応ない場合
			if(Ninshou_wait_timer == 0){
				g_AuthCnt = 0;	//認証リトライ リセット
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN601 );	// 通常モードのメニュー画面へ。
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN601 );		// 画面番号　<-　次の画面
					if ( Pfail_mode_count == 0 ){ 
						MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ（必要無いが念のため）
					}	else	{
						MdCngMode( MD_PFAIL );			// 装置モードを	停電モードへ（必要無いが念のため）
					}
				}
				if(CmrWakFlg == 1){
					ercd = CmrCmdSleep();
				}
				
			}
		}
		
		if ( ercd == E_TMOUT ) continue;
		
		if ( ercd == E_OK ) {
			clear_ts_int();						// タッチセンサ割込みクリア&不許可

			dly_tsk( chata_wait_time );			// 40mSec待ち。

			//if( g_CapTimes > 0 ){
			//	g_CapTimes = 3;
			//}


			if ( ( fpga_inw( SENS_MON ) & 0x0001 ) == 1 ){//　チャタリング・チェック1回目
			
				LedOut(LED_OK, LED_ON);			// デバッグ用LEDをONする（橙）
				dly_tsk( chata_wait_time );		// 40mSec待ち。
				
			} else	{
				ts_tsk_finish();				// チャタリングとして、終了。
				continue;
			}
			
			if ( ( fpga_inw( SENS_MON ) & 0x0001 ) == 1 ){	//　チャタリング・チェック２回目
			
				LedOut(LED_POW, LED_ON);		// デバッグ用LEDをONする（緑）
				dly_tsk( chata_wait_time );		// 40mSec待ち。
				
			} else	{
				ts_tsk_finish();				// チャタリングとして、終了。
				continue;
			}

			if ( ( fpga_inw( SENS_MON ) & 0x0001 ) == 1 ){	//　チャタリング・チェック３回目
			
				LedOut(LED_ERR, LED_ON);		// デバッグ用LEDをONする（赤）						
				dly_tsk( chata_wait_time );		// 40mSec待ち。
				
			} else {
				ts_tsk_finish();				// チャタリングとして、終了。	
				continue;
			}

#if(PCCTRL)	//20160930Miya PCからVA300Sを制御する
			if(g_capallow == 0){	//撮影許可なし
				ts_tsk_finish();				// チャタリングとして、終了。	
				continue;
			}
#endif

			/** カメラ撮影開始の判定 **/
			if ( ( ( MdGetMode() == MD_INITIAL ) && ( ( GetScreenNo() == LCD_SCREEN6 ) 			// マンション(占有部)仕様
												   || ( GetScreenNo() == LCD_SCREEN8 )			// マンション(占有部)仕様
												   || ( GetScreenNo() == LCD_SCREEN405 )		// １対１仕様
												   || ( GetScreenNo() == LCD_SCREEN407 ) ) )	// １対１仕様
			  || ( ( ( MdGetMode() == MD_NORMAL ) || ( MdGetMode() == MD_PFAIL ) )
			  									&& ( ( GetScreenNo() == LCD_SCREEN101 ) 		// マンション(占有部)仕様
								    		  	   || ( GetScreenNo() == LCD_SCREEN102 )
								    		  	   || ( GetScreenNo() == LCD_SCREEN105 )		//20140423Miya 認証リトライ追加
												   || ( GetScreenNo() == LCD_SCREEN121 )
												   || ( GetScreenNo() == LCD_SCREEN127 )
												   || ( GetScreenNo() == LCD_SCREEN129 )
								    			   || ( GetScreenNo() == LCD_SCREEN141 )
												   || ( GetScreenNo() == LCD_SCREEN161 ) 
												   || ( GetScreenNo() == LCD_SCREEN181 ) 
												   || ( GetScreenNo() == LCD_SCREEN601 ) 		//20160108Miya FinKeyS
												   || ( GetScreenNo() == LCD_SCREEN602 ) 		//20160108Miya FinKeyS
												   || ( GetScreenNo() == LCD_SCREEN605 ) 		//20160108Miya FinKeyS
												   || ( GetScreenNo() == LCD_SCREEN610 ) 		//20160108Miya FinKeyS
												   || ( GetScreenNo() == LCD_SCREEN611 ) 		//20160108Miya FinKeyS
												   || ( GetScreenNo() == LCD_SCREEN503 )		// １対１仕様
												   || ( GetScreenNo() == LCD_SCREEN523 )
												   || ( GetScreenNo() == LCD_SCREEN530 )
												   || ( GetScreenNo() == LCD_SCREEN532 )
												   || ( GetScreenNo() == LCD_SCREEN544 ) ) )
			  || ( ( MdGetMode() == MD_MAINTE )  && ( ( GetScreenNo() == LCD_SCREEN203 ) ) ) ){

			if( g_CapTimes == 0 && CmrReloadFlg == 0){
				dbg_ts_flg = 1;


				/**　ここに、カメラの撮影要求処理をメッセージ・キューで記述	**/
				g_CapTimes = 1;	//20131210Miya add
							
				//20140423Miya 認証リトライ追加 認証時リトライ回数リセット ->
				if( GetScreenNo() == LCD_SCREEN105){
					//Ninshou_wait_timer = 0;	//20150928Miya ｢もう1度、・・・｣で30秒反応ない場合
					Ninshou_wait_timer = NINSHOU_WAIT_TIME;	//20150928Miya ｢もう1度、・・・｣で30秒反応ない場合
					
					//20171206Miya 20170915Miya LCD105とLCD106が重なる不具合対策LCD106表示しない
					//ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN106 );	// グレー画面へ。	
					//if ( ercd == E_OK ){
					//	ChgScreenNo( LCD_SCREEN105 );			// 画面番号　<-　次の画面
					//}
				}
				//20160108Miya FinKeyS
				if( GetScreenNo() == LCD_SCREEN605){
					Ninshou_wait_timer = 0;	//｢もう1度、・・・｣で30秒反応ない場合
					
					//20171206Miya 20170915Miya LCD105とLCD106が重なる不具合対策LCD106表示しない
					//ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN606 );	// グレー画面へ。	
					//if ( ercd == E_OK ){
					//	ChgScreenNo( LCD_SCREEN605 );			// 画面番号　<-　次の画面
					//}
				}
			
				//if( GetScreenNo() == LCD_SCREEN101 || GetScreenNo() == LCD_SCREEN121 ){
				if( GetScreenNo() == LCD_SCREEN101 || GetScreenNo() == LCD_SCREEN121
				|| GetScreenNo() == LCD_SCREEN141 || GetScreenNo() == LCD_SCREEN161 || GetScreenNo() == LCD_SCREEN181
				|| GetScreenNo() == LCD_SCREEN601 || GetScreenNo() == LCD_SCREEN610 || GetScreenNo() == LCD_SCREEN611 ){ //20160108Miya FinKeyS
					IrLedOnOffSet(1, irDuty2, irDuty3, irDuty4, irDuty5);	//20160312Miya 極小精度UP
					g_AuthCnt = 1;	//20140423Miya 認証リトライ回数
#if(PCCTRL)	//20160930Miya PCからVA300Sを制御する
					g_AuthCnt = g_pc_authnum;
#endif

#if(AUTHTEST >= 2)	//20160613Miya
					g_AuthCnt = 3;	//20160613Miya 認証リトライ回数
#endif
				}
				//20140423Miya 認証リトライ追加 認証時リトライ回数リセット <-

				ercd = set_flg( ID_FLG_MAIN, FPTN_START_CAP );
				if ( ercd != E_OK ){
					break;
				}
			
				dly_tsk( 1000/MSEC );
			
				/**　カメラ撮影要求処理　終了待ち	**/
				ercd = wai_flg( ID_FLG_TS, FPTN_CMR_TS, TWF_ORW, &flgptn );
				if ( ercd != E_OK ){
					break;
				}
			
				ercd = clr_flg( ID_FLG_TS, ~FPTN_CMR_TS );	// フラグのクリア	
				if ( ercd != E_OK ){
					break;
				}

				ercd = Wait_Brink_end();			// 生体検知センサーのブリンクと、その終了待ち(カメラ画像の送信終了)。
				if ( ercd != E_OK ){
					break;
				}

				LedOut(LED_POW, LED_ON);			// 電源表示LEDをONする（橙）・・・　消灯するタイミングがあるため。


#if ( VA300S == 1 || VA300S == 2 )
				if( s_CapResult == CAP_JUDGE_RT || s_CapResult == CAP_JUDGE_RT2 ){	// 再撮影要求待ちなら
					SetReCap();
				}
#endif
													//　画面が「指をセットして」なら								
				if ( ( GetScreenNo() == LCD_SCREEN6 ) || ( GetScreenNo() == LCD_SCREEN127 ) || ( GetScreenNo() == LCD_SCREEN405 ) || ( GetScreenNo() == LCD_SCREEN530 ) ){ 
					wait_screen();					// 画面が「指を抜いて下さい」または、失敗画面になるのを待つ。
					while(1){//20131210Miya add
						if ( ( GetScreenNo() == LCD_SCREEN7 ) || ( GetScreenNo() == LCD_SCREEN128 ) || ( GetScreenNo() == LCD_SCREEN406 ) || ( GetScreenNo() == LCD_SCREEN531 ) ){ 
							nop();
							dly_tsk( 1000/MSEC );				// 最低１秒間は、「指を抜いて下さい」画面を保持。
							break;
						}
						if ( ( GetScreenNo() == LCD_SCREEN10 ) || ( GetScreenNo() == LCD_SCREEN131 ) || ( GetScreenNo() == LCD_SCREEN409 ) || ( GetScreenNo() == LCD_SCREEN534 ) ){ 
							nop();
							dly_tsk( 1000/MSEC );				// 最低１秒間は、「指を抜いて下さい」画面を保持。
							break;
						}
						dly_tsk( 200/MSEC );
					}
				}

//				dly_tsk( 2000/MSEC );				//20131210Miya del // 最低2秒間は、「指を抜いて下さい」画面を保持。
//				dly_tsk( 1000/MSEC );				// 最低１秒間は、「指を抜いて下さい」画面を保持。

				/** オフ・タッチをメイン・タスクへ通知	**/
				if ( GetSysSpec() == SYS_SPEC_MANTION ){	// マンション(占有部)仕様マンション(占有部)仕様の場合。
					if ( ( MdGetMode() == MD_INITIAL ) && ( GetScreenNo() == LCD_SCREEN7 ) ){ // 初期モード（登録）「指を抜いて下さい」
						while ( check_touch_off() != E_OK ){ // 生体検知センサーのオフ・タッチを検出する
				  			dly_tsk( 20/MSEC );
						}
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN8 );	// 「もう一度、指を...」画面へ。	
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN8 );		// 画面番号　<-　次の画面
							MdCngMode( MD_INITIAL );		// 装置モードを初期登録モードへ	
						}
											
					} else if ( ( ( MdGetMode() == MD_NORMAL ) || ( MdGetMode() == MD_PFAIL ) ) && ( GetScreenNo() == LCD_SCREEN128 ) ){ // 通常モード（登録）「指を抜いて下さい」
						while ( check_touch_off() != E_OK ){ // 生体検知センサーのオフ・タッチを検出する
					  		dly_tsk( 20/MSEC );
						}
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN129 );	// 「もう一度、指を...」画面へ。	
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN129 );		// 画面番号　<-　次の画面
						}					
					}
					
				} else if ( GetSysSpec() == SYS_SPEC_OFFICE ){		// １対１仕様の場合。 {
					if ( ( MdGetMode() == MD_INITIAL ) && ( GetScreenNo() == LCD_SCREEN406 ) ){ // 初期モード（登録）「指を抜いて下さい」
						while ( check_touch_off() != E_OK ){ // 生体検知センサーのオフ・タッチを検出する
				  			dly_tsk( 20/MSEC );
						}
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN407 );	// 「もう一度、指を...」画面へ。	
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN407 );		// 画面番号　<-　次の画面
							MdCngMode( MD_INITIAL );		// 装置モードを初期登録モードへ	
						}
											
					} else if ( ( ( MdGetMode() == MD_NORMAL ) || ( MdGetMode() == MD_PFAIL ) )  && ( GetScreenNo() == LCD_SCREEN531 ) ){ // 通常モード（登録）「指を抜いて下さい」
						while ( check_touch_off() != E_OK ){ // 生体検知センサーのオフ・タッチを検出する
					  		dly_tsk( 20/MSEC );
						}
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN532 );	// 「もう一度、指を...」画面へ。	
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN532 );		// 画面番号　<-　次の画面
						}					
					}
					
				} else {
					nop();
				}
								
				ts_tsk_finish();					// タッチセンサ割込み許可、３色LED OFF
				dbg_ts_flg = 0;
			}
			}
			 
			// 該当画面でない時に、生体検知センサーがONされた場合は、POW_LED（橙）のみONして、残りは消灯。
			nop();
			ts_tsk_finish();					// タッチセンサ割込み許可、３色LED OFF

		} else {
			// ここにくるのは実装エラー
			PrgErrSet();
			slp_tsk();							// エラーのときはタスク終了
		}
	}
	PrgErrSet();								// ここにくるのは実装エラー
	slp_tsk();
}

/*==========================================================================*/
/*
 * 生態検知センサ専用ユーティリティ
 * 画面が「指を抜いて下さい」または、失敗画面になるのを待つ。
 */
/*==========================================================================*/
static void wait_screen( void )
{	
	ER ercd;
	FLGPTN	flgptn;
	
	ercd = twai_flg( ID_FLG_TS, FPTN_WAIT_CHG_SCRN, TWF_ORW, &flgptn, 3000/MSEC );
	if ( ercd == E_OK ){		
		ercd = clr_flg( ID_FLG_TS, ~FPTN_WAIT_CHG_SCRN );	// フラグのクリア	
		nop();
	}	else	{
		nop();
	}
}

/*==========================================================================*/
/*
 * 生態検知センサ専用ユーティリティ
 * 生体検知センサーのオフ・タッチ検出処理
 */
/*==========================================================================*/
static ER check_touch_off( void )
{
	ER ercd = -1;
	
	dly_tsk( chata_wait_time );			// 40mSec待ち。

	if ( ( fpga_inw( SENS_MON ) & 0x0001 ) == 0 ){ //　チャタリング・チェック1回目
	
		dly_tsk( chata_wait_time );		// 40mSec待ち。
				
	} else	{
		return ercd;
	}
			
	if ( ( fpga_inw( SENS_MON ) & 0x0001 ) == 0 ){	//　チャタリング・チェック２回目
			
		dly_tsk( chata_wait_time );		// 40mSec待ち。
				
	} else	{
		return ercd;
	}

	if ( ( fpga_inw( SENS_MON ) & 0x0001 ) == 0 ){	//　チャタリング・チェック３回目
									
		ercd = E_OK;
				
	} else {
		return ercd;
	}

	return ercd;		
}

/*==========================================================================*/
/*
 * 生態検知センサ専用ユーティリティ
 * 生体検知センサーのブリンク終了待ち処理
 */
/*==========================================================================*/
static ER Wait_Brink_end( void ){
	
	ER ercd;
	FLGPTN	flgptn;

	while(1){								// LEDブリンク
		ercd = twai_flg( ID_FLG_TS, FPTN_END_TS, TWF_ORW, &flgptn, 250/MSEC );
		if ( ercd == E_OK ){		
			ercd = clr_flg( ID_FLG_TS, ~FPTN_END_TS );	// フラグのクリア	
			break;
		} else if ( ercd != E_TMOUT ){
			break;
		}	else	{
			LedOut(LED_OK, LED_REVERSE);	// デバッグ用LEDを反転する（橙）
			LedOut(LED_POW, LED_REVERSE);	// デバッグ用LEDを反転する（緑）
			LedOut(LED_ERR, LED_REVERSE);	// デバッグ用LEDを反転する（赤）
		}
	}
	return ercd;
}

/*==========================================================================*/
/*
 * 生態検知センサ専用ユーティリティ
 * 生体検知センサーの終了処理
 */
/*==========================================================================*/
static void ts_tsk_finish( void ){
	
		clear_ts_int();					// タッチセンサ割込みクリア
										// 直前に、生体センサ割込みが再び入っていても、一旦割込みを取消しする。
		
		LedOut(LED_POW, LED_ON);		// 電源表示LEDをONする（橙）
		
		LedOut(LED_OK, LED_OFF);		// デバッグ用LEDをOFFする（緑）
		LedOut(LED_ERR, LED_OFF);		// デバッグ用LEDをOFFする（赤）
		
		enable_ts_int();				// タッチセンサ割込み許可
}