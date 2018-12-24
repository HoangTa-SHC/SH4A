/**
*	VA-300プログラム
*
*	@file tsk_rcv_serial.c
*	@version 1.00
*
*	@author Bionics Co.Ltd T.Nagai
*	@date   2013/10/04
*	@brief  シリアル受信タスク
*
*	Copyright (C) 2013, Bionics Co.Ltd
*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <kernel.h>
#include "sh7750.h"
#include "nosio.h"
#include "nonet.h"
#include "id.h"
#include "err_ctrl.h"

#include "va300.h"
#include "command.h"
#include "version.h"


// 受信データ領域
//#define	RCV_BUF_SIZE	1024 + 4
static UB cSioRcvBuf[ RCV_BUF_SIZE ];	// シリアル受信データバッファ
static unsigned short usSioRcvCount;	// シリアル受信データ数

static UINT sio_mode;
static UINT comm_data_stat;
static UINT Rcv_chr_timer;
static UINT EOT_Wait_timer;
static UINT ACK_Wait_timer;
static UINT Rcv_chr_timer;

static UB send_Ack_buff[ 4 ];  // ACK送信コードデータ・バッファ
static UB send_Nack_buff[ 4 ]; // ACK送信コードデータ・バッファ
static UB send_EOT_buff[ 4 ];  // ACK送信コードデータ・バッファ
static UINT ENQ_sending_cnt;	 // ENQ再送カウンタ
static UINT sio_rcv_comm_len;	 // 受信中コマンドの指定レングス
static UINT sio_rcv_block_num;	 // 受信中コマンドのブロック番号

static unsigned short usSioBunkatuNum;	//20160930Miya PCからVA300Sを制御する

static ER Rcv_serial_main_proc_forPC( int ch ); //20160930Miya PCからVA300Sを制御する // Serial Receive task　メイン処理
static ER Rcv_serial_main_proc( int ch ); // Serial Receive task　メイン処理
static int chk_rcv_cmddata( UB *Buf, int cmd_len  );	// 受信コマンドのチェックサム解析処理。
static int util_rcv_cmddata( UB *Buf );					// 受信コマンド解析（コマンド部の解析）
static int chk_ENQ_code( UB *cSioRcvBuf, UINT usSioRcvCount );		//　ENQコード解析処理
static int chk_ack_nak_code( UB *cSioRcvBuf, UINT usSioRcvCount );	//　Ack/Nakコード解析処理

void store_Test_cmddata( UB *Buf, int cmd_len );		// Testコマンド内データのストア
void store_Alarm_data( UB *Buff, int cmd_len );			// アラーム発生・消去報告コマンド内データのセット。
void store_Time_data( UB *Buff, int cmd_len );			// 時刻合わせコマンド内データのセット。

int send_Comm_Ack( INT ch );
int send_Comm_Nack( INT ch );
int send_Comm_EOT( INT ch );
void util_i_to_char4( unsigned int i, char *buff );
void util_char4_to_i( char * source, unsigned int *i_dist );
void util_i_to_char( unsigned int i, char *buff );
void util_char_to_i( char * source, unsigned int *i_dist );

/*********************************************
 * Receive task
 *
 * @param ch チャンネル番号
 *********************************************/
TASK RcvTask(INT ch)
{
	ER ercd;

	/* initialize */
//	ini_sio(ch, (B *)"115200 B8 PN S1");		//☆モニタプログラム用
//	ini_sio(ch, (B *)"38400 B8 PN S1");		//☆モニタプログラム用
	ini_sio(ch, (B *)"57600 B8 PN S1");		//☆モニタプログラム用
	
	ctl_sio(ch, TSIO_RXE|TSIO_TXE|TSIO_RTSON);
	memset( cSioRcvBuf, 0, RCV_BUF_SIZE);		// 受信バッファクリア
	usSioRcvCount = 0;							// シリアル受信でー多数クリア
	sio_mode = SIO_RCV_IDLE_MODE;

	for (;;)
	{

#if(PCCTRL == 1)	//20160930Miya PCからVA300Sを制御する
		ercd = Rcv_serial_main_proc_forPC( ch ); 	// Receive task　メイン処理
#else
		ercd = Rcv_serial_main_proc( ch ); 	// Receive task　メイン処理
#endif
		if ( ercd != E_OK ){
			nop();			// エラー処理の記述必要。
		}
	}

}



#if(PCCTRL == 1)	//20160930Miya PCからVA300Sを制御する
/*******************************************************************
* Receive task　メイン処理
*
********************************************************************/
static ER Rcv_serial_main_proc_forPC( int ch ) // Receive task　メイン処理
{
	unsigned char	code;
	ER ercd;
	int iercd;
	int ENQ_stat, ack_nak_stat;
	int rcv_nack_cnt, ACK_Wait_TimeOut;
	int th=0;
/**
enum SIO_MODE {
	SIO_RCV_IDLE_MODE,		// RS-232 コマンド受信/送信アイドル
	SIO_RCV_ENQ_MODE,		// RS-232 ENQ受信中状態
	SIO_RCV_WAIT_MODE,		// RS-232 制御Boxからのコマンド受信待ち状態
	SIO_RCV_CMD_MODE,		// RS-232 制御Boxからのコマンド受信中状態。
	SIO_SEND_MODE,			// コマンド送信中。
	SIO_SEND_ACK_WAIT		// コマンド送信済みで、ACK/NAKの受信待ち、および受付中。
};
**/

	//20140125Miya nagaiBug修正　
	//ACK_Wait_TimeOutの初期値設定がないため、通信異常時にACK_Wait_TimeOutが不定値につき終了できない
	ACK_Wait_TimeOut = 0;	//2014Miya add


	comm_data_stat = COMM_DATA_IDLE;

	for(;;){
		ercd = get_sio( ch, (UB*)&code, ( TMO )1000/MSEC /* TMO_FEVR */ );	// 受信データ待ち
		
		 sio_rcv_TSK_wdt = FLG_ON;			// SIO受信タスク　WDTカウンタリセット・リクエスト・フラグ
		
		if ( ercd == E_OK ){  
		//状態ごとの処理
			switch ( sio_mode ){
		
				case SIO_RCV_IDLE_MODE:		// RS-232 コマンド受信/送信アイドル
			
					rcv_nack_cnt = 0;
					ACK_Wait_TimeOut = 0;

					if ( code == '#' ) {		// ENQコードを受信
						usSioRcvCount = 0;
						memset( cSioRcvBuf, 0, RCV_BUF_SIZE );
						cSioRcvBuf[ usSioRcvCount++ ] = code;
						sio_mode = SIO_RCV_CMD_MODE;				// RS-232 制御Boxからのコマンド受信中状態へ。	
						th = 5;					
					} else {
						Rcv_chr_timer = 100;		//  キャラクタ・TimeOut　カウンタセット（1秒）
						nop();						// 読み捨て。
					}
					break;
					
				case SIO_RCV_CMD_MODE:		// RS-232 コマンド受信中
				
					if ( usSioRcvCount <= 5 ){	// コマンド・レングスとブロック番号の確認
						cSioRcvBuf[ usSioRcvCount++ ] = code;		// 受信コマンド・データの格納

						if ( usSioRcvCount == 4 ){					// 受信コマンド・レングスの取得
							sio_rcv_comm_len = ( (UINT)cSioRcvBuf[ 2 ] * 256 ) + (UINT)code;
						}
						
						if ( usSioRcvCount == 6 ){
							sio_rcv_block_num = ( (UINT)cSioRcvBuf[ 4 ] * 256 ) + (UINT)code;	 // 受信中コマンドのブロック番号の取得
						}

					} else if ( ( usSioRcvCount <= sio_rcv_comm_len + 2 ) && ( usSioRcvCount < RCV_BUF_SIZE ) ){ // コマンドコードとCR, LF受信コードの格納。
						cSioRcvBuf[ usSioRcvCount++ ] = code;
											
						// 終端コード受信チェック
						if ( usSioRcvCount >= sio_rcv_comm_len + 3 ){
							if ( ( cSioRcvBuf[ sio_rcv_comm_len + 1 ] == CODE_CR )
							  && ( cSioRcvBuf[ sio_rcv_comm_len + 2 ] == CODE_LF ) ){
								  
								cSioRcvBuf[ usSioRcvCount++ ] = 0;		// 受信バッファの最後にNULLを入れる。
								
								// 受信コマンドのSumチェックと、解析処理。
								comm_data_stat = chk_rcv_cmddata( cSioRcvBuf, sio_rcv_comm_len );					
								dly_tsk( 10/MSEC );
								
											  
								if( comm_data_stat == RCV_TANMATU_INFO ){
									send_sio_TANMATU_INFO();
								}else if( comm_data_stat == RCV_REGDATA_DNLD_STA ){
									send_sio_STANDARD();
								}else if( comm_data_stat == RCV_REGDATA_DNLD_GET ){
									//usSioBunkatuNum = 10 * (unsigned short)cSioRcvBuf[8] + (unsigned short)cSioRcvBuf[9];
									send_sio_STANDARD();
								}else if( comm_data_stat == RCV_REGDATA_DNLD_END ){
									send_sio_STANDARD();
								}else if( comm_data_stat == RCV_REGDATA_UPLD_STA ){
									send_sio_REGDATA_UPLD_STA();
								}else if( comm_data_stat == RCV_REGDATA_UPLD_GET ){
									//usSioBunkatuNum = 10 * (unsigned short)cSioRcvBuf[8] + (unsigned short)cSioRcvBuf[9];
									send_sio_REGDATA_UPLD_GET();
								}else if( comm_data_stat == RCV_REGDATA_UPLD_END ){
									send_sio_STANDARD();
								}else if( comm_data_stat == RCV_REGPROC ){
									g_pcproc_f = 2;
								}else if( comm_data_stat == RCV_AUTHPROC ){
									g_pcproc_f = 1;
								} else {
									iercd = send_Comm_Nack( ch );					// Nak送信処理
								}
					
								sio_mode = SIO_RCV_IDLE_MODE;	// アイドル・モードへ移行
								comm_data_stat = COMM_DATA_IDLE;
							}
						}

					}
					break;
			
				case SIO_SEND_MODE:			// コマンド送信中。
				
					nop();					// 受信データは読み捨て。
											// コマンド送信終了時、処理ルーチンで”SIO_SEND_ACK_WAIT”に移行させること。
					break;					
			
				case SIO_SEND_ACK_WAIT:		// コマンド送信済みで、ACK/NAKの受信受付中。
			
					if ( ( code == CODE_ACK ) || ( code == CODE_NACK ) ){		// ACK/NAKコードを受信
						usSioRcvCount = 0;
						memset( cSioRcvBuf, 0, RCV_BUF_SIZE );
						cSioRcvBuf[ usSioRcvCount++ ] = code;

						if ( comm_data_stat == RCV_SHUTDOWN_OK ){	// 制御Boxからシャットダウン要求応答を受信
							iercd = send_Comm_Ack( ch );					// Ack送信処理
							MdCngSubMode( SUB_MD_IDLE );	//　サブモード・ステータスを、”アイドル”へ。
						}

					
					} else if ( usSioRcvCount <= 2 ){	// ACK/NAKコード 以後の２バイトを受信中。
						cSioRcvBuf[ usSioRcvCount++ ] = code;
					
					} else if ( ACK_Wait_timer <= 0 ){  // ACK/NAK　待ちタイムアウト（ ACK_Wait_timerは、送信処理でSetする。）
						if ( ACK_Wait_TimeOut++ < 3 ){
							sio_mode = SIO_SEND_MODE;	// 送信許可受付中モードへ移行。
						
							SendSerialBinaryData( send_sio_buff, sio_snd_comm_len + 3 );	// コマンドの再送。
													
							ACK_Wait_timer = ACK_WAIT_TIME; // Ack待ちタイムアウト時間を再設定
							sio_mode = SIO_SEND_ACK_WAIT;	// Ack受信待ち中モードへ移行。
						
							usSioRcvCount = 0;
							memset( cSioRcvBuf, 0, RCV_BUF_SIZE );
						
						} else {
							sio_mode = SIO_RCV_IDLE_MODE;	// アイドル・モードへ移行。	
							comm_data_stat = COMM_DATA_IDLE;
							ACK_Wait_TimeOut = 0;
							MdCngSubMode( SUB_MD_IDLE );
						}		
					
					} else {
						nop();
					}
			
					if ( usSioRcvCount >= 3 ){			// ACK/NAK　以後の３バイトを受信完了。
								
						//　Ack/Nakコード解析処理
						ack_nak_stat = chk_ack_nak_code( cSioRcvBuf, usSioRcvCount );

						if ( ack_nak_stat == 1 ){		// ACK受信時
							sio_mode = SIO_RCV_IDLE_MODE;	// アイドル・モードへ移行。
							comm_data_stat = COMM_DATA_IDLE;
							MdCngSubMode( SUB_MD_IDLE );	//　サブモード・ステータスを、”アイドル”へ。
						
						} else if ( ack_nak_stat == 0 ){	// NACK受信時
							if ( rcv_nack_cnt++ < 3 ){		// NACK３回受信で、送信中断。
								sio_mode = SIO_SEND_MODE;	// 送信許可受付中モードへ移行。
						
								SendSerialBinaryData( send_sio_buff, sio_snd_comm_len + 3 );	// コマンドの再送。
													
								ACK_Wait_timer = ACK_WAIT_TIME; // Ack待ちタイムアウト時間を再設定
								sio_mode = SIO_SEND_ACK_WAIT;	// Ack受信待ち中モードへ移行。
							
								usSioRcvCount = 0;
								memset( cSioRcvBuf, 0, RCV_BUF_SIZE );
							
							} else {
								sio_mode = SIO_RCV_IDLE_MODE;	// アイドル・モードへ移行。
								comm_data_stat = COMM_DATA_IDLE;
								rcv_nack_cnt = 0;	
							}	
						} 
					}
					break;
				
				
				default:
					break;
			}
///*		
		} else if ( ercd == E_TMOUT ){	// 受信タイムアウトの場合
		
			 if ( sio_mode == SIO_SEND_ACK_WAIT ){	// コマンド送信済みで、ACK/NAKの受信受付中。
				if ( ACK_Wait_timer <= 0 ){  		// ACK　待ちタイムアウト
					if ( ACK_Wait_TimeOut++ < 3 ){
						sio_mode = SIO_SEND_MODE;	// 送信許可受付中モードへ移行。
						
						SendSerialBinaryData( send_sio_buff, sio_snd_comm_len + 3 );	// コマンドの再送。
													
						ACK_Wait_timer = ACK_WAIT_TIME; // Ack待ちタイムアウト時間を再設定
						sio_mode = SIO_SEND_ACK_WAIT;	// Ack受信待ち中モードへ移行。
						
						usSioRcvCount = 0;
						memset( cSioRcvBuf, 0, RCV_BUF_SIZE );
						
					} else {
						sio_mode = SIO_RCV_IDLE_MODE;	// アイドル・モードへ移行。	
						comm_data_stat = COMM_DATA_IDLE;
						ACK_Wait_TimeOut = 0;
					}
				}		
					
			}
//*/
		} else {
			nop();
		}
	}

}


#else
/*******************************************************************
* Receive task　メイン処理
*
********************************************************************/
static ER Rcv_serial_main_proc( int ch ) // Receive task　メイン処理
{
	char	code;
	ER ercd;
	int iercd;
	int ENQ_stat, ack_nak_stat;
	int rcv_nack_cnt, ACK_Wait_TimeOut;
/**
enum SIO_MODE {
	SIO_RCV_IDLE_MODE,		// RS-232 コマンド受信/送信アイドル
	SIO_RCV_ENQ_MODE,		// RS-232 ENQ受信中状態
	SIO_RCV_WAIT_MODE,		// RS-232 制御Boxからのコマンド受信待ち状態
	SIO_RCV_CMD_MODE,		// RS-232 制御Boxからのコマンド受信中状態。
	SIO_SEND_MODE,			// コマンド送信中。
	SIO_SEND_ACK_WAIT		// コマンド送信済みで、ACK/NAKの受信待ち、および受付中。
};
**/

	//20140125Miya nagaiBug修正　
	//ACK_Wait_TimeOutの初期値設定がないため、通信異常時にACK_Wait_TimeOutが不定値につき終了できない
	ACK_Wait_TimeOut = 0;	//2014Miya add


	comm_data_stat = COMM_DATA_IDLE;

	for(;;){
		ercd = get_sio( ch, (UB*)&code, ( TMO )1000/MSEC /* TMO_FEVR */ );	// 受信データ待ち
		
		 sio_rcv_TSK_wdt = FLG_ON;			// SIO受信タスク　WDTカウンタリセット・リクエスト・フラグ
		
		if ( ercd == E_OK ){  
		//状態ごとの処理
			switch ( sio_mode ){
		
				case SIO_RCV_IDLE_MODE:		// RS-232 コマンド受信/送信アイドル
			
					rcv_nack_cnt = 0;
					ACK_Wait_TimeOut = 0;

					if ( code == CODE_ENQ ) {		// ENQコードを受信
						usSioRcvCount = 0;
						memset( cSioRcvBuf, 0, RCV_BUF_SIZE );
						cSioRcvBuf[ usSioRcvCount++ ] = code;
					
						Rcv_chr_timer = 100;		//  キャラクタ・TimeOut　カウンタセット（1秒）
					
						sio_mode = SIO_RCV_ENQ_MODE;	// ENQ受信中状態モードへ移行。
						
					} else {
						
						Rcv_chr_timer = 100;		//  キャラクタ・TimeOut　カウンタセット（1秒）
						nop();						// ENQコード受信以外は、読み捨て。
					}
					break;
					
				case SIO_RCV_ENQ_MODE:		// RS-232 ENQ受信中状態モード
				
					if ( Rcv_chr_timer <= 0 ){						// キャラクタ受信の1秒タイムアウト
						iercd = send_Comm_Nack( ch );				// Nak送信処理	
						sio_mode = SIO_RCV_IDLE_MODE;				// アイドル・モードへ移行
						comm_data_stat = COMM_DATA_IDLE;
					
					} else if ( usSioRcvCount <= 2 ){
						cSioRcvBuf[ usSioRcvCount++ ] = code;		// 受信コードの格納
						
					} else {
						nop();
					}
					
					if ( usSioRcvCount >= 3 ){		// ENQコード受信確認
						ENQ_stat = chk_ENQ_code( cSioRcvBuf, usSioRcvCount );
						if ( ENQ_stat == 1 ){
							
							/***
							ここに、端末からのコマンド送信の有無チェック処理を記述。
							***/
							
							send_Comm_EOT( ch );					// EOT送信処理（制御Boxへコマンド送信を許可）
							sio_mode = SIO_RCV_WAIT_MODE;			// RS-232 制御Boxからのコマンド受信待ち状態へ。
							usSioRcvCount = 0;
						} else {
							iercd = send_Comm_Nack( ch );			// Nak送信処理	
							sio_mode = SIO_RCV_IDLE_MODE;			// アイドル・モードへ移行
							comm_data_stat = COMM_DATA_IDLE;	
						}
					}
					Rcv_chr_timer = 100;		//  キャラクタ・TimeOut　カウンタセット（1秒）

					break;					
			
				case SIO_RCV_WAIT_MODE:		// RS-232 コマンド受信待ち状態
				
					if ( Rcv_chr_timer <= 0 ){						// キャラクタ受信の1秒タイムアウト
						iercd = send_Comm_Nack( ch );				// Nak送信処理	
						sio_mode = SIO_RCV_IDLE_MODE;				// アイドル・モードへ移行
						comm_data_stat = COMM_DATA_IDLE;
					
					} else if ( ( code == '@' ) && ( usSioRcvCount == 0 ) ){
						usSioRcvCount = 0;
						memset( cSioRcvBuf, 0, RCV_BUF_SIZE );
						cSioRcvBuf[ usSioRcvCount++ ] = code;
						sio_mode = SIO_RCV_CMD_MODE;				// RS-232 制御Boxからのコマンド受信中状態へ。						
						
					} else {
						nop();										// '@'コード以外は、データを無視。
					}
					Rcv_chr_timer = 100;		//  キャラクタ・TimeOut　カウンタセット（1秒）

					break;										
					
				case SIO_RCV_CMD_MODE:		// RS-232 コマンド受信中
				
					if ( usSioRcvCount <= 4 ){	// コマンド・レングスとブロック番号の確認
						cSioRcvBuf[ usSioRcvCount++ ] = code;		// 受信コマンド・データの格納

						if ( usSioRcvCount == 3 ){					// 受信コマンド・レングスの取得
							sio_rcv_comm_len = ( cSioRcvBuf[ 1 ] * 256 ) + code;
						}
						
						if ( usSioRcvCount == 5 ){
							sio_rcv_block_num = ( cSioRcvBuf[ 3 ] * 256 ) + code;	 // 受信中コマンドのブロック番号の取得
						}
					} else if ( ( usSioRcvCount <= sio_rcv_comm_len + 2 ) && ( usSioRcvCount < RCV_BUF_SIZE ) ){ // コマンドコードとCR, LF受信コードの格納。
						cSioRcvBuf[ usSioRcvCount++ ] = code;
											
						// 終端コード受信チェック
						if ( usSioRcvCount >= sio_rcv_comm_len + 3 ){
							if ( ( cSioRcvBuf[ sio_rcv_comm_len + 1 ] == CODE_CR )
							  && ( cSioRcvBuf[ sio_rcv_comm_len + 2 ] == CODE_LF ) ){
								  
								cSioRcvBuf[ usSioRcvCount++ ] = 0;		// 受信バッファの最後にNULLを入れる。
								
								// 受信コマンドのSumチェックと、解析処理。
								comm_data_stat = chk_rcv_cmddata( cSioRcvBuf, sio_rcv_comm_len );								  
								
								if ( comm_data_stat == RCV_TEST_COM ){				// Testコマンドを受信			
									iercd = send_Comm_Ack( ch );					// Ack送信処理
									
									store_Test_cmddata( cSioRcvBuf, sio_rcv_comm_len );	// Testコマンド内データのストア。
																
									MdCngSubMode( SUB_MD_IDLE );	//　サブモード・ステータスを、”アイドル”へ。
												
								} else if ( comm_data_stat == RCV_NINSHOU_DATA ){	// 認証データの受信
									iercd = send_Comm_Ack( ch );					// Ack送信処理
						
								} else if ( comm_data_stat == RCV_TOUROKU_REQ ){	// 制御Boxから登録要求を受信
									iercd = send_Comm_Ack( ch );					// Ack送信処理
							
								} else if ( comm_data_stat == RCV_NINSHOU_OK ){		// 制御Boxから認証許可を受信										
									iercd = send_Comm_Ack( ch );					// Ack送信処理
							
								} else if ( comm_data_stat == RCV_TEIDEN_STAT ){	// 制御Boxから停電状態の応答を受信
									iercd = send_Comm_Ack( ch );					// Ack送信処理
									
									if ( ( cSioRcvBuf[ 7 ] == '0' ) && ( cSioRcvBuf[ 8 ] == '1' ) ){
										Pfail_sense_flg = PFAIL_SENSE_ON;			// 停電検知フラグ　ON
									}	else {
										Pfail_sense_flg = PFAIL_SENSE_NO;			// 停電検知フラグ　OFF
									}
							
								} else if ( comm_data_stat == RCV_SHUTDOWN_OK ){	// 制御Boxからシャットダウン要求応答を受信
									iercd = send_Comm_Ack( ch );					// Ack送信処理

									MdCngSubMode( SUB_MD_IDLE );	//　サブモード・ステータスを、”アイドル”へ。

								} else if ( comm_data_stat == RCV_TEIDEN_EVENT ){	// 停電状態イベント報告の受信
									iercd = send_Comm_Ack( ch );					// Ack送信処理
									
									if ( ( cSioRcvBuf[ 7 ] == '0' ) && ( cSioRcvBuf[ 8 ] == '1' ) ){
										Pfail_sense_flg = PFAIL_SENSE_ON;			// 停電検知フラグ　ON
									}	else {
										Pfail_sense_flg = PFAIL_SENSE_NO;			// 停電検知フラグ　OFF
									}
									
								} else if ( comm_data_stat == RCV_ALARM_STAT ){		// 警報発生・消去イベント報告の受信
									iercd = send_Comm_Ack( ch );					// Ack送信処理
									nop();
									store_Alarm_data( cSioRcvBuf, sio_rcv_comm_len );	// アラーム発生・消去報告コマンド内データのセット。
							
								} else if ( comm_data_stat == RCV_ADJUST_TIME ){	// 時刻合わせコマンドの受信
									iercd = send_Comm_Ack( ch );					// Ack送信処理
									
									store_Time_data( cSioRcvBuf, sio_rcv_comm_len );	// 時刻合わせコマンド内データのセット。
																
//									MdCngSubMode( SUB_MD_IDLE );	//　サブモード・ステータスを、”アイドル”へ。							
								} else {
								
									iercd = send_Comm_Nack( ch );					// Nak送信処理
								}
					
								sio_mode = SIO_RCV_IDLE_MODE;	// アイドル・モードへ移行
								comm_data_stat = COMM_DATA_IDLE;
							}
						}

					}
					break;
			
				case SIO_SEND_MODE:			// コマンド送信中。
				
					nop();					// 受信データは読み捨て。
											// コマンド送信終了時、処理ルーチンで”SIO_SEND_ACK_WAIT”に移行させること。
					break;					
			
				case SIO_SEND_ACK_WAIT:		// コマンド送信済みで、ACK/NAKの受信受付中。
			
					if ( ( code == CODE_ACK ) || ( code == CODE_NACK ) ){		// ACK/NAKコードを受信
						usSioRcvCount = 0;
						memset( cSioRcvBuf, 0, RCV_BUF_SIZE );
						cSioRcvBuf[ usSioRcvCount++ ] = code;

						if ( comm_data_stat == RCV_SHUTDOWN_OK ){	// 制御Boxからシャットダウン要求応答を受信
							iercd = send_Comm_Ack( ch );					// Ack送信処理
							MdCngSubMode( SUB_MD_IDLE );	//　サブモード・ステータスを、”アイドル”へ。
						}

					
					} else if ( usSioRcvCount <= 2 ){	// ACK/NAKコード 以後の２バイトを受信中。
						cSioRcvBuf[ usSioRcvCount++ ] = code;
					
					} else if ( ACK_Wait_timer <= 0 ){  // ACK/NAK　待ちタイムアウト（ ACK_Wait_timerは、送信処理でSetする。）
						if ( ACK_Wait_TimeOut++ < 3 ){
							sio_mode = SIO_SEND_MODE;	// 送信許可受付中モードへ移行。
						
							SendSerialBinaryData( send_sio_buff, sio_snd_comm_len + 3 );	// コマンドの再送。
													
							ACK_Wait_timer = ACK_WAIT_TIME; // Ack待ちタイムアウト時間を再設定
							sio_mode = SIO_SEND_ACK_WAIT;	// Ack受信待ち中モードへ移行。
						
							usSioRcvCount = 0;
							memset( cSioRcvBuf, 0, RCV_BUF_SIZE );
						
						} else {
							sio_mode = SIO_RCV_IDLE_MODE;	// アイドル・モードへ移行。	
							comm_data_stat = COMM_DATA_IDLE;
							ACK_Wait_TimeOut = 0;
							MdCngSubMode( SUB_MD_IDLE );
						}		
					
					} else {
						nop();
					}
			
					if ( usSioRcvCount >= 3 ){			// ACK/NAK　以後の３バイトを受信完了。
								
						//　Ack/Nakコード解析処理
						ack_nak_stat = chk_ack_nak_code( cSioRcvBuf, usSioRcvCount );

						if ( ack_nak_stat == 1 ){		// ACK受信時
							sio_mode = SIO_RCV_IDLE_MODE;	// アイドル・モードへ移行。
							comm_data_stat = COMM_DATA_IDLE;
							MdCngSubMode( SUB_MD_IDLE );	//　サブモード・ステータスを、”アイドル”へ。
						
						} else if ( ack_nak_stat == 0 ){	// NACK受信時
							if ( rcv_nack_cnt++ < 3 ){		// NACK３回受信で、送信中断。
								sio_mode = SIO_SEND_MODE;	// 送信許可受付中モードへ移行。
						
								SendSerialBinaryData( send_sio_buff, sio_snd_comm_len + 3 );	// コマンドの再送。
													
								ACK_Wait_timer = ACK_WAIT_TIME; // Ack待ちタイムアウト時間を再設定
								sio_mode = SIO_SEND_ACK_WAIT;	// Ack受信待ち中モードへ移行。
							
								usSioRcvCount = 0;
								memset( cSioRcvBuf, 0, RCV_BUF_SIZE );
							
							} else {
								sio_mode = SIO_RCV_IDLE_MODE;	// アイドル・モードへ移行。
								comm_data_stat = COMM_DATA_IDLE;
								rcv_nack_cnt = 0;	
							}	
						} 
					}
					break;
				
				
				default:
					break;
			}
///*		
		} else if ( ercd == E_TMOUT ){	// 受信タイムアウトの場合
		
			 if ( sio_mode == SIO_SEND_ACK_WAIT ){	// コマンド送信済みで、ACK/NAKの受信受付中。
				if ( ACK_Wait_timer <= 0 ){  		// ACK　待ちタイムアウト
					if ( ACK_Wait_TimeOut++ < 3 ){
						sio_mode = SIO_SEND_MODE;	// 送信許可受付中モードへ移行。
						
						SendSerialBinaryData( send_sio_buff, sio_snd_comm_len + 3 );	// コマンドの再送。
													
						ACK_Wait_timer = ACK_WAIT_TIME; // Ack待ちタイムアウト時間を再設定
						sio_mode = SIO_SEND_ACK_WAIT;	// Ack受信待ち中モードへ移行。
						
						usSioRcvCount = 0;
						memset( cSioRcvBuf, 0, RCV_BUF_SIZE );
						
					} else {
						sio_mode = SIO_RCV_IDLE_MODE;	// アイドル・モードへ移行。	
						comm_data_stat = COMM_DATA_IDLE;
						ACK_Wait_TimeOut = 0;
					}
				}		
					
			}
//*/
		} else {
			nop();
		}
	}

}
#endif

/*******************************************************************
* 受信コマンドのチェックサム解析処理と、コマンド解析。
*	＊Buf　受信コマンド・バッファ
*　　cmd_len　受信コマンドのレングス
********************************************************************/
static int chk_rcv_cmddata( UB *Buf, int cmd_len  )
{
	int ret = 0;
	UB tmp_str[ 5 ] = { 0, 0, 0, 0, 0 };
	UB tmp_sum[ 4 ] = { 0, 0, 0, 0 };		// Ckeck Sumを求める。
	UB ck_sum[ 4 ] = { 0, 0, 0, 0 };
	int i;

#if(PCCTRL == 1)	//20160930Miya PCからVA300Sを制御する
	if ( Buf[ 0 ] == '#' ){			// "#"　の一致を調べる。

		// Ckeck Sumを求める。
		for ( i=0; i<cmd_len; i++ ){			// コマンドのバイト分の総和を求める
			tmp_sum[ 0 ] = tmp_sum[ 0 ] + Buf[ i ];
		}
		tmp_sum[ 0 ] = tmp_sum[ 0 ] ^ 0xff;		// チェックサムを反転
		ck_sum[ 0 ] = tmp_sum[ 0 ];
		ck_sum[ 1 ] = '\0';
		tmp_str[ 0 ] = Buf[ cmd_len ];
		tmp_str[ 1 ] = 0;						// Ckeck Sumを調べる。

		if ( strcmp( ( char *)tmp_str, ( char *)ck_sum ) == 0 ){	// Ckeck Sumの一致

			ret = util_rcv_cmddata( Buf );		// 受信コマンド解析（コマンド部の解析）
			comm_data_stat = ret;				// コマンド受信ステータスのSet

		}	else	{
			ret = ERR_CKSUM;			// Ckeck Sumの不一致
		}

	}	else	{					// "#"で無い時。
		ret = ERR_FIRST_CODE;
	}
#else
	if ( Buf[ 0 ] == '@' ){			// "@"　の一致を調べる。

		// Ckeck Sumを求める。
		for ( i=0; i<cmd_len; i++ ){			// コマンドのバイト分の総和を求める
			tmp_sum[ 0 ] = tmp_sum[ 0 ] + Buf[ i ];
		}
		tmp_sum[ 0 ] = tmp_sum[ 0 ] ^ 0xff;		// チェックサムを反転
		ck_sum[ 0 ] = tmp_sum[ 0 ];
		ck_sum[ 1 ] = '\0';
		tmp_str[ 0 ] = Buf[ cmd_len ];
		tmp_str[ 1 ] = 0;						// Ckeck Sumを調べる。

		if ( strcmp( ( char *)tmp_str, ( char *)ck_sum ) == 0 ){	// Ckeck Sumの一致

			ret = util_rcv_cmddata( Buf );		// 受信コマンド解析（コマンド部の解析）
			comm_data_stat = ret;				// コマンド受信ステータスのSet

		}	else	{
			ret = ERR_CKSUM;			// Ckeck Sumの不一致
		}

	}	else	{					// "@"で無い時。
		ret = ERR_FIRST_CODE;
	}
#endif
	return ret;

}

/*******************************************************************
*
// 受信コマンド解析（コマンド部の解析）
*
********************************************************************/
static int util_rcv_cmddata( UB *Buf )
{
	int ret = 0;
//	int block_num;
	UB tmp_str[ 5 ] = { 0, 0, 0, 0, 0 };
	int datlen, st, sz;
	int tm;
	
//	block_num = ( Buf[ 4 ] * 256 ) + Buf[ 5 ];	 // 受信中コマンドのブロック番号の取得
//	if ( block_num != 0 ) return;


#if(PCCTRL == 1)
	if ( ( Buf[ 6 ] == 'A' ) && ( Buf[ 7 ] == '0' ) ){				// 端末情報取得 //20160930Miya
		ret = RCV_TANMATU_INFO;

	}else if ( ( Buf[ 6 ] == 'A' ) && ( Buf[ 7 ] == '1' ) ){		// 登録データダウンロード開始 //20160930Miya
		ret = RCV_REGDATA_DNLD_STA;
	
	} else if ( ( Buf[ 6 ] == 'A' ) && ( Buf[ 7 ] == '2' ) ){		// 登録データダウンロード中 //20160930Miya
		ret = RCV_REGDATA_DNLD_GET;
		usSioBunkatuNum = 10 * (unsigned short)Buf[4] + (unsigned short)Buf[5];

		datlen = 2 * 80 * 40 + 20 * 10;
		st = 1000 * usSioBunkatuNum;
		if(st + 1000 > datlen){
			sz = datlen - st;
		}else{
			sz = 1000;
		}
		memcpy(&IfImageBuf[st], &Buf[8], sz);
	
	} else if ( ( Buf[ 6 ] == 'A' ) && ( Buf[ 7 ] == '3' ) ){		// 登録データダウンロード終了 //20160930Miya
		ret = RCV_REGDATA_DNLD_END;
		memcpy(&RegImgBuf[0][0][0][0], &IfImageBuf[0], 80 * 40);
		memcpy(&RegImgBuf[0][0][1][0], &IfImageBuf[80*40], 80 * 40);
		memcpy(&g_RegBloodVesselTagData[0].MinAuthImg[0][0], &IfImageBuf[2*80*40], 20 * 10);
		memcpy(&RegImgBuf[0][1][0][0], &IfImageBuf[0], 80 * 40);
		memcpy(&RegImgBuf[0][1][1][0], &IfImageBuf[80*40], 80 * 40);
		memcpy(&g_RegBloodVesselTagData[0].MinAuthImg[1][0], &IfImageBuf[2*80*40], 20 * 10);
		AddRegImgFromRegImg(1, 0);		//20160312Miya 極小精度UP 登録・学習
	
	} else if ( ( Buf[ 6 ] == 'A' ) && ( Buf[ 7 ] == '4' ) ){		// 登録データアップロード開始 //20160930Miya
		ret = RCV_REGDATA_UPLD_STA;
	
	} else if ( ( Buf[ 6 ] == 'A' ) && ( Buf[ 7 ] == '5' ) ){		// 登録データアップロード中 //20160930Miya
		ret = RCV_REGDATA_UPLD_GET;
		usSioBunkatuNum = 10 * (unsigned short)Buf[8] + (unsigned short)Buf[9];
	
	} else if ( ( Buf[ 6 ] == 'A' ) && ( Buf[ 7 ] == '6' ) ){		// 登録データアップロード終了 //20160930Miya
		ret = RCV_REGDATA_UPLD_END;
	
	} else if ( ( Buf[ 6 ] == 'A' ) && ( Buf[ 7 ] == '7' ) ){		// 登録操作 //20160930Miya
		ret = RCV_REGPROC;
	
	} else if ( ( Buf[ 6 ] == 'A' ) && ( Buf[ 7 ] == '8' ) ){		// 認証操作 //20160930Miya
		ret = RCV_AUTHPROC;
		tm = 1000 * (int)(Buf[26]-0x30) + 100 * (int)(Buf[27]-0x30) + 10 * (int)(Buf[28]-0x30) + (int)(Buf[29]-0x30);
		if(tm > 300 || tm == 0)	tm = 300;
		g_pc_authtime = tm;
		
		g_pc_authnum = Buf[31];
		if(g_pc_authnum == 3)		g_pc_authnum = 1;
		else if(g_pc_authnum == 2)	g_pc_authnum = 2;
		else						g_pc_authnum = 3;
	
	} else {
			ret = KEY_UNNOWN_REQ;		// 不明な命令の受信
	}

#else
	if ( ( Buf[ 5 ] == '5' ) && ( Buf[ 6 ] == '0' ) ){				// テストコマンドの受信
		ret = RCV_TEST_COM;
							
	} else if ( ( Buf[ 5 ] == '5' ) && ( Buf[ 6 ] == '1' ) ){		// 認証データの受信
		ret = RCV_NINSHOU_DATA;
						
	} else if ( ( Buf[ 5 ] == '5' ) && ( Buf[ 6 ] == '2' ) ){		// 制御Boxから登録要求あり						
		ret = RCV_TOUROKU_REQ;
						
	} else if ( ( Buf[ 5 ] == '5' ) && ( Buf[ 6 ] == '3' ) ){		// 制御Boxから認証許可を受信				
		ret = RCV_NINSHOU_OK;		
						
	} else if ( ( Buf[ 5 ] == '5' ) && ( Buf[ 6 ] == '4' ) ){		// 制御Boxから停電状態の応答を受信						
		ret = RCV_TEIDEN_STAT;	
						
	} else if ( ( Buf[ 5 ] == '5' ) && ( Buf[ 6 ] == '5' ) ){		// 制御Boxからシャットダウン要求応答を受信						
		ret = RCV_SHUTDOWN_OK;	
						
	} else if ( ( Buf[ 5 ] == '5' ) && ( Buf[ 6 ] == '6' ) ){		// 停電状態イベント報告の受信						
		ret = RCV_TEIDEN_EVENT;	

	} else if ( ( Buf[ 5 ] == '5' ) && ( Buf[ 6 ] == '7' ) ){		// 警報発生・消去イベント報告の受信						
		ret = RCV_ALARM_STAT;	
						
	} else if ( ( Buf[ 5 ] == '5' ) && ( Buf[ 6 ] == '8' ) ){		// 時刻合わせコマンドの受信		
		ret = RCV_ADJUST_TIME;	
						
	} else {
			ret = KEY_UNNOWN_REQ;		// 不明な命令の受信
	}
#endif
	return ret;
}

/*******************************************************************
// Testコマンド内データのストア
********************************************************************/
void store_Test_cmddata( UB *Buf, int cmd_len )
{

	// 停電状態通知		
	if ( ( Buf[ 7 ] == '0' ) && ( Buf[ 8 ] == '1' ) ){
		Pfail_sense_flg = PFAIL_SENSE_ON;			// 停電検知フラグ　ON
	}	else {
		Pfail_sense_flg = PFAIL_SENSE_NO;			// 停電検知フラグ　OFF
	}
	
	// Version番号通知
	KeyIO_board_soft_VER[ 0 ] = Buf[ 9 ];
	KeyIO_board_soft_VER[ 1 ] = Buf[ 10 ];
	KeyIO_board_soft_VER[ 2 ] = Buf[ 11 ];
	KeyIO_board_soft_VER[ 3 ] = Buf[ 12 ];
	
	// DIP SWの設定内容通知
	dip_sw_data[ 0 ] = Buf[ 13 ] - '0';	// Dip SW状態 Bit0,　0：OFF、1：ON、　左桁からBit0,1,2,3
	dip_sw_data[ 1 ] = Buf[ 14 ] - '0';	// Dip SW状態 Bit1
	dip_sw_data[ 2 ] = Buf[ 15 ] - '0';	// Dip SW状態 Bit2
	dip_sw_data[ 3 ] = Buf[ 16 ] - '0';	// Dip SW状態 Bit3

}


/*******************************************************************
// アラーム発生・消去報告コマンド内データのセット。
********************************************************************/
void store_Alarm_data( UB *Buff, int cmd_len )
{
	UINT alm_Kubun, cnt;
	UINT alm_no;
	UB Fig1, Fig2, Fig3;
	
	alm_Kubun = Buff[ 7 ] - '0';		// アラームの発生区分　-0：消去、=1：発報
	if ( alm_Kubun != 0 ) alm_Kubun = 1;
	
	Fig1 = Buff[ 9 ] - '0';				// アラーム番号
	if ( ( Fig1 < 0 ) && ( Fig1 >= 10 ) ) Fig1 = 0;
	Fig2 = Buff[ 10 ] - '0';
	if ( ( Fig2 < 0 ) && ( Fig2 >= 10 ) ) Fig2 = 0;
	Fig3 = Buff[ 11 ] - '0';
	if ( ( Fig3 < 0 ) && ( Fig3 >= 10 ) ) Fig3 = 0;
	alm_no = ( Fig1 * 100 ) + ( Fig2 * 10 ) + Fig3;
	
	cnt = g_MainteLog.err_wcnt;			// アラームバッファへのアラーム情報書込み
	g_MainteLog.err_buff[ cnt ][ 0 ] = alm_no;		// アラーム番号
	g_MainteLog.err_buff[ cnt ][ 1 ] = count_1hour;	// 発生・消去　時
	g_MainteLog.err_buff[ cnt ][ 2 ] = count_1min;	// 発生・消去　分
	g_MainteLog.err_buff[ cnt ][ 3 ] = count_1sec;	// 発生・消去　秒
	g_MainteLog.err_buff[ cnt ][ 4 ] = alm_Kubun;	// アラームの発生区分
			
	++g_MainteLog.err_wcnt;				//  アラームバッファ・書込みポインター　インクリメント
	g_MainteLog.err_wcnt = g_MainteLog.err_wcnt & 0x7F;
	
	
	DBG_send_cnt++;
			
}

/*******************************************************************
// 時刻合わせコマンド内データのセット。
********************************************************************/
void store_Time_data( UB *Buff, int cmd_len )
{
	UB fig1, fig2;
	int tmp_hour, tmp_min, tmp_sec;

	fig1 = Buff[ 7 ] - '0';				// 時
	fig2 = Buff[ 8 ] - '0';
	tmp_hour = fig1 * 10 + fig2;
	if ( ( tmp_hour < 0 ) || ( tmp_hour >= 24 ) ) return;

	fig1 = Buff[ 9 ] - '0';				// 分
	fig2 = Buff[ 10 ] - '0';
	tmp_min = fig1 * 10 + fig2;
	if ( ( tmp_min < 0 ) || ( tmp_min >= 60 ) ) return;

	fig1 = Buff[ 11 ] - '0';			// 秒
	fig2 = Buff[ 12 ] - '0';
	tmp_sec = fig1 * 10 + fig2;
	if ( ( tmp_sec < 0 ) || ( tmp_sec >= 60 ) ) return;

	count_1hour = tmp_hour;		// 時のストア
	count_1min = tmp_min;		// 分のストア
	count_1sec = tmp_sec;		// 秒のストア
	
	g_MainteLog.now_min = count_1hour;		// "時"をLCD画面表示用メモリに反映
	g_MainteLog.now_hour = count_1min;		// "分"を　　↓
	g_MainteLog.now_sec = count_1sec;		// "秒"を　　↓		

}


//--------------------------------------------
//　ENQコード解析処理
//--------------------------------------------
static int chk_ENQ_code( UB *cSioRcvBuf, UINT usSioRcvCount )
{
	int ret = -1;
	char tmp_cmd[ 4 ] = { 0 };
		
	tmp_cmd[ 0 ] = cSioRcvBuf[ usSioRcvCount - 3 ];		// 受信データ３Byteを取得
	tmp_cmd[ 1 ] = cSioRcvBuf[ usSioRcvCount - 2 ];
	tmp_cmd[ 2 ] = cSioRcvBuf[ usSioRcvCount - 1 ];

	if ( tmp_cmd[ 0 ] == CODE_ENQ ){					// ENQコード　の一致を調べる。

		if ( ( tmp_cmd[ 1 ] == 0x0d ) && ( tmp_cmd[ 2 ] == 0x0a ) ){			// 終端コードの確認。
		
			ret = 1;
		}
	}
	
	return ret;
}


//--------------------------------------------
//　Ack/Nakコード解析処理
//--------------------------------------------
static int chk_ack_nak_code( UB *cSioRcvBuf, UINT usSioRcvCount )
{
	int ret = -1;
	char tmp_cmd[ 4 ] = { 0 };
		
	tmp_cmd[ 0 ] = cSioRcvBuf[ usSioRcvCount - 3 ];		// 受信データ３Byteを取得
	tmp_cmd[ 1 ] = cSioRcvBuf[ usSioRcvCount - 2 ];
	tmp_cmd[ 2 ] = cSioRcvBuf[ usSioRcvCount - 1 ];

	if ( tmp_cmd[ 0 ] == 0x06 || tmp_cmd[ 0 ] == 0x15 ){	// Ack/Nackコード　の一致を調べる。

		if ( ( tmp_cmd[ 1 ] == 0x0d ) && ( tmp_cmd[ 2 ] == 0x0a ) ){			// 終端コードの確認。
			// Ack/Nack受信を確認。
			if ( tmp_cmd[ 0 ] == 0x06 && sio_mode == SIO_SEND_ACK_WAIT ){
				// Ack受信なら、モードを「送受信初期状態」へ。
				ret = 1;
			}	else if ( tmp_cmd[ 0 ] == 0x15 && sio_mode == SIO_SEND_ACK_WAIT ){
				// Nack受信なら、モードを「Nack受信済み。３回までは再送必要。」へ。
				ret = 0;
			}	else	{
				nop();
			}
		}
	}
	
	return ret;
}


/*--------------------------------------*/
/*	Ack送信処理				　　　	  　*/
/*--------------------------------------*/
int send_Comm_Ack( INT ch )
{
	int i = 1;

	// Ackコマンドの準備。	
	send_Ack_buff[ 0 ] = CODE_ACK;
	send_Ack_buff[ 1 ] = CODE_CR;
	send_Ack_buff[ 2 ] = CODE_LF;

	// 送信処理。
	SendSerialBinaryData( send_Ack_buff, 3 );

	return i;
}

/*--------------------------------------*/
/*	Nack送信処理				　　　　*/
/*--------------------------------------*/
int send_Comm_Nack( INT ch )
{
	int i = 1;

	// Nackコマンドの準備。	
	send_Nack_buff[ 0 ] = CODE_NACK;
	send_Nack_buff[ 1 ] = CODE_CR;
	send_Nack_buff[ 2 ] = CODE_LF;

	// 送信処理。
	SendSerialBinaryData( send_Nack_buff, 3 );

	return i;
}


/*---------------------------------------*/
/*	EOT送信処理				　　	 　　*/
/*---------------------------------------*/
int send_Comm_EOT( INT ch )
{
	int i = 1;

	// EOTコマンドの準備。	
	send_EOT_buff[ 0 ] = CODE_EOT;
	send_EOT_buff[ 1 ] = CODE_CR;
	send_EOT_buff[ 2 ] = CODE_LF;

	// 送信処理。
	SendSerialBinaryData( send_EOT_buff, 3 );

	EOT_Wait_timer = EOT_WAIT_TIME;		// EOT待ちタイムアウト時間（１秒）Set。

	return 1;
}


/*-----------------------------------------------*/
/*	intger数字を　char４桁(右詰め)に変換する	 */
/*-----------------------------------------------*/
void util_i_to_char4( unsigned int i, char *buff )
{
	char fig1, fig2, fig3, fig4;
	unsigned int ii;
	
	ii = i;

	fig1 = ii / 1000;
	ii = ii - ( fig1 * 1000 );

	fig2 = ii / 100;
	ii = ii - ( fig2 * 100 );

	fig3 = ii / 10;
	ii = ii - ( fig3 * 10 );
		
	fig4 = ii;

	buff[ 0 ] = fig1 + 0x30;
	buff[ 1 ] = fig2 + 0x30;
	buff[ 2 ] = fig3 + 0x30;
	buff[ 3 ] = fig4 + 0x30;
	buff[ 4 ] = 0x0;

}


/*-----------------------------------------------*/
/*	char４桁(右詰め)を　intger数字に変換する	 */
/*-----------------------------------------------*/
void util_char4_to_i( char * source, unsigned int *i_dist )
{
	int tmp[ 4 ] = { -1, -1, -1, -1 };
	int i;

	for ( i=0; i<4; i++ ){
		if ( source[ i ] >= '0' && source[ i ] <= '9' ){
			tmp[ i ] = source[ i ] - '0';
		}
	}
	
	*i_dist = 0;

	if ( tmp[ 0 ] != -1 ){
		*i_dist = tmp[ 0 ] * 1000;
	}
	if ( tmp[ 1 ] != -1 ){
		*i_dist = *i_dist + ( tmp[ 1 ] * 100 );
	}
	if ( tmp[ 2 ] != -1 ){
		*i_dist = *i_dist + ( tmp[ 2 ] * 10 );
	}
	if ( tmp[ 3 ] != -1 ){
		*i_dist = *i_dist + tmp[ 3 ];
	}

}

/*-----------------------------------------------*/
/*	intger数字を　char５桁に変換する(左詰め)	 */
/*-----------------------------------------------*/
void util_i_to_char( unsigned int i, char * buff )
{
	char fig1, fig2, fig3, fig4, fig5;

	if ( i ){

		fig5 = i / 10000;
		i = i - ( fig5 * 10000 );

		fig4 = i / 1000;
		i = i - ( fig4 * 1000 );

		fig3 = i / 100;
		i = i - ( fig3 * 100 );

		fig2 = i / 10;

		fig1 = i - ( fig2 * 10 );

		if ( fig5 > 0 ){
			buff[ 0 ] = fig5 + 0x30;
			buff[ 1 ] = fig4 + 0x30;
			buff[ 2 ] = fig3 + 0x30;
			buff[ 3 ] = fig2 + 0x30;
			buff[ 4 ] = fig1 + 0x30;
			buff[ 5 ] = 0x0;

		}	else if ( fig4 > 0 ){
			buff[ 0 ] = fig4 + 0x30;
			buff[ 1 ] = fig3 + 0x30;
			buff[ 2 ] = fig2 + 0x30;
			buff[ 3 ] = fig1 + 0x30;
			buff[ 4 ] = 0x0;

		}	else if ( fig3 > 0 ){
			buff[ 0 ] = fig3 + 0x30;
			buff[ 1 ] = fig2 + 0x30;
			buff[ 2 ] = fig1 + 0x30;
			buff[ 3 ] = 0x0;

		}	else if ( fig2 > 0 ){
			buff[ 0 ] = fig2 + 0x30;
			buff[ 1 ] = fig1 + 0x30;
			buff[ 2 ] = 0x0;

		}	else if ( fig1 > 0 ){
			buff[ 0 ] = fig1 + 0x30;
			buff[ 1 ] = 0x0;

		}	else	{
			buff[ 0 ] = 0x30;
			buff[ 1 ] = 0x0;
		}

	}	else	{
		buff[ 0 ] = 0x30;
		buff[ 1 ] = 0x0;
	}	

}


/*----------------------------------------------*/
/*	char５桁(左詰め)を　intger数字に変換する	*/
/*----------------------------------------------*/
void util_char_to_i( char * source, unsigned int *i_dist )
{
	int tmp[ 5 ] = { -1, -1, -1, -1, -1 };
	int i;

	for ( i=0; i<5; i++ ){
		if ( source[ i ] >= '0' && source[ i ] <= '9' ){
			tmp[ i ] = source[ i ] - '0';
		}
	}

	if ( tmp[ 4 ] != -1 ){
		*i_dist = ( tmp[ 0 ] * 10000 ) + ( tmp[ 1 ] * 1000 ) + ( tmp[ 2 ] * 100 )
			 + ( tmp[ 3 ] * 10 ) + tmp[ 4 ];

	} else if ( tmp[ 3 ] != -1 ){
		*i_dist = ( tmp[ 0 ] * 1000 ) + ( tmp[ 1 ] * 100 ) + ( tmp[ 2 ] * 10 ) + tmp[ 3 ];

	} else if ( tmp[ 2 ] != -1 ){
		*i_dist = ( tmp[ 0 ] * 100 ) + ( tmp[ 1 ] * 10 ) + tmp[ 2 ];

	} else if ( tmp[ 1 ] != -1 ){
		*i_dist = ( tmp[ 0 ] * 10 ) + tmp[ 1 ];

	} else if ( tmp[ 0 ] != -1 ){
		*i_dist = tmp[ 0 ];

	} else {
		*i_dist = 0;
	}
}


