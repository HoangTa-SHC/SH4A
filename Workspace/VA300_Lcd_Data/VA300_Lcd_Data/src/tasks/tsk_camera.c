/**
*	VA-300プログラム
*
*	@file tsk_ctlmain.c
*	@version 1.00
*
*	@author Bionics Co.Ltd T.Nagai
*	@date   2013/04/20
*	@brief  VA-300 カメラ撮影コントロール・タスク
*
*	Copyright (C) 2013, Bionics Corporation
*/
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "kernel.h"
#include "sh7750.h"
#include "id.h"
#include "drv_fpga.h"
#include "err_ctrl.h"
#include "drv_cmr.h"
#include "drv_irled.h"

#include "va300.h"

// 定数宣言
//#define CMR_WAIT_TIME	28				// カメラのWait時間
#define CMR_WAIT_TIME	4				// カメラのWait時間

#define CMR_SLEEP_CNTRL 1	// カメラのスリープ制御あり/なし　1:あり、0：なし
#define CMR_FREEZE_CNTRL 1	// カメラのフリーズ制御あり/なし　1:あり、0：なし
#define CAP_Debug_Test  0  // 画像のデバッグとテスト　0:テストなし　1:テストあり

#define IR2_DUTY_DEFAULT	255			// 赤外線LEDのDutyのプログラム初期値
#define IR3_DUTY_DEFAULT	255			// 赤外線LEDのDutyのプログラム初期値
#define IR4_DUTY_DEFAULT	255			// 赤外線LEDのDutyのプログラム初期値
#define IR5_DUTY_DEFAULT	0			// 赤外線LEDのDutyのプログラム初期値

// 変数定義
static ID s_idTsk;

const UH cuhPrmShutter1 = 1;			// Fix Shutter Control値　１回目（露出１） プログラムで定義したDefault値
const UH cuhPrmShutter2 = 3;			// Fix Shutter Control値　２回目（露出２） プログラムで定義したDefault値
const UH cuhPrmShutter3 = 5;			// Fix Shutter Control値　３回目（露出３） プログラムで定義したDefault値

static UB g_ubCapBuf[ ( ( RE_SIZE_X * RE_SIZE_Y ) * 3 ) ];	///< 縮小画像取得用バッファ
static UB g_ubCapBufRaw[ ( 1280 * 720 ) * 3 ];	///< フル画像取得用バッファ
static UB g_ubCapBufTrm[ ( 800 * 400 ) * 3 ];	///< トリミング画像取得用バッファ

static int iStartX, iStartY, iSizeX, iSizeY;// キャプチャ画像のトリミング開始座標とサイズ
static int iReSizeX, iReSizeY;				// 縮小画像のXサイズ、Yサイズ
static int iReSizeMode;						// 縮小画像の縮小率　0:辺1/2、1:辺1/4、2:辺1/8

static RELTIM	dlytime;				// IR control wait値

static UH cmrGain;						// カメラ・ゲイン値　PCからUDPコマンド経由で与えられた値を、通信の指示に従ってカメラに設定する。
static UH cmrFixShutter1;				// Fix Shutter Control値(１回目)　実際にFPGA経由でカメラコントロールの為に参照する値
static UH cmrFixShutter2;				// Fix Shutter Control値(２回目)			同上
static UH cmrFixShutter3;				// Fix Shutter Control値(３回目＝シャッタースピード、初期値を受けたタイミングで、カメラにじかに設定する)　同上

static UH ini_cmrGain;					// カメラ・ゲイン値初期値　
static UH ini_cmrFixShutter1;			// Fix Shutter Control値初期値(１回目)　
static UH ini_cmrFixShutter2;			// Fix Shutter Control値初期値(２回目)			同上
static UH ini_cmrFixShutter3;			// Fix Shutter Control値初期値(３回目）

static UH cmr_wait_time = CMR_WAIT_TIME;// カメラのWait時間

static UB irDuty2;						// IR Duty値irDuty2;
static UB irDuty3;
static UB irDuty4;
static UB irDuty5;

static UB ini_irDuty2;					// IR Duty値irDuty2　初期値;
static UB ini_irDuty3;
static UB ini_irDuty4;
static UB ini_irDuty5;

static TMO trimTimeOut = 1000;			// 画像のトリミングのタイムアウト時間
static TMO resizeTimeOut = 1000;		// 画像の縮小・圧縮のタイムアウト時間

// プロトタイプ宣言
static TASK CameraTask( void );		///< カメラ撮影コントロール・タスク

ER CmrCmdSleep(void);
ER CmrCmdWakeUp(void);
static ER CmrCmdFreezeOff(void);
static ER CmrCmdTest(void);
static ER CmrCmdFixShutterCtl( UB FSC_val );
static ER CmrCmdManualGainCtl( UB gain_val );
static ER CmrCmdLuminanceCtl( void );	//20130619_Miya 追加
static ER SendCmd_204( UB *data, int len );
static ER SendCmd_210( UB *data, int len );
static ER SendCmd_141( UB *data, int len );
int Wait_Ack_and_Retry_forBlock( UB *snd_buff, int len );
static ER Wait_Ack_forBlock(void);
int	Cap_Resize_Picture( UINT command_no );
int	Cap_Raw_Picture( void );
void yb_init_capbuf( void );
void yb_init_all( void );		// 指登録情報のデータ初期化(all)

// タスクの定義
const T_CTSK ctsk_camera    = { TA_HLNG, NULL, CameraTask,    5, 2048, NULL, (B *)"camera" };

static T_YBDATA yb_touroku_data;	// 指登録情報（１指分）
static T_YBDATA20 yb_touroku_data20[21];	// 指登録情報（20指分）//追加　20130510 Miya

static UB kinkyuu_tel_no[17];		// 緊急開錠電話番号１６桁（配列最終番目は区切り記号”,”）
static UB kinkyuu_touroku_no[5];	// 緊急開錠の緊急登録番号４桁（配列最終番目は区切り記号　NUL　）　
static UB kinkyuu_hyouji_no[9];		// 緊急開錠の緊急番号８桁表示データ（配列最終番目は区切り記号　NUL　）　
static UB kinkyuu_kaijyo_no[9];		// 緊急開錠の開錠番号８桁データ（配列最終番目は区切り記号　NUL　）
static UB kinkyuu_input_no[5];		// 緊急開錠時に入力された番号４桁（配列最終番目は区切り記号　NUL　）

static UB mainte_password[5];		// メンテナンス・モード移行時の確認用パスワード４桁。（配列最終番目は区切り記号　NUL　）


/*==========================================================================*/
/*
 * カメラ撮影コントロール・タスク
 */
/*==========================================================================*/
TASK CameraTask( void )
{
	ER		ercd;
	FLGPTN	flgptn;
	int ercdStat;				// エラーコード記憶

	cmrFixShutter1 = cuhPrmShutter1;	// Fix Shutter Control値(１回目)
	cmrFixShutter2 = cuhPrmShutter2;	// Fix Shutter Control値(２回目)
	cmrFixShutter3 = cuhPrmShutter3;	// Fix Shutter Control値(３回目)

	iStartX = TRIM_START_X;			// キャプチャ画像のトリミング開始座標とサイズ
	iStartY = TRIM_START_Y;
	iSizeX = TRIM_SIZE_X;
	iSizeY = TRIM_SIZE_Y;
	
	iReSizeX = RE_SIZE_X;			// トリミング画像の縮小サイズ
	iReSizeY = RE_SIZE_Y;
	
	iReSizeMode = RSZ_MODE_1;		// 圧縮画像の縮小率
	
	irDuty2 = IR2_DUTY_DEFAULT;		// IR LEDの点灯初期値
	irDuty3 = IR3_DUTY_DEFAULT;
	irDuty4 = IR4_DUTY_DEFAULT;
	irDuty5 = IR5_DUTY_DEFAULT;

	// 処理開始
	
	
	/**	カメラのスリープ処理、その他	***/
	ercd = CmrCmdTest();			// カメラからの応答チェック
	if ( ercd != E_OK ){
		ercdStat = 4;
		 ErrCodeSet( ercd );
	}
	
#if (CMR_SLEEP_CNTRL == 1)
	ercd = CmrCmdWakeUp();			// カメラのWakeUP処理
	if ( ercd != E_OK ){
		ercdStat = 5;
		 ErrCodeSet( ercd );
	}	
	dly_tsk( 1000/MSEC );

	ercd = CmrCmdSleep();			// カメラのスリープ処理
	if ( ercd != E_OK ){
		ercdStat = 5;
		 ErrCodeSet( ercd );
	}	
#endif
	
	CmrCmdLuminanceCtl();	//20130619_Miya 追加

	
	for(;;) {
		ercd = wai_flg( ID_FLG_CAMERA, ( FPTN_START_CAP141
					  				   | FPTN_START_CAP204
					  				   | FPTN_START_CAP211
									   | FPTN_SETREQ_GAIN
									   | FPTN_SETREQ_SHUT1 ), TWF_ORW | TWF_CLR, &flgptn );	// カメラ撮影要求の受信待ち
		if ( ercd != E_OK ){
			ercdStat = 6;
			break;
		}
		
		if ( flgptn == FPTN_START_CAP204 ){			// 認証用撮影、登録用撮影の場合
			
//			ercd = clr_flg( ID_FLG_CAMERA, ~( FPTN_START_CAP204 & 0x0fffffff ) );			// フラグのクリア
//			if ( ercd != E_OK ){
//				ercdStat = 7;
//				break;
//			}
			
			MdCngMode( MD_CAP );					// 装置モードをキャプチャー中へ
			ercdStat = Cap_Resize_Picture( COMMAND204 );		// 登録用撮影処理
			
			rxtid = rxtid_org;						// GET_UDPの為に本来の受信タスクID（tsk_rcv）を再セット
					
		
		} else if ( flgptn == FPTN_START_CAP211 ){			// 認証用撮影の場合
			
//			ercd = clr_flg( ID_FLG_CAMERA, ~( FPTN_START_CAP211 & 0x0fffffff ) );			// フラグのクリア
//			if ( ercd != E_OK ){
//				ercdStat = 7;
//				break;
//			}

			MdCngMode( MD_CAP );					// 装置モードをキャプチャー中へ
			ercdStat = Cap_Resize_Picture( COMMAND210 );		// 認証用撮影処理
			
			rxtid = rxtid_org;						// GET_UDPの為に本来の受信タスクID（tsk_rcv）を再セット

			
		} else if ( flgptn == FPTN_START_CAP141 ){	// 生画像撮影要求の場合
			
//			ercd = clr_flg( ID_FLG_CAMERA, ~( FPTN_START_CAP141 & 0x0fffffff ) );			// フラグのクリア
//			if ( ercd != E_OK ){
//				ercdStat = 7;
//				break;
//			}

			ercdStat = Cap_Raw_Picture();			// 生画像撮影処理
			
			rxtid = rxtid_org;						// GET_UDPの為に本来の受信タスクID（tsk_rcv）を再セット

			
	    } else if ( flgptn == FPTN_SETREQ_GAIN ){			// カメラ・ゲイン設定要求の場合
			
//			ercd = clr_flg( ID_FLG_CAMERA, ~( FPTN_SETREQ_GAIN & 0x0fffffff ) );			// フラグのクリア
//			if ( ercd != E_OK ){
//				ercdStat = 8;
//				break;
//			}
			
//			ercd = CmrCmdWakeUp();					// カメラのWakeUP処理
//			if ( ercd != E_OK ){
//		 		ErrCodeSet( ercd );
//			}	
//			dly_tsk( 1000/MSEC );

			ercd = CmrCmdManualGainCtl( ( UB )cmrGain );		// カメラに直接、ゲイン設定値を設定（FPGAを経由しない）
			if ( ercd != E_OK ){
			 	ErrCodeSet( ercd );
			}

//			ercd = CmrCmdSleep();					// カメラのスリープ処理
//			if ( ercd != E_OK ){
//				ErrCodeSet( ercd );
//			}	
			
		} else if ( flgptn == FPTN_SETREQ_SHUT1 ){			// カメラ・シャッタースピード１設定要求の場合
			
//			ercd = clr_flg( ID_FLG_CAMERA, ~( FPTN_SETREQ_SHUT1 & 0x0fffffff ) );			// フラグのクリア
//			if ( ercd != E_OK ){
//				ercdStat = 8;
//				break;
//			}
			
//			ercd = CmrCmdWakeUp();					// カメラのWakeUP処理
//			if ( ercd != E_OK ){
//		 		ErrCodeSet( ercd );
//			}	
//			dly_tsk( 1000/MSEC );

			ercd = CmrCmdFixShutterCtl( ( UB )cmrFixShutter3 );	// カメラに直接、露出３設定値を設定（FPGAを経由しない）
			if ( ercd != E_OK ){
				 ErrCodeSet( ercd );
			}
//			ercd = CmrCmdSleep();					// カメラのスリープ処理
//			if ( ercd != E_OK ){
//				ErrCodeSet( ercd );
//			}	
			
		}	 else {
		
		}
	}
}


//-----------------------------------------------------------------------------
// 認証用撮影、登録用撮影処理
//-----------------------------------------------------------------------------
int	Cap_Resize_Picture( UINT command_no ){
	
	int ercdStat = 0;
	ER ercd;
	T_YBDATA *ybdata;
		
//	ybdata = ( T_YBDATA * )yb_touroku_data;	
	ybdata = &yb_touroku_data;	
	
	yb_init_capbuf();					// 指情報の初期化(属性情報以外)
		
	ercd = IrLedSet( IR_LED_2, irDuty2 );						// 赤外線LED2の階調設定
	if ( ercd != E_OK ){
		ercdStat = 8;
		return ercdStat;
	}
	ercd = IrLedSet( IR_LED_3, irDuty3 );						// 赤外線LED3の階調設定
	if ( ercd != E_OK ){
		ercdStat = 8;
		return ercdStat;
	}
	ercd = IrLedSet( IR_LED_4, irDuty4 );						// 赤外線LED4の階調設定
	if ( ercd != E_OK ){
		ercdStat = 8;
		return ercdStat;
	}
	ercd = IrLedSet( IR_LED_5, irDuty5 );						// 赤外線LED5の階調設定
	if ( ercd != E_OK ){
		ercdStat = 8;
		return ercdStat;
	}

//#if (CMR_SLEEP_CNTRL == 1)
//	ercd = CmrCmdWakeUp();			// カメラのWakeUP処理
//	if ( ercd != E_OK ){
//		ercdStat = 5;
//		 ErrCodeSet( ercd );
//	}		
//#endif

//	dlytime = ( 2000/MSEC );

	IrLedCrlOn( IR_LED_2 | IR_LED_3 | IR_LED_4 ); 				// 赤外線LEDの全点灯(LED2,3,4)
//	IrLedCrlOn( IR_LED_2 | IR_LED_3 | IR_LED_4 | IR_LED_5 ); 	// 赤外線LEDの全点灯
//	dly_tsk( dlytime );
				
	/** カメラ撮影処理	**/
#if (CMR_FREEZE_CNTRL == 1)
	ercd = CmrCmdFreezeOff();									// カメラ・フリーズ・コントロール・オフ
	if ( ercd != E_OK ){
		ercdStat = 9;
//		return ercdStat;
	}
#endif
	CmrPrmShutterSet( cmrFixShutter1 );							
	CmrWakeUpWaitCrl( cmr_wait_time );							// WakeUp時のカメラのWait時間設定
		
	ercd = CmrCapture( CAP_MODE_1, CAP_BANK_0, (2000/MSEC) );	// 画像のキャプチャー、1枚目
	while( IsCmrCapStart() == TRUE ){							// 画像キャプチャーの終了待ち
		dly_tsk( 10/MSEC );
	}
	if ( ercd != E_OK ){
		while( ercd != E_OK ){
			ercd = CmrCapture( CAP_MODE_1, CAP_BANK_0, (2000/MSEC) );	// 画像のキャプチャー、
			while( IsCmrCapStart() == TRUE ){							// 画像キャプチャーの終了待ち
				dly_tsk( 10/MSEC );
			}
		}
	}
		
#if	(CAP_Debug_Test == 1)
	ercd = CmrCapGet( CAP_BANK_0, 0, 0, 1280, 720, &g_ubCapBufRaw[ 0 ] );	// テスト用にフル撮影画像を取得
	if ( ercd != E_OK ){
		ercdStat = 16;
		return ercdStat;
	} 
#endif

	/** キャプチャ画像のトリミング	**/
	ercd = CmrTrim(CAP_BANK_0, TRIM_BANK_0, iStartX, iStartY, iSizeX, iSizeY, trimTimeOut ); // １枚目画像のトリミング
	if ( ercd != E_OK ){
		ercdStat = 19;
		return ercdStat;
	}
	while ( IsCmrTrimStart() == TRUE ){
		dly_tsk( 10/MSEC );	
	}

#if	(CAP_Debug_Test == 1)
	ercd = CmrTrimGet( TRIM_BANK_0, 0, ( 640 * 320 ), &g_ubCapBufTrm[ 0 ] );	// テスト用にトリミング画像を取得、１枚目
	if ( ercd != E_OK ){
		ercdStat = 22;
		return ercdStat;
	}
#endif

	/** トリミング画像の縮小と圧縮	**/
	ercd = CmrResize(TRIM_BANK_0, RSZ_BANK_0, RSZ_MODE_1, iReSizeX, iReSizeY, resizeTimeOut );	// １枚目画像の縮小・圧縮
	if ( ercd != E_OK ){
		ercdStat = 25;
		return ercdStat;
	}
	while ( IsCmrResizeStart() == TRUE ){
		dly_tsk( 10/MSEC );
	}
		
	ercd = CmrResizeGet(RSZ_BANK_0, 0, ( 160 * 80 ), &g_ubCapBuf[ 0 ]);		// １枚目縮小画像の取得
	if ( ercd != E_OK ){
		ercdStat = 28;
		return ercdStat;
	}


#if (CMR_FREEZE_CNTRL == 1)		
	ercd = CmrCmdFreezeOff();									// カメラ・フリーズ・コントロール・オフ
	if ( ercd != E_OK ){
		ercdStat = 11;
//		return ercdStat;
	}
#endif		
	CmrPrmShutterSet( cmrFixShutter2 );
			
	ercd = CmrCapture( CAP_MODE_3, CAP_BANK_1, (2000/MSEC) );	// 画像のキャプチャー、2枚目
	while( IsCmrCapStart() == TRUE ){							// 画像キャプチャーの終了待ち
		dly_tsk( 10/MSEC );
	}
	if ( ercd != E_OK ){
		while( ercd != E_OK ){
			ercd = CmrCapture( CAP_MODE_3, CAP_BANK_1, (2000/MSEC) );	// 画像のキャプチャー、
			while( IsCmrCapStart() == TRUE ){							// 画像キャプチャーの終了待ち
				dly_tsk( 10/MSEC );
			}
		}
	}
		
#if	(CAP_Debug_Test == 1)
	ercd = CmrCapGet( CAP_BANK_1, 0, 0, 1280, 720, &g_ubCapBufRaw[ ( 1280 * 720 ) ] );	// テスト用にフル撮影画像を取得
	if ( ercd != E_OK ){
		ercdStat = 17;
		return ercdStat;
	} 		
#endif

	ercd = CmrTrim(CAP_BANK_1, TRIM_BANK_1, iStartX, iStartY, iSizeX, iSizeY, trimTimeOut ); // ２枚目画像のトリミング
	if ( ercd != E_OK ){
		ercdStat = 20;
		return ercdStat;
	}
	while ( IsCmrTrimStart() == TRUE ){
		dly_tsk( 10/MSEC );	
	}

#if	(CAP_Debug_Test == 1)
	ercd = CmrTrimGet( TRIM_BANK_1, 0, ( 640 * 320 ), &g_ubCapBufTrm[ ( 640 * 320 ) ] );	// テスト用にトリミング画像を取得、１枚目
	if ( ercd != E_OK ){
		ercdStat = 23;
		return ercdStat;
	}
#endif

	ercd = CmrResize(TRIM_BANK_1, RSZ_BANK_1, RSZ_MODE_1, iReSizeX, iReSizeY, resizeTimeOut );	// ２枚目画像の縮小・圧縮
	if ( ercd != E_OK ){
		ercdStat = 26;
		return ercdStat;
	}
	while ( IsCmrResizeStart() == TRUE ){
		dly_tsk( 10/MSEC );
	}

	ercd = CmrResizeGet(RSZ_BANK_1, 0, ( 160 * 80 ), &g_ubCapBuf[( 160 * 80 )] );	// ２枚目縮小画像の取得
	if ( ercd != E_OK ){
		ercdStat = 29;
		return ercdStat;
	}


#if (CMR_FREEZE_CNTRL == 1)
	ercd = CmrCmdFreezeOff();									// カメラ・フリーズ・コントロール・オフ
	if ( ercd != E_OK ){
		ercdStat = 13;
//		return ercdStat;
	}
#endif
	CmrPrmShutterSet( cmrFixShutter3 );

	ercd = CmrCapture( CAP_MODE_3, CAP_BANK_2, (2000/MSEC) );	// 画像のキャプチャー、3枚目
	while( IsCmrCapStart() == TRUE ){							// 画像キャプチャーの終了待ち
		dly_tsk( 10/MSEC );
	}
	if ( ercd != E_OK ){
		while( ercd != E_OK ){
			ercd = CmrCapture( CAP_MODE_3, CAP_BANK_2, (2000/MSEC) );	// 画像のキャプチャー、
			while( IsCmrCapStart() == TRUE ){							// 画像キャプチャーの終了待ち
				dly_tsk( 10/MSEC );
			}
		}
	}
		
#if	(CAP_Debug_Test == 1)
	ercd = CmrCapGet( CAP_BANK_2, 0, 0, 1280, 720, &g_ubCapBufRaw[ ( 1280 * 720 ) * 2 ] );	// テスト用にフル撮影画像を取得
	if ( ercd != E_OK ){
		ercdStat = 18;
		return ercdStat;
	} 
#endif

	ercd = CmrTrim(CAP_BANK_2, TRIM_BANK_2, iStartX, iStartY, iSizeX, iSizeY, trimTimeOut ); // ３枚目画像のトリミング
	if ( ercd != E_OK ){
		ercdStat = 21;
		return ercdStat;
	}
	while ( IsCmrTrimStart() == TRUE ){
		dly_tsk( 10/MSEC );	
	}

#if	(CAP_Debug_Test == 1)
	ercd = CmrTrimGet( TRIM_BANK_2, 0, ( 640 * 320 ), &g_ubCapBufTrm[ ( ( 640 * 320 ) * 2 ) ] );	// テスト用にトリミング画像を取得、１枚目
	if ( ercd != E_OK ){
		ercdStat = 24;
		return ercdStat;
	} 
#endif
	
	ercd = CmrResize(TRIM_BANK_2, RSZ_BANK_2, RSZ_MODE_1, iReSizeX, iReSizeY, resizeTimeOut );	// ３枚目画像の縮小・圧縮
	if ( ercd != E_OK ){
		ercdStat = 27;
		return ercdStat;
	}
	while ( IsCmrResizeStart() == TRUE ){
		dly_tsk( 10/MSEC );
	}

	ercd = CmrResizeGet(RSZ_BANK_2, 0, ( 160 * 80 ), &g_ubCapBuf[ ( ( 160 * 80 ) * 2 ) ] );	// ３枚目縮小画像の取得
	if ( ercd != E_OK ){
		ercdStat = 30;
		return ercdStat;
	}

	memcpy( &ybdata->ybCapBuf, &g_ubCapBuf, ( 160 * 80 * 3 ) );

#if (CMR_FREEZE_CNTRL == 1)	
	ercd = CmrCmdFreezeOff();									// カメラ・フリーズ・コントロール・オフ
	if ( ercd != E_OK ){
		ercdStat = 14;
//		return ercdStat;
	}
#endif

#if (CMR_SLEEP_CNTRL == 1)		
	/**	カメラのスリープ処理	***/
	ercd = CmrCmdSleep();
	if ( ercd != E_OK ){
		ercdStat = 15;
		return ercdStat;
	}		
	/** 	同	終了	**/
#endif

	/** IR LEDの消灯処理　**/
	IrLedCrlOff( IR_LED_2 | IR_LED_3 | IR_LED_4 | IR_LED_5 ); 	// 赤外線LEDの全消灯
		
	/** カメラ撮影の終了処理　**/
	ercd = set_flg( ID_FLG_TS, FPTN_CMR_TS );					// 生体検知タスクへ、カメラ撮影終了を通知
	if ( ercd != E_OK ){
		ercdStat = 31;
		return ercdStat;
	}

	/** 画像をUDP送信タスクへ送る	**/
	if ( command_no == COMMAND204 ){
		ercd = SendCmd_204( ( UB* )ybdata, ( 42 + ( 160 * 80 * 3 ) + 2 ) );		// コマンド204、縮小画像（属性情報付き）
//		ercd = SendCmd_204( ( UB* )ybdata->ybCapBuf, ( ( 160 * 80 ) * 3 ) );		// コマンド204、縮小画像（属性情報なし）
		if ( ercd != E_OK ){
			ercdStat = 34;
//			return ercdStat;
		}
	} else if	( command_no == COMMAND210 ){
		ercd = SendCmd_210( ( UB* )ybdata->ybCapBuf, ( ( 160 * 80 ) * 3 ) + 2 );	// コマンド210、縮小画像３枚（属性情報なし）
		if ( ercd != E_OK ){
			ercdStat = 34;
//			return ercdStat;
		}
	}

	/** カメラ撮影の終了処理　**/
	ercd = set_flg( ID_FLG_TS, FPTN_END_TS );				// 生体検知タスクへ、カメラ撮影終了を通知
	if ( ercd != E_OK ){
		ercdStat = 35;
		return ercdStat;
	}
	return ercdStat;
}

//-----------------------------------------------------------------------------
// 生画像撮影処理
//-----------------------------------------------------------------------------
int	Cap_Raw_Picture( void ){
	
	int ercdStat = 0;
	ER ercd;
		
	ercd = IrLedSet( IR_LED_2, irDuty2 );						// 赤外線LED2の階調設定
	if ( ercd != E_OK ){
		ercdStat = 8;
		return ercdStat;
	}
	ercd = IrLedSet( IR_LED_3, irDuty3 );						// 赤外線LED3の階調設定
	if ( ercd != E_OK ){
		ercdStat = 8;
		return ercdStat;
	}
	ercd = IrLedSet( IR_LED_4, irDuty4 );						// 赤外線LED4の階調設定
	if ( ercd != E_OK ){
		ercdStat = 8;
		return ercdStat;
	}
	ercd = IrLedSet( IR_LED_5, irDuty5 );						// 赤外線LED5の階調設定
	if ( ercd != E_OK ){
		ercdStat = 8;
		return ercdStat;
	}

#if (CMR_SLEEP_CNTRL == 1)
	ercd = CmrCmdWakeUp();			// カメラのWakeUP処理
	if ( ercd != E_OK ){
		ercdStat = 5;
		 ErrCodeSet( ercd );
	}		
#endif
	dlytime = ( 2000/MSEC );

	IrLedCrlOn( IR_LED_2 | IR_LED_3 | IR_LED_4 ); 				// 赤外線LEDの全点灯(LED2,3,4)
//	IrLedCrlOn( IR_LED_2 | IR_LED_3 | IR_LED_4 | IR_LED_5 ); 	// 赤外線LEDの全点灯
	dly_tsk( dlytime );
				
	/** カメラ撮影処理	**/
#if (CMR_FREEZE_CNTRL == 1)
	ercd = CmrCmdFreezeOff();									// カメラ・フリーズ・コントロール・オフ
	if ( ercd != E_OK ){
		ercdStat = 9;
//		return ercdStat;
	}
#endif
	CmrPrmShutterSet( cmrFixShutter1 );							
	CmrWakeUpWaitCrl( cmr_wait_time );							// WakeUp時のカメラのWait時間設定
		
	ercd = CmrCapture( CAP_MODE_1, CAP_BANK_0, (2000/MSEC) );	// 画像のキャプチャー、1枚目
	while( IsCmrCapStart() == TRUE ){							// 画像キャプチャーの終了待ち
		dly_tsk( 10/MSEC );
	}
	if ( ercd != E_OK ){
		while( ercd != E_OK ){
			ercd = CmrCapture( CAP_MODE_1, CAP_BANK_0, (2000/MSEC) );	// 画像のキャプチャー、
			while( IsCmrCapStart() == TRUE ){							// 画像キャプチャーの終了待ち
				dly_tsk( 10/MSEC );
			}
		}
	}
		
	ercd = CmrCapGet( CAP_BANK_0, 0, 0, 1280, 720, &g_ubCapBufRaw[ 0 ] );	// フル撮影画像を取得
	if ( ercd != E_OK ){
		ercdStat = 16;
		return ercdStat;
	} 

#if (CMR_FREEZE_CNTRL == 1)		
	ercd = CmrCmdFreezeOff();									// カメラ・フリーズ・コントロール・オフ
	if ( ercd != E_OK ){
		ercdStat = 11;
//		return ercdStat;
	}
#endif		

#if (CMR_SLEEP_CNTRL == 1)		
	/**	カメラのスリープ処理	***/
	ercd = CmrCmdSleep();
	if ( ercd != E_OK ){
		ercdStat = 15;
		return ercdStat;
	}		
	/** 	同	終了	**/
#endif

Camera_End_Process:
	/** IR LEDの消灯処理　**/
	IrLedCrlOff( IR_LED_2 | IR_LED_3 | IR_LED_4 | IR_LED_5 ); 	// 赤外線LEDの全消灯
		
	/** カメラ撮影の終了処理　**/
	ercd = set_flg( ID_FLG_TS, FPTN_CMR_TS );					// 生体検知タスクへ、カメラ撮影終了を通知
	if ( ercd != E_OK ){
		ercdStat = 31;
		return ercdStat;
	}


	/** 画像をUDP送信タスクへ送る	**/
	ercd = SendCmd_141( g_ubCapBufRaw, ( 1280 * 720 ) );		// コマンド141、フル画像 1枚分
	if ( ercd != E_OK ){
		ercdStat = 32;
//		return ercdStat;
	}

	/** カメラ撮影の終了処理　**/
	ercd = set_flg( ID_FLG_TS, FPTN_END_TS );		// 生体検知タスクへ、カメラ撮影終了を通知
	if ( ercd != E_OK ){
		ercdStat = 35;
		return ercdStat;
	}
	return ercdStat;
}


//-----------------------------------------------------------------------------
// カメラコマンド: カメラ Manual Gain Control
//-----------------------------------------------------------------------------
static ER CmrCmdManualGainCtl( UB gain_val )
{
	UB cCmd[] = { 0x00, 0x00, 0x28, 0x05, 0x02, 0x06, 0x00, 0x01, 0x00 };
	const int iRecvSize = 5;
	UB	cRetBuf[ 10 ];
	UB	*p;
	ER	ercd;
	int i;
	
	cCmd[ 8 ] = gain_val;
	
	ercd = CmrPktSend( cCmd, sizeof cCmd, iRecvSize );			// コマンド送信
	if (ercd == E_OK) {
		ercd = CmrPktRecv( cRetBuf, iRecvSize, (3000/MSEC));	// 応答受信
		if (ercd == E_OK) {
			if (((cRetBuf[ 0 ] & 0x7f) == cCmd[ 1 ]) && (cRetBuf[ 2 ] == cCmd[ 2 ]) && (cRetBuf[ 3 ] == 0x01)	// 応答内容チェック
				&& ((cRetBuf[ 4 ] & 0x01 ) == 0x00 )) {
			} else {
				ercd = E_OBJ;
			}
		}	else if ( ercd == E_TMOUT ) {
			nop();
		}	else if ( ercd == E_RLWAI ) {
			nop();
		}	else if ( ercd == E_CTX ) {
			nop();
		}	else {
			nop();
		}
	}
	return ercd;
}


//-----------------------------------------------------------------------------
// カメラコマンド: カメラ Fix Shutter Control
//-----------------------------------------------------------------------------
static ER CmrCmdFixShutterCtl( UB FSC_val )
{
	UB cCmd[] = { 0x00, 0x00, 0x28, 0x05, 0x02, 0x03, 0x00, 0x01, 0x00 };
	const int iRecvSize = 5;
	UB	cRetBuf[ 10 ];
	UB	*p;
	ER	ercd;
	int i;

	cCmd[ 8 ] = 0x0f & FSC_val;
	
	ercd = CmrPktSend( cCmd, sizeof cCmd, iRecvSize );				// コマンド送信
	if (ercd == E_OK) {
		ercd = CmrPktRecv( cRetBuf, iRecvSize, (3000/MSEC));	// 応答受信
		if (ercd == E_OK) {
			if (((cRetBuf[ 0 ] & 0x7f) == cCmd[ 1 ]) && (cRetBuf[ 2 ] == cCmd[ 2 ]) && (cRetBuf[ 3 ] == 0x01)	// 応答内容チェック
				&& ((cRetBuf[ 4 ] & 0x01 ) == 0x00 )) {
			} else {
				ercd = E_OBJ;
			}
		}	else if ( ercd == E_TMOUT ) {
			nop();
		}	else if ( ercd == E_RLWAI ) {
			nop();
		}	else if ( ercd == E_CTX ) {
			nop();
		}	else {
			nop();
		}
	}
	return ercd;
}


//-----------------------------------------------------------------------------
// カメラコマンド: カメラ Sleep
//-----------------------------------------------------------------------------
ER CmrCmdSleep(void)
{
	const UB cCmd[] = { 0x00, 0x00, 0x18, 0x01, 0x00 };
	const int iRecvSize = 5;
	UB	cRetBuf[ 10 ];
	UB	*p;
	ER	ercd;
	
	ercd = CmrPktSend( cCmd, sizeof cCmd, iRecvSize );			// コマンド送信
	if (ercd == E_OK) {
		ercd = CmrPktRecv( cRetBuf, iRecvSize, (3000/MSEC));		// 応答受信
		if (ercd == E_OK) {
			if (((cRetBuf[ 0 ] & 0x7f) == cCmd[ 1 ]) && (cRetBuf[ 2 ] == cCmd[ 2 ]) && (cRetBuf[ 3 ] == cCmd[ 3 ])	// 応答内容チェック
			/* && ((cRetBuf[ 4 ] & 0x01) == 0x00)*/ ) {
			} else {
				ercd = E_OBJ;
			}
		}	else if ( ercd == E_TMOUT ) {
			nop();
		}	else if ( ercd == E_RLWAI ) {
			nop();
		}	else if ( ercd == E_CTX ) {
			nop();
		}	else {
			nop();
		}
	}
	return ercd;
}


//-----------------------------------------------------------------------------
// カメラコマンド: カメラ WakeUp
//-----------------------------------------------------------------------------
ER CmrCmdWakeUp(void)
{
	const UB cCmd[] = { 0x00, 0x00, 0x18, 0x01, 0x01 };
	const int iRecvSize = 5;
	UB	cRetBuf[ 10 ];
	UB	*p;
	ER	ercd;
	
	ercd = CmrPktSend( cCmd, sizeof cCmd, iRecvSize );			// コマンド送信
	if (ercd == E_OK) {
		ercd = CmrPktRecv( cRetBuf, iRecvSize, (3000/MSEC));		// 応答受信
		if (ercd == E_OK) {
			if (((cRetBuf[ 0 ] & 0x7f) == cCmd[ 1 ]) && (cRetBuf[ 2 ] == cCmd[ 2 ]) && (cRetBuf[ 3 ] == cCmd[ 3 ])	// 応答内容チェック
			/* && ((cRetBuf[ 4 ] & 0x01) == 0x01)*/ ) {
			} else {
				ercd = E_OBJ;
			}
		}	else if ( ercd == E_TMOUT ) {
			nop();
		}	else if ( ercd == E_RLWAI ) {
			nop();
		}	else if ( ercd == E_CTX ) {
			nop();
		}	else {
			nop();
		}
	}
	return ercd;
}

//-----------------------------------------------------------------------------
// カメラコマンド: カメラ Freeze Control OFF
//-----------------------------------------------------------------------------
static ER CmrCmdFreezeOff(void)
{
	const UB cCmd[] = { 0x00, 0x00, 0x28, 0x05, 0x08, 0x26, 0x00, 0x01, 0x02 };
	const int iRecvSize = 5;
	UB	cRetBuf[ 10 ];
	UB	*p;
	ER	ercd;
	
	ercd = CmrPktSend( cCmd, sizeof cCmd, iRecvSize );			// コマンド送信
	if (ercd == E_OK) {
		ercd = CmrPktRecv( cRetBuf, iRecvSize, (3000/MSEC));	// 応答受信
		if (ercd == E_OK) {
			if ((cRetBuf[ 0 ] & 0x7f) == cCmd[ 1 ] && (cRetBuf[ 2 ] == cCmd[ 2 ]) && (cRetBuf[ 3 ] == 0x01)	// 応答内容チェック
				&& ((cRetBuf[ 4 ] & 0x01 ) == 0x00 )) {
			} else {
				ercd = E_OBJ;
			}
		}	else if ( ercd == E_TMOUT ) {
			nop();
		}	else if ( ercd == E_RLWAI ) {
			nop();
		}	else if ( ercd == E_CTX ) {
			nop();
		}	else {
			nop();
		}
	}
	return ercd;
}

//-----------------------------------------------------------------------------
// カメラコマンドテスト : Request Response Command
//-----------------------------------------------------------------------------
static ER CmrCmdTest(void)
{
	char *cParm;
	const UB cCmd[] = { 0x00, 0x00, 0x10, 0x00 };
	const int iRecvSize = 5;
	UB	cRetBuf[ 10 ];
	UB	*p;
	ER	ercd;
	
	ercd = CmrPktSend( cCmd, sizeof cCmd, iRecvSize);	// コマンド送信
	if (ercd == E_OK) {
		ercd = CmrPktRecv( cRetBuf, iRecvSize, (5000 / MSEC));		// 応答受信
		if (ercd == E_OK) {
			if (((cRetBuf[ 0 ] & 0x7f) == cCmd[ 1 ]) && (cRetBuf[ 2 ] == cCmd[ 2 ]) && (cRetBuf[ 3 ] == 0x01)
				&& (cRetBuf[ 4 ] == 0x00)) {
			} else {
				ercd = E_OBJ;
			}
		}
	}
	return ercd;
}


//20130619_Miya
//-----------------------------------------------------------------------------
// カメラコマンド: カメラ Luminance Control
//-----------------------------------------------------------------------------
static ER CmrCmdLuminanceCtl( void )
{
	UB cCmd[] = { 0x00, 0x00, 0x28, 0x05, 0x02, 0x00, 0x00, 0x01, 0x00 };
	const int iRecvSize = 5;
	UB	cRetBuf[ 10 ];
	UB	*p;
	ER	ercd;
	int i;

	cCmd[ 8 ] = 0;	//Manual Shutter Control
	
	ercd = CmrPktSend( cCmd, sizeof cCmd, iRecvSize );				// コマンド送信
	if (ercd == E_OK) {
		ercd = CmrPktRecv( cRetBuf, iRecvSize, (3000/MSEC));	// 応答受信
		if (ercd == E_OK) {
			if (((cRetBuf[ 0 ] & 0x7f) == cCmd[ 1 ]) && (cRetBuf[ 2 ] == cCmd[ 2 ]) && (cRetBuf[ 3 ] == 0x01)	// 応答内容チェック
				&& ((cRetBuf[ 4 ] & 0x01 ) == 0x00 )) {
			} else {
				ercd = E_OBJ;
			}
		}	else if ( ercd == E_TMOUT ) {
			nop();
		}	else if ( ercd == E_RLWAI ) {
			nop();
		}	else if ( ercd == E_CTX ) {
			nop();
		}	else {
			nop();
		}
	}
	return ercd;
}

//-----------------------------------------------------------------------------
// 画像データのUDP送信（指画像データ、コマンド番号204）
//-----------------------------------------------------------------------------
static ER SendCmd_204( UB *data, int len )
{

	char header[ 12 ];
	char tmp_str[ 5 ];
	int max_blk_num, blk_num;
	unsigned short cmd_len, tmp_len;
	int i;
	int len_rest;
	char *ptr;
	UB snd_data[ 1024 ];		// UDP 送信データ １ブロック
	int	send_cnt;
	ER ercd = E_OK;

	int escape;

	header[ 0 ] = 0x27;			//　ヘッダ　1Byte　ASCII
	header[ 1 ] = 0;			//　データ長　２バイトBinary
	header[ 2 ] = 0;
	header[ 3 ] = 0x31;			//　送信元種別　1Byte　ASCII
	header[ 4 ] = '2';			//　コマンド番号　３桁ASCII
	header[ 5 ] = '0';
	header[ 6 ] = '4';
	header[ 7 ] = '0';			//　ブロック番号 ４桁ASCII
	header[ 8 ] = '0';
	header[ 9 ] = '0';
	header[ 10 ] = '0';
	header[ 11 ] = 0;	

/**		2013.6.27 Commented Out By T.N
	//仮対策 20130510 Miya  PC側で解錠認証or登録認証の区別がつかないため
	if( (GetScreenNo() == LCD_SCREEN8) || (GetScreenNo() == LCD_SCREEN129) ){
		header[ 3 ] = 0x32;			//　送信元種別　1Byte　ASCII
	}
**/

	len_rest = len;

	// 最大ブロック数の計算	
	if ( len > 1024 -13 )
	{
		max_blk_num = len / ( 1024 - 13 );
	}	else	{
		max_blk_num = 0;	
	}
	blk_num = max_blk_num;
	
	// ブロック毎の処理	
	while( blk_num >= 0 )
	{
		// データ長の計算　と格納	
		if ( len > 0 )
		{
			if ( len_rest >= ( 1024 - 13 ) )
			{
				cmd_len = 1024 - 2;
			} else {
				cmd_len = ( len_rest % ( 1024 - 13 ) ) + 11;	
			}
			header[ 2 ] = ( char )cmd_len;
			tmp_len = cmd_len >> 8;
			header[ 1 ] = ( char )tmp_len;
		}

		// ブロック番号の計算　と格納	
		strcpy( tmp_str, "    \0" );
		sprintf( ( char *)tmp_str, /*5,*/ "%d", blk_num );
		tmp_len = strlen( tmp_str );
		if ( tmp_len == 1 )
		{
			header[ 10 ] = tmp_str[ 0 ];
		} else if ( tmp_len == 2 ){
			header[ 9 ] = tmp_str[ 0 ];
			header[ 10 ] = tmp_str[ 1 ];
		} else if ( tmp_len == 3 ){
			header[ 8 ] = tmp_str[ 0 ];
			header[ 9 ] = tmp_str[ 1 ];
			header[ 10 ] = tmp_str[ 2 ];
		} else if ( tmp_len == 4 ){
			header[ 7 ] = tmp_str[ 0 ];
			header[ 8 ] = tmp_str[ 1 ];
			header[ 9 ] = tmp_str[ 2 ];
			header[ 10 ] = tmp_str[ 3 ];
		}	

		/** ブロックの送信準備		**/
		snd_data[ 0 ] = header[ 0 ];
		snd_data[ 1 ] = header[ 1 ];
		snd_data[ 2 ] = header[ 2 ];
		snd_data[ 3 ] = header[ 3 ];
		snd_data[ 4 ] = header[ 4 ];
		snd_data[ 5 ] = header[ 5 ];
		snd_data[ 6 ] = header[ 6 ];
		snd_data[ 7 ] = header[ 7 ];
		snd_data[ 8 ] = header[ 8 ];
		snd_data[ 9 ] = header[ 9 ];
		snd_data[ 10 ] = header[ 10 ];
		
		// もし先頭ブロックなら。
		if ( blk_num == max_blk_num ){
		// 一度目の認証データ／二度目の認証データかの区分情報をＳｅｔ
			if ( ( GetScreenNo() == LCD_SCREEN6 ) || ( GetScreenNo() == LCD_SCREEN121 )	// 一回目？
		      || ( GetScreenNo() == LCD_SCREEN127 ) || ( GetScreenNo() == LCD_SCREEN141 ) ){
			  
				snd_data[ 11 ] = '1';			//　送信元種別　1Byte　ASCII
			
			} else if ( ( GetScreenNo() == LCD_SCREEN8 ) || ( GetScreenNo() == LCD_SCREEN101 )	// 二回目？
		      || ( GetScreenNo() == LCD_SCREEN102 ) || ( GetScreenNo() == LCD_SCREEN129 ) ){
			  			  
				snd_data[ 11 ] = '2';			//　1Byte　ASCII
			
			} else {
				snd_data[ 11 ] = ' ';			//　1Byte　ASCII	ここに来たら実装異常。		
			}
			snd_data[ 12 ] = ',';				//　送信元種別　1Byte　ASCII
			
			for ( i=0; i<=cmd_len-13; i++ ){	//　画像データのコマンドセット。
			
				snd_data[ 13+i ] = data[ ( ( max_blk_num - blk_num ) * ( 1024 - 13 ) ) + i ];
			}
			
		}	else { // もし先頭ブロックでないなら。
			for ( i=0; i<=cmd_len-11; i++ ){	//　画像データのコマンドセット。
			
				snd_data[ 11+i ] = data[ ( ( max_blk_num - blk_num ) * ( 1024 - 13 ) ) + i ];
			}
				
		}		  
		snd_data[ cmd_len ] = CODE_CR;
		snd_data[ cmd_len + 1 ] = CODE_LF;

		// １ブロックのデータ送信	
		SendBinaryData( &snd_data, ( cmd_len + 2 ) );		// １ブロックのデータ送信

		escape = Wait_Ack_and_Retry_forBlock( snd_data, ( cmd_len + 2 ) );// ブロックデータ送信後のAck応答待ちと、リトライ時のブロック転送
		if ( escape != 0 ){
			ercd = E_OBJ;
			break;				// TimeOut以外のシステムエラー（ブロック転送中断
		}

		--blk_num;								// 残りのブロック数
		len_rest = len_rest - ( 1024 - 13 );	// 残りの送信データ数

		header[ 7 ] = '0';			//　ブロック番号 ４桁ASCIIの再初期化
		header[ 8 ] = '0';
		header[ 9 ] = '0';
		header[ 10 ] = '0';

	}
	
	return ercd;
}


//-----------------------------------------------------------------------------
// 画像データのUDP送信（指画像データ、コマンド番号210）
//-----------------------------------------------------------------------------
static ER SendCmd_210( UB *data, int len )
{

	char header[ 12 ];
	char tmp_str[ 5 ];
	int max_blk_num, blk_num;
	unsigned short cmd_len, tmp_len;
	int i;
	int len_rest;
	char *ptr;
	UB snd_data[ 1024 ];		// UDP 送信データ １ブロック
	int	send_cnt;
	ER ercd = E_OK;

	int escape;

	header[ 0 ] = 0x27;			//　ヘッダ　1Byte　ASCII
	header[ 1 ] = 0;			//　データ長　２バイトBinary
	header[ 2 ] = 0;
	header[ 3 ] = 0x31;			//　送信元種別　1Byte　ASCII
	header[ 4 ] = '2';			//　コマンド番号　３桁ASCII
	header[ 5 ] = '1';
	header[ 6 ] = '0';
	header[ 7 ] = '0';			//　ブロック番号 ４桁ASCII
	header[ 8 ] = '0';
	header[ 9 ] = '0';
	header[ 10 ] = '0';
	header[ 11 ] = 0;	

/**		2013.6.27 Commented Out By T.N
	//仮対策 20130510 Miya  PC側で解錠認証or登録認証の区別がつかないため
	if(GetScreenNo() != LCD_SCREEN101){
		header[ 3 ] = 0x32;			//　送信元種別　1Byte　ASCII
	}
**/

	len_rest = len;

	// 最大ブロック数の計算	
	if ( len > 1024 -13 )
	{
		max_blk_num = len / ( 1024 - 13 );
	}	else	{
		max_blk_num = 0;	
	}
	blk_num = max_blk_num;
	
	// ブロック毎の処理	
	while( blk_num >= 0 )
	{
		// データ長の計算　と格納	
		if ( len > 0 )
		{
			if ( len_rest >= ( 1024 - 13 ) )
			{
				cmd_len = 1024 - 2;
			} else {
				cmd_len = ( len_rest % ( 1024 - 13 ) ) + 11;	
			}
			header[ 2 ] = ( char )cmd_len;
			tmp_len = cmd_len >> 8;
			header[ 1 ] = ( char )tmp_len;
		}

		// ブロック番号の計算　と格納	
		strcpy( tmp_str, "    \0" );
		sprintf( ( char *)tmp_str, /*5,*/ "%d", blk_num );
		tmp_len = strlen( tmp_str );
		if ( tmp_len == 1 )
		{
			header[ 10 ] = tmp_str[ 0 ];
		} else if ( tmp_len == 2 ){
			header[ 9 ] = tmp_str[ 0 ];
			header[ 10 ] = tmp_str[ 1 ];
		} else if ( tmp_len == 3 ){
			header[ 8 ] = tmp_str[ 0 ];
			header[ 9 ] = tmp_str[ 1 ];
			header[ 10 ] = tmp_str[ 2 ];
		} else if ( tmp_len == 4 ){
			header[ 7 ] = tmp_str[ 0 ];
			header[ 8 ] = tmp_str[ 1 ];
			header[ 9 ] = tmp_str[ 2 ];
			header[ 10 ] = tmp_str[ 3 ];
		}	

		/** ブロックの送信準備		**/
		snd_data[ 0 ] = header[ 0 ];
		snd_data[ 1 ] = header[ 1 ];
		snd_data[ 2 ] = header[ 2 ];
		snd_data[ 3 ] = header[ 3 ];
		snd_data[ 4 ] = header[ 4 ];
		snd_data[ 5 ] = header[ 5 ];
		snd_data[ 6 ] = header[ 6 ];
		snd_data[ 7 ] = header[ 7 ];
		snd_data[ 8 ] = header[ 8 ];
		snd_data[ 9 ] = header[ 9 ];
		snd_data[ 10 ] = header[ 10 ];

		// もし先頭ブロックなら。
		if ( blk_num == max_blk_num ){
		// 登録用の認証データ／解錠用の認証データかの区分情報をＳｅｔ
			if ( ( GetScreenNo() == LCD_SCREEN101 ) || ( GetScreenNo() == LCD_SCREEN102 ) ){	// 解錠用？
			  
				snd_data[ 11 ] = '1';			//　1Byte　ASCII
			
			} else {		//　上記以外は、全て登録、または登録確認用
				snd_data[ 11 ] = '0';			//　1Byte　ASCII		
			}
			snd_data[ 12 ] = ',';

			for ( i=0; i<=cmd_len-13; i++ ){	//　画像データのコマンドセット。
			
				snd_data[ 13+i ] = data[ ( ( max_blk_num - blk_num ) * ( 1024 - 13 ) ) + i ];
			}
			
		}	else { // もし先頭ブロックでないなら。
			for ( i=0; i<=cmd_len-11; i++ ){	//　画像データのコマンドセット。
			
				snd_data[ 11+i ] = data[ ( ( max_blk_num - blk_num ) * ( 1024 - 13 ) ) + i ];
			}
				
		}
		snd_data[ cmd_len ] = CODE_CR;
		snd_data[ cmd_len + 1 ] = CODE_LF;

		// １ブロックのデータ送信	
		SendBinaryData( &snd_data, ( cmd_len + 2 ) );		// １ブロックのデータ送信

		escape = Wait_Ack_and_Retry_forBlock( snd_data, ( cmd_len + 2 ) );// ブロックデータ送信後のAck応答待ちと、リトライ時のブロック転送
		if ( escape != 0 ){
			ercd = E_OBJ;
			break;				// TimeOut以外のシステムエラー（ブロック転送中断
		}

		--blk_num;								// 残りのブロック数
		len_rest = len_rest - ( 1024 - 13 );	// 残りの送信データ数

		header[ 7 ] = '0';			//　ブロック番号 ４桁ASCIIの再初期化
		header[ 8 ] = '0';
		header[ 9 ] = '0';
		header[ 10 ] = '0';

	}
	
	return ercd;
}


//-----------------------------------------------------------------------------
// 画像データのUDP送信（指画像データ、コマンド番号210）
//-----------------------------------------------------------------------------
static ER SendCmd_141( UB *data, int len )
{

	char header[ 12 ];
	char tmp_str[ 5 ];
	int max_blk_num, blk_num;
	unsigned short cmd_len, tmp_len;
	int i;
	int len_rest;
	char *ptr;
	UB snd_data[ 1024 ];		// UDP 送信データ １ブロック
	int	send_cnt;
	ER ercd = E_OK;

	int escape;

	header[ 0 ] = 0x27;			//　ヘッダ　1Byte　ASCII
	header[ 1 ] = 0;			//　データ長　２バイトBinary
	header[ 2 ] = 0;
	header[ 3 ] = 0x31;			//　送信元種別　1Byte　ASCII
	header[ 4 ] = '1';			//　コマンド番号　３桁ASCII
	header[ 5 ] = '4';
	header[ 6 ] = '1';
	header[ 7 ] = '0';			//　ブロック番号 ４桁ASCII
	header[ 8 ] = '0';
	header[ 9 ] = '0';
	header[ 10 ] = '0';
	header[ 11 ] = 0;	

	len_rest = len;

	// 最大ブロック数の計算	
	if ( len > 1024 -13 )
	{
		max_blk_num = len / ( 1024 - 13 );
	}	else	{
		max_blk_num = 0;	
	}
	blk_num = max_blk_num;
	
	// ブロック毎の処理	
	while( blk_num >= 0 )
	{
		// データ長の計算　と格納	
		if ( len > 0 )
		{
			if ( len_rest >= ( 1024 - 13 ) )
			{
				cmd_len = 1024 - 2;
			} else {
				cmd_len = ( len_rest % ( 1024 - 13 ) ) + 11;	
			}
			header[ 2 ] = ( char )cmd_len;
			tmp_len = cmd_len >> 8;
			header[ 1 ] = ( char )tmp_len;
		}

		// ブロック番号の計算　と格納	
		strcpy( tmp_str, "    \0" );
		sprintf( ( char *)tmp_str, /*5,*/ "%d", blk_num );
		tmp_len = strlen( tmp_str );
		if ( tmp_len == 1 )
		{
			header[ 10 ] = tmp_str[ 0 ];
		} else if ( tmp_len == 2 ){
			header[ 9 ] = tmp_str[ 0 ];
			header[ 10 ] = tmp_str[ 1 ];
		} else if ( tmp_len == 3 ){
			header[ 8 ] = tmp_str[ 0 ];
			header[ 9 ] = tmp_str[ 1 ];
			header[ 10 ] = tmp_str[ 2 ];
		} else if ( tmp_len == 4 ){
			header[ 7 ] = tmp_str[ 0 ];
			header[ 8 ] = tmp_str[ 1 ];
			header[ 9 ] = tmp_str[ 2 ];
			header[ 10 ] = tmp_str[ 3 ];
		}	

		/** ブロックの送信準備		**/
		snd_data[ 0 ] = header[ 0 ];
		snd_data[ 1 ] = header[ 1 ];
		snd_data[ 2 ] = header[ 2 ];
		snd_data[ 3 ] = header[ 3 ];
		snd_data[ 4 ] = header[ 4 ];
		snd_data[ 5 ] = header[ 5 ];
		snd_data[ 6 ] = header[ 6 ];
		snd_data[ 7 ] = header[ 7 ];
		snd_data[ 8 ] = header[ 8 ];
		snd_data[ 9 ] = header[ 9 ];
		snd_data[ 10 ] = header[ 10 ];

		for ( i=0; i<=cmd_len-11; i++ )
		{
			snd_data[ 11+i ] = data[ ( ( max_blk_num - blk_num ) * ( 1024 - 13 ) ) + i ];
		} 
		snd_data[ cmd_len ] = CODE_CR;
		snd_data[ cmd_len + 1 ] = CODE_LF;

		// １ブロックのデータ送信	
		SendBinaryData( &snd_data, ( cmd_len + 2 ) );		// １ブロックのデータ送信

		escape = Wait_Ack_and_Retry_forBlock( snd_data, ( cmd_len + 2 ) );// ブロックデータ送信後のAck応答待ちと、リトライ時のブロック転送
		if ( escape != 0 ){
			ercd = E_OBJ;
			break;				// TimeOut以外のシステムエラー（ブロック転送中断
		}

		--blk_num;								// 残りのブロック数
		len_rest = len_rest - ( 1024 - 13 );	// 残りの送信データ数

		header[ 7 ] = '0';			//　ブロック番号 ４桁ASCIIの再初期化
		header[ 8 ] = '0';
		header[ 9 ] = '0';
		header[ 10 ] = '0';

	}
	CmmSendEot();				// EOTを送信してブロック送信を終了。
	
	return ercd;
}


//-----------------------------------------------------------------------------
// ブロックデータ送信後のAck応答待ちと、リトライ時のブロック転送
//-----------------------------------------------------------------------------
int Wait_Ack_and_Retry_forBlock( UB *snd_buff, int len ){
	
	int err = 0;
	int send_cnt = 3;
	ER ercd;
	
	while( send_cnt >= 0 ){
			
		ercd = Wait_Ack_forBlock();			// ブロックデータ送信後のAck応答待ち

		if ( ercd == E_OK ){
			err = 0;
			break;							// Ack応答があれば、次のブロック転送へ。
				
		}	else if ( ercd == E_TMOUT ){
			// １秒　Ack応答なしの場合、再送３回
			if ( send_cnt >= 1 ){
				SendBinaryData( snd_buff, len );		// １ブロックのデータを再送信
			}	else	{
				// EOT送信		
				//CmmSendEot();				// 再送３回で、無応答の場合、EOTを送信してブロック送信を終了。
				err = 1;
				break;
			}

		} else if ( ercd == CODE_NACK ){	// Nack応答があれば、同じブロックを再送。
			if ( send_cnt >= 0 ){
				SendBinaryData( snd_buff, len );		// １ブロックのデータを再送信
				send_cnt = 3;				// 再送カウンタを、３回に戻す。
				err = 0;
				continue;
			}	else	{
				// EOT送信		
				// CmmSendEot();				// 再送３回後で、Nack応答受信の場合、EOTを送信してブロック送信を終了。
				err = 1;					// あり得ない？
				break;
			}
			
		} else	{
			err = 1;						// TimeOut以外のシステムエラー
			break;				
		}

		--send_cnt;							// 再送カウンタをデクリメント。
			
	}
	return err;
}


//-----------------------------------------------------------------------------
// ブロックデータ送信後のAck応答待ち
//-----------------------------------------------------------------------------
static ER Wait_Ack_forBlock( void )
{
	int Fcnt = 0;
	ER ercd, err_code;
	UB buff[ 10 ], code;
	
	rxtid = vget_tid();		// get_udp の為に、仮の（現在の）割込み待ちタスクIDをセット
	
	for(;;){
		err_code = get_udp( (UB*)&code, 1000/MSEC);// 受信データ待ち
		if ( err_code == E_OK ){
			buff[ Fcnt ] = code;
			Fcnt++;
			if ( Fcnt >= 3){
				if ( ( buff[0] == CODE_ACK ) &&
					 ( buff[1] == CODE_CR ) &&
					 ( buff[2] == CODE_LF) ){
					ercd = E_OK;				// Ack応答受信
					Fcnt = 0;
					break;
					
				} else if 	 ( ( buff[0] == CODE_NACK ) &&
					 ( buff[1] == CODE_CR ) &&
					 ( buff[2] == CODE_LF) ){
					ercd = CODE_NACK;			// Nack応答受信
					Fcnt = 0;
					break;
				
				}	else {
					ercd = E_OBJ;				// その他の受信データ
					Fcnt = 0;
					//continue;
					break;
				}
				continue;
			}
			continue;
			
		}	else if ( err_code == E_TMOUT ){
			Fcnt = 0;
			ercd = E_TMOUT;						// Ack応答受信のTimeOut
			break;

		}	else	{
			Fcnt = 0;
			ercd = err_code;					// Ack応答でも、TimeOutでも、その他の受信データでもない場合
			break;
		}
	}
	return ercd;
}

//-----------------------------------------------------------------------------
// 指登録情報のデータ初期化(画像情報)
//-----------------------------------------------------------------------------
void yb_init_capbuf( void ){
	
	memset( yb_touroku_data.ybCapBuf, 0, ( 160 * 80 * 3 ) );
	
	yb_touroku_data.tou_no[ 2 ] 		= ',';
	yb_touroku_data.user_id[ 4 ]		= ',';
	yb_touroku_data.yubi_seq_no[ 3 ]	= ',';
	yb_touroku_data.kubun[ 1 ]			= ',';
	yb_touroku_data.yubi_no[ 2 ]		= ',';
	yb_touroku_data.name[ 24 ]			= ',';
	
}

//-----------------------------------------------------------------------------
// 指登録情報のデータ初期化(all)
//-----------------------------------------------------------------------------
void yb_init_all( void ){
	
	int i;
	
	for ( i=0; i<16; i++ ){
		kinkyuu_tel_no[ i ] 			= ' ';		// 緊急開錠電話番号
	}
	kinkyuu_tel_no[ 16 ] 				= ',';
	
	memset( &yb_touroku_data, 0, ( 42 + ( 160 * 80 * 3 ) ) );
	
	yb_touroku_data.tou_no[ 0 ] 		= '0';		// 棟番号
	yb_touroku_data.tou_no[ 1 ] 		= '0';
	yb_touroku_data.tou_no[ 2 ] 		= ',';

	yb_touroku_data.user_id[ 0 ]		= '0';		// ユーザーID
	yb_touroku_data.user_id[ 1 ]		= '0';
	yb_touroku_data.user_id[ 2 ]		= '0';
	yb_touroku_data.user_id[ 3 ]		= '0';
	yb_touroku_data.user_id[ 4 ]		= ',';

	yb_touroku_data.yubi_seq_no[ 0 ]	= '0';		// 責任者/一般者の登録指情報
	yb_touroku_data.yubi_seq_no[ 1 ]	= '0';
	yb_touroku_data.yubi_seq_no[ 2 ]	= '0';
	yb_touroku_data.yubi_seq_no[ 3 ]	= ',';

	yb_touroku_data.kubun[ 0 ]			= '2';		// 責任者/一般者区分、デホルトは”一般者”
	yb_touroku_data.kubun[ 1 ]			= ',';

	yb_touroku_data.yubi_no[ 0 ]		= '0';		// 登録指番号
	yb_touroku_data.yubi_no[ 1 ]		= '0';
	yb_touroku_data.yubi_no[ 2 ]		= ',';

	yb_touroku_data.name[ 24 ]			= ',';		// 登録者の名前、デホルトはNULL
	
	
	kinkyuu_touroku_no[0]				= ' ';		// 緊急開錠の緊急登録番号４桁（配列最終番目は区切り記号　NUL　）
	kinkyuu_touroku_no[1]				= ' ';
	kinkyuu_touroku_no[2]				= ' ';
	kinkyuu_touroku_no[3]				= ' ';
	kinkyuu_touroku_no[4]				= 0;

	kinkyuu_hyouji_no[0]				= ' ';		// 緊急開錠の緊急番号８桁表示データ（配列最終番目は区切り記号　NUL　）
	kinkyuu_hyouji_no[1]				= ' ';	
	kinkyuu_hyouji_no[2]				= ' ';	
	kinkyuu_hyouji_no[3]				= ' ';	
	kinkyuu_hyouji_no[4]				= ' ';	
	kinkyuu_hyouji_no[5]				= ' ';	
	kinkyuu_hyouji_no[6]				= ' ';	
	kinkyuu_hyouji_no[7]				= ' ';	
	kinkyuu_hyouji_no[8]				= 0;

	kinkyuu_kaijyo_no[0]				= ' ';		// 緊急開錠の開錠番号８桁データ（配列最終番目は区切り記号　NUL　）
	kinkyuu_kaijyo_no[1]				= ' ';	
	kinkyuu_kaijyo_no[2]				= ' ';
	kinkyuu_kaijyo_no[3]				= ' ';	
	kinkyuu_kaijyo_no[4]				= ' ';	
	kinkyuu_kaijyo_no[5]				= ' ';	
	kinkyuu_kaijyo_no[6]				= ' ';	
	kinkyuu_kaijyo_no[7]				= ' ';		
	kinkyuu_kaijyo_no[8]				= 0;
	
	mainte_password[0]					= ' ';		// メンテナンス・モード移行時の確認用パスワード４桁。（配列最終番目は区切り記号　NUL　）
	mainte_password[1]					= ' ';
	mainte_password[2]					= ' ';
	mainte_password[3]					= ' ';
	mainte_password[4]					= 0;

}

/*==========================================================================*/
/**
 * カメラ撮影コントロール・タスク初期化
 *
 * @param idTsk タスクID
 * @retval E_OK 正常起動
 */
/*==========================================================================*/
ER CameraTaskInit(ID idTsk)
{
	ER ercd;
	
	// タスクの生成
	if (idTsk > 0) {
		ercd = cre_tsk(idTsk, &ctsk_camera);
		if (ercd == E_OK) {
			s_idTsk = idTsk;
		}
	} else {
		ercd = acre_tsk(&ctsk_camera);
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

