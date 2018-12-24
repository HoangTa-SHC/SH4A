/**
	VA-300プログラム
	@file tsk_lcmd.c
	@version 1.00
	
	@author OYO Electric Co.Ltd
	@date 2012/07/31
	@brief UDPからのコマンド処理タスク
*/
/*=========================================================
	コマンド解釈
===========================================================*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <kernel.h>
#include "sh7750.h"
#include "nonet.h"
#include "id.h"
#include "err_ctrl.h"

#include "drv_led.h"

#include "command.h"
#include "va300.h"
#include "drv_cmr.h"

/*-----------------
	広域変数
		マクロ変数
-------------------*/

// 変数定義
static UINT s_SubMode;			// メンテナンスモードのサブ状態
static UINT s_NormalSubMode;	// 通常モードのサブ状態
//static enum MD_LAN_CMD_STATUS s_LanCmdSubMode;	// LANコマンドモードのサブ状態

static UB MdGetMode(void);					// モードの取得（モード・パラメータは、"va300.h"を参照のこと）
static void MdCngMode( UINT eNextMode );	// モード遷移
static UB MdGetSubMode(void);				// サブモードの取得
static void MdCngSubMode( UINT eNextMode );	// サブモード遷移
static UB rcv_cmd_proc_nomal( T_COMMSG *rcv_msg );		// 通常モード時受信コマンド処理メイン
static UB rcv_cmd211( T_COMMSG *rcv_msg );	// 受信コマンド"211"指登録　OK/NG判定処理
static UB rcv_cmd205( T_COMMSG *rcv_msg );	// 受信コマンド"205"指認証　OK/NG判定処理
static UB rcv_cmd206( T_COMMSG *rcv_msg );	// 受信コマンド"206"指データの再撮影処理
static UB rcv_cmd207( T_COMMSG *rcv_msg );	//	受信コマンド"207"カメラ・パラメータの変更処理
static UB rcv_cmd208( T_COMMSG *rcv_msg );	//	受信コマンド"208"LED光量の変更処理
static UB rcv_cmd101( T_COMMSG *rcv_msg );	// 受信コマンド　メンテナンス・モード移行処理(コマンド101の処理)
static UB rcv_cmd_proc_power_on( T_COMMSG *rcv_msg );	// パワーオンモード時受信コマンド処理メイン
static UB rcv_cmd_proc_power_off( T_COMMSG *rcv_msg );	// パワーオフモード時受信コマンド処理メイン
static UB rcv_cmd_proc_init( T_COMMSG *rcv_msg );		// 初期登録時受信コマンド処理メイン
static UB rcv_cmd_proc_meinte( T_COMMSG *rcv_msg );		// メンテナンスモード時受信コマンド処理メイン
static UB rcv_cmd022( T_COMMSG *rcv_msg );	// 初期モード設定要求 処理（コマンド022の処理）
static UB rcv_cmd012( T_COMMSG *rcv_msg );	// カメラ・パラメータの初期値送信 処理（コマンド022の処理）
static UB rcv_cmd015( T_COMMSG *rcv_msg );	// 画像処理の初期値送信 処理（コマンド015の処理）
static UB rcv_cmd013( T_COMMSG *rcv_msg );	// LED光量数値の初期値送信 処理（コマンド013の処理）
static UB rcv_cmd020( T_COMMSG *rcv_msg );	// 登録情報の初期値送信 処理（コマンド020の処理）
static UB rcv_cmd205_211_ok_proc( void );	// 受信コマンド"205"指認証,"211"指登録のOK判定の受信処理
static UB rcv_cmd205_211_ng_proc( void );	// 受信コマンド"205"指認証,"211"指登録のNG判定の受信処理
static void rcv_cmd205_211_rt_proc( void );	// 受信コマンド"205"指認証,"211"指登録のリトライ要求の受信処理
static UB rcv_cmd002( T_COMMSG *rcv_msg );  // 手順Aの場合のOK/NGの応答受信処理
static UB rcv_cmd272( T_COMMSG *rcv_msg );	// 緊急番号(８桁)表示データ受信処理（コマンド271の応答コマンド）


/*=========================================================*/
/**
 *	LANコマンド処理タスク
 *
 *	@param	iCh	チャンネル
 *	
 */
/*=========================================================*/
TASK LanCmdTask(INT iCh)
{
    T_COMMSG *msg;
	ER_UINT	msg_size;
	ER	ercd;
	TMO	tmout;
	
	tmout = TMO_FEVR;				// = -1、無限待ち(タイムアウトなし)
	
	if (iCh > 0) {
		// ここにくるのは実装エラー
		PrgErrSet();
		while(1) {
			slp_tsk();
		}
	}
	
	for (;;) {
		
		ercd = rcv_mbx( MBX_CMD_LAN, ( T_MSG** )&msg );
//		ercd = trcv_mbx( MBX_CMD_LAN, ( T_MSG** )&msg, tmout );
		if ( ercd == E_OK ){				

			switch  ( sys_Mode ){
		  		case MD_POWER_ON:			// パワーON
					// 受信コマンド処理メイン
					ercd = rcv_cmd_proc_power_on( msg );
					break;
			
		  		case MD_POWER_OFF:			// パワーOFF
					// 受信コマンド処理メイン
					ercd = rcv_cmd_proc_power_off( msg );
					break;
			
		  		case MD_INITIAL:			// 初期登録時
					// 受信コマンド処理メイン
					ercd = rcv_cmd_proc_init( msg );
					
					ercd = rcv_cmd_proc_power_on( msg );	// パワーオン・モードの最初で、モード切り替え要求があり,
															// 初期登録モードへ移行した場合の為。
					break;		

	  	  		case MD_MAINTE:				// メンテナンス・モード
					// 受信コマンド処理メイン
					ercd = rcv_cmd_proc_meinte( msg );
					break;
		
		  		case MD_NORMAL:				// 通常モード
		  		case MD_CAP:				// カメラ撮影中モード
					// 受信コマンド処理メイン
					ercd = rcv_cmd_proc_nomal( msg );		// 通常モード、カメラ撮影中モードの場合
					
					ercd = rcv_cmd_proc_power_on( msg );	// パワーオン・モードの最初で、モード切り替え要求があり,
															// 通常モードへ移行した場合の為。
					break;					
			
	      		default:
					break;
			}	
			
			rel_mpf( MPF_COM, ( VP )msg );		// 処理後はメモリ解放
			
		} else if ( msg_size == E_RLWAI ){	
			// 強制待ち解除のときの処理(コマンド発行)
			tmout = McnParamRcvTmoutGet() / MSEC;	// タイムアウトを設定
			nop();
		
		} else if ( msg_size == E_TMOUT ) {	
			// タイムアウトのときの処理(今のコーディングでは、ここへ来ない)
			nop();


		} else {
			ercd = msg_size;
			ErrCodeSet( ercd );
		}
	}
}


/*==========================================================================*/
/**
 *	通常モード時　受信コマンド解析と処理メイン
 *	@return 解析成功/失敗
 */
/*==========================================================================*/
static UB rcv_cmd_proc_nomal( T_COMMSG *rcv_msg )
{
	UB ercd;
	
	nop();

	if ( ( rcv_msg->buf[ 4 ] == '2' )			// コマンド211なら、指登録のOK/NGの判定
	  && ( rcv_msg->buf[ 5 ] == '1' )
	  && ( rcv_msg->buf[ 6 ] == '1' ) ){
					  
		ercd = rcv_cmd211( rcv_msg );			// コマンド211の処理
	
	} else if ( ( rcv_msg->buf[ 4 ] == '2' )	// コマンド205なら、指認証のOK/NGの判定
	 		 && ( rcv_msg->buf[ 5 ] == '0' )
	  		 && ( rcv_msg->buf[ 6 ] == '5' ) ){
					  
		ercd = rcv_cmd205( rcv_msg );			// コマンド205の処理
/**	
	} else if ( ( rcv_msg->buf[ 4 ] == '2' )	// コマンド201なら、通常モード切り替えの処理
	 		 && ( rcv_msg->buf[ 5 ] == '0' )
	  		 && ( rcv_msg->buf[ 6 ] == '1' ) ){
					  
		MdCngMode( MD_NORMAL );					// 通常モード切り替え
		
		MdCngSubMode( SUB_MD_IDLE );			// サブモードを、IDLEへ設定
**/	
	} else if ( ( rcv_msg->buf[ 4 ] == '2' )	// コマンド206なら、指データの再撮影処理
	 		 && ( rcv_msg->buf[ 5 ] == '0' )
	  		 && ( rcv_msg->buf[ 6 ] == '6' ) ){
					  
		ercd = rcv_cmd206( rcv_msg );			// コマンド206の処理
		
	} else if ( ( rcv_msg->buf[ 4 ] == '2' )	// コマンド207なら、カメラ・パラメータの変更処理
	 		 && ( rcv_msg->buf[ 5 ] == '0' )
	  		 && ( rcv_msg->buf[ 6 ] == '7' ) ){
					  
		ercd = rcv_cmd207( rcv_msg );			// コマンド207の処理
		
	} else if ( ( rcv_msg->buf[ 4 ] == '2' )	// コマンド208なら、LED光量の変更処理
	 		 && ( rcv_msg->buf[ 5 ] == '0' )
	  		 && ( rcv_msg->buf[ 6 ] == '8' ) ){
					  
		ercd = rcv_cmd208( rcv_msg );			// コマンド208の処理
		
	} else if ( ( rcv_msg->buf[ 4 ] == '1' )	// コマンド101なら、メンテナンスモード移行処理
	 		 && ( rcv_msg->buf[ 5 ] == '0' )
	  		 && ( rcv_msg->buf[ 6 ] == '1' ) ){
					  
		ercd = rcv_cmd101( rcv_msg );			// コマンド101の処理
		
	} else if ( ( rcv_msg->buf[ 4 ] == '0' )	// コマンド002なら、手順Aの場合のOK/NGの応答処理
	 		 && ( rcv_msg->buf[ 5 ] == '0' )
	  		 && ( rcv_msg->buf[ 6 ] == '2' ) ){
					  
		if ( MdGetSubMode() != SUB_MD_IDLE ){
			ercd = rcv_cmd002( rcv_msg );			// コマンド002の処理
		}
		
	} else if ( ( rcv_msg->buf[ 4 ] == '2' )	// コマンド272なら、緊急番号(８桁)表示データ受信処理（コマンド271の応答コマンド）
	 		 && ( rcv_msg->buf[ 5 ] == '7' )
	  		 && ( rcv_msg->buf[ 6 ] == '2' ) ){
					  
		ercd = rcv_cmd272( rcv_msg );			// コマンド272の処理
		
	}

	return ercd;				
}


/*==========================================================================*/
//	受信コマンド"205"指認証のOK/NGの判定の処理
//	@return 処理の成功/失敗
/*==========================================================================*/
static UB rcv_cmd205( T_COMMSG *rcv_msg )
{
	UB ercd = 0xff;

	nop();
	
	 // 判定結果はOK？
 	if ( ( ( rcv_msg->buf[ 11 ] == 'O' ) && ( rcv_msg->buf[ 12 ] == 'K' ) ) 
      || ( ( rcv_msg->buf[ 11 ] == 'o' ) && ( rcv_msg->buf[ 12 ] == 'k' ) )
	  || ( ( rcv_msg->buf[ 11 ] == 'O' ) && ( rcv_msg->buf[ 12 ] == 'k' ) )
	  || ( ( rcv_msg->buf[ 11 ] == 'o' ) && ( rcv_msg->buf[ 12 ] == 'K' ) ) ){
						  
		s_CapResult = CAP_JUDGE_OK;
		
		ercd = rcv_cmd205_211_ok_proc();	//　OK判定の受信処理
		
	 // 判定結果はNG？
	} else  if ( ( ( rcv_msg->buf[ 11 ] == 'N' ) && ( rcv_msg->buf[ 12 ] == 'G' ) )
		      || ( ( rcv_msg->buf[ 11 ] == 'n' ) && ( rcv_msg->buf[ 12 ] == 'g' ) )
		      || ( ( rcv_msg->buf[ 11 ] == 'N' ) && ( rcv_msg->buf[ 12 ] == 'g' ) )
		      || ( ( rcv_msg->buf[ 11 ] == 'n' ) && ( rcv_msg->buf[ 12 ] == 'G' ) ) ){
						
	 	s_CapResult = CAP_JUDGE_NG;
		
		ercd = rcv_cmd205_211_ng_proc();	//　NG判定の受信処理

	} else if ( ( ( rcv_msg->buf[ 11 ] == 'R' ) && ( rcv_msg->buf[ 12 ] == 'T' ) ) 
	       || ( ( rcv_msg->buf[ 11 ] == 'r' ) && ( rcv_msg->buf[ 12 ] == 't' ) )
	       || ( ( rcv_msg->buf[ 11 ] == 'R' ) && ( rcv_msg->buf[ 12 ] == 't' ) )
	   	   || ( ( rcv_msg->buf[ 11 ] == 'r' ) && ( rcv_msg->buf[ 12 ] == 'T' ) ) ){
						  
		s_CapResult = CAP_JUDGE_RT;
		
		rcv_cmd205_211_rt_proc();			//　リトライ要求の場合の受信処理

	} else if ( ( ( rcv_msg->buf[ 11 ] == 'E' ) && ( rcv_msg->buf[ 12 ] == '1' ) ) 
	       || ( ( rcv_msg->buf[ 11 ] == 'e' ) && ( rcv_msg->buf[ 12 ] == '1' ) ) ){
						  
		s_CapResult = CAP_JUDGE_E1;
		
		ercd = rcv_cmd205_211_ng_proc();	//　NG判定と同じ受信処理
		
	} else if ( ( ( rcv_msg->buf[ 11 ] == 'E' ) && ( rcv_msg->buf[ 12 ] == '2' ) ) 
	       || ( ( rcv_msg->buf[ 11 ] == 'e' ) && ( rcv_msg->buf[ 12 ] == '2' ) ) ){
						  
		s_CapResult = CAP_JUDGE_E2;
		
		ercd = rcv_cmd205_211_ng_proc();	//　NG判定と同じ受信処理
	}
	 
	return ercd;
} 


/*==========================================================================*/
//	受信コマンド"211"指登録のOK/NGの判定の処理
//	@return 処理の成功/失敗
/*==========================================================================*/
static UB rcv_cmd211( T_COMMSG *rcv_msg )
{
	UB ercd = 0xff;
	
	nop();

	 // 判定結果はOK？
 	if ( ( ( rcv_msg->buf[ 11 ] == 'O' ) && ( rcv_msg->buf[ 12 ] == 'K' ) ) 
      || ( ( rcv_msg->buf[ 11 ] == 'o' ) && ( rcv_msg->buf[ 12 ] == 'k' ) )
	  || ( ( rcv_msg->buf[ 11 ] == 'O' ) && ( rcv_msg->buf[ 12 ] == 'k' ) )
	  || ( ( rcv_msg->buf[ 11 ] == 'o' ) && ( rcv_msg->buf[ 12 ] == 'K' ) ) ){
						  
		s_CapResult = CAP_JUDGE_OK;
		
		ercd = rcv_cmd205_211_ok_proc();	//　OK判定の受信処理
	
	 // 判定結果はNG？
	} else  if ( ( ( rcv_msg->buf[ 11 ] == 'N' ) && ( rcv_msg->buf[ 12 ] == 'G' ) )
		      || ( ( rcv_msg->buf[ 11 ] == 'n' ) && ( rcv_msg->buf[ 12 ] == 'g' ) )
		      || ( ( rcv_msg->buf[ 11 ] == 'N' ) && ( rcv_msg->buf[ 12 ] == 'g' ) )
		      || ( ( rcv_msg->buf[ 11 ] == 'n' ) && ( rcv_msg->buf[ 12 ] == 'G' ) ) ){
				
		s_CapResult = CAP_JUDGE_NG;
		
		ercd = rcv_cmd205_211_ng_proc();	//　NG判定の受信処理

	} else if ( ( ( rcv_msg->buf[ 11 ] == 'R' ) && ( rcv_msg->buf[ 12 ] == 'T' ) ) 
	       || ( ( rcv_msg->buf[ 11 ] == 'r' ) && ( rcv_msg->buf[ 12 ] == 't' ) )
	       || ( ( rcv_msg->buf[ 11 ] == 'R' ) && ( rcv_msg->buf[ 12 ] == 't' ) )
	   	   || ( ( rcv_msg->buf[ 11 ] == 'r' ) && ( rcv_msg->buf[ 12 ] == 'T' ) ) ){
						  
		s_CapResult = CAP_JUDGE_RT;
		
		rcv_cmd205_211_rt_proc();			//　リトライ要求の場合の受信処理

	} else if ( ( ( rcv_msg->buf[ 11 ] == 'E' ) && ( rcv_msg->buf[ 12 ] == '1' ) ) 
	       || ( ( rcv_msg->buf[ 11 ] == 'e' ) && ( rcv_msg->buf[ 12 ] == '1' ) ) ){
						  
		s_CapResult = CAP_JUDGE_E1;
		
		ercd = rcv_cmd205_211_ng_proc();	//　NG判定と同じ受信処理
		
	} else if ( ( ( rcv_msg->buf[ 11 ] == 'E' ) && ( rcv_msg->buf[ 12 ] == '2' ) ) 
	       || ( ( rcv_msg->buf[ 11 ] == 'e' ) && ( rcv_msg->buf[ 12 ] == '2' ) ) ){
						  
		s_CapResult = CAP_JUDGE_E2;
		
		ercd = rcv_cmd205_211_ng_proc();	//　NG判定と同じ受信処理
	}	
		
	return ercd;
} 


/*==========================================================================*/
//	受信コマンド"205"指認証,"211"指登録のOK判定の受信処理
/*==========================================================================*/
static UB rcv_cmd205_211_ok_proc( void )
{
	UB ercd = 0xff;
		
	// 初期登録モードの場合
	if ( GetScreenNo() == LCD_SCREEN6 ){	// 認証時「指をセットして下さい..」

		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN7 );	// 	登録「指を抜いて..」画面へ。	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN7 );			// 画面番号　<-　次の画面
		}
		MdCngMode( MD_INITIAL );			// 装置モードを初期登録モードへ

		ercd = set_flg( ID_FLG_TS, FPTN_WAIT_CHG_SCRN );// TSタスクへ画面が「指を抜いて下さい」または、失敗画面になるのを通知。
		if ( ercd != E_OK ){
			nop();			// エラー処理の記述
		}						
	}
		
	if ( GetScreenNo() == LCD_SCREEN8 ){	// 認証時「もう一度指をセットして下さい..」

		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN9 );	// 登録完了画面へ。	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN9 );			// 画面番号　<-　次の画面
		}
		MdCngMode( MD_INITIAL );			// 装置モードを初期登録モードへ
		
		reload_CAP_Param();			// Gain. FixShutter,IR_LEDのパラメータを、初期要求値に戻す。（CAPのRetry時に変わった可能性がある為。）

	}

	// 通常モードの場合		
	if ( ( GetScreenNo() == LCD_SCREEN101 ) || ( GetScreenNo() == LCD_SCREEN102 ) ){
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN103 );	// 認証完了画面へ。	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN103 );			// 画面番号　<-　次の画面
		}
		MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ
					
		reload_CAP_Param();			// Gain. FixShutter,IR_LEDのパラメータを、初期要求値に戻す。（CAPのRetry時に変わった可能性がある為。）
			
	}
		
	if ( GetScreenNo() == LCD_SCREEN121 ){
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN122 );	// 登録時の責任者認証完了画面へ。	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN122 );			// 画面番号　<-　次の画面
		}
		MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ	
		
		reload_CAP_Param();			// Gain. FixShutter,IR_LEDのパラメータを、初期要求値に戻す。（CAPのRetry時に変わった可能性がある為。）
			
	}
		
	if ( GetScreenNo() == LCD_SCREEN127 ){	// 「指をセットして...」画面
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN128 );	// 「指を抜いて下さい」画面へ。	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN128 );			// 画面番号　<-　次の画面
		}
		MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ	
			
		ercd = set_flg( ID_FLG_TS, FPTN_WAIT_CHG_SCRN );// TSタスクへ画面が「指を抜いて下さい」または、失敗画面になるのを通知。
		if ( ercd != E_OK ){
			nop();			// エラー処理の記述
		}			
	}
		
	if ( GetScreenNo() == LCD_SCREEN129 ){	// 「もう一度指をセットして...」画面
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN130 );	// 認証完了画面へ。	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN130 );			// 画面番号　<-　次の画面
		}
		MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ	
		
		reload_CAP_Param();			// Gain. FixShutter,IR_LEDのパラメータを、初期要求値に戻す。（CAPのRetry時に変わった可能性がある為。）

	}
		
	if ( GetScreenNo() == LCD_SCREEN141 ){	// 削除時の責任者認証完了画面へ。
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN142 );	// 認証完了画面へ。	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN142 );			// 画面番号　<-　次の画面
		}
		MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ	
		
		reload_CAP_Param();			// Gain. FixShutter,IR_LEDのパラメータを、初期要求値に戻す。（CAPのRetry時に変わった可能性がある為。）

	}
		
	if ( GetScreenNo() == LCD_SCREEN161 ){	// 緊急開錠番号登録時の責任者認証完了画面へ。
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN162 );	// 認証完了○画面へ。	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN162 );			// 画面番号　<-　次の画面
		}
		MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ	
		
		reload_CAP_Param();			// Gain. FixShutter,IR_LEDのパラメータを、初期要求値に戻す。（CAPのRetry時に変わった可能性がある為。）

	}
	
	// メンテナンス・モードの場合
	if ( GetScreenNo() == LCD_SCREEN203 ){	// メンテナンス・フル画像送信時「指をセットして下さい..」

		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN204 );	// 	撮影○画面へ。	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN204 );			// 画面番号　<-　次の画面
		}
		MdCngMode( MD_MAINTE );			// 装置モードをメンテナンスモードへ
	}
	
	return ercd;

}

/*==========================================================================*/
//	受信コマンド"205"指認証,"211"指登録のNG判定の受信処理
/*==========================================================================*/
static UB rcv_cmd205_211_ng_proc( void )
{
	UB ercd = 0xff;;
							
	// 初期登録モードの場合
	if ( ( GetScreenNo() == LCD_SCREEN6 ) ){
		ercd = set_flg( ID_FLG_TS, FPTN_WAIT_CHG_SCRN );// TSタスクへ画面が「指を抜いて下さい」または、失敗画面になるのを通知。
		if ( ercd != E_OK ){
			nop();			// エラー処理の記述
		}
	}

	if ( ( GetScreenNo() == LCD_SCREEN6 ) || ( GetScreenNo() == LCD_SCREEN8 ) ){	// 認証時「指をセットして下さい..」

		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN10 );	// 認証失敗画面へ。	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN10 );			// 画面番号　<-　次の画面
		}
		MdCngMode( MD_INITIAL );			// 装置モードを初期登録モードへ

		reload_CAP_Param();			// Gain. FixShutter,IR_LEDのパラメータを、初期要求値に戻す。（CAPのRetry時に変わった可能性がある為。）

	}		

	// 通常モードの場合					
	if ( ( GetScreenNo() == LCD_SCREEN101 ) || ( GetScreenNo() == LCD_SCREEN102 ) ){
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN104 );	// 認証失敗画面へ。	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN104 );			// 画面番号　<-　次の画面
		}
		MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ

		reload_CAP_Param();			// Gain. FixShutter,IR_LEDのパラメータを、初期要求値に戻す。（CAPのRetry時に変わった可能性がある為。）			
			
	}
		
	if ( GetScreenNo() == LCD_SCREEN121 ){
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN123 );	// 認証失敗画面へ。	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN123 );			// 画面番号　<-　次の画面
		}
		MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ	
			
		reload_CAP_Param();			// Gain. FixShutter,IR_LEDのパラメータを、初期要求値に戻す。（CAPのRetry時に変わった可能性がある為。）

	}

	if ( GetScreenNo() == LCD_SCREEN127 ){
		ercd = set_flg( ID_FLG_TS, FPTN_WAIT_CHG_SCRN );// TSタスクへ画面が「指を抜いて下さい」または、失敗画面になるのを通知。
		if ( ercd != E_OK ){
			nop();			// エラー処理の記述
		}
	}
				
	if ( ( GetScreenNo() == LCD_SCREEN127 ) || ( GetScreenNo() == LCD_SCREEN129 ) ){
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN131 );	// 認証失敗画面へ。	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN131 );			// 画面番号　<-　次の画面
		}
		MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ	

		reload_CAP_Param();			// Gain. FixShutter,IR_LEDのパラメータを、初期要求値に戻す。（CAPのRetry時に変わった可能性がある為。）			
			
	}
		
	if ( GetScreenNo() == LCD_SCREEN141 ){
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN143 );	// 認証失敗画面へ。	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN143 );			// 画面番号　<-　次の画面
		}
		MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ	
			
		reload_CAP_Param();			// Gain. FixShutter,IR_LEDのパラメータを、初期要求値に戻す。（CAPのRetry時に変わった可能性がある為。）
			
	}
		
		
	if ( GetScreenNo() == LCD_SCREEN161 ){	// 緊急開錠番号登録時の責任者認証失敗画面へ。
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN163 );	// 認証失敗×画面へ。	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN163 );			// 画面番号　<-　次の画面
		}
		MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ	
		
		reload_CAP_Param();			// Gain. FixShutter,IR_LEDのパラメータを、初期要求値に戻す。（CAPのRetry時に変わった可能性がある為。）

	}

	// メンテナンス・モードの場合
	if ( GetScreenNo() == LCD_SCREEN203 ){	// メンテナンス・フル画像送信「指をセットして下さい..」

		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN205 );	// 	撮影×画面へ。	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN205 );			// 画面番号　<-　次の画面
		}
		MdCngMode( MD_MAINTE );			// 装置モードをメンテナンス・モードへ
	}
	
	return ercd;
	
}



/*==========================================================================*/
//	受信コマンド"205"指認証,"211"指登録のリトライ要求の受信処理
/*==========================================================================*/
static void rcv_cmd205_211_rt_proc( void )
{
		
	CmmChangeStatus( ST_COM_STANDBY );		// 受信状態=受信データ待ちへ戻る(EOT待ちを取消し)

	// 初期登録モードの場合
	if ( ( GetScreenNo() == LCD_SCREEN6 ) || ( GetScreenNo() == LCD_SCREEN8 ) ){	// 認証時「指をセットして下さい..」

		MdCngMode( MD_INITIAL );		// 装置モードを初期登録モードへ
	}		
		
	// 通常モードの場合					
	if ( ( GetScreenNo() == LCD_SCREEN101 ) || ( GetScreenNo() == LCD_SCREEN102 ) ){

		MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ
	}
		
	if ( GetScreenNo() == LCD_SCREEN121 ){

		MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ	
	}
		
	if ( ( GetScreenNo() == LCD_SCREEN127 ) || ( GetScreenNo() == LCD_SCREEN129 ) ){

		MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ	
	}
		
	if ( GetScreenNo() == LCD_SCREEN141 ){

		MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ	
	}
	
	if ( GetScreenNo() == LCD_SCREEN161 ){

		MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ	
	}
		
/**	
	// メンテナンス・モードの場合
	if ( GetScreenNo() == LCD_SCREEN204 ){	// メンテナンス認証時「指をセットして下さい..」

		MdCngMode( MD_MAINTE );			// 装置モードをメンテナンス・モードへ
	}
**/	
}

/*==========================================================================*/
//	受信コマンド"206"指データの再撮影処理
//	@return 処理の成功/失敗
/*==========================================================================*/
static UB rcv_cmd206( T_COMMSG *rcv_msg )			// コマンド206の処理
{
	UB ercd;

	nop();
	
 	if ( ( ( rcv_msg->buf[ 11 ] == 'T' ) && ( rcv_msg->buf[ 12 ] == 'O' ) ) 
      || ( ( rcv_msg->buf[ 11 ] == 't' ) && ( rcv_msg->buf[ 12 ] == 'o' ) )
	  || ( ( rcv_msg->buf[ 11 ] == 'T' ) && ( rcv_msg->buf[ 12 ] == 'o' ) )
	  || ( ( rcv_msg->buf[ 11 ] == 't' ) && ( rcv_msg->buf[ 12 ] == 'O' ) ) ){	

		if ( ( MdGetMode() == MD_NORMAL ) || ( MdGetMode() == MD_INITIAL ) ){	// ノーマルモード、イニシャルモードなら
						
			if ( s_CapResult == CAP_JUDGE_RT ){		// 再撮影要求待ちなら
			
				ercd = set_flg( ID_FLG_CAMERA, FPTN_START_CAP204 );			// カメラ撮影+登録処理（コマンド204）へ。
			}
		}
		
	} else if ( ( ( rcv_msg->buf[ 11 ] == 'N' ) && ( rcv_msg->buf[ 12 ] == 'I' ) ) 
      	     || ( ( rcv_msg->buf[ 11 ] == 'n' ) && ( rcv_msg->buf[ 12 ] == 'i' ) )
	         || ( ( rcv_msg->buf[ 11 ] == 'N' ) && ( rcv_msg->buf[ 12 ] == 'i' ) )
	         || ( ( rcv_msg->buf[ 11 ] == 'n' ) && ( rcv_msg->buf[ 12 ] == 'I' ) ) ){
		  
		if ( ( MdGetMode() == MD_NORMAL ) || ( MdGetMode() == MD_INITIAL ) ){	// ノーマルモード、イニシャルモードなら

			if ( s_CapResult == CAP_JUDGE_RT ){		// 再撮影要求待ちなら
									
				ercd = set_flg( ID_FLG_CAMERA, FPTN_START_CAP211 );			// 認証用撮影処理（コマンド210）へ。
			}
		}		  
	}
	 
	return ercd;
}


/*==========================================================================*/
//	受信コマンド"207"カメラ・パラメータの変更処理
//	@return 処理の成功/失敗
/*==========================================================================*/
static UB rcv_cmd207( T_COMMSG *rcv_msg )			// コマンド207の処理
{
	UB ercd = E_OK;

	nop();
	
	if ( rcv_msg->buf[ 11 ] == 0 ){		// 画像取得方法：マニュアル取得

		if ( ( rcv_msg->buf[ 12 ] >= 0 ) && ( rcv_msg->buf[ 12 ] <= 15 ) ){	//　カメラゲイン設定
			cmrGain = rcv_msg->buf[ 12 ];
			ercd = set_flg( ID_FLG_CAMERA, FPTN_SETREQ_GAIN );	// カメラタスクに、直接のゲイン設定値を設定を依頼（FPGAを経由しない）
		}
	
		if ( ( rcv_msg->buf[ 13 ] >= 0 ) && ( rcv_msg->buf[ 13 ] <= 15 ) ){	//　露出１設定
			cmrFixShutter1 = rcv_msg->buf[ 13 ];
		}
	
		if ( ( rcv_msg->buf[ 14 ] >= 0 ) && ( rcv_msg->buf[ 14 ] <= 15 ) ){	//　露出２設定
			cmrFixShutter2 = rcv_msg->buf[ 14 ];
		}
	
		if ( ( rcv_msg->buf[ 15 ] >= 0 ) && ( rcv_msg->buf[ 15 ] <= 15 ) ){	//　露出３設定
			cmrFixShutter3 = rcv_msg->buf[ 15 ];
			ercd = set_flg( ID_FLG_CAMERA, FPTN_SETREQ_SHUT1 );	// カメラタスクに、直接の露出３設定値を設定を依頼（FPGAを経由しない）
		}
				
	} else if ( rcv_msg->buf[ 11 ] == 1 ){	// 画像取得方法：AES+マニュアル取得
	
		nop();
	}
	
	return ercd;
}


/*==========================================================================*/
//	受信コマンド"208"LED光量の変更処理
//	@return 処理の成功/失敗
/*==========================================================================*/
static UB rcv_cmd208( T_COMMSG *rcv_msg )			// コマンド208の処理
{
	UB ercd = E_OK;

	nop();
	
	irDuty2 = rcv_msg->buf[ 11 ];		// IR LEDの点灯初期値
	irDuty3 = rcv_msg->buf[ 12 ];
	irDuty4 = rcv_msg->buf[ 13 ];
	irDuty5 = rcv_msg->buf[ 14 ];
	
	return ercd;

}

/*==========================================================================*/
//	手順Aの場合のOK/NGの応答受信処理
//	@return 処理の成功/失敗
/*==========================================================================*/
static UB rcv_cmd002( T_COMMSG *rcv_msg )			// コマンド002の処理
{
	UB ercd = E_OK;

	nop();

	 // 受信結果はOK？
 	if ( ( ( rcv_msg->buf[ 11 ] == 'O' ) && ( rcv_msg->buf[ 12 ] == 'K' ) ) 
      || ( ( rcv_msg->buf[ 11 ] == 'o' ) && ( rcv_msg->buf[ 12 ] == 'k' ) )
	  || ( ( rcv_msg->buf[ 11 ] == 'O' ) && ( rcv_msg->buf[ 12 ] == 'k' ) )
	  || ( ( rcv_msg->buf[ 11 ] == 'o' ) && ( rcv_msg->buf[ 12 ] == 'K' ) ) ){
						  
		nop();					//　OK判定の受信処理
		if ( MdGetSubMode() == SUB_MD_DONGURU_CHK ){	// PCへドングルの有無確認の確認通信中なら
			s_DongleResult = DONGURU_JUDGE_OK;			// ドングルの有無確認の結果
			
			if ( GetScreenNo() == LCD_SCREEN101 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN200 );	// メンテナンスモード画面へ。
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN200 );		// 画面番号　<-　次の画面
					
				} else {
					nop();			// エラー処理の記述
				}
			}			
			MdCngSubMode( SUB_MD_IDLE );	//　サブモード・ステータスを、”アイドル”へ。
			
		} else if ( MdGetSubMode() == SUB_MD_PASSWORD_CHK ){
			s_PasswordResult = PASSWORD_JUDGE_OK;		// パスワード確認の結果
			
			if ( GetScreenNo() == LCD_SCREEN201 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN202 );	// メンテナンス・メニュー画面へ。
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN202 );		// 画面番号　<-　次の画面
					
					MdCngMode( MD_MAINTE );				// 装置モードをメンテナンスモードへ
//					ercd = SndCmdCngMode( MD_MAINTE );	// PCへ	メンテナンスモード切替え通知を送信
					if ( ercd != E_OK ){
						nop();		// エラー処理の記述	
					}				
				} else {
					nop();			// エラー処理の記述
				}
			}			
			MdCngSubMode( SUB_MD_IDLE );	//　サブモード・ステータスを、”アイドル”へ。
			
		} else if ( MdGetSubMode() == SUB_MD_KINKYU_TOUROKU ){
			s_KinkyuuTourokuResult = KINKYU_TOUROKU_JUDGE_OK;		// 緊急開錠番号登録送信の結果
			
			if ( GetScreenNo() == LCD_SCREEN164 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// 通常モード・初期画面へ。
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN100 );		// 画面番号　<-　次の画面
					MdCngMode( MD_NORMAL );				// 装置モードを通常モードへ（不要のはずだが念のため）
				
				} else {
					nop();			// エラー処理の記述
				}
			}			
			MdCngSubMode( SUB_MD_IDLE );	//　サブモード・ステータスを、”アイドル”へ。
						
		} else if ( MdGetSubMode() == SUB_MD_KINKYU_KAIJYO_SEND ){	// 緊急開錠番号通知通信中
			
			if ( GetScreenNo() == LCD_SCREEN184 ){		// 「８桁番号を入力して下さい」画面なら
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN185 );	// 緊急番号入力画面へ。
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN185 );		// 画面番号　<-　次の画面
					MdCngMode( MD_NORMAL );				// 装置モードを通常モードへ（不要のはずだが念のため）
				
				} else {
					nop();			// エラー処理の記述
				}
			}			
			MdCngSubMode( SUB_MD_IDLE );	//　サブモード・ステータスを、”アイドル”へ。
						
		} else if ( MdGetSubMode() == SUB_MD_KINKYU_NO_CHK_REQ ){	// 入力された緊急開錠番号妥当性確認の通信中
			
			if ( GetScreenNo() == LCD_SCREEN185 ){		// 「緊急番号を入力して下さい...」画面なら
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN186 );	// 開錠OKの○画面へ。
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN186 );		// 画面番号　<-　次の画面
					MdCngMode( MD_NORMAL );				// 装置モードを通常モードへ（不要のはずだが念のため）
				
				} else {
					nop();			// エラー処理の記述
				}
			}			
			MdCngSubMode( SUB_MD_IDLE );	//　サブモード・ステータスを、”アイドル”へ。
						
		} else if ( MdGetSubMode() == SUB_MD_CHG_NORMAL ){	// ノーマルモードへ移行中
						
			MdCngSubMode( SUB_MD_IDLE );	//　サブモード・ステータスを、”アイドル”へ。
						
		} else if ( MdGetSubMode() == SUB_MD_CHG_MAINTE ){	// メンテナンスモードへ移行中
						
			MdCngSubMode( SUB_MD_IDLE );	//　サブモード・ステータスを、”アイドル”へ。
						
		}


			
	 // 受信結果はNG？
	} else  if ( ( ( rcv_msg->buf[ 11 ] == 'N' ) && ( rcv_msg->buf[ 12 ] == 'G' ) )
		      || ( ( rcv_msg->buf[ 11 ] == 'n' ) && ( rcv_msg->buf[ 12 ] == 'g' ) )
		      || ( ( rcv_msg->buf[ 11 ] == 'N' ) && ( rcv_msg->buf[ 12 ] == 'g' ) )
		      || ( ( rcv_msg->buf[ 11 ] == 'n' ) && ( rcv_msg->buf[ 12 ] == 'G' ) ) ){
				
		nop();					//　NG判定の受信処理
		if ( MdGetSubMode() == SUB_MD_DONGURU_CHK ){	// PCへドングルの有無確認の確認通信中なら
			s_DongleResult = DONGURU_JUDGE_NG;			// ドングルの有無確認の結果
			
			// ドングル無しの場合は、画面遷移処理はしない。
			if ( GetScreenNo() == LCD_SCREEN101 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN101 );	// 通常モード・メニュー画面へ。
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN101 );		// 画面番号　<-　次の画面
					
				} else {
					nop();			// エラー処理の記述
				}
			}			
			MdCngSubMode( SUB_MD_IDLE );	//　サブモード・ステータスを、”アイドル”へ。
			
		} else if ( MdGetSubMode() == SUB_MD_PASSWORD_CHK ){
			s_PasswordResult = PASSWORD_JUDGE_NG;		// パスワード確認の結果
			
			// パスワードNGの場合は、通常モード・メニュー画面へ
			if ( GetScreenNo() == LCD_SCREEN201 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// 通常モード・初期画面へ。
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN100 );		// 画面番号　<-　次の画面
					
				} else {
					nop();			// エラー処理の記述
				}
			}			
			MdCngSubMode( SUB_MD_IDLE );	//　サブモード・ステータスを、”アイドル”へ。
						
		} else if ( MdGetSubMode() == SUB_MD_KINKYU_TOUROKU ){
			s_KinkyuuTourokuResult = KINKYU_TOUROKU_JUDGE_NG;		// 緊急開錠番号登録送信の結果
			
			if ( GetScreenNo() == LCD_SCREEN164 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN161 );	// 通常モード・緊急開錠番号設定「指を入れて...」へ。
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN161 );		// 画面番号　<-　次の画面
					MdCngMode( MD_NORMAL );				// 装置モードを通常モードへ（不要のはずだが念のため）
				
				} else {
					nop();			// エラー処理の記述
				}
			}			
			MdCngSubMode( SUB_MD_IDLE );	//　サブモード・ステータスを、”アイドル”へ。
						
		} else if ( MdGetSubMode() == SUB_MD_KINKYU_KAIJYO_SEND ){	// 緊急開錠番号通知通信中
			
			if ( GetScreenNo() == LCD_SCREEN184 ){		// 「８桁番号を入力して下さい」画面なら
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN187 );	// 失敗×画面へ。
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN187 );		// 画面番号　<-　次の画面
					MdCngMode( MD_NORMAL );				// 装置モードを通常モードへ（不要のはずだが念のため）
				
				} else {
					nop();			// エラー処理の記述
				}
			}			
			MdCngSubMode( SUB_MD_IDLE );	//　サブモード・ステータスを、”アイドル”へ。
						
		} else if ( MdGetSubMode() == SUB_MD_KINKYU_NO_CHK_REQ ){	// 入力された緊急開錠番号妥当性確認の通信中
			
			if ( GetScreenNo() == LCD_SCREEN185 ){		// 「緊急番号を入力して下さい...」画面なら
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN187 );	// 開錠NGの×画面へ。
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN187 );		// 画面番号　<-　次の画面
					MdCngMode( MD_NORMAL );				// 装置モードを通常モードへ（不要のはずだが念のため）
				
				} else {
					nop();			// エラー処理の記述
				}
			}			
			MdCngSubMode( SUB_MD_IDLE );	//　サブモード・ステータスを、”アイドル”へ。
						
		} else if ( MdGetSubMode() == SUB_MD_CHG_NORMAL ){	// ノーマルモードへ移行中
						
			MdCngSubMode( SUB_MD_IDLE );	//　サブモード・ステータスを、”アイドル”へ。
						
		} else if ( MdGetSubMode() == SUB_MD_CHG_MAINTE ){	// メンテナンスモードへ移行中
						
			MdCngSubMode( SUB_MD_IDLE );	//　サブモード・ステータスを、”アイドル”へ。
						
		}
			
	}						  

	return ercd;
}

/*==========================================================================*/
//	緊急番号(８桁)表示データ受信処理（コマンド271の応答コマンド）
//	@return 処理の成功/失敗
/*==========================================================================*/
static UB rcv_cmd272( T_COMMSG *rcv_msg )			// コマンド272の処理
{
	UB ercd = E_OK;

	nop();
	
	if ( ( rcv_msg->buf[ 11 ] >= '0') && ( rcv_msg->buf[ 11 ] <= '9') ){
		kinkyuu_hyouji_no[0] = rcv_msg->buf[ 11 ];		// 緊急開錠の緊急番号８桁表示データ（配列最終番目は区切り記号　NUL　）
	} else {
		kinkyuu_hyouji_no[0] = ' ';
	}
	
	if ( ( rcv_msg->buf[ 12 ] >= '0') && ( rcv_msg->buf[ 12 ] <= '9') ){
		kinkyuu_hyouji_no[1] = rcv_msg->buf[ 12 ];
	} else {
		kinkyuu_hyouji_no[1] = ' ';
	}
	
	if ( ( rcv_msg->buf[ 13 ] >= '0') && ( rcv_msg->buf[ 13 ] <= '9') ){				
		kinkyuu_hyouji_no[2] = rcv_msg->buf[ 13 ];
	} else {
		kinkyuu_hyouji_no[2] = ' ';
	}		

	if ( ( rcv_msg->buf[ 14 ] >= '0') && ( rcv_msg->buf[ 14 ] <= '9') ){					
		kinkyuu_hyouji_no[3] = rcv_msg->buf[ 14 ];
	} else {
		kinkyuu_hyouji_no[3] = ' ';
	}

	if ( ( rcv_msg->buf[ 15 ] >= '0') && ( rcv_msg->buf[ 15 ] <= '9') ){
		kinkyuu_hyouji_no[4] = rcv_msg->buf[ 15 ];
	} else {
		kinkyuu_hyouji_no[4] = ' ';
	}

	if ( ( rcv_msg->buf[ 16 ] >= '0') && ( rcv_msg->buf[ 16 ] <= '9') ){			
		kinkyuu_hyouji_no[5] = rcv_msg->buf[ 16 ];
	} else {
		kinkyuu_hyouji_no[5] = ' ';
	}

	if ( ( rcv_msg->buf[ 17 ] >= '0') && ( rcv_msg->buf[ 17 ] <= '9') ){			
		kinkyuu_hyouji_no[6] = rcv_msg->buf[ 17 ];
	} else {
		kinkyuu_hyouji_no[6] = ' ';
	}
		
	if ( ( rcv_msg->buf[ 18 ] >= '0') && ( rcv_msg->buf[ 18 ] <= '9') ){			
		kinkyuu_hyouji_no[7] = rcv_msg->buf[ 18 ];
	} else {
		kinkyuu_hyouji_no[7] = ' ';
	}		
			
	kinkyuu_hyouji_no[8] = 0;
	
	if ( MdGetSubMode() == SUB_MD_KINKYU_8KETA_REQ ){	// 緊急８桁番号要求通信中なら、	
		if ( GetScreenNo() == LCD_SCREEN182 ){  	// 「ID番号を入力して下さい...」画面の時
		
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN183 );	// 「番号をコールセンターへ...」画面を、LCDタスクへ表示要求する。
			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN183 );		// 画面番号　<-　次の画面
				MdCngMode( MD_NORMAL );				// 装置モードを通常モードへ（不要のはずだが念のため）
			}
		}
		MdCngSubMode( SUB_MD_IDLE );	//　サブモード・ステータスを、”アイドル”へ。						  
	}
	
	return ercd;
}		

/*==========================================================================*/
//	受信コマンド"101"メンテナンス・モード移行処理
//	@return 処理の成功/失敗
/*==========================================================================*/
static UB rcv_cmd101( T_COMMSG *rcv_msg )			// コマンド101の処理
{
	UB ercd = E_OK;

	nop();
	
	MdCngMode( MD_MAINTE );					// 装置モードを、メンテナンス・モードへ設定
	
	MdCngSubMode( SUB_MD_IDLE );			// サブモードを、IDLEへ設定

	return ercd;
}

/*==========================================================================*/
/**
 *	パワーオンモード時受信コマンド処理メイン
 *	@return 解析成功/失敗
 */
/*==========================================================================*/
static UB rcv_cmd_proc_power_on( T_COMMSG *rcv_msg )
{
	UB ercd;
	
	nop();

	if ( ( rcv_msg->buf[ 4 ] == '0' )			// コマンド022なら、初期モード設定要求
	  && ( rcv_msg->buf[ 5 ] == '2' )
	  && ( rcv_msg->buf[ 6 ] == '2' ) ){
					  
		ercd = rcv_cmd022( rcv_msg );			// コマンド022の処理

		MdCngSubMode( SUB_MD_IDLE );			// サブモードを、IDLEへ設定
	
	} else if ( ( rcv_msg->buf[ 4 ] == '0' )	// コマンド012なら、カメラ・パラメータの初期値送信
	 		 && ( rcv_msg->buf[ 5 ] == '1' )
	  		 && ( rcv_msg->buf[ 6 ] == '2' ) ){
					  
		ercd = rcv_cmd012( rcv_msg );			// コマンド012の処理
		
		MdCngSubMode( SUB_MD_IDLE );			// サブモードを、IDLEへ設定
	
	} else if ( ( rcv_msg->buf[ 4 ] == '0' )	// コマンド015なら、画像処理の初期値送信
	 		 && ( rcv_msg->buf[ 5 ] == '1' )
	  		 && ( rcv_msg->buf[ 6 ] == '5' ) ){
					  
		ercd = rcv_cmd015( rcv_msg );			// コマンド015の処理
		
		MdCngSubMode( SUB_MD_IDLE );			// サブモードを、IDLEへ設定
	
	} else if ( ( rcv_msg->buf[ 4 ] == '0' )	// コマンド013なら、LED光量数値の初期値送信
	 		 && ( rcv_msg->buf[ 5 ] == '1' )
	  		 && ( rcv_msg->buf[ 6 ] == '3' ) ){
					  
		ercd = rcv_cmd013( rcv_msg );			// コマンド013の処理
		
		MdCngSubMode( SUB_MD_IDLE );			// サブモードを、IDLEへ設定
	
	} else if ( ( rcv_msg->buf[ 4 ] == '0' )	// コマンド020なら、登録情報の初期値送信
	 		 && ( rcv_msg->buf[ 5 ] == '2' )
	  		 && ( rcv_msg->buf[ 6 ] == '0' ) ){
					  
		ercd = rcv_cmd020( rcv_msg );			// コマンド020の処理
		
		MdCngSubMode( SUB_MD_IDLE );			// サブモードを、IDLEへ設定
		
//		MdCngMode( MD_NORMAL );					// 装置モードを通常モードへ

	
	} else if ( ( rcv_msg->buf[ 4 ] == '2' )	// コマンド201なら、通常モード切り替えの処理
	 		 && ( rcv_msg->buf[ 5 ] == '0' )
	  		 && ( rcv_msg->buf[ 6 ] == '1' ) ){
					  
		MdCngMode( MD_NORMAL );					// 通常モード切り替え
		
		MdCngSubMode( SUB_MD_IDLE );			// サブモードを、IDLEへ設定

	} else if ( ( rcv_msg->buf[ 4 ] == '0' )	// コマンド002なら、手順Aの場合のOK/NGの応答処理
	 		 && ( rcv_msg->buf[ 5 ] == '0' )
	  		 && ( rcv_msg->buf[ 6 ] == '2' ) ){
					  
		if ( MdGetSubMode() != SUB_MD_IDLE ){
			ercd = rcv_cmd002( rcv_msg );			// コマンド002の処理
		}
	}
	 
	 
	 return ercd;

}



/*==========================================================================*/
//	初期モード設定要求 処理（コマンド022の処理）
//	@return 処理成功/失敗
/*==========================================================================*/
static UB rcv_cmd022( T_COMMSG *rcv_msg )
{
	UB ercd = E_OK;
	
	nop();
	
	if ( rcv_msg->buf[ 11 ] == '0' ){
		
		switch  ( rcv_msg->buf[ 12 ] ){
		  	case '1':			// 初期登録モードへ
				MdCngMode( MD_INITIAL );	
				break;
			
		  	case '2':			// 通常モードへ
				MdCngMode( MD_NORMAL );	
				break;
			
		  	case '3':			// メンテナンス・モードへ
				MdCngMode( MD_MAINTE );	
				break;		

	  	  	case '4':			// 停電モードへ
				MdCngMode( MD_POWER_FAIL );	
				break;
		
		  	case '5':			// 非常時解錠モードへ
				MdCngMode( MD_PANIC );	
				break;
			
	      	default:
				ercd = -1;		// データ異常
				break;
		}

	} else {
		
		ercd = -1;		// データ異常
	}
	
	return ercd;
}

/*==========================================================================*/
//	カメラ・パラメータの初期値送信 処理（コマンド012の処理）
//	@return 処理成功/失敗
/*==========================================================================*/
static UB rcv_cmd012( T_COMMSG *rcv_msg ) 
{
	UB ercd = E_OK;
	
	nop();
	
	if ( rcv_msg->buf[ 13 ] == 0 ){		// 画像取得方法：マニュアル取得

		if ( ( rcv_msg->buf[ 14 ] >= 0 ) && ( rcv_msg->buf[ 14 ] <= 15 ) ){	//　カメラゲイン設定
			cmrGain = rcv_msg->buf[ 14 ];
			ini_cmrGain = rcv_msg->buf[ 14 ];					//　初期設定値を記憶。
			
			ercd = set_flg( ID_FLG_CAMERA, FPTN_SETREQ_GAIN );	// カメラタスクに、直接のゲイン設定値を設定を依頼（FPGAを経由しない）
		}
	
		if ( ( rcv_msg->buf[ 15 ] >= 0 ) && ( rcv_msg->buf[ 15 ] <= 15 ) ){	//　露出１設定
			cmrFixShutter1 = rcv_msg->buf[ 15 ];
			ini_cmrFixShutter1 = rcv_msg->buf[ 15 ];			//　初期設定値を記憶。
		}
	
		if ( ( rcv_msg->buf[ 16 ] >= 0 ) && ( rcv_msg->buf[ 16 ] <= 15 ) ){	//　露出２設定
			cmrFixShutter2 = rcv_msg->buf[ 16 ];
			ini_cmrFixShutter2 = rcv_msg->buf[ 16 ];			//　初期設定値を記憶。
		}
	
		if ( ( rcv_msg->buf[ 17 ] >= 0 ) && ( rcv_msg->buf[ 17 ] <= 15 ) ){	//　露出３設定
			cmrFixShutter3 = rcv_msg->buf[ 17 ];
			ini_cmrFixShutter3 = rcv_msg->buf[ 17 ];			//　初期設定値を記憶。

			ercd = set_flg( ID_FLG_CAMERA, FPTN_SETREQ_SHUT1 );	// カメラタスクに、直接の露出３設定値を設定を依頼（FPGAを経由しない）
		}
				
	} else if ( rcv_msg->buf[ 13 ] == 1 ){	// 画像取得方法：AES+マニュアル取得
	
	
	}	
	return ercd;
}


/*==========================================================================*/
//	画像処理の初期値送信 処理（コマンド015の処理）
//	@return 処理成功/失敗
/*==========================================================================*/
static UB rcv_cmd015( T_COMMSG *rcv_msg )
{
	UB ercd = E_OK;
	unsigned short tmp1, tmp2;
	unsigned short posX = 0, posY = 0;
	
	nop();
	
	// 	画像切り出しサイズを設定する
	switch  ( rcv_msg->buf[ 11 ] ){		// 画像切り出しサイズ指定
		  case '1':	
			iSizeX = 800;
			iSizeY = 400;
			break;
			
		  case '2':	
			iSizeX = 640;
			iSizeY = 400;
			break;
			
		 case '3':
			iSizeX = 640;
			iSizeY = 360;
			break;		

	  	  case '4':	
			iSizeX = 640;
			iSizeY = 320;
			break;
		
		  case '5':
			iSizeX = 640;
			iSizeY = 280;	
			break;
			
	      default:
			ercd = -1;		// データ異常
			break;
	}

	// 	トリミングの座標を設定する
	tmp1 = rcv_msg->buf[ 12 ];
	tmp1 = tmp1 << 8;
	posX = tmp1 + rcv_msg->buf[ 13 ]; //　X座標
	if ( ( posX < 0 ) || ( posX > 720 ) ){
		posX = 720;
	}
	
	tmp2 = rcv_msg->buf[ 14 ];
	tmp2 = tmp2 << 8;
	posY = tmp2 + rcv_msg->buf[ 15 ]; 
	if ( ( posY < 0) || ( posY > 720) ){
		posY = 720;
	}

	iStartX = posX;
	iStartY = posY;
	
	// リサイズの縮小率を設定する
	switch  ( rcv_msg->buf[ 16 ] ){		// 画像切り出しサイズ指定
		  case '1':	
			iReSizeMode = RSZ_MODE_0;	//< 辺1/2
			break;
			
		  case '2':	
			iReSizeMode = RSZ_MODE_1;	//< 辺1/4
			break;
			
		 case '3':
			iReSizeMode = RSZ_MODE_2;	//< 辺1/8
			break;		

	      default:
			ercd = -1;		// データ異常
			break;
	}	
		
	return ercd;
}


/*==========================================================================*/
//	LED光量数値の初期値送信 処理（コマンド013の処理）
//	@return 処理成功/失敗
/*==========================================================================*/
static UB rcv_cmd013( T_COMMSG *rcv_msg )
{
	UB ercd = E_OK;
	
	nop();

	
	irDuty2 = rcv_msg->buf[ 11 ];		// IR LEDの点灯初期値
	irDuty3 = rcv_msg->buf[ 12 ];
	irDuty4 = rcv_msg->buf[ 13 ];
	irDuty5 = rcv_msg->buf[ 14 ];
	
	ini_irDuty2 = rcv_msg->buf[ 11 ];		// IR LEDの点灯初期値を記憶
	ini_irDuty3 = rcv_msg->buf[ 12 ];
	ini_irDuty4 = rcv_msg->buf[ 13 ];
	ini_irDuty5 = rcv_msg->buf[ 14 ];
	
	return ercd;
}


/*==========================================================================*/
//	登録情報の初期値送信 処理（コマンド020の処理）
//	@return 処理成功/失敗
/*==========================================================================*/
static UB rcv_cmd020( T_COMMSG *rcv_msg )
{
	UB ercd = E_OK;
	int i, j, cnt;
	
	nop();
	
	cnt = 11;
	
	for ( i=0; i<17; i++ ){
		kinkyuu_tel_no[ i ] 			= rcv_msg->buf[ cnt++ ];		// 緊急開錠電話番号
	}

	yb_touroku_data.tou_no[ 0 ] 		= rcv_msg->buf[ cnt++ ];	// 棟番号
	yb_touroku_data.tou_no[ 1 ] 		= rcv_msg->buf[ cnt++ ];
	yb_touroku_data.tou_no[ 2 ]			= rcv_msg->buf[ cnt++ ];

	yb_touroku_data.user_id[ 0 ]		= rcv_msg->buf[ cnt++ ];	// ユーザーID
	yb_touroku_data.user_id[ 1 ]		= rcv_msg->buf[ cnt++ ];
	yb_touroku_data.user_id[ 2 ]		= rcv_msg->buf[ cnt++ ];
	yb_touroku_data.user_id[ 3 ]		= rcv_msg->buf[ cnt++ ];
	yb_touroku_data.user_id[ 4 ]		= rcv_msg->buf[ cnt++ ];

/**	
	yb_touroku_data.tou_no[ 0 ] 		= rcv_msg->buf[ 11 ];	// 棟番号
	yb_touroku_data.tou_no[ 1 ] 		= rcv_msg->buf[ 12 ];
	yb_touroku_data.tou_no[ 2 ]			= ',';

	yb_touroku_data.user_id[ 0 ]		= rcv_msg->buf[ 14 ];	// ユーザーID
	yb_touroku_data.user_id[ 1 ]		= rcv_msg->buf[ 15 ];
	yb_touroku_data.user_id[ 2 ]		= rcv_msg->buf[ 16 ];
	yb_touroku_data.user_id[ 3 ]		= rcv_msg->buf[ 17 ];
	yb_touroku_data.user_id[ 4 ]		= ',';

	yb_touroku_data.yubi_seq_no[ 0 ]	= rcv_msg->buf[ 19 ];	// 責任者/一般者の登録指情報
	yb_touroku_data.yubi_seq_no[ 1 ]	= rcv_msg->buf[ 20 ];
	yb_touroku_data.yubi_seq_no[ 2 ]	= rcv_msg->buf[ 21 ];
	yb_touroku_data.yubi_seq_no[ 3 ]	= ',';

	yb_touroku_data.kubun[ 0 ]			= rcv_msg->buf[ 23 ];	// 責任者/一般者区分、デホルトは”一般者”
	yb_touroku_data.kubun[ 1 ]			= ',';

	yb_touroku_data.yubi_no[ 0 ]		= rcv_msg->buf[ 25 ];	// 登録指番号（指種別）
	yb_touroku_data.yubi_no[ 1 ]		= rcv_msg->buf[ 26 ];
	yb_touroku_data.yubi_no[ 2 ]		= ',';


	for ( i=0; i<24; i++ ){
		yb_touroku_data.name[ i ] = rcv_msg->buf[ 28 + i ];	
	}
**/

//	cnt = 19;
	for( j = 0 ; j < 21 ; j++ ){	//　受信配列情報が、２０回分→２１回分に変更。2013.7.15
		yb_touroku_data20[j].yubi_seq_no[ 0 ]	= rcv_msg->buf[ cnt++ ];	// 責任者/一般者の登録指情報
		yb_touroku_data20[j].yubi_seq_no[ 1 ]	= rcv_msg->buf[ cnt++ ];
		yb_touroku_data20[j].yubi_seq_no[ 2 ]	= rcv_msg->buf[ cnt++ ];
		yb_touroku_data20[j].yubi_seq_no[ 3 ]	= rcv_msg->buf[ cnt++ ];

		yb_touroku_data20[j].kubun[ 0 ]			= rcv_msg->buf[ cnt++ ];	// 責任者/一般者区分、デホルトは”一般者”
		yb_touroku_data20[j].kubun[ 1 ]			= rcv_msg->buf[ cnt++ ];

		yb_touroku_data20[j].yubi_no[ 0 ]		= rcv_msg->buf[ cnt++ ];	// 登録指番号（指種別）
		yb_touroku_data20[j].yubi_no[ 1 ]		= rcv_msg->buf[ cnt++ ];
		yb_touroku_data20[j].yubi_no[ 2 ]		= rcv_msg->buf[ cnt++ ];
		
		for ( i=0; i<24; i++ ){
			yb_touroku_data20[j].name[ i ] = rcv_msg->buf[ cnt++ ];	
		}
		++cnt;
	}

	// 2013.7.15　の仕様変更により、受信した登録データのうち、２番目以後の配列情報から有効とすることになった。
	yb_touroku_data.yubi_seq_no[ 0 ]	= yb_touroku_data20[ 1 ].yubi_seq_no[ 0 ];	// 責任者/一般者の登録指情報
	yb_touroku_data.yubi_seq_no[ 1 ]	= yb_touroku_data20[ 1 ].yubi_seq_no[ 1 ];
	yb_touroku_data.yubi_seq_no[ 2 ]	= yb_touroku_data20[ 1 ].yubi_seq_no[ 2 ];
	yb_touroku_data.yubi_seq_no[ 3 ]	= ',';

	yb_touroku_data.kubun[ 0 ]			= yb_touroku_data20[ 1 ].kubun[ 0 ];	// 責任者/一般者区分、デホルトは”一般者”
	yb_touroku_data.kubun[ 1 ]			= ',';

	yb_touroku_data.yubi_no[ 0 ]		= yb_touroku_data20[ 1 ].yubi_no[ 0 ];	// 登録指番号（指種別）
	yb_touroku_data.yubi_no[ 1 ]		= yb_touroku_data20[ 1 ].yubi_no[ 1 ];
	yb_touroku_data.yubi_no[ 2 ]		= ',';


	for ( i=0; i<24; i++ ){
		yb_touroku_data.name[ i ] = yb_touroku_data20[ 1 ].name[ i ];	
	}


	
	return ercd;
}


/*==========================================================================*/
/**
 *	初期登録時受信コマンド処理メイン
 *	@return 解析成功/失敗
 */
/*==========================================================================*/
static UB rcv_cmd_proc_init( T_COMMSG *rcv_msg )
{
	UB ercd;
	
	nop();

	if ( ( rcv_msg->buf[ 4 ] == '2' )			// コマンド211なら、指登録のOK/NGの判定
	  && ( rcv_msg->buf[ 5 ] == '1' )
	  && ( rcv_msg->buf[ 6 ] == '1' ) ){
					  
		ercd = rcv_cmd211( rcv_msg );			// コマンド211の処理
	
//	} else if ( ( rcv_msg->buf[ 4 ] == '2' )	// コマンド205なら、指認証のOK/NGの判定
//	 		 && ( rcv_msg->buf[ 5 ] == '0' )
//	  		 && ( rcv_msg->buf[ 6 ] == '5' ) ){
					  
//		ercd = rcv_cmd205( rcv_msg );			// コマンド205の処理
	
	} else if ( ( rcv_msg->buf[ 4 ] == '2' )	// コマンド201なら、通常モード切り替えの処理
	 		 && ( rcv_msg->buf[ 5 ] == '0' )
	  		 && ( rcv_msg->buf[ 6 ] == '1' ) ){
					  
		MdCngMode( MD_NORMAL );					// 通常モード切り替え
		
		MdCngSubMode( SUB_MD_IDLE );			// サブモードを、IDLEへ設定
	
	} else if ( ( rcv_msg->buf[ 4 ] == '2' )	// コマンド206なら、指データの再撮影処理
	 		 && ( rcv_msg->buf[ 5 ] == '0' )
	  		 && ( rcv_msg->buf[ 6 ] == '6' ) ){
					  
		ercd = rcv_cmd206( rcv_msg );			// コマンド206の処理
		
	} else if ( ( rcv_msg->buf[ 4 ] == '2' )	// コマンド207なら、カメラ・パラメータの変更処理
	 		 && ( rcv_msg->buf[ 5 ] == '0' )
	  		 && ( rcv_msg->buf[ 6 ] == '7' ) ){
					  
		ercd = rcv_cmd207( rcv_msg );			// コマンド207の処理
		
	} else if ( ( rcv_msg->buf[ 4 ] == '2' )	// コマンド208なら、LED光量の変更処理
	 		 && ( rcv_msg->buf[ 5 ] == '0' )
	  		 && ( rcv_msg->buf[ 6 ] == '8' ) ){
					  
		ercd = rcv_cmd208( rcv_msg );			// コマンド208の処理
		
	} else if ( ( rcv_msg->buf[ 4 ] == '0' )	// コマンド002なら、手順Aの場合のOK/NGの応答処理
	 		 && ( rcv_msg->buf[ 5 ] == '0' )
	  		 && ( rcv_msg->buf[ 6 ] == '2' ) ){
					  
		if ( MdGetSubMode() != SUB_MD_IDLE ){
			ercd = rcv_cmd002( rcv_msg );			// コマンド002の処理
		}

	}

	return ercd;
}


/*==========================================================================*/
/**
 *	メンテナンスモード時受信コマンド処理メイン
 *	@return 解析成功/失敗
 */
/*==========================================================================*/
static UB rcv_cmd_proc_meinte( T_COMMSG *rcv_msg )
{
	UB ercd;
	
	nop();
	
	if ( ( rcv_msg->buf[ 4 ] == '1' )			// コマンド101なら、メンテナンス・モード切り替え処理
	  && ( rcv_msg->buf[ 5 ] == '0' )
	  && ( rcv_msg->buf[ 6 ] == '1' ) ){
	  
		MdCngMode( MD_MAINTE );					// メンテナンス・モード切り替え
		
		MdCngSubMode( SUB_MD_IDLE );			// サブモードを、IDLEへ設定
		
	
	} else if ( ( rcv_msg->buf[ 4 ] == '2' )	// コマンド201なら、通常モード切り替えの処理
	 		 && ( rcv_msg->buf[ 5 ] == '0' )
	  		 && ( rcv_msg->buf[ 6 ] == '1' ) ){
					  
		MdCngMode( MD_NORMAL );					// 通常モード切り替え
		
		MdCngSubMode( SUB_MD_IDLE );			// サブモードを、IDLEへ設定
		
	} else if ( ( rcv_msg->buf[ 4 ] == '2' )	// コマンド211なら、指フル画像送信結果のOK/NGの判定
	  		 && ( rcv_msg->buf[ 5 ] == '1' )	// コマンド211/コマンド205のどちらで返信があるか分からない為。
	  		 && ( rcv_msg->buf[ 6 ] == '1' ) ){
					  
		ercd = rcv_cmd211( rcv_msg );			// コマンド211の処理
	
	} else if ( ( rcv_msg->buf[ 4 ] == '2' )	// コマンド205なら、指フル画像送信結果のOK/NGの判定
	 		 && ( rcv_msg->buf[ 5 ] == '0' )	// コマンド211/コマンド205のどちらで返信があるか分からない為。
	  		 && ( rcv_msg->buf[ 6 ] == '5' ) ){
					  
		ercd = rcv_cmd205( rcv_msg );			// コマンド205の処理

	} else if ( ( rcv_msg->buf[ 4 ] == '0' )	// コマンド002なら、手順Aの場合のOK/NGの応答処理
	 		 && ( rcv_msg->buf[ 5 ] == '0' )
	  		 && ( rcv_msg->buf[ 6 ] == '2' ) ){
					  
		if ( MdGetSubMode() != SUB_MD_IDLE ){
			ercd = rcv_cmd002( rcv_msg );			// コマンド002の処理
		}

	}
	 
	return ercd;
}

/*==========================================================================*/
/**
 *	パワーオフモード時受信コマンド処理メイン
 *	@return 解析成功/失敗
 */
/*==========================================================================*/
static UB rcv_cmd_proc_power_off( T_COMMSG *rcv_msg )
{
	UB ercd;
	
	nop();

	if ( ( rcv_msg->buf[ 4 ] == '0' )	// コマンド002なら、手順Aの場合のOK/NGの応答処理
	 		 && ( rcv_msg->buf[ 5 ] == '0' )
	  		 && ( rcv_msg->buf[ 6 ] == '2' ) ){
					  
		if ( MdGetSubMode() != SUB_MD_IDLE ){
			ercd = rcv_cmd002( rcv_msg );			// コマンド002の処理
		}

	}

	return ercd;
}


/*==========================================================================*/
/**
 *	マシン状態を返す（モード参照）
 *	@return マシンの状態
 */
/*==========================================================================*/
static UB MdGetMode(void)
{
	return sys_Mode;
}

/*==========================================================================*/
/**
 *	モード遷移
 *	@param eNextMode 次の状態
 */
/*==========================================================================*/
static void MdCngMode( UINT eNextMode )
{
	// モード変更の前処理
	switch ( sys_Mode ){
	  case MD_POWER_ON:

	  	break;
	  case MD_NORMAL:				// 通常モード

		break;
	}
	
	// 次のモード設定
	sys_Mode = eNextMode;				
	
	// モード変更の後処理
	switch ( sys_Mode ){
	  case MD_INITIAL:				// 初期登録モード
		break;		

	  case MD_MAINTE:				// メンテナンスモード
		break;
		
	  case MD_NORMAL:				// 通常モード
		break;
	}
	
	// LED表示
	LedOut( LED_ERR, LED_OFF );
	switch ( sys_Mode ) {
	case MD_PANIC:
		LedOut( LED_ERR,  LED_ON );
		break;
	}
}


/*==========================================================================*/
/**
 *	マシンサブ状態を返す（モード参照）
 *	@return マシンの状態
 */
/*==========================================================================*/
static UB MdGetSubMode(void)
{
	return sys_SubMode;
}

/*==========================================================================*/
/**
 *	サブモード遷移
 *	@param eNextMode 次の状態
 */
/*==========================================================================*/
static void MdCngSubMode( UINT eNextMode )
{
	// 次のモード設定
	sys_SubMode = eNextMode;				
}

