/**
	VA-300プログラム
	@file Next_screen_ctrl.c
	@version 1.00
	
	@author Bionics Co.Ltd　T.N
	@date 2012/07/31
	@brief LCD画面制御メイン関数
*/

#include "id.h"
#include "udp.h"
#include "net_prm.h"
#include "va300.h"
#include "err_ctrl.h"
#include "drv_eep.h"
#include "drv_led.h"

ER NextScrn_Control_mantion( void );	// 次の画面コントロール（マンション・占有部仕様の場合）
ER NextScrn_Control_office( void );		// 次の画面コントロール（１対１仕様の場合）
ER NextScrn_Control( void );			// 次の画面コントロール（共通仕様の場合）
static UB Chk_shutdown_ok( void );		// 認証操作中かどうかをチェックする
static ER Pfail_shutdown( void );		// 停電モード、停電検知通知受信の場合の、シャットダウン実行

int dbg_Auth_8cnt;

/*==========================================================================*/
/**
 *	次の画面表示要求フラグセット（１対１仕様の場合）
 */
/*==========================================================================*/
ER NextScrn_Control_office( void )
{
	ER ercd;
	UB S_No;
	ER_UINT i;
	ER_UINT rcv_length;
	UB msg[ 128 ];	
	
	S_No = GetScreenNo();
	
	switch ( S_No ) {
		
		
	//　初期登録モード
		case LCD_SCREEN401:		// 初期登録 or メンテナンスボタン選択画面

			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
						
			if ( ( msg[ 0 ] == LCD_INIT_INPUT ) && ( rcv_length >= 1 ) ){	// 	初期登録ボタンが押された
				
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN402 );	// 責任者番号　選択画面へ。	
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN402 );		// 画面番号　<-　次の画面
				}
				
			} else {		// メンテナンス・ボタン押下は無視する。
				nop();	
			}
			break;
			
		case LCD_SCREEN402:		// 初期登録時のID入力画面。
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( ( msg[ 0 ] == LCD_NEXT ) || ( msg[ 0 ] == LCD_USER_ID ) && ( rcv_length >= 5 ) ){
				
				yb_touroku_data.user_id[ 0 ] = msg[ 1 ];	// ユーザーID
				yb_touroku_data.user_id[ 1 ] = msg[ 2 ];
				yb_touroku_data.user_id[ 2 ] = msg[ 3 ];
				yb_touroku_data.user_id[ 3 ] = msg[ 4 ];
				yb_touroku_data.user_id[ 4 ] = ',';
				
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN403 );	// 名前入力画面へ。	
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN403 );		// 画面番号　<-　次の画面
				}
			} else {
				nop();	
			}
			break;

		case LCD_SCREEN403:		// 氏名　入力画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( ( msg[ 0 ] == LCD_NAME ) && ( rcv_length >= 25 ) ){	

				for(i = 0 ; i < 24 ; i++ ){					// 氏名
					yb_touroku_data.name[ i ] = msg[ 1 + i ];
				}
				
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN404 );	// 	指種別画面へ。
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN404 );		// 画面番号　<-　次の画面
				}
				
			} else if ( ( msg[ 0 ] == LCD_CANCEL ) && ( rcv_length >= 1 ) ){
			
				befor_scrn_no = FPTN_LCD_SCREEN403;
				befor_scrn_for_ctrl = LCD_SCREEN403;
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN411 );	// 	「中止しても..」画面へ。
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN411 );	// 画面番号　<-　次の画面
				}	
								
			} else {
				nop();	
			}
			break;
			

		case LCD_SCREEN404:		// 指種別選択画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( ( msg[ 0 ] == LCD_YUBI_SHUBETU ) && ( rcv_length >= 3 ) ){	
			
				yb_touroku_data.yubi_no[ 0 ] = msg[ 1 ];	// 指種別
				yb_touroku_data.yubi_no[ 1 ] = msg[ 2 ];
				yb_touroku_data.yubi_no[ 2 ] = ',';
				
				yb_touroku_data.kubun[ 0 ] = '0';		// 責任者・一般者区分に、監督者をSet。
				yb_touroku_data.kubun[ 1 ] = ',';
				
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN405 );	// 	「指をセットして..」画面へ。
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN405 );		// 画面番号　<-　次の画面
				}
				
			} else if ( ( msg[ 0 ] == LCD_CANCEL ) && ( rcv_length >= 1 ) ){
			
				befor_scrn_no = FPTN_LCD_SCREEN404;
				befor_scrn_for_ctrl = LCD_SCREEN404;
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN411 );	// 	「中止しても..」画面へ。
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN411 );	// 画面番号　<-　次の画面
				}	
								
			} else {
				nop();	
			}
			break;			
		
		case LCD_SCREEN405:		// 「指をセットして..」画面。
			break;				// 登録の生体センサー検知ON待ち。LCDタスクは、ここへのchange Reqは出さないはず。

		case LCD_SCREEN406:		// 「指を抜いて下さい..」画面。
			break;				// 登録の生体センサー検知OFF待ち。LCDタスクは、ここへのchange Reqは出さないはず。

		case LCD_SCREEN407:		// 「もう一度指をセットして..」画面。
			break;				// 登録の生体センサー検知待ち。LCDタスクは、ここへのchange Reqは出さないはず。

		case LCD_SCREEN408:		// 登録完了○画面。
			
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN410 );	// 「登録を続けますか」画面へ。
			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN410 );		// 画面番号　<-　次の画面
			}		
			break;		

		case LCD_SCREEN409:		// 登録失敗×画面。
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN404 );	// ID番号入力画面へ。
			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN404 );		// 画面番号　<-　次の画面
			}
			break;		

		case LCD_SCREEN410:		// 登録続行の「はい」「いいえ」選択画面。
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_YES ){			// はい
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN402 );	// 責任者番号選択画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN402 );		// 画面番号　<-　次の画面
					}
				} else if ( msg[ 0 ] == LCD_NO ){	// いいえ
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN500 );	// 通常画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN500 );		// 画面番号　<-　次の画面
						MdCngMode( MD_NORMAL );				// 装置モードを通常モードへ
#if ( VA300S == 0 )
						ercd = SndCmdCngMode( (UINT)MD_NORMAL );	// PCへ	通常モード切替え通知を送信
						if ( ercd != E_OK ){
							nop();		// エラー処理の記述	
						}
#endif
					}
				} else {
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN410 );	// 「登録を続けますか」画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN410 );		// 画面番号　<-　次の画面
					}		
				}					
			}
			break;		

		case LCD_SCREEN411:		// 中止の「はい」「いいえ」選択画面。
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_YES ){			// はい
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN401 );	// 初期登録メニュー画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN401 );		// 画面番号　<-　次の画面
					}
				} else if ( msg[ 0 ] == LCD_NO ){	// いいえ
					ercd = set_flg( ID_FLG_LCD, ( FLGPTN )befor_scrn_no );	// 通常画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( ( UB )befor_scrn_for_ctrl );		// 画面番号　<-　次の画面
					}
				}					
			}			
			break;		


		
	// 通常モード				
		case LCD_SCREEN500:		// ブランク画面。
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN501 );	// 通常モードの待機画面
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN501 );		// 画面番号　<-　次の画面
				}
			}	
			break;
		
		case LCD_SCREEN501:		// 通常モードの待機画面(ID番号入力画面)
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( ( msg[ 0 ] == LCD_NEXT ) || ( msg[ 0 ] == LCD_USER_ID ) && ( rcv_length >= 5 ) ){
				
				yb_touroku_data.user_id[ 0 ] = msg[ 1 ];	// ユーザーID
				yb_touroku_data.user_id[ 1 ] = msg[ 2 ];
				yb_touroku_data.user_id[ 2 ] = msg[ 3 ];
				yb_touroku_data.user_id[ 3 ] = msg[ 4 ];
				yb_touroku_data.user_id[ 4 ] = ',';
				
#if ( VA300S == 0 )						
				send_ID_No_check_req_Wait_Ack_Retry();	// PCへ、ID番号問合わせを送信。	
#endif										
				// 画面移行は、ID番号問合わせ（コマンド214）受信処理内で、受信した応答OK/NGに従って実行する。
				
			} else if ( msg[ 0 ] == LCD_MENU ){
				
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN520 );	// 通常モードのメニュー待機画面へ。
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN520 );		// 画面番号　<-　次の画面
					if ( Pfail_mode_count == 0 ){ 
						MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ（必要無いが念のため）
					}	else	{
						MdCngMode( MD_PFAIL );			// 装置モードを	停電モードへ（必要無いが念のため）
					}
				}

			} else {
				nop();	
			}
			break;
		
		case LCD_SCREEN502:		// 通常モードの待機画面のブランク表示
			break;				// LCDタスクは、ここへのchange Reqは出さないはず。
			
		case LCD_SCREEN503:		// 「指をセットして..」画面。
			break;				// 登録の生体センサー検知ON待ち。LCDタスクは、ここへのchange Reqは出さないはず。

		case LCD_SCREEN504:		// 認証完了○画面。
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );	
			
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN501 );	// 通常モードの待機画面へ。
			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN501 );		// 画面番号　<-　次の画面
				if ( Pfail_mode_count == 0 ){ 
					MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ（必要無いが念のため）
				}	else	{
					MdCngMode( MD_PFAIL );			// 装置モードを	停電モードへ（必要無いが念のため）
				}
			}
			break;
		
		case LCD_SCREEN505:		// 認証完了×画面。
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
		
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN501 );	// 通常モードのメニュー画面へ。
			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN501 );		// 画面番号　<-　次の画面
				if ( Pfail_mode_count == 0 ){ 
					MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ（必要無いが念のため）
				}	else	{
					MdCngMode( MD_PFAIL );			// 装置モードを	停電モードへ（必要無いが念のため）
				}
			}		
			break;


		case LCD_SCREEN506:		// 「ID番号が登録されていません」画面。
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
		
			if ( msg[ 0 ] == LCD_OK ){
				
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN501 );	// 通常モードのメニュー待機画面へ。
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN501 );		// 画面番号　<-　次の画面
				if ( Pfail_mode_count == 0 ){ 
					MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ（必要無いが念のため）
				}	else	{
					MdCngMode( MD_PFAIL );			// 装置モードを	停電モードへ（必要無いが念のため）
				}
				}
			}
			break;

	// 通常モード・登録
		case LCD_SCREEN520:		// 通常モード・登録のブランク画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN521 );	// 通常モードのメニュー画面
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN521 );		// 画面番号　<-　次の画面
				}
			}			
			break;

		case LCD_SCREEN521:		// 通常モードのメニュー画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_TOUROKU ){				// 登録ボタン押下
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN522 );	// 通常モード登録・権限者ID入力画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN522 );		// 画面番号　<-　次の画面
					}
				} else if ( msg[ 0 ] == LCD_SAKUJYO ){		// 削除ボタン押下
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN543 );	// 通常モード削除画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN543 );		// 画面番号　<-　次の画面
					}													
				} else if ( msg[ 0 ] == LCD_MAINTE ){		// メンテナンス・ボタン押下

#if ( VA300S == 0 )						
					send_donguru_chk_Wait_Ack_Retry();	// PCへ、ドングルの有無確認を送信。						
#else
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN200 );	// メンテナンスモード画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN200 );		// 画面番号　<-　次の画面
					} else {
						nop();			// エラー処理の記述
					}
#endif					
					// メンテナンスモード画面移行は、コマンド002受信処理内で、OK受信した場合に実行する。

				} else if ( msg[ 0 ] == LCD_CANCEL ){	// 終了・ボタン押下
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN538 );	// 「中止しますか？」画面へ。
					if ( ercd == E_OK ){
						befor_scrn_no = FPTN_LCD_SCREEN521;
						befor_scrn_for_ctrl = LCD_SCREEN521;
						ChgScreenNo( LCD_SCREEN538 );		// 画面番号　<-　次の画面
					}					
				} else {
					
				}
			}
			break;
			
		case LCD_SCREEN522:		// 登録時の権限者ID入力画面。
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( ( msg[ 0 ] == LCD_NEXT ) || ( msg[ 0 ] == LCD_USER_ID ) && ( rcv_length >= 5 ) ){
				
				yb_touroku_data.user_id[ 0 ] = msg[ 1 ];	// ユーザーID
				yb_touroku_data.user_id[ 1 ] = msg[ 2 ];
				yb_touroku_data.user_id[ 2 ] = msg[ 3 ];
				yb_touroku_data.user_id[ 3 ] = msg[ 4 ];
				yb_touroku_data.user_id[ 4 ] = ',';
#if ( VA300S == 0 ) 				
				send_ID_Authority_check_req_Wait_Ack_Retry();	// PCへ、ID権限問合わせを送信。	
#endif										
				// 画面移行は、ID権限問合わせ（コマンド215）受信処理内で、受信した応答OK/NG/CNに従って実行する。

			} else if ( msg[ 0 ] == LCD_CANCEL ){			// 中止ボタン押下
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN538 );	// 「中止しますか？」画面へ。
				if ( ercd == E_OK ){
					befor_scrn_no = FPTN_LCD_SCREEN522;
					befor_scrn_for_ctrl = LCD_SCREEN522;
					ChgScreenNo( LCD_SCREEN538 );		// 画面番号　<-　次の画面
				}					
			} else {
				nop();	
			}
			break;
			
			
		case LCD_SCREEN523:		// 「指をセットして..」画面。
			break;				// 登録の生体センサー検知ON待ち。LCDタスクは、ここへのchange Reqは出さないはず。

		case LCD_SCREEN524:		// 認証完了○画面。
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );	
			
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN526 );	// 登録モードのID番号入力画面へ。
			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN526 );		// 画面番号　<-　次の画面
				MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ（必要無いが念のため）
			}
			break;
		
		case LCD_SCREEN525:		// 認証完了×画面。
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
		
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN522 );	// 登録時の権限者ID入力画面へ。
			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN522 );		// 画面番号　<-　次の画面
				MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ（必要無いが念のため）
			}		
			break;

		case LCD_SCREEN526:		// 登録の場合の責任者選択画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_KANTOKU ){				// 監督者ボタン押下
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN527 );	// 権限者ID入力画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN527 );		// 画面番号　<-　次の画面
						
						yb_touroku_data.kubun[ 0 ] = '0';		// 「責任者区分」に監督をSet。
						yb_touroku_data.kubun[ 1 ] = ',';
					}

				} else if ( msg[ 0 ] == LCD_KANRI ){		// 管理者ボタン押下
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN527 );	// 権限者ID入力画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN527 );		// 画面番号　<-　次の画面
						
						yb_touroku_data.kubun[ 0 ] = '1';		// 「責任者区分」に管理者をSet。
						yb_touroku_data.kubun[ 1 ] = ',';
					}
									
				} else if ( msg[ 0 ] == LCD_IPPAN ){		// 一般者ボタン押下				
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN527 );	// 権限者ID入力画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN527 );		// 画面番号　<-　次の画面
						
						yb_touroku_data.kubun[ 0 ] = '2';		// 「責任者区分」に一般者をSet。
						yb_touroku_data.kubun[ 1 ] = ',';
					}

				} else if ( msg[ 0 ] == LCD_CANCEL ){		// 終了・ボタン押下
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN538 );	// 「中止しますか？」画面へ。
					if ( ercd == E_OK ){
						befor_scrn_no = FPTN_LCD_SCREEN526;
						befor_scrn_for_ctrl = LCD_SCREEN526;
						ChgScreenNo( LCD_SCREEN538 );		// 画面番号　<-　次の画面
					}					
				} else {
					nop();
				}
			}
			break;

		case LCD_SCREEN527:		// 登録用のID入力画面。
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( ( msg[ 0 ] == LCD_NEXT ) || ( msg[ 0 ] == LCD_USER_ID ) && ( rcv_length >= 5 ) ){
				
				yb_touroku_data.user_id[ 0 ] = msg[ 1 ];	// ユーザーID
				yb_touroku_data.user_id[ 1 ] = msg[ 2 ];
				yb_touroku_data.user_id[ 2 ] = msg[ 3 ];
				yb_touroku_data.user_id[ 3 ] = msg[ 4 ];
				yb_touroku_data.user_id[ 4 ] = ',';
				
#if ( VA300S == 0 )				
				send_ID_No_check_req_Wait_Ack_Retry();	// PCへ、ID番号問合わせを送信。	
#endif										
				// 画面移行は、ID番号問合わせ（コマンド214）受信処理内で、受信した応答OK/NGに従って実行する。

			} else if ( msg[ 0 ] == LCD_CANCEL ){			// 中止ボタン押下
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN538 );	// 「中止しますか？」画面へ。
				if ( ercd == E_OK ){
					befor_scrn_no = FPTN_LCD_SCREEN527;
					befor_scrn_for_ctrl = LCD_SCREEN527;
					ChgScreenNo( LCD_SCREEN538 );		// 画面番号　<-　次の画面
				}					
			} else {
				nop();	
			}
			break;
			
			
		case LCD_SCREEN528:		// 名前入力画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( ( msg[ 0 ] == LCD_NAME ) && ( rcv_length >= 25 ) ){	

				for(i = 0 ; i < 24 ; i++ ){					// 氏名
					yb_touroku_data.name[ i ] = msg[ 1 + i ];
				}
				
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN529 );	// 	指種別画面へ。
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN529 );		// 画面番号　<-　次の画面
				}
				
			} else if ( ( msg[ 0 ] == LCD_CANCEL ) && ( rcv_length >= 1 ) ){
			
				befor_scrn_no = FPTN_LCD_SCREEN528;
				befor_scrn_for_ctrl = LCD_SCREEN528;
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN538 );	// 	「中止しても..」画面へ。
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN538 );	// 画面番号　<-　次の画面
				}	
								
			} else {
				nop();	
			}
			break;
			
		case LCD_SCREEN529:		// 指種別選択画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( ( msg[ 0 ] == LCD_YUBI_SHUBETU ) && ( rcv_length >= 3 ) ){	
			
				yb_touroku_data.yubi_no[ 0 ] = msg[ 1 ];	// 指種別
				yb_touroku_data.yubi_no[ 1 ] = msg[ 2 ];
				yb_touroku_data.yubi_no[ 2 ] = ',';
				
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN530 );	// 	「指をセットして..」画面へ。
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN530 );		// 画面番号　<-　次の画面
				}
				
			} else if ( ( msg[ 0 ] == LCD_CANCEL ) && ( rcv_length >= 1 ) ){
			
				befor_scrn_no = FPTN_LCD_SCREEN529;
				befor_scrn_for_ctrl = LCD_SCREEN529;
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN538 );	// 	「中止しても..」画面へ。
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN538 );	// 画面番号　<-　次の画面
				}	
								
			} else {
				nop();	
			}
			break;
			
		case LCD_SCREEN530:		// 「指をセットして...」画面
			break;				// 生体センサー検知待ち。LCDタスクは、ここへのchange Reqは出さないはず。
			
		case LCD_SCREEN531:		// 「指を抜いて下さい」画面
			break;				// 生体センサー検知待ち。LCDタスクは、ここへのchange Reqは出さないはず。
				
		case LCD_SCREEN532:		// 「もう一度、指を...」画面
			break;				// 生体センサー検知待ち。LCDタスクは、ここへのchange Reqは出さないはず。
			
		case LCD_SCREEN533:		// 登録成功○画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN526);	// 責任者・一般者の登録番号選択画面へ
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN526 );		// 画面番号　<-　次の画面
				}
			}	
			break;
			
		case LCD_SCREEN534:		// 登録失敗×画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN529);	// 指種別選択画面へ
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN529 );		// 画面番号　<-　次の画面
				}
			}
			break;
		
		case LCD_SCREEN535:		// 「ID番号が登録されています」画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_OK ){				// 確認ボタンが押下された
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN527);	// 登録ID番号入力画面へ
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN527 );		// 画面番号　<-　次の画面
					}
				}
			}
			break;
		
		case LCD_SCREEN536:		// 「ID番号が登録されていません」画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_OK ){				// 確認ボタンが押下された
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN522 );	// 登録ID番号入力画面へ
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN522 );		// 画面番号　<-　次の画面
					}
				}
			}
			break;
		
		case LCD_SCREEN537:		// 「操作する権限がありません」画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_OK ){					// 確認ボタンが押下された
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN522 );	// 権限ID番号入力画面へ
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN522 );		// 画面番号　<-　次の画面
					}
				}
			}
			break;
			
		case LCD_SCREEN538:		// 中止の「はい」「いいえ」選択画面。
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_YES ){				// はい
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN500 );	// ユーザーID番号選択画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN500 );	// 画面番号　<-　次の画面
					}
					
					yb_touroku_data.kubun[ 0 ] = '2';		// 「責任者区分」に一般者をSet。（デフォルト）
					yb_touroku_data.kubun[ 1 ] = ',';
						
				} else if ( msg[ 0 ] == LCD_NO ){		// いいえ
					ercd = set_flg( ID_FLG_LCD, ( FLGPTN )befor_scrn_no );	// 元の画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( ( UB )befor_scrn_for_ctrl );		// 画面番号　<-　次の画面
					}

				}					
			}			
			break;

			
	// 通常モード・削除
		case LCD_SCREEN543:		// 登録時の権限者ID入力画面。
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( ( msg[ 0 ] == LCD_NEXT ) || ( msg[ 0 ] == LCD_USER_ID ) && ( rcv_length >= 5 ) ){
				
				yb_touroku_data.user_id[ 0 ] = msg[ 1 ];	// ユーザーID
				yb_touroku_data.user_id[ 1 ] = msg[ 2 ];
				yb_touroku_data.user_id[ 2 ] = msg[ 3 ];
				yb_touroku_data.user_id[ 3 ] = msg[ 4 ];
				yb_touroku_data.user_id[ 4 ] = ',';
				
#if ( VA300S == 0 )				
				send_ID_Authority_check_req_Wait_Ack_Retry();	// PCへ、ID権限問合わせを送信。	
#endif										
				// 画面移行は、ID権限問合わせ（コマンド215）受信処理内で、受信した応答OK/NG/CNに従って実行する。

			} else if ( msg[ 0 ] == LCD_CANCEL ){			// 中止ボタン押下
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN551 );	// 「中止しますか？」画面へ。
				if ( ercd == E_OK ){
					befor_scrn_no = FPTN_LCD_SCREEN543;
					befor_scrn_for_ctrl = LCD_SCREEN543;
					ChgScreenNo( LCD_SCREEN551 );		// 画面番号　<-　次の画面
				}					
			} else {
				nop();	
			}
			break;

			
		case LCD_SCREEN544:		// 「指をセットして..」画面。
			break;				// 削除の生体センサー検知ON待ち。LCDタスクは、ここへのchange Reqは出さないはず。

		case LCD_SCREEN545:		// 認証完了○画面。
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );	
			
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN547 );	// 登録モードのID番号入力画面へ。
			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN547 );		// 画面番号　<-　次の画面
				MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ（必要無いが念のため）
			}
			break;
		
		case LCD_SCREEN546:		// 認証完了×画面。
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
		
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN543 );	// 登録時の権限者ID入力画面へ。
			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN543 );		// 画面番号　<-　次の画面
				MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ（必要無いが念のため）
			}		
			break;

		case LCD_SCREEN547:		// 削除用のID入力画面。
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( ( msg[ 0 ] == LCD_NEXT ) || ( msg[ 0 ] == LCD_USER_ID ) && ( rcv_length >= 5 ) ){
				
				yb_touroku_data.user_id[ 0 ] = msg[ 1 ];	// ユーザーID
				yb_touroku_data.user_id[ 1 ] = msg[ 2 ];
				yb_touroku_data.user_id[ 2 ] = msg[ 3 ];
				yb_touroku_data.user_id[ 3 ] = msg[ 4 ];
				yb_touroku_data.user_id[ 4 ] = ',';
				
#if ( VA300S == 0 )				
				send_ID_Authority_check_req_Wait_Ack_Retry();	// PCへ、ID権限問合わせを送信。	
#endif										
				// 画面移行は、ID権限問合わせ（コマンド215）受信処理内で、受信した応答OK/NGに従って実行する。

			} else if ( msg[ 0 ] == LCD_CANCEL ){			// 中止ボタン押下
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN551 );	// 「中止しますか？」画面へ。
				if ( ercd == E_OK ){
					befor_scrn_no = FPTN_LCD_SCREEN547;
					befor_scrn_for_ctrl = LCD_SCREEN547;
					ChgScreenNo( LCD_SCREEN551 );		// 画面番号　<-　次の画面
				}					
			} else {
				nop();	
			}
			break;
	
		
		case LCD_SCREEN548:		// 「削除してもよろしいですか？」画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_YES ){			// はい
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN547 );	// 削除番号選択画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN547 );		// 画面番号　<-　次の画面
#if ( VA300S == 0 )
						send_touroku_delete_Wait_Ack_Retry();	// PCへ、指登録情報１件の削除要求送信。
#endif
					}

				} else if ( msg[ 0 ] == LCD_NO ){	// いいえ
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN547 );	// 中止の「はい」「いいえ」選択画面
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN547 );		// 画面番号　<-　次の画面
					}
				}	
/**/				
			}
			break;
		
		case LCD_SCREEN549:		// 「ID番号が登録されていません」画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_OK ){					// 確認ボタンが押下された
					ercd = set_flg( ID_FLG_LCD, ( FLGPTN )befor_scrn_no );	// 登録ID番号入力画面へ
					if ( ercd == E_OK ){
						ChgScreenNo( ( UB )befor_scrn_for_ctrl );		// 画面番号　<-　次の画面
					}
				}
			}
			break;
		
		case LCD_SCREEN550:		// 「操作する権限がありません」画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_OK ){					// 確認ボタンが押下された
					ercd = set_flg( ID_FLG_LCD, ( FLGPTN )befor_scrn_no );	// 権限ID番号入力画面へ
					if ( ercd == E_OK ){
						ChgScreenNo( ( UB )befor_scrn_for_ctrl );		// 画面番号　<-　次の画面
					}
				}
			}
			break;
			
		case LCD_SCREEN551:		// 中止の「はい」「いいえ」選択画面。
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_YES ){				// はい
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN500 );	// ユーザーID番号選択画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN500 );	// 画面番号　<-　次の画面
					}
				} else if ( msg[ 0 ] == LCD_NO ){		// いいえ
					ercd = set_flg( ID_FLG_LCD, ( FLGPTN )befor_scrn_no );	// 元の画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( ( UB )befor_scrn_for_ctrl );		// 画面番号　<-　次の画面
					}

				}					
			}			
			break;

			
		default:
			break;
	}	
}

/*==========================================================================*/
/**
 *	次の画面表示要求フラグセット（マンション・占有部仕様の場合）
 */
/*==========================================================================*/
ER NextScrn_Control_mantion( void )
{
	ER ercd;
	UB S_No;
	ER_UINT i;
	ER_UINT rcv_length;
	UB msg[ 128 ];	
	int cnt0, cnt1, cnt2;
	UB ubtmp;
	unsigned short	tp_num;
	UB rtn;

	S_No = GetScreenNo();
	//memset(msg, 0, sizeof(msg));
	
	switch ( S_No ) {
		
		
	//　初期登録モード
		case LCD_SCREEN1:		// 初期登録時のID入力画面。
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( ( msg[ 0 ] == LCD_NEXT ) || ( msg[ 0 ] == LCD_USER_ID ) && ( rcv_length >= 5 ) ){
				
				yb_touroku_data.user_id[ 0 ] = msg[ 1 ];	// ユーザーID
				yb_touroku_data.user_id[ 1 ] = msg[ 2 ];
				yb_touroku_data.user_id[ 2 ] = msg[ 3 ];
				yb_touroku_data.user_id[ 3 ] = msg[ 4 ];
				yb_touroku_data.user_id[ 4 ] = ',';
				
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN2 );	// 初期登録/メンテナンスの選択画面へ。	
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN2 );		// 画面番号　<-　次の画面
				}
			} else {
				nop();	
			}
			break;

		case LCD_SCREEN2:		// 初期登録 or メンテナンスボタン選択画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
						
			if ( ( msg[ 0 ] == LCD_INIT_INPUT ) && ( rcv_length >= 1 ) ){	// 	初期登録ボタンが押された
				
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN3 );	// 責任者番号　選択画面へ。	
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN3 );		// 画面番号　<-　次の画面
				}
				
			} else if ( ( msg[ 0 ] == LCD_MAINTE ) && ( rcv_length >= 1 ) ){// 	メンテナンス・ボタンが押された

				if( g_RegUserInfoData.RegSts == 0 ){	//登録データなし
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN201 );	// メンテナンス画面・責任者番号　選択画面へ。	
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN201 );		// 画面番号　<-　次の画面
						MdCngMode( MD_MAINTE );				// 装置モードをメンテナンス・モードへ
#if ( VA300S == 0 )
						ercd = SndCmdCngMode( (UINT)MD_MAINTE );	// PCへ	メンテナンス・モード切替え通知を送信
						if ( ercd != E_OK ){
							nop();		// エラー処理の記述	
						}
#endif
					}
				}else{
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN2 );	// 初期登録/メンテナンスの選択画面へ。	
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN2 );		// 画面番号　<-　次の画面
					}
				}					

			} else {
				nop();	
			}
			break;

		case LCD_SCREEN3:		// 責任者指番号　選択画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( ( msg[ 0 ] == LCD_YUBI_ID ) && ( rcv_length >= 4 ) ){	
			
				yb_touroku_data.yubi_seq_no[ 0 ] = msg[ 1 ];	// 責任者指番号
				yb_touroku_data.yubi_seq_no[ 1 ] = msg[ 2 ];
				yb_touroku_data.yubi_seq_no[ 2 ] = msg[ 3 ];
				yb_touroku_data.yubi_seq_no[ 3 ] = ',';
				
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN4 );	// 氏名　入力画面へ
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN4 );		// 画面番号　<-　次の画面
				}
				
			} else if ( ( msg[ 0 ] == LCD_CANCEL ) && ( rcv_length >= 1 ) ){
			
				befor_scrn_no = FPTN_LCD_SCREEN3;
				befor_scrn_for_ctrl = LCD_SCREEN3;
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN12 );	// 	「中止しても..」画面へ。
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN12 );	// 画面番号　<-　次の画面
				}	
								
			} else {
				nop();	
			}
			break;

		case LCD_SCREEN4:		// 氏名　入力画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( ( msg[ 0 ] == LCD_NAME ) && ( rcv_length >= 25 ) ){	

				for(i = 0 ; i < 24 ; i++ ){					// 氏名
					yb_touroku_data.name[ i ] = msg[ 1 + i ];
				}
				
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN5 );	// 	指種別画面へ。
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN5 );		// 画面番号　<-　次の画面
				}
				
			} else if ( ( msg[ 0 ] == LCD_CANCEL ) && ( rcv_length >= 1 ) ){
			
				befor_scrn_no = FPTN_LCD_SCREEN4;
				befor_scrn_for_ctrl = LCD_SCREEN4;
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN12 );	// 	「中止しても..」画面へ。
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN12 );	// 画面番号　<-　次の画面
				}	
								
			} else {
				nop();	
			}
			break;

		case LCD_SCREEN5:		// 指種別選択画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( ( msg[ 0 ] == LCD_YUBI_SHUBETU ) && ( rcv_length >= 3 ) ){	
			
				yb_touroku_data.yubi_no[ 0 ] = msg[ 1 ];	// 指種別
				yb_touroku_data.yubi_no[ 1 ] = msg[ 2 ];
				yb_touroku_data.yubi_no[ 2 ] = ',';
				
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN6 );	// 	「指をセットして..」画面へ。
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN6 );		// 画面番号　<-　次の画面
				}
				
			} else if ( ( msg[ 0 ] == LCD_CANCEL ) && ( rcv_length >= 1 ) ){
			
				befor_scrn_no = FPTN_LCD_SCREEN5;
				befor_scrn_for_ctrl = LCD_SCREEN5;
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN12 );	// 	「中止しても..」画面へ。
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN12 );	// 画面番号　<-　次の画面
				}	
								
			} else {
				nop();	
			}
			break;			
		
		case LCD_SCREEN6:		// 「指をセットして..」画面。
			break;				// 登録の生体センサー検知ON待ち。LCDタスクは、ここへのchange Reqは出さないはず。

		case LCD_SCREEN7:		// 「指を抜いて下さい..」画面。
			break;				// 登録の生体センサー検知OFF待ち。LCDタスクは、ここへのchange Reqは出さないはず。

		case LCD_SCREEN8:		// 「もう一度指をセットして..」画面。
			break;				// 登録の生体センサー検知待ち。LCDタスクは、ここへのchange Reqは出さないはず。

		case LCD_SCREEN9:		// 登録完了○画面。
			
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN11 );	// 「登録を続けますか」画面へ。
			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN11 );		// 画面番号　<-　次の画面
			}		
			break;		

		case LCD_SCREEN10:		// 登録失敗×画面。
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN5 );	// 指選択画面へ。
			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN5 );			// 画面番号　<-　次の画面
			}
//			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN11 );	// 「登録を続けますか」画面へ。
//			if ( ercd == E_OK ){
//				ChgScreenNo( LCD_SCREEN11 );		// 画面番号　<-　次の画面
//			}
			break;		

		case LCD_SCREEN11:		// 登録続行の「はい」「いいえ」選択画面。
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_YES ){			// はい
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN3 );	// 責任者番号選択画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN3 );		// 画面番号　<-　次の画面
					}
				} else if ( msg[ 0 ] == LCD_NO ){	// いいえ
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// 通常画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN100 );		// 画面番号　<-　次の画面
						MdCngMode( MD_NORMAL );				// 装置モードを通常モードへ
#if ( VA300S == 0 )
						ercd = SndCmdCngMode( (UINT)MD_NORMAL );	// PCへ	通常モード切替え通知を送信
						if ( ercd != E_OK ){
							nop();		// エラー処理の記述	
						}
#else
						//フラッシュに登録データを保存する
						dly_tsk( 1000/MSEC );
						ercd = SaveBkAuthDataFl();
						ercd = SaveRegImgFlArea(0);
#endif
					}
				} else {
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN11 );	// 「登録を続けますか」画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN11 );		// 画面番号　<-　次の画面
					}		
				}					
			}
			break;		

		case LCD_SCREEN12:		// 中止の「はい」「いいえ」選択画面。
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_YES ){			// はい
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN2 );	// ユーザーID番号選択画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN2 );		// 画面番号　<-　次の画面
					}
				} else if ( msg[ 0 ] == LCD_NO ){	// いいえ
					ercd = set_flg( ID_FLG_LCD, ( FLGPTN )befor_scrn_no );	// 通常画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( ( UB )befor_scrn_for_ctrl );		// 画面番号　<-　次の画面
					}
				}					
			}			
			break;		



	// 通常モード				
		case LCD_SCREEN100:		// ブランク画面。
		
			g_MainteLvl = 0;	//20160711Miya デモ機
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			send_sio_BPWR(FLG_OFF);//20160905Miya B-PWR制御
				
			if ( rcv_length >= 0 ){
				if(g_TechMenuData.SysSpec == 0){	//20160108Miya FinKeyS
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN101 );	// 通常モードの待機画面
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN101 );		// 画面番号　<-　次の画面
					}
				}else{
					g_PasswordOpen.sw = FLG_ON;
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN601 );	// 通常モードの待機画面
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN601 );		// 画面番号　<-　次の画面
					}
				}
			}	
			break;
		
		case LCD_SCREEN101:		// 通常モードのメニュー画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_TOUROKU ){		// 登録ボタン押下
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN120 );	// 通常モード登録画面へ。
					if ( ercd == E_OK ){
#if ( VA300S == 1 || VA300S == 2 )						
						g_RegFlg = 0;
#endif
						ChgScreenNo( LCD_SCREEN120 );		// 画面番号　<-　次の画面
					}
				} else if ( msg[ 0 ] == LCD_SAKUJYO ){	// 削除ボタン押下
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN140 );	// 通常モード削除画面へ。
					if ( ercd == E_OK ){
#if ( VA300S == 1 || VA300S == 2 )						
						g_RegFlg = 0;
#endif
						ChgScreenNo( LCD_SCREEN140 );		// 画面番号　<-　次の画面
					}
				} else if ( msg[ 0 ] == LCD_KINKYUU_SETTEI ){	// "緊急番号設定"ボタンが押された
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN160 );	// 緊急番号設定・初期画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN160 );		// 画面番号　<-　次の画面
					}					
				} else if ( msg[ 0 ] == LCD_KINKYUU_KAIJYOU ){	//  "緊急解錠"ボタンが押された"
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN170 );	// 緊急開錠・初期画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN170 );		// 画面番号　<-　次の画面
					}													
				} else if ( msg[ 0 ] == LCD_MAINTE ){	// メンテナンス・ボタン押下
#if ( VA300S == 0 )						
					send_donguru_chk_Wait_Ack_Retry();	// PCへ、ドングルの有無確認を送信。						
#else
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN200 );	// メンテナンスモード画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN200 );		// 画面番号　<-　次の画面
					} else {
						nop();			// エラー処理の記述
					}
#endif						
					// メンテナンスモード画面移行は、コマンド002受信処理内で、OK受信した場合に実行する。

				} else if ( msg[ 0 ] == LCD_PASSWORD_OPEN ){//20140925Miya password_open	//  "パスワード開錠ボタンが押された"
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN108 );	// 緊急開錠・初期画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN108 );		// 画面番号　<-　次の画面
					}													
				} else if ( msg[ 0 ] == LCD_ERR_REQ ){//20140925Miya add err	//  エラーえお検出した
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN109 );	// エラー表示画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN109 );		// 画面番号　<-　次の画面
					}													
				}

#if(PCCTRL == 1)	//20160930Miya PCからVA300Sを制御する
				if ( msg[ 0 ] == LCD_BACK ){		// 登録ボタン押下
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN101 );	// 通常モード登録画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN101 );		// 画面番号　<-　次の画面
					}
				}
#endif						

			}
			break;
		
		case LCD_SCREEN102:		// 通常モードの待機画面のブランク表示
			break;				// LCDタスクは、ここへのchange Reqは出さないはず。
			
		
		case LCD_SCREEN103:		// 認証完了○画面。	
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
					
//#if ( VA300S == 1 || VA300S == 2 )						
#if ( (VA300S == 1 || VA300S == 2) && PCCTRL == 0 )		//20160930Miya PCからVA300Sを制御する					
			//フラッシュに登録データを保存する
			if( g_AuthOkCnt >= 50 ){
			//if( g_AuthOkCnt >= 5 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN110 );	// 通常モードの待機画面へ。
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN110 );		// 画面番号　<-　次の画面
					if ( Pfail_mode_count == 0 ){ 
						MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ（必要無いが念のため）
					}	else	{
						MdCngMode( MD_PFAIL );			// 装置モードを	停電モードへ（必要無いが念のため）
					}
				}
				break;
				//	g_AuthOkCnt = 0;
				//	ercd = SaveBkAuthDataFl();
				//	ercd = SaveRegImgFlArea( 0 );
				//	ercd = SaveRegImgFlArea( 10 );
			}
#endif

#if(AUTHTEST >= 1)	//20160613Miya
			if(g_sv_okcnt == 8){
				ReadTestRegImgArea(0, 0, 0, 0);
				SaveTestRegImgFlArea(0);
				g_sv_okcnt = 0;
			}
#endif

			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN101 );	// 通常モードの待機画面へ。

			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN101 );		// 画面番号　<-　次の画面
				if ( Pfail_mode_count == 0 ){ 
					MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ（必要無いが念のため）
				}	else	{
					MdCngMode( MD_PFAIL );			// 装置モードを	停電モードへ（必要無いが念のため）
				}
			}
			
#if(PCCTRL)	//20160930Miya PCからVA300Sを制御する
			send_sio_AUTHPROC(0);
#endif

			break;
		
		case LCD_SCREEN104:		// 認証完了×画面。
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

#if(AUTHTEST >= 1)	//20160715Miya
			if(g_sv_okcnt == 8){
				ReadTestRegImgArea(0, 0, 0, 0);
				SaveTestRegImgFlArea(0);
				g_sv_okcnt = 0;
			}
#endif
			
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN101 );	// 通常モードのメニュー画面へ。
			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN101 );		// 画面番号　<-　次の画面
				if ( Pfail_mode_count == 0 ){ 
					MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ（必要無いが念のため）
				}	else	{
					MdCngMode( MD_PFAIL );			// 装置モードを	停電モードへ（必要無いが念のため）
				}
			}

#if(PCCTRL)	//20160930Miya PCからVA300Sを制御する
			send_sio_AUTHPROC(1);
#endif

			break;
		
		//20140925Miya password_open
		case LCD_SCREEN108:		// 開錠用パスワード入力画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_KEYIN_PASSWORD ){		// 決定ボタン
					if( rcv_length >= 5 ){
						g_InPasswordOpen.password[ 0 ] = msg[ 1 ];		// パスワード８桁番号
						g_InPasswordOpen.password[ 1 ] = msg[ 2 ];
						g_InPasswordOpen.password[ 2 ] = msg[ 3 ];
						g_InPasswordOpen.password[ 3 ] = msg[ 4 ];
						g_InPasswordOpen.password[ 4 ] = msg[ 5 ];
						g_InPasswordOpen.password[ 5 ] = msg[ 6 ];
						g_InPasswordOpen.password[ 6 ] = msg[ 7 ];
						g_InPasswordOpen.password[ 7 ] = msg[ 8 ];
						g_InPasswordOpen.password[ 8 ] = 0;
						
						dly_tsk((500/MSEC));	//500msecのウエイト
						ercd = ChekPasswordOpenKey();
						if( ercd == E_OK ){
							g_AuthType = 1;	//20160120Miya
							ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN103 );	// 開錠OKの○画面へ。
							if ( ercd == E_OK ){
								ChgScreenNo( LCD_SCREEN103 );		// 画面番号　<-　次の画面
								dly_tsk( 500/MSEC );
								send_sio_Ninshou(1, 1, 0);				//解錠コマンド発行
							}
						}else{
							ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN104 );	// 開錠OKの×画面へ。
							if ( ercd == E_OK ){
								ChgScreenNo( LCD_SCREEN104 );		// 画面番号　<-　次の画面
							}
						}
					}
				}else if(msg[ 0 ] == LCD_MENU){
					if ( rcv_length >= 0 ){
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN180 );	// 
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN180 );		// 画面番号　<-　次の画面
						}
					}
				}else if(msg[ 0 ] == LCD_CANCEL){
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN101 );	// 通常モードの待機画面へ。

					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN101 );		// 画面番号　<-　次の画面
						if ( Pfail_mode_count == 0 ){ 
							MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ（必要無いが念のため）
						}	else	{
							MdCngMode( MD_PFAIL );			// 装置モードを	停電モードへ（必要無いが念のため）
						}
					}
				}else if(msg[ 0 ] == LCD_MAINTE){
#if ( VA300S == 0 )						
					send_donguru_chk_Wait_Ack_Retry();	// PCへ、ドングルの有無確認を送信。						
#else
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN202 );	// メンテナンスモード画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN202 );		// 画面番号　<-　次の画面
					} else {
						nop();			// エラー処理の記述
					}

					MdCngMode( MD_MAINTE );				// 装置モードをメンテナンスモードへ
					ercd = set_flg( ID_FLG_MAIN, FPTN_SEND_REQ_MAINTE_CMD );	// メインTaskへ、メンテナンスモード切替え通知送信を依頼。
					if ( ercd != E_OK ){
						nop();		// エラー処理の記述	
					}
#endif						
				} else if ( msg[ 0 ] == LCD_ERR_REQ ){//20140925Miya add err	//  エラーを検出した
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN109 );	// エラー表示画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN109 );		// 画面番号　<-　次の画面
					}													
				}

			}
			break;
		case LCD_SCREEN109:		// 「エラー表示」画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_BACK ){							// 「戻る」が押された
				
					//++g_MainteLog.err_rcnt;
					//g_MainteLog.err_rcnt = g_MainteLog.err_rcnt & 0x7F;

					//エラーが10個以上ある場合
					cnt1 = (int)g_MainteLog.err_wcnt;
					cnt2 = (int)g_MainteLog.err_rcnt;
					if( cnt1 != cnt2 ){
						if( cnt1 > 0x7F || cnt2 > 0x7F ){
							g_MainteLog.err_wcnt = 0;
							g_MainteLog.err_rcnt = 0;
						}else{
							if( cnt1 > cnt2 ){
								cnt0 = cnt1 - cnt2;
							}else{
								cnt0 = 128 - cnt2;
								cnt0 = cnt0 + cnt1;
							}
							if( cnt0 > 10 ){
								cnt2 = cnt1 - 10;
								if( cnt2 < 0 ){
									cnt2 = 128 + cnt2;
								}
							}
							g_MainteLog.err_rcnt = (unsigned short)cnt2;
						}
					}

					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN101 );	// メンテナンス・メニュー画面(情報)へ
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN101 );		// 画面番号　<-　次の画面
						if ( Pfail_mode_count == 0 ){ 
							MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ（必要無いが念のため）
						}	else	{
							MdCngMode( MD_PFAIL );			// 装置モードを	停電モードへ（必要無いが念のため）
						}
					}
				}
			}
			break;
		case LCD_SCREEN110:		// 「しばらくおまちください」画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			send_sio_BPWR(FLG_ON);//20160905Miya B-PWR制御

			if ( rcv_length >= 1 ){
				g_Diagnosis_start = 1;
			}
			break;
	// 通常モード・登録
		case LCD_SCREEN120:		// 通常モード・登録のブランク画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			send_sio_BPWR(FLG_ON);//20160905Miya B-PWR制御

#if(PCCTRL == 1) //20160930Miya PCからVA300Sを制御する
			if ( rcv_length >= 0 ){
				IrLedOnOffSet(1, irDuty2, irDuty3, irDuty4, irDuty5);	//20160312Miya 極小精度UP
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN126 );	// 
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN126 );		// 画面番号　<-　次の画面
				}
			}
			break;
#endif

#if(AUTHTEST >= 1)	//20160613Miya
			if ( rcv_length >= 0 ){
				IrLedOnOffSet(1, irDuty2, irDuty3, irDuty4, irDuty5);	//20160312Miya 極小精度UP
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN124 );	// 
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN124 );		// 画面番号　<-　次の画面
				}
			}
#else
			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN121 );	// 
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN121 );		// 画面番号　<-　次の画面
				}
			}
#endif
			break;
			
		case LCD_SCREEN121:		// 登録時・責任者の認証画面
			break;				// 生体センサー検知待ち。LCDタスクは、ここへのchange Reqは出さないはず。
			
		case LCD_SCREEN122:		// 責任者の認証成功○画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN124);	// 責任者・一般者の登録番号選択画面へ
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN124 );		// 画面番号　<-　次の画面
				}
			}
			break;
			
		case LCD_SCREEN123:		// 責任者の認証失敗×画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// 通常モードのブランク画面
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN100 );		// 画面番号　<-　次の画面
				}
			}
			break;
			
		case LCD_SCREEN124:		// 責任者・一般者の登録番号選択画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( ( msg[ 0 ] == LCD_YUBI_ID ) && ( rcv_length >= 4 ) ){	
			
				yb_touroku_data.yubi_seq_no[ 0 ] = msg[ 1 ];	// 責任者指番号
				yb_touroku_data.yubi_seq_no[ 1 ] = msg[ 2 ];
				yb_touroku_data.yubi_seq_no[ 2 ] = msg[ 3 ];
				yb_touroku_data.yubi_seq_no[ 3 ] = ',';
				
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN125 );	// 氏名　入力画面へ
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN125 );		// 画面番号　<-　次の画面
				}
				
			} else if ( ( msg[ 0 ] == LCD_CANCEL ) && ( rcv_length >= 1 ) ){
			
				befor_scrn_no = FPTN_LCD_SCREEN124;
				befor_scrn_for_ctrl = LCD_SCREEN124;
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN133 );	// 	「中止しても..」画面へ。
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN133 );	// 画面番号　<-　次の画面
				}	
								
			} else {
				nop();	
			}	
			break;
			
		case LCD_SCREEN125:		// 名前入力画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( ( msg[ 0 ] == LCD_NAME ) && ( rcv_length >= 25 ) ){	

				for(i = 0 ; i < 24 ; i++ ){					// 氏名
					yb_touroku_data.name[ i ] = msg[ 1 + i ];
				}
				
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN126 );	// 	指種別画面へ。
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN126 );		// 画面番号　<-　次の画面
				}
				
			} else if ( ( msg[ 0 ] == LCD_CANCEL ) && ( rcv_length >= 1 ) ){
			
				befor_scrn_no = FPTN_LCD_SCREEN125;
				befor_scrn_for_ctrl = LCD_SCREEN125;
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN132 );	// 	「中止しても..」画面へ。
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN132 );	// 画面番号　<-　次の画面
				}	
								
			} else {
				nop();	
			}
			break;
			
		case LCD_SCREEN126:		// 指種別選択画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( ( msg[ 0 ] == LCD_YUBI_SHUBETU ) && ( rcv_length >= 3 ) ){	
			
				yb_touroku_data.yubi_no[ 0 ] = msg[ 1 ];	// 指種別
				yb_touroku_data.yubi_no[ 1 ] = msg[ 2 ];
				yb_touroku_data.yubi_no[ 2 ] = ',';
				
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN127 );	// 	「指をセットして..」画面へ。
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN127 );		// 画面番号　<-　次の画面
				}
				
			} else if ( ( msg[ 0 ] == LCD_CANCEL ) && ( rcv_length >= 1 ) ){
			
				befor_scrn_no = FPTN_LCD_SCREEN126;
				befor_scrn_for_ctrl = LCD_SCREEN126;
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN132 );	// 	「中止しても..」画面へ。
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN132 );	// 画面番号　<-　次の画面
				}	
								
			} else {
				nop();	
			}
			break;
			
		case LCD_SCREEN127:		// 「指をセットして...」画面
			break;				// 生体センサー検知待ち。LCDタスクは、ここへのchange Reqは出さないはず。
			
		case LCD_SCREEN128:		// 「指を抜いて下さい」画面
			break;				// 生体センサー検知待ち。LCDタスクは、ここへのchange Reqは出さないはず。
			
		case LCD_SCREEN129:		// 「もう一度、指を...」画面
			break;				// 生体センサー検知待ち。LCDタスクは、ここへのchange Reqは出さないはず。
			
		case LCD_SCREEN130:		// 登録成功○画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
#if(PCCTRL == 1) //20160930Miya PCからVA300Sを制御する
			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// ユーザーID番号選択画面へ。
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN100 );		// 画面番号　<-　次の画面
/*
					if(g_RegFlg > 0){
						dly_tsk( 1000/MSEC );
						ercd = SaveBkAuthDataFl();
						if(g_RegFlg == 1 || g_RegFlg == 3){
							//フラッシュに登録データを保存する
							ercd = SaveRegImgFlArea(0);
						}
						if(g_RegFlg == 2 || g_RegFlg == 3){
							//フラッシュに登録データを保存する
							ercd = SaveRegImgFlArea(10);
						}
					}
*/
				}
				
				send_sio_REGPROC(0);
			}
#else
			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN124);	// 責任者・一般者の登録番号選択画面へ
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN124 );		// 画面番号　<-　次の画面
				}
			}	
#endif
			break;
			
		case LCD_SCREEN131:		// 登録失敗×画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
#if(PCCTRL == 1) //20160930Miya PCからVA300Sを制御する
			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// ユーザーID番号選択画面へ。
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN100 );		// 画面番号　<-　次の画面
				}
				send_sio_REGPROC(1);
			}
#else
			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN126);	// 指種別選択画面へ
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN126 );		// 画面番号　<-　次の画面
				}
//				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN124);	// 責任者・一般者の登録番号選択画面へ
//				if ( ercd == E_OK ){
//					ChgScreenNo( LCD_SCREEN124 );		// 画面番号　<-　次の画面
//				}
			}
				
#endif
			break;

		case LCD_SCREEN132:		// 中止の「はい」「いいえ」選択画面。
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_YES ){			// はい
#if(PCCTRL == 1) //20160930Miya PCからVA300Sを制御する
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// ユーザーID番号選択画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN100 );		// 画面番号　<-　次の画面
					}
					send_sio_REGPROC(1);
#else
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN124 );	// ユーザーID番号選択画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN124 );		// 画面番号　<-　次の画面
					}
#endif
				} else if ( msg[ 0 ] == LCD_NO ){	// いいえ
					//仮対応　20130510 Miya
//					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN124 );	// ユーザーID番号選択画面へ。
//					if ( ercd == E_OK ){
//						ChgScreenNo( LCD_SCREEN124 );		// 画面番号　<-　次の画面
//					}

					ercd = set_flg( ID_FLG_LCD, ( FLGPTN )befor_scrn_no );	// 通常画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( ( UB )befor_scrn_for_ctrl );		// 画面番号　<-　次の画面
					}

				}					
			}			
			break;
			
		case LCD_SCREEN133:		// 中止の「はい」「いいえ」選択画面。
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_YES ){			// はい
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// ユーザーID番号選択画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN100 );		// 画面番号　<-　次の画面

#if ( VA300S == 1 || VA300S == 2 )
						if(g_RegFlg > 0){
							dly_tsk( 1000/MSEC );
							ercd = SaveBkAuthDataFl();
							if(g_RegFlg == 1 || g_RegFlg == 3){
								//フラッシュに登録データを保存する
								ercd = SaveRegImgFlArea(0);
							}
							if(g_RegFlg == 2 || g_RegFlg == 3){
								//フラッシュに登録データを保存する
								ercd = SaveRegImgFlArea(10);
							}
						}
#endif

					}
				} else if ( msg[ 0 ] == LCD_NO ){	// いいえ
					ercd = set_flg( ID_FLG_LCD, ( FLGPTN )befor_scrn_no );	// 通常画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( ( UB )befor_scrn_for_ctrl );		// 画面番号　<-　次の画面
					}

				}					
			}			
			break;
			
		
	// 通常モード・削除
		case LCD_SCREEN140:		// 通常モード・削除のブランク画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			send_sio_BPWR(FLG_ON);//20160905Miya B-PWR制御

			//20130620_Miya デモ用に削除画面遷移変更
/*
			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN145 );	// 通常モード削除時・責任者の認証画面
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN145 );		// 画面番号　<-　次の画面
				}
			}
*/

/*　20130620_Miya デモ用に削除画面遷移変更　元 */
			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN141 );	// 通常モード削除時・責任者の認証画面
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN141 );		// 画面番号　<-　次の画面
				}
			}
/**/

			break;
			
		case LCD_SCREEN141:		// 削除時・責任者の認証画面
			break;				// 生体センサー検知待ち。LCDタスクは、ここへのchange Reqは出さないはず。
			
		case LCD_SCREEN142:		// 責任者の認証成功○画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN144);	// 責任者・一般者の削除番号選択画面へ
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN144 );		// 画面番号　<-　次の画面
				}
			}	
			break;
			
		case LCD_SCREEN143:		// 責任者の認証失敗×画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100);	// 通常モードブランク画面へ
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN100 );		// 画面番号　<-　次の画面
				}
			}
			break;
			
		case LCD_SCREEN144:		// 責任者・一般者の削除番号選択画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
		
			if ( ( msg[ 0 ] == LCD_YUBI_ID ) && ( rcv_length >= 4 ) ){	
			
				yb_touroku_data.yubi_seq_no[ 0 ] = msg[ 1 ];	// 削除候補の指番号
				yb_touroku_data.yubi_seq_no[ 1 ] = msg[ 2 ];
				yb_touroku_data.yubi_seq_no[ 2 ] = msg[ 3 ];
				yb_touroku_data.yubi_seq_no[ 3 ] = ',';
				
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN145 );	// 「削除してもよろしいですか」画面へ
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN145 );		// 画面番号　<-　次の画面
				}
				
			} else if ( ( msg[ 0 ] == LCD_CANCEL ) && ( rcv_length >= 1 ) ){
			
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN146 );	// 	「中止しても..」画面へ。
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN146 );	// 画面番号　<-　次の画面
				}	
								
			} else {
				nop();	
			}	
			break;
			
		case LCD_SCREEN145:		// 「削除してもよろしいですか」画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );


			if ( rcv_length >= 1 ){
/*
				//20130620_Miya デモ用に削除画面遷移変更
				if ( msg[ 0 ] == LCD_YES ){			// はい
//					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// 削除番号選択画面へ。
//					if ( ercd == E_OK ){
//						ChgScreenNo( LCD_SCREEN100 );		// 画面番号　<-　次の画面
						//send_touroku_delete_Wait_Ack_Retry();	// PCへ、指登録情報１件の削除要求送信。
						send_touroku_init_Wait_Ack_Retry();		// PCへ、登録データの初期化要求送信。
					if ( ercd == E_OK ){
						req_restart = 1;			// パワーオンプロセスの再起動要求フラグのセット。
					}
						if ( ercd == E_OK ){
							req_restart = 1;		// パワーオンプロセスの再起動要求フラグのセット。
						}
//					}

				} else if ( msg[ 0 ] == LCD_NO ){	// いいえ
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// 中止の「はい」「いいえ」選択画面
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN100 );		// 画面番号　<-　次の画面
					}
				}					
*/				
				
/*　20130620_Miya デモ用に削除画面遷移変更　元 */
				if ( msg[ 0 ] == LCD_YES ){			// はい
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN144 );	// 削除番号選択画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN144 );		// 画面番号　<-　次の画面
#if ( VA300S == 0 )
						send_touroku_delete_Wait_Ack_Retry();	// PCへ、指登録情報１件の削除要求送信。
#else
						rtn = DelImgAuthPara();
						if(rtn == 0){	//20161031Miya Ver2204
							send_sio_Touroku_Del();				// VA300s 錠制御基板へ、指登録情報１件の削除を報告。
							MdCngSubMode( SUB_MD_DELETE );		// サブモード削除中
						}else{
							set_reg_param_for_Lcd();	//20161031Miya Ver2204
						}
#endif
					}

				} else if ( msg[ 0 ] == LCD_NO ){	// いいえ
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN144 );	// 中止の「はい」「いいえ」選択画面
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN144 );		// 画面番号　<-　次の画面
					}
				}	
/**/				
			}
			break;
			
		case LCD_SCREEN146:		// 中止の「はい」「いいえ」選択画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_YES ){			// はい
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// ユーザーID番号選択画面へ。
					if ( ercd == E_OK ){
#if ( VA300S == 1 || VA300S == 2 )
						if(g_RegFlg > 0){
							dly_tsk( 1000/MSEC );
							ercd = SaveBkAuthDataFl();
							if(g_RegFlg == 1 || g_RegFlg == 3){
								//フラッシュに登録データを保存する
								ercd = SaveRegImgFlArea(0);
							}
							if(g_RegFlg == 2 || g_RegFlg == 3){
								//フラッシュに登録データを保存する
								ercd = SaveRegImgFlArea(10);
							}
						}
#endif
						ChgScreenNo( LCD_SCREEN100 );		// 画面番号　<-　次の画面
					}
				} else if ( msg[ 0 ] == LCD_NO ){	// いいえ
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN144 );	// 通常画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN144 );		// 画面番号　<-　次の画面
					}
				}					
			}	
			break;

	// 通常モード・緊急開錠番号設定
		case LCD_SCREEN160:		// 通常モード・緊急開錠番号設定のブランク画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			send_sio_BPWR(FLG_ON);//20160905Miya B-PWR制御
				
			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN161 );	// 「責任者の指を...」画面
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN161 );		// 画面番号　<-　次の画面
				}
			}
			break;

		case LCD_SCREEN161:		// 通常モード・緊急開錠番号設定・「責任者の指を...」画面
			break;				// 生体センサー検知待ち。LCDタスクは、ここへのchange Reqは出さないはず。
			
		case LCD_SCREEN162:		// 通常モード・緊急開錠番号設定・責任者の認証成功○画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN164 );	// 緊急開錠番号の設定画面
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN164 );		// 画面番号　<-　次の画面
				}
			}
			break;
			
		case LCD_SCREEN163:		// 通常モード・緊急開錠番号設定・責任者の認証失敗×画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// 通常モード・初期画面
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN100 );		// 画面番号　<-　次の画面
				}
			}
			break;

		case LCD_SCREEN164:		// 通常モード・緊急開錠番号の設定画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			if ( rcv_length >= 1 ){			
				if ( ( ( msg[ 0 ] == LCD_NEXT ) || ( msg[ 0 ] == LCD_KINKYUU_BANGOU ) ) && ( rcv_length >= 5 ) ){
				
					kinkyuu_touroku_no[ 0 ] = msg[ 1 ];		// 緊急開錠番号
					kinkyuu_touroku_no[ 1 ] = msg[ 2 ];
					kinkyuu_touroku_no[ 2 ] = msg[ 3 ];
					kinkyuu_touroku_no[ 3 ] = msg[ 4 ];
					kinkyuu_touroku_no[ 4 ] = 0;
#if ( VA300S == 0 )				
					send_kinkyuu_touroku_Wait_Ack_Retry();	// PCへ、緊急開錠番号通知送信。
#else
					g_RegUserInfoData.KinkyuNum[0] = kinkyuu_touroku_no[0];
					g_RegUserInfoData.KinkyuNum[1] = kinkyuu_touroku_no[1];
					g_RegUserInfoData.KinkyuNum[2] = kinkyuu_touroku_no[2];
					g_RegUserInfoData.KinkyuNum[3] = kinkyuu_touroku_no[3];
					//フラッシュに登録データを保存する
					dly_tsk( 500/MSEC );
					ercd = SaveBkAuthDataFl();

					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// 通常モード・初期画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN100 );		// 画面番号　<-　次の画面
						MdCngMode( MD_NORMAL );				// 装置モードを通常モードへ（不要のはずだが念のため）
					}

#endif								
					// 次画面（通常モード・初期画面）への遷移は、緊急開錠番号通知送信のOK結果を受信後、その受信コマンド処理で実行する。	

				} else if ( msg[ 0 ] == LCD_CANCEL ){		// 「中止」ボタンが押された
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN165 );	// 通常モード初期画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN165 );		// 画面番号　<-　次の画面
					}	
				}
			}
			break;
			
		case LCD_SCREEN165:		// 通常モード・緊急開錠番号「中止してもよろしいですか？」画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_YES ){					// 「はい」が押された
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// 通常モード・初期画面
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN100 );		// 画面番号　<-　次の画面
					}
										
				} else if ( msg[ 0 ] == LCD_NO ){			// 「いいえ」が押された
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN164 );	// 通常モード・緊急開錠番号の設定画面
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN164 );		// 画面番号　<-　次の画面
					}					
				}
			}
			break;			

			
	// 通常モード・緊急開錠
		case LCD_SCREEN170:		// 通常モード・緊急開錠のブランク画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			send_sio_BPWR(FLG_ON);//20160905Miya B-PWR制御

			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN171 );	// 「コールセンターに連絡して...」画面
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN171 );		// 画面番号　<-　次の画面
				}					
			}
			break;

		case LCD_SCREEN171:		// 通常モード・緊急開錠の「コールセンターに連絡して...」画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 0 ){
				
				if ( msg[ 0 ] == LCD_NEXT ){			// 「ページ」ボタンが押された
					if(g_TechMenuData.SysSpec == 2){	//20160120Miya FinKeyS
						MakeOpenKeyNum();
						befor_scrn_no = FPTN_LCD_SCREEN171;
						befor_scrn_for_ctrl = LCD_SCREEN171;
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN173 );	// 通常モード・緊急開錠「中止しますか？」画面
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN173 );		// 画面番号　<-　次の画面
						}	
					}else{
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN172 );	// 「ID番号を入力して...」画面
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN172 );	// 画面番号　<-　次の画面
						}
					}
					
				} else if ( msg[ 0 ] == LCD_CANCEL ){	// 「中止」ボタンが押された
					befor_scrn_no = FPTN_LCD_SCREEN171;
					befor_scrn_for_ctrl = LCD_SCREEN171;
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN178 );	// 通常モード・緊急開錠「中止しますか？」画面
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN178 );	// 画面番号　<-　次の画面
					}	
				}					
			}
			break;

		case LCD_SCREEN172:		// 通常モード・緊急開錠の「ID番号を入力して...」画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( ( ( msg[ 0 ] == LCD_NEXT ) || ( msg[ 0 ] == LCD_USER_ID ) ) && ( rcv_length >= 5 ) ){
					if ( ( yb_touroku_data.user_id[ 0 ] == msg[ 1 ] )
					  && ( yb_touroku_data.user_id[ 1 ] == msg[ 2 ] )
					  && ( yb_touroku_data.user_id[ 2 ] == msg[ 3 ] )
					  && ( yb_touroku_data.user_id[ 3 ] == msg[ 4 ] ) ){	// ID番号が一致した時。
#if ( VA300S == 0 ) 						
						send_kinkyuu_8keta_Wait_Ack_Retry();	// PCへ、緊急８桁番号データ要求送信、Ack・Nack待ちとリトライ付き
#else
						MakeOpenKeyNum();
						befor_scrn_no = FPTN_LCD_SCREEN172;
						befor_scrn_for_ctrl = LCD_SCREEN172;
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN173 );	// 通常モード・緊急開錠「中止しますか？」画面
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN173 );		// 画面番号　<-　次の画面
						}	
#endif						
						// 次画面への遷移は、緊急８桁番号データ要求の応答で、番号通知を受け取った時に受信処理内で行う。
						
					} else {	// 登録済みユーザーID番号と、入力した番号が不一致の時、
						
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN177 );	// 認証失敗×画面
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN177 );		// 画面番号　<-　次の画面
						}						
					}
					 
				} else if ( msg[ 0 ] == LCD_CANCEL ){	// 「中止」ボタンが押された
					befor_scrn_no = FPTN_LCD_SCREEN172;
					befor_scrn_for_ctrl = LCD_SCREEN172;
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN178 );	// 通常モード・緊急開錠「中止しますか？」画面
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN178 );		// 画面番号　<-　次の画面
					}	
				}
			}
			break;

		case LCD_SCREEN173:		// 通常モード・緊急開錠の「番号をコールセンターへ...」画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_NEXT ){			// 「ページ」ボタンが押された
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN174 );	// 「８桁番号を入力して...」画面
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN174 );	// 画面番号　<-　次の画面
					}			
				
				} else if ( msg[ 0 ] == LCD_CANCEL ){	// 「中止」ボタンが押された
					befor_scrn_no = FPTN_LCD_SCREEN173;
					befor_scrn_for_ctrl = LCD_SCREEN173;
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN178 );	// 通常モード・緊急開錠「中止しますか？」画面
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN178 );		// 画面番号　<-　次の画面
					}	
				}
			}
			break;			


		case LCD_SCREEN174:		// 通常モード・緊急開錠の「８桁番号を入力して下さい...」画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( ( ( msg[ 0 ] == LCD_NEXT ) || ( msg[ 0 ] == LCD_KINKYUU_KAIJYOU_BANGOU ) ) && ( rcv_length >= 9 ) ){								
					kinkyuu_kaijyo_no[ 0 ] = msg[ 1 ];		// 緊急８桁番号
					kinkyuu_kaijyo_no[ 1 ] = msg[ 2 ];
					kinkyuu_kaijyo_no[ 2 ] = msg[ 3 ];
					kinkyuu_kaijyo_no[ 3 ] = msg[ 4 ];
					kinkyuu_kaijyo_no[ 4 ] = msg[ 5 ];
					kinkyuu_kaijyo_no[ 5 ] = msg[ 6 ];
					kinkyuu_kaijyo_no[ 6 ] = msg[ 7 ];
					kinkyuu_kaijyo_no[ 7 ] = msg[ 8 ];
					kinkyuu_kaijyo_no[ 8 ] = 0;
#if ( VA300S == 0 ) 
					send_kinkyuu_kaijyou_Wait_Ack_Retry();	// PCへ、緊急開錠番号送信、Ack・Nack待ちとリトライ付き
#else
					g_UseProcNum.InOpenCode[0] = kinkyuu_kaijyo_no[0];
					g_UseProcNum.InOpenCode[1] = kinkyuu_kaijyo_no[1];
					g_UseProcNum.InOpenCode[2] = kinkyuu_kaijyo_no[2];
					g_UseProcNum.InOpenCode[3] = kinkyuu_kaijyo_no[3];
					g_UseProcNum.InOpenCode[4] = kinkyuu_kaijyo_no[4];
					g_UseProcNum.InOpenCode[5] = kinkyuu_kaijyo_no[5];
					g_UseProcNum.InOpenCode[6] = kinkyuu_kaijyo_no[6];
					g_UseProcNum.InOpenCode[7] = kinkyuu_kaijyo_no[7];

					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN175 );	// 緊急番号入力画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN175 );		// 画面番号　<-　次の画面
					}

#endif								
					// 次画面（緊急番号入力画面）への遷移は、緊急開錠番号８桁送信のOK結果を受信後、その受信コマンド処理で実行する。			
				
				} else if ( msg[ 0 ] == LCD_CANCEL ){	// 「中止」ボタンが押された
					befor_scrn_no = FPTN_LCD_SCREEN174;
					befor_scrn_for_ctrl = LCD_SCREEN174;
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN178 );	// 通常モード・緊急開錠「中止しますか？」画面
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN178 );		// 画面番号　<-　次の画面
					}	
				}
			}
			break;


		case LCD_SCREEN175:		// 通常モード・「緊急番号を入力して下さい...」画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){	
				if ( ( ( msg[ 0 ] == LCD_NEXT ) || ( msg[ 0 ] == LCD_KINKYUU_BANGOU ) ) && ( rcv_length >= 5 ) ){								
					kinkyuu_input_no[ 0 ] = msg[ 1 ];		// 入力された緊急番号
					kinkyuu_input_no[ 1 ] = msg[ 2 ];
					kinkyuu_input_no[ 2 ] = msg[ 3 ];
					kinkyuu_input_no[ 3 ] = msg[ 4 ];
					kinkyuu_input_no[ 4 ] = 0;
#if ( VA300S == 0 )					
					send_kinkyuu_input_Wait_Ack_Retry();	// PCへ、緊急番号の妥当性問い合わせ確認要求送信、Ack・Nack待ちとリトライ付き
#endif								
//#else
#if ( VA300S == 1 )	//20140905Miya				
					g_UseProcNum.InKinkyuNum[0] = kinkyuu_input_no[0]; 
					g_UseProcNum.InKinkyuNum[1] = kinkyuu_input_no[1]; 
					g_UseProcNum.InKinkyuNum[2] = kinkyuu_input_no[2]; 
					g_UseProcNum.InKinkyuNum[3] = kinkyuu_input_no[3]; 

					ercd = ChekKinkyuKaijyouKey();
					if( ercd == E_OK ){
						g_AuthType = 2;	//20160120Miya
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN176 );	// 開錠OKの○画面へ。
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN176 );		// 画面番号　<-　次の画面
							dly_tsk( 500/MSEC );
							send_sio_Ninshou(1, 2, 0);				//解錠コマンド発行
						}
					}else{
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN177 );	// 開錠OKの×画面へ。
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN177 );		// 画面番号　<-　次の画面
						}
						
					}
#endif								
					// 次画面（OK/NG画面）への遷移は、緊急番号妥当性確認要求送信のOK結果を受信後、その受信コマンド処理で実行する。			
				
				} else if ( msg[ 0 ] == LCD_CANCEL ){	// 「中止」ボタンが押された
					befor_scrn_no = FPTN_LCD_SCREEN175;
					befor_scrn_for_ctrl = LCD_SCREEN175;
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN178 );	// 通常モード・緊急開錠「中止しますか？」画面
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN178 );		// 画面番号　<-　次の画面
					}	
				}
			}
			break;

		case LCD_SCREEN176:		// 通常モード・緊急開錠の認証成功○画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// 通常モード・初期画面へ。
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN100 );		// 画面番号　<-　次の画面
				}
			}
			break;
			
		case LCD_SCREEN177:		// 通常モード・緊急開錠の認証失敗×画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// 通常モード・初期画面へ。
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN100 );		// 画面番号　<-　次の画面
				}
			}
			break;

			
		case LCD_SCREEN178:		// 通常モード・緊急開錠時「中止しますか？」画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_YES ){					// 「はい」が押された
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// 通常モード・初期画面
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN100 );		// 画面番号　<-　次の画面
					}
										
				} else if ( msg[ 0 ] == LCD_NO ){			// 「いいえ」が押された
					ercd = set_flg( ID_FLG_LCD, ( FLGPTN )befor_scrn_no );	// 前画面へ戻る
					if ( ercd == E_OK ){
						ChgScreenNo( ( UB )befor_scrn_for_ctrl );	// 画面番号　<-　次の画面
					}					
				}
			}
			break;

		case LCD_SCREEN180:		// 通常モード・パスワード開錠設定のブランク画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			send_sio_BPWR(FLG_ON);//20160905Miya B-PWR制御
				
			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN181 );	// 「責任者の指を...」画面
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN181 );		// 画面番号　<-　次の画面
				}
			}
			break;

		case LCD_SCREEN181:		// 通常モード・パスワード開錠設定・「責任者の指を...」画面
			break;				// 生体センサー検知待ち。LCDタスクは、ここへのchange Reqは出さないはず。
			
		case LCD_SCREEN182:		// 通常モード・パスワード開錠設定・責任者の認証成功○画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN184 );	// パスワード開錠の設定画面
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN184 );		// 画面番号　<-　次の画面
				}
			}
			break;
			
		case LCD_SCREEN183:		// 通常モード・パスワード開錠設定・責任者の認証失敗×画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// 通常モード・初期画面
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN100 );		// 画面番号　<-　次の画面
				}
			}
			break;
		case LCD_SCREEN184:		// 開錠用パスワード変更・メニュー画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_PASS_HENKOU_REQ ){						// パスワード変更ボタン
					//if(g_TechMenuData.SysSpec == 0){	//20160108Miya FinKeyS
					if(g_PasswordOpen2.family_sw == 0){	//20160120Miya FinKeyS
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN185 );		// 「パスワード変更表示」画面へ。
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN185 );		// 画面番号　<-　次の画面
						}
					}else{
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN188 );		// 「パスワード変更表示」画面へ。
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN188 );		// 画面番号　<-　次の画面
						}
					}
				}else if ( msg[ 0 ] == LCD_PASS_SETTEI_HENKOU_REQ ){		// パスワード開錠設定ボタン
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN186 );		// 「パスワード開錠設定表示」画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN186 );		// 画面番号　<-　次の画面
					}			
				}else if ( msg[ 0 ] == LCD_NOUSE ){					// 未使用ボタン
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN184 );
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN184 );		// 画面番号　<-　次の画面
					}
				}else if ( msg[ 0 ] == LCD_MAINTE_END ){					// 終了ボタン
					//20160108Miya FinKeyS
					if( GetSysSpec() == SYS_SPEC_KOUJIM || GetSysSpec() == SYS_SPEC_KOUJIO || GetSysSpec() == SYS_SPEC_KOUJIS ){
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN620 );	// 通常モード・初期画面
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN620 );		// 画面番号　<-　次の画面
						}
					}else{
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// 通常モード・初期画面
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN100 );		// 画面番号　<-　次の画面
						}
					}
				}
			}
			break;

		case LCD_SCREEN185:		// 開錠用パスワード変更画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_KEYIN_PASSWORD ){		// 決定ボタン
					if( rcv_length >= 5 ){
						//if(g_TechMenuData.SysSpec == 0){	//20160108Miya FinKeyS
						if(g_PasswordOpen2.family_sw == 0){	//20160120Miya FinKeyS
							g_PasswordOpen.password[ 0 ] = msg[ 1 ];		// パスワード８桁番号
							g_PasswordOpen.password[ 1 ] = msg[ 2 ];
							g_PasswordOpen.password[ 2 ] = msg[ 3 ];
							g_PasswordOpen.password[ 3 ] = msg[ 4 ];
							g_PasswordOpen.password[ 4 ] = msg[ 5 ];
							g_PasswordOpen.password[ 5 ] = msg[ 6 ];
							g_PasswordOpen.password[ 6 ] = msg[ 7 ];
							g_PasswordOpen.password[ 7 ] = msg[ 8 ];
							g_PasswordOpen.password[ 8 ] = 0;
						}else{
							g_PasswordOpen.password[ 0 ] = msg[ 1 ];		// パスワード８桁番号
							g_PasswordOpen.password[ 1 ] = msg[ 2 ];
							g_PasswordOpen.password[ 2 ] = msg[ 3 ];
							g_PasswordOpen.password[ 3 ] = msg[ 4 ];
							g_PasswordOpen.password[ 4 ] = msg[ 5 ];
							g_PasswordOpen.password[ 5 ] = msg[ 6 ];
							g_PasswordOpen.password[ 6 ] = msg[ 7 ];
							g_PasswordOpen.password[ 7 ] = msg[ 8 ];
							g_PasswordOpen.password[ 8 ] = 0;

							if(g_PasswordOpen2.family_sw == 1){
								g_PasswordOpen2.keta[g_PasswordOpen2.num] = g_PasswordOpen.keta;
								g_PasswordOpen2.password[g_PasswordOpen2.num][ 0 ] = msg[ 1 ];		// パスワード８桁番号
								g_PasswordOpen2.password[g_PasswordOpen2.num][ 1 ] = msg[ 2 ];
								g_PasswordOpen2.password[g_PasswordOpen2.num][ 2 ] = msg[ 3 ];
								g_PasswordOpen2.password[g_PasswordOpen2.num][ 3 ] = msg[ 4 ];
								g_PasswordOpen2.password[g_PasswordOpen2.num][ 4 ] = msg[ 5 ];
								g_PasswordOpen2.password[g_PasswordOpen2.num][ 5 ] = msg[ 6 ];
								g_PasswordOpen2.password[g_PasswordOpen2.num][ 6 ] = msg[ 7 ];
								g_PasswordOpen2.password[g_PasswordOpen2.num][ 7 ] = msg[ 8 ];
								g_PasswordOpen2.password[g_PasswordOpen2.num][ 8 ] = 0;
							}
						}


						//フラッシュに登録データを保存する
						//dly_tsk( 500/MSEC );
						ercd = SaveBkAuthDataFl();
					}

					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN184 );	// 通常モードの待機画面へ。

					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN184 );		// 画面番号　<-　次の画面
					}

				}else if(msg[ 0 ] == LCD_MENU){

				}else if(msg[ 0 ] == LCD_CANCEL){
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN184 );	// 通常モードの待機画面へ。

					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN184 );		// 画面番号　<-　次の画面
					}
				}else if(msg[ 0 ] == LCD_MAINTE){
				}

			}
			break;
		case LCD_SCREEN186:		// 「パスワード開錠」画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_BACK ){				// 「戻る」が押された
				
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN184 );	// メンテナンス・メニュー画面(設定変更)へ
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN184 );		// 画面番号　<-　次の画面
					}
										
				} else if ( ( msg[ 0 ] == LCD_ENTER ) && ( rcv_length >= 4 ) ){	// 「確定」が押された
					if( msg[ 1 ] == 1 )	g_PasswordOpen.hide_num = FLG_ON;
					else				g_PasswordOpen.hide_num = FLG_OFF;

					if( msg[ 2 ] == 1 )	g_PasswordOpen.kigou_inp = FLG_ON;
					else				g_PasswordOpen.kigou_inp = FLG_OFF;

					if( msg[ 3 ] == 1 )	g_PasswordOpen.random_key = FLG_ON;
					else				g_PasswordOpen.random_key = FLG_OFF;

					if( msg[ 4 ] == 1 )	g_PasswordOpen2.family_sw = FLG_ON;
					else				g_PasswordOpen2.family_sw = FLG_OFF;

					for(i = 0 ; i < 10 ; i++ ){
						g_key_arry[i] = i;	//パスワード開錠用キー配列初期化
					}

					//フラッシュに登録データを保存する
					//dly_tsk( 500/MSEC );
					ercd = SaveBkAuthDataFl();
			
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN184 );
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN184 );		// 画面番号　<-　次の画面
					}
				}
			}
			break;
		case LCD_SCREEN187:		// 通常モード・開錠用パスワード変更時「中止しますか？」画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_YES ){					// 「はい」が押された
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// 通常モード・初期画面
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN100 );		// 画面番号　<-　次の画面
					}
										
				} else if ( msg[ 0 ] == LCD_NO ){			// 「いいえ」が押された
					ercd = set_flg( ID_FLG_LCD, ( FLGPTN )befor_scrn_no );	// 前画面へ戻る
					if ( ercd == E_OK ){
						ChgScreenNo( ( UB )befor_scrn_for_ctrl );	// 画面番号　<-　次の画面
					}					
				}
			}
			break;
//20160108Miya FinKryS ->
		case LCD_SCREEN188:		// 責任者・一般者の削除番号選択画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
		
			if ( ( msg[ 0 ] == LCD_YUBI_ID ) && ( rcv_length >= 4 ) ){	
			
				//yb_touroku_data.yubi_seq_no[ 0 ] = msg[ 1 ];	// 削除候補の指番号
				//yb_touroku_data.yubi_seq_no[ 1 ] = msg[ 2 ];
				//yb_touroku_data.yubi_seq_no[ 2 ] = msg[ 3 ];
				//yb_touroku_data.yubi_seq_no[ 3 ] = ',';

				ubtmp = msg[1] - 0x30;
				tp_num = 100 * (unsigned short)ubtmp;
				ubtmp = msg[2] - 0x30;
				tp_num += (10 * (unsigned short)ubtmp);
				ubtmp = msg[3] - 0x30;
				tp_num += (unsigned short)ubtmp;
				
				if(tp_num > 0){
					tp_num -= 1;
					g_PasswordOpen2.num = (short)tp_num;
				
					if(g_RegBloodVesselTagData[tp_num].RegInfoFlg == 1){	//登録あり
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN185 );
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN185 );		// 画面番号　<-　次の画面
						}
					}else{
						yb_touroku_data20[tp_num+1].yubi_seq_no[ 0 ] = '0';
						yb_touroku_data20[tp_num+1].yubi_seq_no[ 1 ] = '0';
						yb_touroku_data20[tp_num+1].yubi_seq_no[ 2 ] = '0';
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN188 );
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN188 );		// 画面番号　<-　次の画面
						}
					}
				}else{
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN188 );
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN188 );		// 画面番号　<-　次の画面
					}
				}
			} else if ( ( msg[ 0 ] == LCD_CANCEL ) && ( rcv_length >= 1 ) ){
			
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN184 );
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN184 );	// 画面番号　<-　次の画面
				}	
								
			} else {
				nop();	
			}	
			break;
//20160108Miya FinKryS <-

//20160108Miya FinKryS ->
//通常モード2 FinKeyS用
		case LCD_SCREEN601:		// 通常モードのメニュー画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			ode_oru_sw = 0;
			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_ODEKAKE ){							// おでかけボタン押下
					ode_oru_sw = 1;
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN610 );
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN610 );					// 画面番号　<-　次の画面
					}
				} else if ( msg[ 0 ] == LCD_ORUSUBAN ){					// お留守番ボタン押下
					ode_oru_sw = 2;
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN611 );
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN611 );					// 画面番号　<-　次の画面
					}
				} else if ( msg[ 0 ] == LCD_SETTEI ){					// 設定ボタンが押された
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN612 );
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN612 );					// 画面番号　<-　次の画面
					}					
				} else if ( msg[ 0 ] == LCD_KINKYUU_KAIJYOU ){			//  "緊急解錠"ボタンが押された"
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN170 );
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN170 );					// 画面番号　<-　次の画面
					}													
				} else if ( msg[ 0 ] == LCD_PASSWORD_OPEN ){			//  "パスワード開錠ボタンが押された"
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN608 );
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN608 );					// 画面番号　<-　次の画面
					}													
				} else if ( msg[ 0 ] == LCD_ERR_REQ ){					// エラーを検出した
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN609 );
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN609 );					// 画面番号　<-　次の画面
					}													
				}
			}
			break;
		
		case LCD_SCREEN602:		// 通常モードの待機画面のブランク表示
			break;				// LCDタスクは、ここへのchange Reqは出さないはず。
			
		
		case LCD_SCREEN603:		// 認証完了○画面。	
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
					
#if ( VA300S == 1 || VA300S == 2 )						
			//フラッシュに登録データを保存する
			if( g_AuthOkCnt >= 50 && g_AuthType == 0 && ode_oru_sw == 0){	//20160120Miya
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN613 );	// しばらくお待ち下さい
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN613 );		// 画面番号　<-　次の画面
					if ( Pfail_mode_count == 0 ){ 
						MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ（必要無いが念のため）
					}	else	{
						MdCngMode( MD_PFAIL );			// 装置モードを	停電モードへ（必要無いが念のため）
					}
				}
				break;
			}
#endif

			ode_oru_sw = 0;	//20160120Miya
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN601 );	// 通常モードの待機画面へ。

			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN601 );		// 画面番号　<-　次の画面
				if ( Pfail_mode_count == 0 ){ 
					MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ（必要無いが念のため）
				}	else	{
					MdCngMode( MD_PFAIL );			// 装置モードを	停電モードへ（必要無いが念のため）
				}
			}
			break;
		
		case LCD_SCREEN604:		// 認証完了×画面。
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			ode_oru_sw = 0;	//20160120Miya
				
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN601 );	// 通常モードのメニュー画面へ。
			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN601 );		// 画面番号　<-　次の画面
				if ( Pfail_mode_count == 0 ){ 
					MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ（必要無いが念のため）
				}	else	{
					MdCngMode( MD_PFAIL );			// 装置モードを	停電モードへ（必要無いが念のため）
				}
			}
			break;
		
		//20140925Miya password_open
		case LCD_SCREEN608:		// 開錠用パスワード入力画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_KEYIN_PASSWORD ){		// 決定ボタン
					if( rcv_length >= 5 ){
						g_InPasswordOpen.password[ 0 ] = msg[ 1 ];		// パスワード８桁番号
						g_InPasswordOpen.password[ 1 ] = msg[ 2 ];
						g_InPasswordOpen.password[ 2 ] = msg[ 3 ];
						g_InPasswordOpen.password[ 3 ] = msg[ 4 ];
						g_InPasswordOpen.password[ 4 ] = msg[ 5 ];
						g_InPasswordOpen.password[ 5 ] = msg[ 6 ];
						g_InPasswordOpen.password[ 6 ] = msg[ 7 ];
						g_InPasswordOpen.password[ 7 ] = msg[ 8 ];
						g_InPasswordOpen.password[ 8 ] = 0;
						
						dly_tsk((500/MSEC));	//500msecのウエイト
						ercd = ChekPasswordOpenKey();
						if( ercd == E_OK ){
							g_AuthType = 1;	//20160120Miya
							ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN603 );	// 開錠OKの○画面へ。
							if ( ercd == E_OK ){
								ChgScreenNo( LCD_SCREEN603 );		// 画面番号　<-　次の画面
								dly_tsk( 500/MSEC );
								send_sio_Ninshou(1, 1, ode_oru_sw);
							}
						}else{
							ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN604 );	// 開錠OKの×画面へ。
							if ( ercd == E_OK ){
								ChgScreenNo( LCD_SCREEN604 );		// 画面番号　<-　次の画面
							}
						}
					}
				}else if(msg[ 0 ] == LCD_MENU){
					if ( rcv_length >= 0 ){
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN180 );	// 
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN180 );		// 画面番号　<-　次の画面
						}
					}
				}else if(msg[ 0 ] == LCD_CANCEL){
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN601 );	// 通常モードの待機画面へ。

					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN601 );		// 画面番号　<-　次の画面
						if ( Pfail_mode_count == 0 ){ 
							MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ（必要無いが念のため）
						}	else	{
							MdCngMode( MD_PFAIL );			// 装置モードを	停電モードへ（必要無いが念のため）
						}
					}
				}else if(msg[ 0 ] == LCD_MAINTE){
#if ( VA300S == 0 )						
					send_donguru_chk_Wait_Ack_Retry();	// PCへ、ドングルの有無確認を送信。						
#else
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN202 );	// メンテナンスモード画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN202 );		// 画面番号　<-　次の画面
					} else {
						nop();			// エラー処理の記述
					}

					MdCngMode( MD_MAINTE );				// 装置モードをメンテナンスモードへ
					ercd = set_flg( ID_FLG_MAIN, FPTN_SEND_REQ_MAINTE_CMD );	// メインTaskへ、メンテナンスモード切替え通知送信を依頼。
					if ( ercd != E_OK ){
						nop();		// エラー処理の記述	
					}
#endif						
				} else if ( msg[ 0 ] == LCD_ERR_REQ ){					//  エラーを検出した
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN609 );
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN609 );					// 画面番号　<-　次の画面
					}													
				}

			}
			break;
		case LCD_SCREEN609:		// 「エラー表示」画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_BACK ){							// 「戻る」が押された
				
					//++g_MainteLog.err_rcnt;
					//g_MainteLog.err_rcnt = g_MainteLog.err_rcnt & 0x7F;

					//エラーが10個以上ある場合
					cnt1 = (int)g_MainteLog.err_wcnt;
					cnt2 = (int)g_MainteLog.err_rcnt;
					if( cnt1 != cnt2 ){
						if( cnt1 > 0x7F || cnt2 > 0x7F ){
							g_MainteLog.err_wcnt = 0;
							g_MainteLog.err_rcnt = 0;
						}else{
							if( cnt1 > cnt2 ){
								cnt0 = cnt1 - cnt2;
							}else{
								cnt0 = 128 - cnt2;
								cnt0 = cnt0 + cnt1;
							}
							if( cnt0 > 10 ){
								cnt2 = cnt1 - 10;
								if( cnt2 < 0 ){
									cnt2 = 128 + cnt2;
								}
							}
							g_MainteLog.err_rcnt = (unsigned short)cnt2;
						}
					}

					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN601 );	/// 通常モードの待機画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN601 );		// 画面番号　<-　次の画面
						if ( Pfail_mode_count == 0 ){ 
							MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ（必要無いが念のため）
						}	else	{
							MdCngMode( MD_PFAIL );			// 装置モードを	停電モードへ（必要無いが念のため）
						}
					}
				}
			}
			break;
		case LCD_SCREEN610:		// おでかけ設定画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 0 ){
				
				if ( msg[ 0 ] == LCD_PASSWORD_OPEN ){					//  "パスワード開錠ボタンが押された"
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN608 );
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN608 );					// 画面番号　<-　次の画面
					}													
				} else if ( msg[ 0 ] == LCD_CANCEL ){					// 「中止」ボタンが押された
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN601 );	// 通常モードの待機画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN601 );					// 画面番号　<-　次の画面
					}
				}					
			}
			break;
		case LCD_SCREEN611:		// お留守番設定画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 0 ){
				
				if ( msg[ 0 ] == LCD_PASSWORD_OPEN ){					//  "パスワード開錠ボタンが押された"
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN608 );
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN608 );					// 画面番号　<-　次の画面
					}													
				} else if ( msg[ 0 ] == LCD_CANCEL ){					// 「中止」ボタンが押された
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN601 );	// 通常モードの待機画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN601 );					// 画面番号　<-　次の画面
					}
				}					
			}
			break;
		case LCD_SCREEN612:		// 通常モードの設定画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_TOUROKU ){							// 登録ボタン押下
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN120 );	// 通常モード登録画面へ。
					if ( ercd == E_OK ){
#if ( VA300S == 1 || VA300S == 2 )						
						g_RegFlg = 0;
#endif
						ChgScreenNo( LCD_SCREEN120 );					// 画面番号　<-　次の画面
					}
				} else if ( msg[ 0 ] == LCD_SAKUJYO ){					// 削除ボタン押下
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN140 );	// 通常モード削除画面へ。
					if ( ercd == E_OK ){
#if ( VA300S == 1 || VA300S == 2 )						
						g_RegFlg = 0;
#endif
						ChgScreenNo( LCD_SCREEN140 );					// 画面番号　<-　次の画面
					}
				} else if ( msg[ 0 ] == LCD_KINKYUU_SETTEI ){			// "緊急番号設定"ボタンが押された
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN160 );	// 緊急番号設定・初期画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN160 );					// 画面番号　<-　次の画面
					}					
				} else if ( msg[ 0 ] == LCD_BACK ){						// 戻るボタンが押された
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN601 );
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN601 );					// 画面番号　<-　次の画面
					}													
				} else if ( msg[ 0 ] == LCD_NOUSE ){					// "未使用"ボタンが押された
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN612 );
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN612 );					// 元の画面に戻る
					}													
				}
			}
			break;
		case LCD_SCREEN613:		// 「しばらくおまちください」画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				g_Diagnosis_start = 1;
			}
			break;

	// 工事モード
		case LCD_SCREEN620:		// ブランク画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			send_sio_BPWR(FLG_OFF);//20160905Miya B-PWR制御

			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN621 );	// 
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN621 );		// 画面番号　<-　次の画面
				}
			}
			break;
			
		case LCD_SCREEN621:		// 
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_KEYIN_PASSWORD ){		// 決定ボタン
					if( rcv_length >= 5 ){
						g_InPasswordOpen.password[ 0 ] = msg[ 1 ];		// パスワード８桁番号
						g_InPasswordOpen.password[ 1 ] = msg[ 2 ];
						g_InPasswordOpen.password[ 2 ] = msg[ 3 ];
						g_InPasswordOpen.password[ 3 ] = msg[ 4 ];
						g_InPasswordOpen.password[ 4 ] = msg[ 5 ];
						g_InPasswordOpen.password[ 5 ] = msg[ 6 ];
						g_InPasswordOpen.password[ 6 ] = msg[ 7 ];
						g_InPasswordOpen.password[ 7 ] = msg[ 8 ];
						g_InPasswordOpen.password[ 8 ] = 0;
						
						dly_tsk((500/MSEC));	//500msecのウエイト
						ercd = ChekPasswordOpenKey();
						if( ercd == E_OK ){
							g_AuthType = 1;	//20160120Miya
							ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN623 );	// 開錠OKの○画面へ。
							if ( ercd == E_OK ){
								ChgScreenNo( LCD_SCREEN623 );		// 画面番号　<-　次の画面
								dly_tsk( 500/MSEC );
								send_sio_Ninshou(1, 1, ode_oru_sw);
							}
						}else{
							ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN624 );	// 開錠OKの×画面へ。
							if ( ercd == E_OK ){
								ChgScreenNo( LCD_SCREEN624 );		// 画面番号　<-　次の画面
							}
						}
					}
				}else if(msg[ 0 ] == LCD_MENU){
					if( rcv_length >= 5 ){
						g_InPasswordOpen.password[ 0 ] = msg[ 1 ];		// パスワード８桁番号
						g_InPasswordOpen.password[ 1 ] = msg[ 2 ];
						g_InPasswordOpen.password[ 2 ] = msg[ 3 ];
						g_InPasswordOpen.password[ 3 ] = msg[ 4 ];
						g_InPasswordOpen.password[ 4 ] = msg[ 5 ];
						g_InPasswordOpen.password[ 5 ] = msg[ 6 ];
						g_InPasswordOpen.password[ 6 ] = msg[ 7 ];
						g_InPasswordOpen.password[ 7 ] = msg[ 8 ];
						g_InPasswordOpen.password[ 8 ] = 0;
						
						dly_tsk((500/MSEC));	//500msecのウエイト
						ercd = ChekPasswordOpenKey();
						if( ercd == E_OK ){
							ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN184 );	// 
							if ( ercd == E_OK ){
								ChgScreenNo( LCD_SCREEN184 );		// 画面番号　<-　次の画面
							}
						}else{
							ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN624 );	// 開錠OKの×画面へ。
							if ( ercd == E_OK ){
								ChgScreenNo( LCD_SCREEN624 );		// 画面番号　<-　次の画面
							}
						}
					}else{
						if(g_PasswordOpen.keta == 0){
							ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN184 );	// 
							if ( ercd == E_OK ){
								ChgScreenNo( LCD_SCREEN184 );		// 画面番号　<-　次の画面
							}
						}else{
							ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN624 );	// 開錠OKの×画面へ。
							if ( ercd == E_OK ){
								ChgScreenNo( LCD_SCREEN624 );		// 画面番号　<-　次の画面
							}
						}
					}
				}else if(msg[ 0 ] == LCD_CANCEL){
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN621 );	// 通常モードの待機画面へ。

					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN621 );		// 画面番号　<-　次の画面
						if ( Pfail_mode_count == 0 ){ 
							MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ（必要無いが念のため）
						}	else	{
							MdCngMode( MD_PFAIL );			// 装置モードを	停電モードへ（必要無いが念のため）
						}
					}
				}else if(msg[ 0 ] == LCD_MAINTE){
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN202 );	// メンテナンスモード画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN202 );		// 画面番号　<-　次の画面
					} else {
						nop();			// エラー処理の記述
					}

					MdCngMode( MD_MAINTE );				// 装置モードをメンテナンスモードへ
					ercd = set_flg( ID_FLG_MAIN, FPTN_SEND_REQ_MAINTE_CMD );	// メインTaskへ、メンテナンスモード切替え通知送信を依頼。
					if ( ercd != E_OK ){
						nop();		// エラー処理の記述	
					}
				} else if ( msg[ 0 ] == LCD_ERR_REQ ){					//  エラーを検出した
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN625 );
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN625 );					// 画面番号　<-　次の画面
					}													
				}

			}
			break;
		case LCD_SCREEN623:		// 認証完了○画面。	
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
					
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN621 );	// 通常モードの待機画面へ。

			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN621 );		// 画面番号　<-　次の画面
				if ( Pfail_mode_count == 0 ){ 
					MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ（必要無いが念のため）
				}	else	{
					MdCngMode( MD_PFAIL );			// 装置モードを	停電モードへ（必要無いが念のため）
				}
			}
			break;
		
		case LCD_SCREEN624:		// 認証完了×画面。
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN621 );	// 通常モードのメニュー画面へ。
			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN621 );		// 画面番号　<-　次の画面
				if ( Pfail_mode_count == 0 ){ 
					MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ（必要無いが念のため）
				}	else	{
					MdCngMode( MD_PFAIL );			// 装置モードを	停電モードへ（必要無いが念のため）
				}
			}
			break;

		case LCD_SCREEN625:		// 「エラー表示」画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_BACK ){							// 「戻る」が押された
				
					//エラーが10個以上ある場合
					cnt1 = (int)g_MainteLog.err_wcnt;
					cnt2 = (int)g_MainteLog.err_rcnt;
					if( cnt1 != cnt2 ){
						if( cnt1 > 0x7F || cnt2 > 0x7F ){
							g_MainteLog.err_wcnt = 0;
							g_MainteLog.err_rcnt = 0;
						}else{
							if( cnt1 > cnt2 ){
								cnt0 = cnt1 - cnt2;
							}else{
								cnt0 = 128 - cnt2;
								cnt0 = cnt0 + cnt1;
							}
							if( cnt0 > 10 ){
								cnt2 = cnt1 - 10;
								if( cnt2 < 0 ){
									cnt2 = 128 + cnt2;
								}
							}
							g_MainteLog.err_rcnt = (unsigned short)cnt2;
						}
					}

					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN621 );	/// 通常モードの待機画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN621 );		// 画面番号　<-　次の画面
						if ( Pfail_mode_count == 0 ){ 
							MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ（必要無いが念のため）
						}	else	{
							MdCngMode( MD_PFAIL );			// 装置モードを	停電モードへ（必要無いが念のため）
						}
					}
				}
			}
			break;

		default:
			break;
	}
}


/*==========================================================================*/
/**
 *	次の画面表示要求フラグセット（共通仕様の場合）
 */
/*==========================================================================*/
ER NextScrn_Control( void )
{
	ER ercd;
	UB S_No;
	ER_UINT i;
	ER_UINT rcv_length;
	UB msg[ 128 ];
	UB tmp;	
	unsigned short dat;
	
	S_No = GetScreenNo();
	
	switch ( S_No ) {
		

	// メンテナンス画面
		case LCD_SCREEN200:		// メンテナンス・モードのブランク画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			send_sio_BPWR(FLG_ON);//20160905Miya B-PWR制御

			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN201 );	// メンテナンス・モードのID番号入力画面
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN201 );		// 画面番号　<-　次の画面
				}					
			}
			break;

		case LCD_SCREEN201:		// メンテナンス画面のパスワード番号入力画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){			
				if ( ( ( msg[ 0 ] == LCD_NEXT ) || ( msg[ 0 ] == LCD_USER_ID ) ) && ( rcv_length >= 5 ) ){
				
					mainte_password[ 0 ] = msg[ 1 ];	// パスワード番号
					mainte_password[ 1 ] = msg[ 2 ];
					mainte_password[ 2 ] = msg[ 3 ];
					mainte_password[ 3 ] = msg[ 4 ];
					mainte_password[ 4 ] = 0;
#if ( VA300S == 0 )				
					send_password_chk_Wait_Ack_Retry();	// PCへ、パスワードの一致確認要求送信。
#else

#if(KOUJYOUCHK == 1 || AUTHTEST >= 1)
					if(msg[1] == '9' &&  msg[2] == '9' && msg[3] == '9' && msg[4] == '9'){
						mainte_password[ 0 ] = '3';	// パスワード番号
						mainte_password[ 1 ] = '2';
						mainte_password[ 2 ] = '2';
						mainte_password[ 3 ] = '7';
					}
#endif

					if( mainte_password[0] == '3' && mainte_password[1] == '2' && mainte_password[2] == '2' && mainte_password[3] == '7' ){
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN202 );	// メンテナンス・メニュー画面へ。
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN202 );		// 画面番号　<-　次の画面
							MdCngMode( MD_MAINTE );				// 装置モードをメンテナンスモードへ
							ercd = set_flg( ID_FLG_MAIN, FPTN_SEND_REQ_MAINTE_CMD );	// メインTaskへ、メンテナンスモード切替え通知送信を依頼。
							if ( ercd != E_OK ){
								nop();		// エラー処理の記述	
							}
						}
					}else{
						if ( GetSysSpec() == SYS_SPEC_MANTION ){	// マンション占有部仕様
							ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// 通常モード・初期画面へ。
							if ( ercd == E_OK ){
								ChgScreenNo( LCD_SCREEN100 );		// 画面番号　<-　次の画面
							} else {
								nop();			// エラー処理の記述
							}
						} else if ( GetSysSpec() == SYS_SPEC_OFFICE ){//　１対１仕様（オフィス仕様）
							ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN500 );	// 通常モード・初期画面へ。
							if ( ercd == E_OK ){
								ChgScreenNo( LCD_SCREEN500 );		// 画面番号　<-　次の画面
							} else {
								nop();		// エラー処理の記述
							}
						} else if ( GetSysSpec() == SYS_SPEC_KOUJIM || GetSysSpec() == SYS_SPEC_KOUJIO || GetSysSpec() == SYS_SPEC_KOUJIS ){
							ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN620 );	// 工事画面へ。
							if ( ercd == E_OK ){
								ChgScreenNo( LCD_SCREEN620 );		// 画面番号　<-　次の画面
							} else {
								nop();		// エラー処理の記述
							}
						}
					}	
#endif								
					// メンテナンス・メニュー画面への遷移は、パスワードの一致確認要求送信の結果がOKだった時、その受信コマンド処理で実行する。。	

				} else if ( msg[ 0 ] == LCD_CANCEL ){	// 「中止」ボタンが押された

					if ( GetSysSpec() == SYS_SPEC_MANTION ){ 	// マンション・占有部仕様の場合
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// 通常モード初期画面へ。
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN100 );		// 画面番号　<-　次の画面
							MdCngMode( MD_NORMAL );				// 装置モードを通常モードへ
#if ( VA300S == 0 ) 
							ercd = SndCmdCngMode( (UINT)MD_NORMAL );	// PCへ	通常モード切替え通知を送信
#endif	
							}
					} else if ( GetSysSpec() == SYS_SPEC_OFFICE ){//　１対１仕様（オフィス仕様）
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN500 );	// 通常モード初期画面へ。
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN500 );		// 画面番号　<-　次の画面
							MdCngMode( MD_NORMAL );				// 装置モードを通常モードへ
#if ( VA300S == 0 ) 
							ercd = SndCmdCngMode( (UINT)MD_NORMAL );	// PCへ	通常モード切替え通知を送信
#endif	
						}
					} else if ( GetSysSpec() == SYS_SPEC_KOUJIM || GetSysSpec() == SYS_SPEC_KOUJIO || GetSysSpec() == SYS_SPEC_KOUJIS ){
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN620 );	// 工事画面へ。
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN620 );		// 画面番号　<-　次の画面
						} else {
							nop();		// エラー処理の記述
						}
					}
				}
			}
			break;
			
		case LCD_SCREEN202:		// メンテナンス・メニュー画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( rcv_length >= 1 ){

//20140925Miya cng mainte
				if ( msg[ 0 ] == LCD_JYOUHOU_REQ ){	// 情報ボタン
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN220 );	// 「バージョン表示」画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN220 );		// 画面番号　<-　次の画面
					}			
				} else if ( msg[ 0 ] == LCD_SETTEI_HENKOU_REQ ){			// ”設定変更”ボタンが押された
					if(g_MainteLvl == 1){	//20160711Miya デモ機
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN202 );
						if ( ercd == E_OK ){
							dbg_Auth_hcnt = 0;	//20141014Miya
							ChgScreenNo( LCD_SCREEN202 );		// 画面番号　<-　次の画面
						}
						break;
					}
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN240 );	// 「設定変更」画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN240 );		// 画面番号　<-　次の画面
					}
				} else if ( msg[ 0 ] == LCD_SINDAN_REQ ){			// ”診断”ボタンが押された
					if(g_MainteLvl == 1){	//20160711Miya デモ機
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN202 );
						if ( ercd == E_OK ){
							dbg_Auth_hcnt = 0;	//20141014Miya
							ChgScreenNo( LCD_SCREEN202 );		// 画面番号　<-　次の画面
						}
						break;
					}
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN203 );	// 「しばらくおまちください」画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN203 );		// 画面番号　<-　次の画面
					}			
				} else if ( msg[ 0 ] == LCD_SYOKI_SETTEI_REQ ){			// ”初期設定”ボタンが押された
					//20160112Miya FinKeyS
					if( GetSysSpec() == SYS_SPEC_KOUJIM || GetSysSpec() == SYS_SPEC_KOUJIO || GetSysSpec() == SYS_SPEC_KOUJIS ){
						dip_sw_data[0] = 1;
					}
				
					if(dip_sw_data[0] == 1){
						//g_cmr_dbgcnt = 0;	//20160610Miya
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN260 );	// 「技術メンテメニュー」画面へ。
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN260 );		// 画面番号　<-　次の画面
						}
					}else{
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN202 );	// 「技術メンテメニュー」画面へ。
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN202 );		// 画面番号　<-　次の画面
						}
					}
/*
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN206 );	// 初期化移行「削除してもよろしいですか？」画面へ
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN206 );		// 画面番号　<-　次の画面
					}
*/
				} else if ( msg[ 0 ] == LCD_MAINTE_END ){	// 終了ボタン

					if ( GetSysSpec() == SYS_SPEC_MANTION ){ 	// マンション・占有部仕様の場合
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// 通常モード画面へ。
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN100 );		// 画面番号　<-　次の画面
							MdCngMode( MD_NORMAL );				// 装置モードを通常モードへ
#if ( VA300S == 0 ) 
							ercd = SndCmdCngMode( (UINT)MD_NORMAL );	// PCへ	通常モード切替え通知を送信
#endif
						}
					}else if( GetSysSpec() == SYS_SPEC_KOUJIM || GetSysSpec() == SYS_SPEC_KOUJIO || GetSysSpec() == SYS_SPEC_KOUJIS ){
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN620 );	// 通常モード画面へ。
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN620 );		// 画面番号　<-　次の画面
							MdCngMode( MD_INITIAL );
						}
					} else {									// １対１認証仕様の場合
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN500 );	// 通常モード画面へ。
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN500 );		// 画面番号　<-　次の画面
							MdCngMode( MD_NORMAL );				// 装置モードを通常モードへ
#if ( VA300S == 0 )
							ercd = SndCmdCngMode( (UINT)MD_NORMAL );	// PCへ	通常モード切替え通知を送信
#endif
						}						
					}
				}


/*
				if ( msg[ 0 ] == LCD_MAINTE_SHOKIKA_REQ ){	// 初期化ボタン
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN206 );	// 初期化移行「削除してもよろしいですか？」画面へ
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN206 );		// 画面番号　<-　次の画面
					}
					
				} else if ( msg[ 0 ] == LCD_JYOUHOU_REQ ){			// ”情報”ボタンが押された
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN207 );	// 「バージョン表示」画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN207 );		// 画面番号　<-　次の画面
					}			
					
				} else if ( msg[ 0 ] == LCD_SPEC_CHG_REQ ){			// ,			// ”仕様切替”ボタンが押された
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN208 );	// 「仕様切替」画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN208 );		// 画面番号　<-　次の画面
					}
										
				} else if ( msg[ 0 ] == LCD_FULL_PIC_SEND_REQ ){	// フル画像送信ボタン
#if ( VA300S == 2 ) 
					DebugSendCmd_210();
#else
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN203 );	// 「指をセットして...」画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN203 );		// 画面番号　<-　次の画面
					}			
#endif			
				} else if ( msg[ 0 ] == LCD_MAINTE_END ){	// 終了ボタン
					if ( GetSysSpec() == SYS_SPEC_MANTION ){ 	// マンション・占有部仕様の場合
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// 通常モード画面へ。
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN100 );		// 画面番号　<-　次の画面
							MdCngMode( MD_NORMAL );				// 装置モードを通常モードへ
#if ( VA300S == 0 ) 
							ercd = SndCmdCngMode( (UINT)MD_NORMAL );	// PCへ	通常モード切替え通知を送信
#endif
						}
					} else {									// １対１認証仕様の場合
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN500 );	// 通常モード画面へ。
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN500 );		// 画面番号　<-　次の画面
							MdCngMode( MD_NORMAL );				// 装置モードを通常モードへ
#if ( VA300S == 0 )
							ercd = SndCmdCngMode( (UINT)MD_NORMAL );	// PCへ	通常モード切替え通知を送信
#endif
						}						
					}
				}
*/		
			break;
			
		case LCD_SCREEN203:		// 「しばらくおまちください」画面
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 0 ){
				g_Diagnosis_start = 1;
			}
			break;				// 生体センサー検知待ち。LCDタスクは、ここへのchange Reqは出さないはず。

			
		case LCD_SCREEN204:		// フル画像送信・指画像の取得成功○画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 0 ){
				g_Diagnosis_start = 0;
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN202 );	// メンテナンス・メニュー画面へ
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN202 );		// 画面番号　<-　次の画面
				}
			}
			break;
			
		case LCD_SCREEN205:		// フル画像送信・指画像の取得失敗×画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN202 );	// メンテナンス・メニュー画面へ
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN202 );		// 画面番号　<-　次の画面
				}
			}
			break;
			
		case LCD_SCREEN206:		// 初期化移行「削除してもよろしいですか？」画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_YES ){				// 「はい」が押された
				
#if ( VA300S == 0 )
					send_touroku_init_Wait_Ack_Retry();		// PCへ、登録データの初期化要求送信。
					if ( ercd == E_OK ){
						req_restart = 1;			// パワーオンプロセスの再起動要求フラグのセット。
					}
#else
					ercd = InitBkAuthData();
					send_sio_Touroku_AllDel();		// VA300S制御Boxへシリアルで登録データ初期化（一括削除）コマンド(05)を送信。
					MdCngSubMode( SUB_MD_ALLDEL );		// 一括削除送信中（VA300ｓ）
					//ercd = InitRegImgArea();
					//FpgaSetWord(FPGA_RECONF, 0x01, 0x01);
					while ( MdGetSubMode() != SUB_MD_IDLE ){	// ACK受信するまで待機。
						dly_tsk( 25/MSEC );			
					}
					req_restart = 1;			// パワーオンプロセスの再起動要求フラグのセット。
#endif
										
				} else if ( msg[ 0 ] == LCD_NO ){		// 「いいえ」が押された
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN202 );	// メンテナンス・メニュー選択画面
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN202 );		// 画面番号　<-　次の画面
					}					
				}
			}
			break;
			
		case LCD_SCREEN207:		// 「バージョン表示」画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_BACK ){			// 「戻る」が押された
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN202 );	// メンテナンス・メニュー画面へ
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN202 );		// 画面番号　<-　次の画面
					}
				}
			}
			break;
			
		case LCD_SCREEN208:		// 仕様切替え画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_BACK ){				// 「戻る」が押された
				
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN202 );	// メンテナンス・メニュー画面へ
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN202 );		// 画面番号　<-　次の画面
					}
										
				} else if ( ( msg[ 0 ] == LCD_ENTER ) && ( rcv_length >= 3 ) ){	// 「確定」が押された
				
					if ( ( msg[ 1 ] >= 0 ) && ( msg[ 1 ] <= 2 ) ){	// va300.h "enum SYS_SPEC"を参照のこと。
						if ( ( msg[ 2 ] != SYS_SPEC_DEMO ) ){
							tmp = msg[ 1 ];					// DEMOモードでない場合
						} else {
							tmp = msg[ 1 ] + 3;				// DEMOモードの場合
						}
						ercd = lan_set_eep( EEP_SYSTEM_SPEC, tmp );	// 設定仕様情報のEEPROMへの書込み。
						if (ercd != E_OK) {
							// エラー処理の記述。
						}

#if ( VA300S == 0 )
						send_touroku_init_Wait_Ack_Retry();		// PCへ、登録データの初期化要求送信。
						if ( ercd == E_OK ){
							req_restart = 1;			// パワーオンプロセスの再起動要求フラグのセット。
						}
#else
						ercd = InitBkAuthData();
						req_restart = 1;			// パワーオンプロセスの再起動要求フラグのセット。
#endif
					}				
				}
			}
			break;
						
		}		 

//20140925Miya cng mainte
		case LCD_SCREEN220:		// メンテナンス・モードのブランク画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN221 );	// メンテナンス・モードのID番号入力画面
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN221 );		// 画面番号　<-　次の画面
				}					
			}
			break;
		case LCD_SCREEN221:		// メンテナンス・メニュー画面(情報)
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_VERSION_REQ ){							// バージョンボタン
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN222 );		// 「バージョン表示」画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN222 );		// 画面番号　<-　次の画面
					}			
				}else if ( msg[ 0 ] == LCD_ERROR_REREKI_REQ ){				// エラー履歴ボタン
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN223 );		// 「エラー履歴表示」画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN223 );		// 画面番号　<-　次の画面
					}			
				}else if ( msg[ 0 ] == LCD_NINSYOU_JYOUKYOU_REQ ){			// 認証状況ボタン
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN224 );		// 「認証状況表示」画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN224 );		// 画面番号　<-　次の画面
					}			
				}else if ( msg[ 0 ] == LCD_JIKOKU_HYOUJI_REQ ){					// 未使用ボタン
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN225 );
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN225 );		// 画面番号　<-　次の画面
					}
				}else if ( msg[ 0 ] == LCD_MAINTE_END ){					// 終了ボタン
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN202 );		// メンテナンス・メニュー画面へ
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN202 );		// 画面番号　<-　次の画面
					}
				}
			}
			break;
		case LCD_SCREEN222:		// 「バージョン表示」画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_BACK ){							// 「戻る」が押された
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN221 );	// メンテナンス・メニュー画面(情報)へ
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN221 );		// 画面番号　<-　次の画面
					}
				}
			}
			break;
		case LCD_SCREEN223:		// 「エラー履歴表示」画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_BACK ){							// 「戻る」が押された
					g_MainteLog.cmr_seq_err_f = 0;
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN221 );	// メンテナンス・メニュー画面(情報)へ
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN221 );		// 画面番号　<-　次の画面
					}
				}
			}
			break;
		case LCD_SCREEN224:		// 「認証状況表示」画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_BACK ){							// 「戻る」が押された
/*				
					if(dbg_cap_flg == 0){
						dbg_cap_flg = 1;
					}else if(dbg_cap_flg == 1){
						dbg_cap_flg = 2;
					}else if(dbg_cap_flg == 2){
						dbg_cap_flg = 3;
					}else if(dbg_cap_flg == 3){
						dbg_cap_flg = 4;
					}else if(dbg_cap_flg == 4){
						dbg_cap_flg = 1;
					}
*/

#if(AUTHTEST >= 1)	//20160613Miya
					if(g_sv_okcnt > 0){
						ReadTestRegImgArea(0, 0, 0, 0);
						SaveTestRegImgFlArea(0);
						g_sv_okcnt = 0;
					}
#endif

					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN221 );	// メンテナンス・メニュー画面(情報)へ
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN221 );		// 画面番号　<-　次の画面
					}
				}
			}
			break;
		case LCD_SCREEN225:		// 「時刻状況表示」画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_BACK ){							// 「戻る」が押された
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN221 );	// メンテナンス・メニュー画面(情報)へ
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN221 );		// 画面番号　<-　次の画面
					}
				}
			}
			break;
		case LCD_SCREEN240:		// メンテナンス・モードのブランク画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN241 );	// メンテナンス・モードのID番号入力画面
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN241 );		// 画面番号　<-　次の画面
				}					
			}
			break;
		case LCD_SCREEN241:		// メンテナンス・メニュー画面(設定変更)
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_PASS_KAIJYOU_REQ ){					// パスワード開錠ボタン
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN242 );		// 「パスワード開錠」画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN242 );		// 画面番号　<-　次の画面
					}			
				}else if ( msg[ 0 ] == LCD_CALLCEN_TEL_REQ ){				// コールセンターTELボタン
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN243 );		// 「コールセンターTEL表示」画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN243 );		// 画面番号　<-　次の画面
					}			
				}else if ( msg[ 0 ] == LCD_JIKOKU_AWASE_REQ ){				// 時刻合わせボタン
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN246 );		// 「時刻合わせ表示」画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN246 );		// 画面番号　<-　次の画面
					}			
				}else if ( msg[ 0 ] == LCD_ITI_TYOUSEI_REQ ){				// LCD位置調整ボタン
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN248 );		// 「LCD位置調整表示」画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN248 );		// 画面番号　<-　次の画面
					}			
				}else if ( msg[ 0 ] == LCD_MAINTE_END ){					// 終了ボタン
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN202 );		// メンテナンス・メニュー画面へ
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN202 );		// 画面番号　<-　次の画面
					}
				}
			}
			break;
		case LCD_SCREEN242:		// 「パスワード開錠」画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_BACK ){				// 「戻る」が押された
				
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN241 );	// メンテナンス・メニュー画面(設定変更)へ
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN241 );		// 画面番号　<-　次の画面
					}
										
				} else if ( ( msg[ 0 ] == LCD_ENTER ) && ( rcv_length >= 5 ) ){	// 「確定」が押された
					if( msg[ 1 ] == 1 )	g_PasswordOpen.hide_num = FLG_ON;
					else				g_PasswordOpen.hide_num = FLG_OFF;

					if( msg[ 2 ] == 1 )	g_PasswordOpen.kigou_inp = FLG_ON;
					else				g_PasswordOpen.kigou_inp = FLG_OFF;

					if( msg[ 3 ] == 1 )	g_PasswordOpen.random_key = FLG_ON;
					else				g_PasswordOpen.random_key = FLG_OFF;

					if( msg[ 4 ] == 1 )	g_PasswordOpen2.family_sw = FLG_ON;		//20160112Miya FinKeyS
					else				g_PasswordOpen2.family_sw = FLG_OFF;

					if( msg[ 5 ] == 1 )	g_PasswordOpen.sw = FLG_ON;
					else				g_PasswordOpen.sw = FLG_OFF;

					for(i = 0 ; i < 10 ; i++ ){
						g_key_arry[i] = i;	//パスワード開錠用キー配列初期化
					}
				
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN249 );	// しばらくおまちください画面へ
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN249 );		// 画面番号　<-　次の画面
					}
				}
			}
			break;
		case LCD_SCREEN243:		// コールセンター電話番号変更(市外局番)画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){	
				if ( ( ( msg[ 0 ] == LCD_NEXT ) || ( msg[ 0 ] == LCD_CALLCEN_TEL_REQ ) ) && ( rcv_length >= 1 ) ){								
					g_RegUserInfoData.MainteTelNum[ 0 ] = msg[ 1 ];		// 入力された緊急番号
					g_RegUserInfoData.MainteTelNum[ 1 ] = msg[ 2 ];		// 入力された緊急番号
					g_RegUserInfoData.MainteTelNum[ 2 ] = msg[ 3 ];		// 入力された緊急番号
					g_RegUserInfoData.MainteTelNum[ 3 ] = msg[ 4 ];		// 入力された緊急番号
					g_RegUserInfoData.MainteTelNum[ 4 ] = 0x20;
					g_RegUserInfoData.MainteTelNum[ 5 ] = 0x20;
					g_RegUserInfoData.MainteTelNum[ 6 ] = 0x20;
					g_RegUserInfoData.MainteTelNum[ 7 ] = 0x20;
				
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN244 );
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN244 );		// 画面番号　<-　次の画面
					}	
				} else if ( msg[ 0 ] == LCD_CANCEL ){	// 「中止」ボタンが押された
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN241 );	// メンテナンス・メニュー画面(設定変更)
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN241 );		// 画面番号　<-　次の画面
					}	
				}
			}
			break;
		case LCD_SCREEN244:		// コールセンター電話番号変更(市内局番)画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){	
				if ( ( ( msg[ 0 ] == LCD_NEXT ) || ( msg[ 0 ] == LCD_CALLCEN_TEL_REQ ) ) && ( rcv_length >= 1 ) ){								
					g_RegUserInfoData.MainteTelNum[ 8 ] = msg[ 1 ];		// 入力された緊急番号
					g_RegUserInfoData.MainteTelNum[ 9 ] = msg[ 2 ];		// 入力された緊急番号
					g_RegUserInfoData.MainteTelNum[ 10 ] = msg[ 3 ];		// 入力された緊急番号
					g_RegUserInfoData.MainteTelNum[ 11 ] = msg[ 4 ];		// 入力された緊急番号
				
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN245 );
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN245 );		// 画面番号　<-　次の画面
					}	
				} else if ( msg[ 0 ] == LCD_CANCEL ){	// 「中止」ボタンが押された
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN241 );	// メンテナンス・メニュー画面(設定変更)
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN241 );		// 画面番号　<-　次の画面
					}	
				}
			}
			break;
		case LCD_SCREEN245:		// コールセンター電話番号変更(加入者番号)画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){	
				if ( ( ( msg[ 0 ] == LCD_NEXT ) || ( msg[ 0 ] == LCD_CALLCEN_TEL_REQ ) ) && ( rcv_length >= 1 ) ){								
					g_RegUserInfoData.MainteTelNum[ 12 ] = msg[ 1 ];		// 入力された緊急番号
					g_RegUserInfoData.MainteTelNum[ 13 ] = msg[ 2 ];		// 入力された緊急番号
					g_RegUserInfoData.MainteTelNum[ 14 ] = msg[ 3 ];		// 入力された緊急番号
					g_RegUserInfoData.MainteTelNum[ 15 ] = msg[ 4 ];		// 入力された緊急番号

					for( i = 0 ; i < 16 ; i++ ){
						kinkyuu_tel_no[i] = g_RegUserInfoData.MainteTelNum[i];
					}
				
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN249 );	// しばらくおまちください画面
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN249 );		// 画面番号　<-　次の画面
					}	
				} else if ( msg[ 0 ] == LCD_CANCEL ){	// 「中止」ボタンが押された
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN241 );	// メンテナンス・メニュー画面(設定変更)
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN241 );		// 画面番号　<-　次の画面
					}	
				}
			}
			break;
		case LCD_SCREEN246:		// 現在時刻入力画面
//20161031Miya Ver2204 LCDADJ ->		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			if ( rcv_length >= 1 ){	
				if ( msg[ 0 ] == LCD_JIKOKU_AWASE_REQ ){
					LcdPosAdj(1);
					SaveBkDataNoClearFl();								
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN241 );	// メンテナンス・メニュー画面(設定変更)
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN241 );		// 画面番号　<-　次の画面
					}	
				} else if ( msg[ 0 ] == LCD_CANCEL ){	// 「中止」ボタンが押された
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN241 );	// メンテナンス・メニュー画面(設定変更)
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN241 );		// 画面番号　<-　次の画面
					}	
				}
			}
//20161031Miya Ver2204 LCDADJ <-		

/*
			if ( rcv_length >= 1 ){	
				if ( ( ( msg[ 0 ] == LCD_NEXT ) || ( msg[ 0 ] == LCD_JIKOKU_AWASE_REQ ) ) && ( rcv_length >= 1 ) ){								
					dat = 10 * ((unsigned short)msg[ 1 ] - 0x30);
					dat = dat + ((unsigned short)msg[ 2 ] - 0x30);
					g_MainteLog.now_hour = dat;
					dat = 10 * ((unsigned short)msg[ 3 ] - 0x30);
					dat = dat + ((unsigned short)msg[ 4 ] - 0x30);
					g_MainteLog.now_min = dat;
					g_MainteLog.now_sec = 0;

					count_1hour = g_MainteLog.now_hour;		// "時"をLCD画面表示用メモリからストア
					count_1min = g_MainteLog.now_min;		// "分"を　　↓
					count_1sec = g_MainteLog.now_sec;		// "秒"を　　↓

#if ( VA300S == 1 )
					send_sio_init_time();		// VA300S制御Boxへシリアルで時刻の初期設定コマンド(10)を送信。
//					MdCngSubMode( SUB_MD_INIT_TIME );			// 時刻の初期設定コマンド（VA300ｓ）
//					while ( MdGetSubMode() != SUB_MD_IDLE ){	// ACK受信するまで待機。
//						dly_tsk( 25/MSEC );		
//					}
#endif

					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN247 );	// メンテナンス・メニュー画面(設定変更)
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN247 );		// 画面番号　<-　次の画面
					}	
				} else if ( msg[ 0 ] == LCD_CANCEL ){	// 「中止」ボタンが押された
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN241 );	// メンテナンス・メニュー画面(設定変更)
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN241 );		// 画面番号　<-　次の画面
					}	
				}
			}
*/
			break;

		case LCD_SCREEN247:		// 自己診断開始時刻入力画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){	
				if ( ( ( msg[ 0 ] == LCD_NEXT ) || ( msg[ 0 ] == LCD_JIKOKU_AWASE_REQ ) ) && ( rcv_length >= 1 ) ){								
					dat = 10 * ((unsigned short)msg[ 1 ] - 0x30);
					dat = dat + ((unsigned short)msg[ 2 ] - 0x30);
					g_MainteLog.st_hour = dat;
					dat = 10 * ((unsigned short)msg[ 3 ] - 0x30);
					dat = dat + ((unsigned short)msg[ 4 ] - 0x30);
					g_MainteLog.st_min = dat;
					g_MainteLog.st_sec = 0;

					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN249 );	// しばらくおまちください画面
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN249 );		// 画面番号　<-　次の画面
					}	
				} else if ( msg[ 0 ] == LCD_CANCEL ){	// 「中止」ボタンが押された
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN241 );	// メンテナンス・メニュー画面(設定変更)
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN241 );		// 画面番号　<-　次の画面
					}	
				}
			}
			break;

		case LCD_SCREEN248:		// 「LCD位置調整表示」画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_BACK ){							// 「戻る」が押された

#if ( VA300S == 1 )
					//20141120 強制施錠コマンド発行
					send_sio_force_lock_close();
#endif
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN241 );	// メンテナンス・メニュー画面(設定変更)へ
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN241 );		// 画面番号　<-　次の画面
					}
				}
			}
			break;

		case LCD_SCREEN249:		// 「しばらくおまちください」画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				//フラッシュに登録データを保存する
				//dly_tsk( 500/MSEC );
				ercd = SaveBkAuthDataFl();
				
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN241 );	// メンテナンス・メニュー画面(設定変更)へ
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN241 );		// 画面番号　<-　次の画面
				}
			}
			break;
		case LCD_SCREEN260:		// メンテナンス・モードのブランク画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN261 );	// メンテナンス・モードのID番号入力画面
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN261 );		// 画面番号　<-　次の画面
				}					
			}
			break;
		case LCD_SCREEN261:		// メンテナンス・メニュー画面(技術モード)
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_MAINTE_SHOKIKA_REQ ){					// 初期化ボタン
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN262 );		// 「初期化」画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN262 );		// 画面番号　<-　次の画面
					}			
				}else if ( msg[ 0 ] == LCD_SPEC_CHG_REQ ){				// 仕様切替ボタン
					if(g_MainteLvl == 1){	//20160711Miya デモ機
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN261 );
						if ( ercd == E_OK ){
							dbg_Auth_hcnt = 0;	//20141014Miya
							ChgScreenNo( LCD_SCREEN261 );		// 画面番号　<-　次の画面
						}
						break;
					}
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN263 );		// 「仕様切替」画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN263 );		// 画面番号　<-　次の画面
					}			
				}else if ( msg[ 0 ] == LCD_IMAGE_KAKUNIN_REQ ){				// 画像確認ボタン
					if(g_MainteLvl == 1){	//20160711Miya デモ機
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN261 );
						if ( ercd == E_OK ){
							dbg_Auth_hcnt = 0;	//20141014Miya
							ChgScreenNo( LCD_SCREEN261 );		// 画面番号　<-　次の画面
						}
						break;
					}
//#if(AUTHTEST >= 1)	//20160613Miya
#if(AUTHTEST >= 2)	//20160902Miya FPGA高速化 forDebug
					if( g_TechMenuData.DebugHyouji == FLG_OFF ){
						if(g_sv_okcnt0 > 0){
							dbg_Auth_8cnt = 0;
							//g_sv_okcnt = 0;
							ReadTestRegImgArea(0, 1, 1, 0);
						}else{
							ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN202 );		// メンテナンス・メニュー画面へ
							if ( ercd == E_OK ){
								ChgScreenNo( LCD_SCREEN202 );		// 画面番号　<-　次の画面
							}
							break;
						}
					}
#endif
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN264 );		// 「画像確認表示」画面へ。
					if ( ercd == E_OK ){
						dbg_Auth_hcnt = 0;	//20141014Miya
						ChgScreenNo( LCD_SCREEN264 );		// 画面番号　<-　次の画面
					}
				}else if ( msg[ 0 ] == LCD_NOUSE ){					// 未使用ボタン
					if(g_MainteLvl == 1){	//20160711Miya デモ機
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN261 );
						if ( ercd == E_OK ){
							dbg_Auth_hcnt = 0;	//20141014Miya
							ChgScreenNo( LCD_SCREEN261 );		// 画面番号　<-　次の画面
						}
						break;
					}
					g_TestCap_start = 0;
#if ( NEWCMR == 1 )	//20160613Miya
					g_cmr_dbgcnt = 0;
#endif
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN266 );
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN266 );		// 画面番号　<-　次の画面
					}
				}else if ( msg[ 0 ] == LCD_MAINTE_END ){					// 終了ボタン
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN202 );		// メンテナンス・メニュー画面へ
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN202 );		// 画面番号　<-　次の画面
					}
				}
			}
			break;
		case LCD_SCREEN262:		// メンテナンス・メニュー画面(技術モード)
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_YES ){				// 「はい」が押された
				
#if ( VA300S == 0 )
					send_touroku_init_Wait_Ack_Retry();		// PCへ、登録データの初期化要求送信。
					if ( ercd == E_OK ){
						req_restart = 1;			// パワーオンプロセスの再起動要求フラグのセット。
					}
#else

					if( GetSysSpec() == SYS_SPEC_KOUJIS || g_TechMenuData.SysSpec == 2 ){
						ercd = lan_set_eep( EEP_SYSTEM_SPEC, SYS_SPEC_SMT );	// 設定仕様情報のEEPROMへの書込み。
					}
					ercd = InitBkAuthData();
					send_sio_Touroku_AllDel();		// VA300S制御Boxへシリアルで登録データ初期化（一括削除）コマンド(05)を送信。
					MdCngSubMode( SUB_MD_ALLDEL );		// 一括削除送信中（VA300ｓ）
					//ercd = InitRegImgArea();
					//FpgaSetWord(FPGA_RECONF, 0x01, 0x01);
					while ( MdGetSubMode() != SUB_MD_IDLE ){	// ACK受信するまで待機。
						dly_tsk( 25/MSEC );		
					}
					req_restart = 1;			// パワーオンプロセスの再起動要求フラグのセット。
#endif
										
				} else if ( msg[ 0 ] == LCD_NO ){		// 「いいえ」が押された
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN261 );	// メンテナンス・メニュー選択画面
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN261 );		// 画面番号　<-　次の画面
					}					
				}else{
#if(KOUJYOUCHK)
		g_TechMenuData.SysSpec = 2;
		g_PasswordOpen.sw = 1;
		g_PasswordOpen2.family_sw = 1;
#endif
					if(g_TechMenuData.SysSpec == 2){	//20160112Miya FinKeyS
						ercd = lan_set_eep( EEP_SYSTEM_SPEC, SYS_SPEC_KOUJIS );	// 設定仕様情報のEEPROMへの書込み。

						ercd = InitBkAuthData();
						send_sio_Touroku_AllDel();		// VA300S制御Boxへシリアルで登録データ初期化（一括削除）コマンド(05)を送信。
						MdCngSubMode( SUB_MD_ALLDEL );		// 一括削除送信中（VA300ｓ）
						//ercd = InitRegImgArea();
						//FpgaSetWord(FPGA_RECONF, 0x01, 0x01);
						while ( MdGetSubMode() != SUB_MD_IDLE ){	// ACK受信するまで待機。
							dly_tsk( 25/MSEC );		
						}
						req_restart = 1;			// パワーオンプロセスの再起動要求フラグのセット。
					}else{
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN262 );	// メンテナンス・メニュー選択画面
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN262 );		// 画面番号　<-　次の画面
						}
					}					
				}
			}
		case LCD_SCREEN263:		// メンテナンス・メニュー画面(技術モード)
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				
				if ( msg[ 0 ] == LCD_BACK ){				// 「戻る」が押された
				
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN261 );	// メンテナンス・メニュー画面(技術モード)へ
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN261 );		// 画面番号　<-　次の画面
					}
										
				} else if ( ( msg[ 0 ] == LCD_ENTER ) && ( rcv_length >= 5 ) ){	// 「確定」が押された
					//if( msg[ 1 ] == 1 )	g_TechMenuData.SysSpec = FLG_ON;
					//else				g_TechMenuData.SysSpec = FLG_OFF;

					if( msg[ 1 ] == 0 ){
						g_TechMenuData.SysSpec = 0;
					}else if( msg[ 1 ] == 1 ){
						//g_TechMenuData.SysSpec = 1;
						g_TechMenuData.SysSpec = 0;
					}else if( msg[ 1 ] == 2 ){
						g_TechMenuData.SysSpec = 2;
						g_PasswordOpen.sw = 1;
						g_PasswordOpen2.family_sw = 1;
					}

					if( msg[ 2 ] == 1 )	g_TechMenuData.DemoSw = FLG_ON;
					else				g_TechMenuData.DemoSw = FLG_OFF;

					if( msg[ 3 ] == 1 ){
						g_TechMenuData.HijyouRemocon = FLG_ON;
#if ( VA300S == 1 )
						send_sio_WakeUp();					// Testコマンド送信（リモコン設定情報送信）
#endif
					} else	{
						g_TechMenuData.HijyouRemocon = FLG_OFF;
#if ( VA300S == 1 )
						send_sio_WakeUp();					// Testコマンド送信（リモコン設定情報送信）
#endif
					}

					if( msg[ 4 ] == 1 )	g_TechMenuData.DebugHyouji = FLG_ON;
					else				g_TechMenuData.DebugHyouji = FLG_OFF;
//20161031Miya Ver2204 ->
					if( msg[ 5 ] == 2 ){
						g_BkDataNoClear.LiveCnt = 0;	//20161031Miya Ver2204
					}

					if( msg[ 6 ] == 0 )			{g_BkDataNoClear.LedPosi = 0;}
					else if( msg[ 6 ] == 1 )	{g_BkDataNoClear.LedPosi = 1;}
					else						{g_BkDataNoClear.LedPosi = 0;}
//20161031Miya Ver2204 <-

					for(i = 0 ; i < 10 ; i++ ){
						g_key_arry[i] = i;	//パスワード開錠用キー配列初期化
					}
				
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN265);	// しばらくおまちください画面へ
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN265 );		// 画面番号　<-　次の画面
					}
				}
			}
			break;
		case LCD_SCREEN264:		// メンテナンス・メニュー画面(技術モード)
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
#if(AUTHTEST >= 1)	//20160613Miya
			if ( rcv_length >= 1 ){
				if( g_TechMenuData.DebugHyouji == FLG_ON ){
#if ( VA300S == 2 ) 
					DebugSendCmd_210();
#endif
					dbg_Auth_hcnt++;
					if(dbg_Auth_hcnt >= g_RegUserInfoData.TotalNum ){
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN261 );	// メンテナンス・メニュー画面(設定変更)へ
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN261 );		// 画面番号　<-　次の画面
						}
					}else{
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN264 );	// メンテナンス・メニュー画面(設定変更)へ
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN264 );		// 画面番号　<-　次の画面
						}
					}
				}else{
#if ( VA300S == 2 ) 
					DebugSendCmd_210();
#endif
					dbg_Auth_hcnt++;
					if(	dbg_Auth_hcnt == 8 ){
						dbg_Auth_hcnt = 0;
						++dbg_Auth_8cnt;
						if(dbg_Auth_8cnt < 4){
							ReadTestRegImgArea(0, 1, 1, dbg_Auth_8cnt);
						}else if(dbg_Auth_8cnt < 8){
							ReadTestRegImgArea(0, 1, 2, (dbg_Auth_8cnt - 4));
						}else if(dbg_Auth_8cnt < 12){
							ReadTestRegImgArea(0, 1, 3, (dbg_Auth_8cnt - 8));
						}else{
							ReadTestRegImgArea(0, 1, 4, (dbg_Auth_8cnt - 12));
						}
					}
					
					if(g_sv_okcnt0 <= 8 * dbg_Auth_8cnt + dbg_Auth_hcnt){
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN261 );	// メンテナンス・メニュー画面(設定変更)へ
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN261 );		// 画面番号　<-　次の画面
						}
					}else{
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN264 );	// メンテナンス・メニュー画面(設定変更)へ
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN264 );		// 画面番号　<-　次の画面
						}
					}
				}
			}
#else
			if ( rcv_length >= 1 ){
				if( g_TechMenuData.DebugHyouji == FLG_ON ){
#if ( VA300S == 2 ) 
					DebugSendCmd_210();
#endif
					dbg_Auth_hcnt++;
					if(dbg_Auth_hcnt >= g_RegUserInfoData.TotalNum ){
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN261 );	// メンテナンス・メニュー画面(設定変更)へ
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN261 );		// 画面番号　<-　次の画面
						}
					}else{
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN264 );	// メンテナンス・メニュー画面(設定変更)へ
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN264 );		// 画面番号　<-　次の画面
						}
					}
				}else{
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN261 );	// メンテナンス・メニュー画面(設定変更)へ
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN261 );		// 画面番号　<-　次の画面
					}
				}
			}
#endif
			break;
		 
		case LCD_SCREEN265:		// 「しばらくおまちください」画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				//フラッシュに登録データを保存する
				ercd = SaveBkAuthDataFl();
				ercd = SaveBkDataNoClearFl();	//20161031Miya Ver2204 LCDADJ
				
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN261 );	// メンテナンス・メニュー画面(設定変更)へ
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN261 );		// 画面番号　<-　次の画面
				}
			}
			break;
		case LCD_SCREEN266:		// メンテナンス・メニュー画面(技術モード)
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			g_TestCap_start = 1;

			break;
		 
		case LCD_SCREEN267:		// メンテナンス・メニュー画面(技術モード)
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				//20160601Miya forDebug
#if ( VA300S == 2 ) 
				DebugSendCmd_210();
#endif

#if ( NEWCMR == 1 )	//20150613Miya
				if(g_cmr_dbgcnt > 0){
					g_TestCap_start = 0;
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN266 );	// メンテナンス・メニュー画面(設定変更)へ
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN266 );		// 画面番号　<-　次の画面
					}
				}else{
					g_TestCap_start = 0;
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN261 );	// メンテナンス・メニュー画面(設定変更)へ
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN261 );		// 画面番号　<-　次の画面
					}
				}
#else
				g_TestCap_start = 0;
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN261 );	// メンテナンス・メニュー画面(設定変更)へ
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN261 );		// 画面番号　<-　次の画面
				}
#endif				
				
			}
			break;
		 
		default:
			break;
	}
}

/*==========================================================================*/
/**
 *	認証操作中かどうかをチェックする
 */
/*==========================================================================*/
static UB Chk_shutdown_ok( void )
{
	UB ret;
	
	if ( ( ( GetScreenNo() >= LCD_SCREEN100 ) && ( GetScreenNo() == LCD_SCREEN104 ) )	// マンション(占有部)仕様/・認証操作中の場合
	  || ( ( GetScreenNo() >= LCD_SCREEN170 ) && ( GetScreenNo() <= LCD_SCREEN178 )	)	// 緊急開錠中
	  || ( ( GetScreenNo() == LCD_SCREEN500 ) && ( GetScreenNo() <= LCD_SCREEN506 )	)  	// マンション(１対１)仕様・認証操作中の場合
	  || ( MdGetMode() == MD_CAP ) ){
		  
		ret = SHUTDOWN_NO;
		
	}	else	{
		ret = SHUTDOWN_OK;
	}
	
	return ret;
}

/*==========================================================================*/
/**
 *	停電モード、停電検知通知受信の場合の、シャットダウン実行
 */
/*==========================================================================*/
static ER Pfail_shutdown( void )
{
	ER ercd;
	
	ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN999 );	// 「シャットダウンします」画面の表示を要求
	if ( ercd == E_OK ){
		ChgScreenNo( LCD_SCREEN999 );		// 画面番号　<-　次の画面
	}
	
#if ( VA300S == 1 || VA300S == 2 )
	//ercd = SaveBkAuthDataFl();
	//ercd = SaveRegImgFlArea(0);
	//ercd = SaveRegImgFlArea(10);

	LedOut(LED_OK, LED_ON);
	dly_tsk( 5000/MSEC );
	LedOut(LED_ERR, LED_ON);
	send_sio_ShutDown();					// SIO経由で、VA-300ｓへシャットダウン要求の送信

#else
	send_shutdown_req_Wait_Ack_Retry(); 	// シャットダウン要求の送信
	
	slp_tsk();
	
#endif	


	return ercd;
}