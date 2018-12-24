/**
*	VA-300プログラム
*
*	@file tsk_snd_serial.c
*	@version 1.00
*
*	@author Bionics Co.Ltd T.Nagai
*	@date   2013/10/04
*	@brief  シリアル送信タスク
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


//#define SND_SIO_BUF_SIZE	1024 + 4 (va300.h で定義)
static UB send_sio_buff[ SND_SIO_BUF_SIZE ];	 // シリアル送信コマンドデータ・バッファ
static UINT sio_snd_comm_len;	 // 送信中コマンドの指定レングス
static UINT sio_snd_block_num;	 // 送信中コマンドのブロック番号

static UB IfImageBuf[2*80*40 + 20*10];


static void SendSerialBinaryData( UB *data, UINT cnt );	// Serialデータの送信（汎用）
static void send_sio_WakeUp( void );			// VA300S制御BoxへシリアルでWakeUpの問い合わせを行う（Test	コマンド、電源ON時）
static void send_sio_Touroku( void );			// VA300S制御Boxへシリアルで登録完了コマンド(01)を送信。
static void send_sio_Touroku_Init( int j );		// VA300S制御Boxへシリアルで登録完了コマンド(01)を送信(電源ON時の一括送信)。
static void send_sio_Touroku_InitAll( void );	// VA300S制御Boxへシリアルで登録完了コマンド(01)を送信メイン(電源ON時の一括送信)。
//static void send_sio_Ninshou( UB result );		// VA300S制御Boxへシリアルで認証完了コマンド(03)を送信。
static void send_sio_Ninshou( UB result, UB auth_type, UB info_type );		//20160108Miya FinKeyS // VA300S制御Boxへシリアルで認証完了コマンド(03)を送信。
static void send_sio_Touroku_Del( void );		// VA300S制御Boxへシリアルで登録データ削除コマンド(04)を送信。
static void send_sio_Touroku_AllDel( void );	// VA300S制御Boxへシリアルで登録データ初期化（一括削除）コマンド(05)を送信。
static void send_sio_ShutDown( void );			// VA300S制御Boxへシリアルでシャット・ダウン要求コマンド(08)を送信。
static void send_sio_Kakaihou_time( void );		// VA300s制御Boxへシリアルで過開放時間の設定要求コマンド(09)を送信する。
static void send_sio_init_time( void );			// VA300s制御Boxへ初期時刻の設定要求コマンド(10)を送信する。
static void send_sio_force_lock_close( void );	// VA300s制御Boxへ強制解錠コマンド(11)を送信する。
static void send_sio_BPWR( int sw );		//20160905Miya B-PWR制御
static void send_sio_VA300Reset( void );	//20161031Miya Ver2204 端末リセット
static void send_sio_STANDARD( void );			//20160905Miya PCからVA300Sを制御する
static void send_sio_TANMATU_INFO( void );		//20160905Miya PCからVA300Sを制御する
static void send_sio_REGDATA_UPLD_STA( void );	//20160905Miya PCからVA300Sを制御する
static void send_sio_REGDATA_UPLD_GET( void );	//20160905Miya PCからVA300Sを制御する
static void send_sio_REGPROC( int rslt );		//20160905Miya PCからVA300Sを制御する
static void send_sio_AUTHPROC( int rslt );		//20160905Miya PCからVA300Sを制御する

/*******************************
 * Send Task
 *
 * @param ch チャンネル番号
 *******************************/
TASK SndTask(INT ch)
{
	T_COMMSG *msg;
	UINT i;
	UB c;
	ER	ercd;

	for (;;)
	{
		/* Wait message */

//		ercd = rcv_mbx(MBX_SND+ch, &msg);
		ercd = rcv_mbx(MBX_SND_SIO, &msg);
		
		if( ercd == E_OK) {
	        
	        /* Send 1 line */
			for (i = 0;i < msg->cnt;) {
				c = msg->buf[i++];
				put_sio(ch, c, TMO_FEVR);
			}

			/* Release memory block */
			rel_mpf(MPF_SND_SIO, msg);

			/* Wait completion */
			fls_sio(ch, TMO_FEVR);
			
    	} else {							/* コーディングエラー */
	    	ErrCodeSet( ercd );
    	}
    }
}


//=============================================================================
/**
 *	Serialデータの送信（汎用）
 *	@param	data　送信データ
 *	@param	cnt　送信バイト数
 */
//=============================================================================
void SendSerialBinaryData( UB *data, UINT cnt )
{
	T_COMMSG *msg;
	ER		ercd;
	
	ercd = tget_mpf( MPF_SND_SIO, ( VP* )&msg, ( 10/MSEC ) );
	
	sio_snd_TSK_wdt = FLG_ON;			// SIO送信タスク　WDTカウンタリセット・リクエスト・フラグ
	
	if( ercd == E_OK ) {		
		memset( &(msg->buf), 0, sizeof( msg->buf ) );
	
		msg->cnt    = cnt;
		msg->msgpri = 1;
		memcpy( &( msg->buf ), data, cnt );
	
		snd_mbx( MBX_SND_SIO, ( T_MSG* )msg );
	}
	
}


/*==========================================================================*/
/**
 *	VA300S制御BoxへシリアルでWakeUpの問い合わせを行う（Test	コマンド、電源ON時）
 */
/*==========================================================================*/
static void send_sio_WakeUp( void )
{
	int i;
	UB str1;

#if(PCCTRL)	//20160930Miya PCからVA300Sを制御する
	MdCngSubMode( SUB_MD_IDLE );	//　サブモード・ステータスを、”アイドル”へ。
	return;
#endif
	
	/** コマンドデータの準備	**/
	send_sio_buff[ 0 ] = '*';			// ヘッダ　1Byte　ASCII '*'
	send_sio_buff[ 1 ] = 0;				// データ長　２バイトBinary
	send_sio_buff[ 2 ] = 0x0A;
	send_sio_buff[ 3 ] = 0x00;			// ブロック番号 ２バイトバイナリ
	send_sio_buff[ 4 ] = 0x00;			
	send_sio_buff[ 5 ] = '0';			// コマンド種別 ２桁ASCII
	send_sio_buff[ 6 ] = '0';

										//  仕様種別　２桁ASCII
	if ( sys_demo_flg != SYS_SPEC_DEMO ){	// デモ仕様の有無
		send_sio_buff[ 7 ] = '0';			//　通常仕様
	} else {
		send_sio_buff[ 7 ] = '1';			//　デモ仕様	
	}
	
	if ( GetSysSpec() == SYS_SPEC_MANTION ){				// マンション・占有部仕様
		send_sio_buff[ 8 ] = '0';

	} else if ( GetSysSpec() == SYS_SPEC_OFFICE ){			// １対１仕様（オフィス仕様）
		send_sio_buff[ 8 ] = '1';
	
	} else if ( GetSysSpec() == SYS_SPEC_ENTRANCE ){		// マンション・共用部仕様
		send_sio_buff[ 8 ] = '2';
	
	} else if ( GetSysSpec() == SYS_SPEC_OFFICE_NO_ID ){	// １対多仕様（オフィス・ID番号無し仕様）
		send_sio_buff[ 8 ] = '3';
	
	} else {
		send_sio_buff[ 8 ] = '0';		// 仕様設定が不定の場合は、マンション・占有部仕様
	}

	send_sio_buff[ 9 ] = (UINT)g_TechMenuData.HijyouRemocon + '0';			// 機器設定（非常時屋内解錠リモコンの有無）

	sio_snd_comm_len = 10;				// コマンド長
	
	// チェックSUMの準備。
	str1 = 0;
	for ( i=0; i<sio_snd_comm_len; i++ ){	// コマンドのバイト分の総和を求める
		str1 = str1 + send_sio_buff[ i ];
	}
	send_sio_buff[ 10 ] = str1 ^ 0xff;	// チェックサムを反転
	
	send_sio_buff[ 11 ] = CODE_CR;		//　終端コード
	send_sio_buff[ 12 ] = CODE_LF;
	
	/** コマンド送信	**/
	SendSerialBinaryData( (UB*)send_sio_buff, ( sio_snd_comm_len + 3 ) );

	sio_mode = SIO_SEND_ACK_WAIT;				// シリアル送受信モード　→　Ack/Nack応答待ち
	ACK_Wait_timer = ACK_WAIT_TIME;				// Ack待ちタイムアウト時間Set

	usSioRcvCount = 0;
	memset( cSioRcvBuf, 0, RCV_BUF_SIZE );

}


/*==========================================================================*/
/**
 *	登録完了コマンド(01)
 */
/*==========================================================================*/
static void send_sio_Touroku( void )
{
	int i, ii, k;
	UB str1;
	int cnt;

#if(PCCTRL)	//20160930Miya PCからVA300Sを制御する
	MdCngSubMode( SUB_MD_IDLE );	//　サブモード・ステータスを、”アイドル”へ。
	return;
#endif
	
	/** コマンドデータの準備	**/
	cnt = 0;
	send_sio_buff[ cnt++ ] = '*';			// ヘッダ　1Byte　ASCII '*'
	send_sio_buff[ cnt++ ] = 0;				// データ長　２バイトBinary
	send_sio_buff[ cnt++ ] = 0;
	send_sio_buff[ cnt++ ] = 0x00;			// ブロック番号 ２バイトバイナリ
	send_sio_buff[ cnt++ ] = 0x00;			
	send_sio_buff[ cnt++ ] = '0';			// コマンド種別 ２桁ASCII
	send_sio_buff[ cnt++ ] = '1';
										
	// 棟番号　ASCII ３桁
	send_sio_buff[ cnt++ ] = ' ';
	send_sio_buff[ cnt++ ] = yb_touroku_data.tou_no[ 0 ];
	send_sio_buff[ cnt++ ] = yb_touroku_data.tou_no[ 1 ];		

	// ユーザーID　ASCII ４桁
	send_sio_buff[ cnt++ ] = yb_touroku_data.user_id[ 0 ];
	send_sio_buff[ cnt++ ] = yb_touroku_data.user_id[ 1 ];
	send_sio_buff[ cnt++ ] = yb_touroku_data.user_id[ 2 ];
	send_sio_buff[ cnt++ ] = yb_touroku_data.user_id[ 3 ];
	
	// 登録指ID　ASCII ３桁
	send_sio_buff[ cnt++ ] = yb_touroku_data.yubi_seq_no[ 0 ];
	send_sio_buff[ cnt++ ] = yb_touroku_data.yubi_seq_no[ 1 ];
	send_sio_buff[ cnt++ ] = yb_touroku_data.yubi_seq_no[ 2 ];
	
	// 責任者/一般者区分　ASCII １桁
	send_sio_buff[ cnt++ ] = yb_touroku_data.kubun[ 0 ];

	// 登録指種別　ASCII ２桁
	send_sio_buff[ cnt++ ] = yb_touroku_data.yubi_no[ 0 ];
	send_sio_buff[ cnt++ ] = yb_touroku_data.yubi_no[ 1 ];
	
	// 名前　ASCII ２４桁
	ii = 0;
	for ( i = 0 ; i < 24 ; i++  ){
		//send_sio_buff[ cnt++ ] = yb_touroku_data.name[ i ];
		if(yb_touroku_data.name[ ii ] == 0){
			send_sio_buff[ cnt++ ] = 0x20;
			k = 0;
		}else{
			send_sio_buff[ cnt++ ] = CngNameCharaCode( yb_touroku_data.name[ ii ], &k );
		}
		if(k == 2){
			send_sio_buff[ cnt++ ] = 0xDE; //濁点
			++i;
		}
		if(k == 3){
			send_sio_buff[ cnt++ ] = 0xDF; //半濁点
			++i;
		}
		++ii;
	}

	sio_snd_comm_len = cnt;					// コマンド長
	send_sio_buff[ 2 ] = sio_snd_comm_len;
	
	// チェックSUMの準備。
	str1 = 0;
	for ( i = 0; i < sio_snd_comm_len; i++ ){	// コマンドのバイト分の総和を求める
		str1 = str1 + send_sio_buff[ i ];
	}
	send_sio_buff[ cnt++ ] = str1 ^ 0xff;	// チェックサムを反転
	
	send_sio_buff[ cnt++ ] = CODE_CR;		//　終端コード
	send_sio_buff[ cnt++ ] = CODE_LF;
	
	/** コマンド送信	**/
	SendSerialBinaryData( (UB*)send_sio_buff, ( sio_snd_comm_len + 3 ) );

	sio_mode = SIO_SEND_ACK_WAIT;				// シリアル送受信モード　→　Ack/Nack応答待ち
	ACK_Wait_timer = ACK_WAIT_TIME;				// Ack待ちタイムアウト時間Set

	usSioRcvCount = 0;
	memset( cSioRcvBuf, 0, RCV_BUF_SIZE );

}

/*==========================================================================*/
/**
 *	登録初期送信コマンド(01)
 */
/*==========================================================================*/
static void send_sio_Touroku_InitAll( void )
{
	int i;
	UB str1;
	int cnt;

#if(PCCTRL)	//20160930Miya PCからVA300Sを制御する
	MdCngSubMode( SUB_MD_IDLE );	//　サブモード・ステータスを、”アイドル”へ。
	return;
#endif
	
	for ( i = 0; i < 20 ; i++ ){
		
		if ( ( ( yb_touroku_data20[ i + 1 ].yubi_seq_no[ 0 ] >= '0') && ( yb_touroku_data20[ i + 1 ].yubi_seq_no[ 0 ] <= '9') )
	 	  && ( ( yb_touroku_data20[ i + 1 ].yubi_seq_no[ 1 ] >= '0') && ( yb_touroku_data20[ i + 1 ].yubi_seq_no[ 1 ] <= '9') )
		  && ( ( yb_touroku_data20[ i + 1 ].yubi_seq_no[ 2 ] >= '0') && ( yb_touroku_data20[ i + 1 ].yubi_seq_no[ 2 ] <= '9') ) ){

			if ( ( yb_touroku_data20[ i + 1 ].yubi_seq_no[ 0 ] != '0' )
			  || ( yb_touroku_data20[ i + 1 ].yubi_seq_no[ 1 ] != '0' )
			  || ( yb_touroku_data20[ i + 1 ].yubi_seq_no[ 2 ] != '0' ) ){
				send_sio_Touroku_Init( i );			// 登録初期送信コマンド(01)送信。
				MdCngSubMode( SUB_MD_TOUROKU );		// サブモードを、登録データ送信中へ。
			
				while ( MdGetSubMode() != SUB_MD_IDLE ){	// 送信後のAck受信待ち。
					dly_tsk( 25/MSEC );
				}	
			}
		}
	}
}


/*==========================================================================*/
/**
 *	登録初期送信コマンド(01)
 */
/*==========================================================================*/
static void send_sio_Touroku_Init( int j )
{
	int i, ii, k;
	UB str1;
	int cnt;

#if(PCCTRL)	//20160930Miya PCからVA300Sを制御する
	MdCngSubMode( SUB_MD_IDLE );	//　サブモード・ステータスを、”アイドル”へ。
	return;
#endif
	
	/** コマンドデータの準備	**/
	cnt = 0;
	send_sio_buff[ cnt++ ] = '*';			// ヘッダ　1Byte　ASCII '*'
	send_sio_buff[ cnt++ ] = 0;				// データ長　２バイトBinary
	send_sio_buff[ cnt++ ] = 0;
	send_sio_buff[ cnt++ ] = 0x00;			// ブロック番号 ２バイトバイナリ
	send_sio_buff[ cnt++ ] = 0x00;			
	send_sio_buff[ cnt++ ] = '0';			// コマンド種別 ２桁ASCII
	send_sio_buff[ cnt++ ] = '1';
										
	// 棟番号　ASCII ３桁
	send_sio_buff[ cnt++ ] = ' ';
	send_sio_buff[ cnt++ ] = yb_touroku_data.tou_no[ 0 ];
	send_sio_buff[ cnt++ ] = yb_touroku_data.tou_no[ 1 ];		

	// ユーザーID　ASCII ４桁
	send_sio_buff[ cnt++ ] = yb_touroku_data.user_id[ 0 ];
	send_sio_buff[ cnt++ ] = yb_touroku_data.user_id[ 1 ];
	send_sio_buff[ cnt++ ] = yb_touroku_data.user_id[ 2 ];
	send_sio_buff[ cnt++ ] = yb_touroku_data.user_id[ 3 ];
	
	// 登録指ID　ASCII ３桁
	send_sio_buff[ cnt++ ] = yb_touroku_data20[ j + 1 ].yubi_seq_no[ 0 ];
	send_sio_buff[ cnt++ ] = yb_touroku_data20[ j + 1 ].yubi_seq_no[ 1 ];
	send_sio_buff[ cnt++ ] = yb_touroku_data20[ j + 1 ].yubi_seq_no[ 2 ];
	
	// 責任者/一般者区分　ASCII １桁
	send_sio_buff[ cnt++ ] = yb_touroku_data20[ j + 1 ].kubun[ 0 ];

	// 登録指種別　ASCII ２桁
	send_sio_buff[ cnt++ ] = yb_touroku_data20[ j + 1 ].yubi_no[ 0 ];
	send_sio_buff[ cnt++ ] = yb_touroku_data20[ j + 1 ].yubi_no[ 1 ];
	
	// 名前　ASCII ２４桁
	ii = k = 0;
	for ( i = 0 ; i < 24 ; i++  ){
		//send_sio_buff[ cnt++ ] = yb_touroku_data20[ j + 1 ].name[ i ];
		if(yb_touroku_data20[ j + 1 ].name[ ii ] == 0){
			send_sio_buff[ cnt++ ] = 0x20;
			k = 0;
		}else{
			send_sio_buff[ cnt++ ] = CngNameCharaCode( yb_touroku_data20[ j + 1 ].name[ ii ], &k );
		}
		if(k == 2){
			send_sio_buff[ cnt++ ] = 0xDE; //濁点
			++i;
		}
		if(k == 3){
			send_sio_buff[ cnt++ ] = 0xDF; //半濁点
			++i;
		}
		++ii;
	}

	sio_snd_comm_len = cnt;					// コマンド長
	send_sio_buff[ 2 ] = sio_snd_comm_len;
	
	// チェックSUMの準備。
	str1 = 0;
	for ( i = 0; i < sio_snd_comm_len; i++ ){	// コマンドのバイト分の総和を求める
		str1 = str1 + send_sio_buff[ i ];
	}
	send_sio_buff[ cnt++ ] = str1 ^ 0xff;	// チェックサムを反転
	
	send_sio_buff[ cnt++ ] = CODE_CR;		//　終端コード
	send_sio_buff[ cnt++ ] = CODE_LF;
	
	/** コマンド送信	**/
	SendSerialBinaryData( (UB*)send_sio_buff, ( sio_snd_comm_len + 3 ) );

	sio_mode = SIO_SEND_ACK_WAIT;				// シリアル送受信モード　→　Ack/Nack応答待ち
	ACK_Wait_timer = ACK_WAIT_TIME;				// Ack待ちタイムアウト時間Set

	usSioRcvCount = 0;
	memset( cSioRcvBuf, 0, RCV_BUF_SIZE );

}


/*==========================================================================*/
/**
 *	認証完了コマンド(03)
 */
/*==========================================================================*/
//static void send_sio_Ninshou( UB result )
static void send_sio_Ninshou( UB result, UB auth_type, UB info_type )
{
	int i;
	UB str1;
	int cnt;

#if(PCCTRL)	//20160930Miya PCからVA300Sを制御する
	MdCngSubMode( SUB_MD_IDLE );	//　サブモード・ステータスを、”アイドル”へ。
	return;
#endif
	
	/** コマンドデータの準備	**/
	cnt = 0;
	send_sio_buff[ cnt++ ] = '*';			// ヘッダ　1Byte　ASCII '*'
	send_sio_buff[ cnt++ ] = 0;				// データ長　２バイトBinary
	send_sio_buff[ cnt++ ] = 0x0b;
	send_sio_buff[ cnt++ ] = 0x00;			// ブロック番号 ２バイトバイナリ
	send_sio_buff[ cnt++ ] = 0x00;			
	send_sio_buff[ cnt++ ] = '0';			// コマンド種別 ２桁ASCII
	send_sio_buff[ cnt++ ] = '3';
										//  仕様種別　２桁ASCII
	if ( result == 1 ){	// 認証結果
		send_sio_buff[ cnt++ ] = 'O';			//　認証OK
		send_sio_buff[ cnt++ ] = 'K';			//　認証OK
	}else{
		send_sio_buff[ cnt++ ] = 'N';			//　認証NG	
		send_sio_buff[ cnt++ ] = 'G';			//　認証OK
	}
	
	//20160108Miya FinKetS
	if( auth_type == 1 ){						//パスワード開錠
		i = g_PasswordOpen2.num / 10;
		send_sio_buff[ cnt++ ] = 0x30 + i;
		i = g_PasswordOpen2.num % 10;
		send_sio_buff[ cnt++ ] = 0x30 + i;
		
		//20160108Miya FinKetS
		send_sio_buff[ cnt++ ] = '1';
	}else if( auth_type == 2 ){					//緊急開錠
		send_sio_buff[ cnt++ ] = 0x30;
		send_sio_buff[ cnt++ ] = 0x30;

		//20160108Miya FinKetS
		send_sio_buff[ cnt++ ] = '2';
	}else{										//指開錠
		i = g_RegUserInfoData.RegNum / 10;
		send_sio_buff[ cnt++ ] = 0x30 + i;
		i = g_RegUserInfoData.RegNum % 10;
		send_sio_buff[ cnt++ ] = 0x30 + i;

		//20160108Miya FinKetS
		send_sio_buff[ cnt++ ] = '0';
	}

	//20160108Miya FinKetS
	if( info_type == 1 ){						//お出かけ設定
		send_sio_buff[ cnt++ ] = '1';
	}else if( info_type == 2 ){					//お留守番設定
		send_sio_buff[ cnt++ ] = '2';
	}else{
		send_sio_buff[ cnt++ ] = '0';
	}
		
	sio_snd_comm_len = cnt;					// コマンド長
	send_sio_buff[ 2 ] = sio_snd_comm_len;
	
	// チェックSUMの準備。
	str1 = 0;
	for ( i=0; i<sio_snd_comm_len; i++ ){	// コマンドのバイト分の総和を求める
		str1 = str1 + send_sio_buff[ i ];
	}
	send_sio_buff[ cnt++ ] = str1 ^ 0xff;	// チェックサムを反転
	
	send_sio_buff[ cnt++ ] = CODE_CR;		//　終端コード
	send_sio_buff[ cnt++ ] = CODE_LF;
	
	/** コマンド送信	**/
	SendSerialBinaryData( (UB*)send_sio_buff, ( sio_snd_comm_len + 3 ) );

	sio_mode = SIO_SEND_ACK_WAIT;				// シリアル送受信モード　→　Ack/Nack応答待ち
	ACK_Wait_timer = ACK_WAIT_TIME;				// Ack待ちタイムアウト時間Set

	usSioRcvCount = 0;
	memset( cSioRcvBuf, 0, RCV_BUF_SIZE );

}

/*==========================================================================*/
/**
 *	登録データ削除コマンド(04)
 */
/*==========================================================================*/
static void send_sio_Touroku_Del( void )
{
	int i;
	UB str1;
	int cnt;

#if(PCCTRL)	//20160930Miya PCからVA300Sを制御する
	MdCngSubMode( SUB_MD_IDLE );	//　サブモード・ステータスを、”アイドル”へ。
	return;
#endif
	
	/** コマンドデータの準備	**/
	cnt = 0;
	send_sio_buff[ cnt++ ] = '*';			// ヘッダ　1Byte　ASCII '*'
	send_sio_buff[ cnt++ ] = 0;				// データ長　２バイトBinary
	send_sio_buff[ cnt++ ] = 0;
	send_sio_buff[ cnt++ ] = 0x00;			// ブロック番号 ２バイトバイナリ
	send_sio_buff[ cnt++ ] = 0x00;			
	send_sio_buff[ cnt++ ] = '0';			// コマンド種別 ２桁ASCII
	send_sio_buff[ cnt++ ] = '4';		

	// ユーザーID　ASCII ４桁
	send_sio_buff[ cnt++ ] = yb_touroku_data.user_id[ 0 ];
	send_sio_buff[ cnt++ ] = yb_touroku_data.user_id[ 1 ];
	send_sio_buff[ cnt++ ] = yb_touroku_data.user_id[ 2 ];
	send_sio_buff[ cnt++ ] = yb_touroku_data.user_id[ 3 ];
	
	// 登録指ID　ASCII ３桁
	send_sio_buff[ cnt++ ] = yb_touroku_data.yubi_seq_no[ 0 ];
	send_sio_buff[ cnt++ ] = yb_touroku_data.yubi_seq_no[ 1 ];
	send_sio_buff[ cnt++ ] = yb_touroku_data.yubi_seq_no[ 2 ];

	sio_snd_comm_len = cnt;					// コマンド長
	send_sio_buff[ 2 ] = sio_snd_comm_len;
	
	// チェックSUMの準備。
	str1 = 0;
	for ( i = 0; i < sio_snd_comm_len; i++ ){	// コマンドのバイト分の総和を求める
		str1 = str1 + send_sio_buff[ i ];
	}
	send_sio_buff[ cnt++ ] = str1 ^ 0xff;	// チェックサムを反転
	
	send_sio_buff[ cnt++ ] = CODE_CR;		//　終端コード
	send_sio_buff[ cnt++ ] = CODE_LF;
	
	/** コマンド送信	**/
	SendSerialBinaryData( (UB*)send_sio_buff, ( sio_snd_comm_len + 3 ) );

	sio_mode = SIO_SEND_ACK_WAIT;				// シリアル送受信モード　→　Ack/Nack応答待ち
	ACK_Wait_timer = ACK_WAIT_TIME;				// Ack待ちタイムアウト時間Set

	usSioRcvCount = 0;
	memset( cSioRcvBuf, 0, RCV_BUF_SIZE );

}


/*==========================================================================*/
/**
 *	登録データ初期化（一括削除）コマンド(05)
 */
/*==========================================================================*/
static void send_sio_Touroku_AllDel( void )
{
	int i;
	UB str1;
	int cnt;

#if(PCCTRL)	//20160930Miya PCからVA300Sを制御する
	MdCngSubMode( SUB_MD_IDLE );	//　サブモード・ステータスを、”アイドル”へ。
	return;
#endif
	
	/** コマンドデータの準備	**/
	cnt = 0;
	send_sio_buff[ cnt++ ] = '*';			// ヘッダ　1Byte　ASCII '*'
	send_sio_buff[ cnt++ ] = 0;				// データ長　２バイトBinary
	send_sio_buff[ cnt++ ] = 0;
	send_sio_buff[ cnt++ ] = 0x00;			// ブロック番号 ２バイトバイナリ
	send_sio_buff[ cnt++ ] = 0x00;			
	send_sio_buff[ cnt++ ] = '0';			// コマンド種別 ２桁ASCII
	send_sio_buff[ cnt++ ] = '5';		

	// 伝送データ　ASCII ２桁
	send_sio_buff[ cnt++ ] = ' ';
	send_sio_buff[ cnt++ ] = ' ';

	sio_snd_comm_len = cnt;					// コマンド長
	send_sio_buff[ 2 ] = sio_snd_comm_len;
	
	// チェックSUMの準備。
	str1 = 0;
	for ( i = 0; i < sio_snd_comm_len; i++ ){	// コマンドのバイト分の総和を求める
		str1 = str1 + send_sio_buff[ i ];
	}
	send_sio_buff[ cnt++ ] = str1 ^ 0xff;	// チェックサムを反転
	
	send_sio_buff[ cnt++ ] = CODE_CR;		//　終端コード
	send_sio_buff[ cnt++ ] = CODE_LF;
	
	/** コマンド送信	**/
	SendSerialBinaryData( (UB*)send_sio_buff, ( sio_snd_comm_len + 3 ) );

	sio_mode = SIO_SEND_ACK_WAIT;				// シリアル送受信モード　→　Ack/Nack応答待ち
	ACK_Wait_timer = ACK_WAIT_TIME;				// Ack待ちタイムアウト時間Set

	usSioRcvCount = 0;
	memset( cSioRcvBuf, 0, RCV_BUF_SIZE );

}


/*==========================================================================*/
/**
 *	VA300S制御Boxへシリアルでシャット・ダウン要求コマンド(08)を送信する。
 */
/*==========================================================================*/
static void send_sio_ShutDown( void )
{
	int i;
	UB str1;

#if(PCCTRL)	//20160930Miya PCからVA300Sを制御する
	MdCngSubMode( SUB_MD_IDLE );	//　サブモード・ステータスを、”アイドル”へ。
	return;
#endif

	/** コマンドデータの準備	**/
	send_sio_buff[ 0 ] = '*';			// ヘッダ　1Byte　ASCII '*'
	send_sio_buff[ 1 ] = 0;				// データ長　２バイトBinary
	send_sio_buff[ 2 ] = 0x09;
	send_sio_buff[ 3 ] = 0x00;			// ブロック番号 ２バイトバイナリ
	send_sio_buff[ 4 ] = 0x00;			
	send_sio_buff[ 5 ] = '0';			// コマンド種別 ２桁ASCII
	send_sio_buff[ 6 ] = '8';

	send_sio_buff[ 7 ] = ' ';			//  伝送データ　２桁ASCII
	send_sio_buff[ 8 ] = ' ';			

	sio_snd_comm_len = 9;				// コマンド長
	
	// チェックSUMの準備。
	str1 = 0;
	for ( i=0; i<sio_snd_comm_len; i++ ){	// コマンドのバイト分の総和を求める
		str1 = str1 + send_sio_buff[ i ];
	}
	send_sio_buff[ 9 ] = str1 ^ 0xff;	// チェックサムを反転
	
	send_sio_buff[ 10 ] = CODE_CR;		//　終端コード
	send_sio_buff[ 11 ] = CODE_LF;
	
	/** コマンド送信	**/
	SendSerialBinaryData( (UB*)send_sio_buff, ( sio_snd_comm_len + 3 ) );

	sio_mode = SIO_SEND_ACK_WAIT;				// シリアル送受信モード　→　Ack/Nack応答待ち
	ACK_Wait_timer = ACK_WAIT_TIME;				// Ack待ちタイムアウト時間Set

	usSioRcvCount = 0;
	memset( cSioRcvBuf, 0, RCV_BUF_SIZE );

}


/*==========================================================================*/
/**
 *  VA300s制御Boxへ過開放時間の設定要求コマンド(09)を送信する。
 */
/*==========================================================================*/
static void send_sio_Kakaihou_time( void )
{
	int i;
	UB str1, str4[ 4 ];
	int cnt;

#if(PCCTRL)	//20160930Miya PCからVA300Sを制御する
	MdCngSubMode( SUB_MD_IDLE );	//　サブモード・ステータスを、”アイドル”へ。
	return;
#endif
	
	/** コマンドデータの準備	**/
	cnt = 0;
	send_sio_buff[ cnt++ ] = '*';			// ヘッダ　1Byte　ASCII '*'
	send_sio_buff[ cnt++ ] = 0;				// データ長　２バイトBinary
	send_sio_buff[ cnt++ ] = 0;
	send_sio_buff[ cnt++ ] = 0x00;			// ブロック番号 ２バイトバイナリ
	send_sio_buff[ cnt++ ] = 0x00;			
	send_sio_buff[ cnt++ ] = '0';			// コマンド種別 ２桁ASCII
	send_sio_buff[ cnt++ ] = '9';		

	// 過開放設定時間　ASCII ４桁
	if ( door_open_over_time >= 9999 ){
		send_sio_buff[ cnt++ ] = '9';
		send_sio_buff[ cnt++ ] = '9';
		send_sio_buff[ cnt++ ] = '9';
		send_sio_buff[ cnt++ ] = '9';
			
	}	else {
		util_i_to_char4( door_open_over_time, str4 );		// intger数字を　char４桁(右詰め)に変換する.
		
		send_sio_buff[ cnt++ ] = str4[ 0 ];
		send_sio_buff[ cnt++ ] = str4[ 1 ];
		send_sio_buff[ cnt++ ] = str4[ 2 ];
		send_sio_buff[ cnt++ ] = str4[ 3 ];
	}

	sio_snd_comm_len = cnt;					// コマンド長
	send_sio_buff[ 2 ] = sio_snd_comm_len;
	
	// チェックSUMの準備。
	str1 = 0;
	for ( i = 0; i < sio_snd_comm_len; i++ ){	// コマンドのバイト分の総和を求める
		str1 = str1 + send_sio_buff[ i ];
	}
	send_sio_buff[ cnt++ ] = str1 ^ 0xff;	// チェックサムを反転
	
	send_sio_buff[ cnt++ ] = CODE_CR;		//　終端コード
	send_sio_buff[ cnt++ ] = CODE_LF;
	
	/** コマンド送信	**/
	SendSerialBinaryData( (UB*)send_sio_buff, ( sio_snd_comm_len + 3 ) );

	sio_mode = SIO_SEND_ACK_WAIT;				// シリアル送受信モード　→　Ack/Nack応答待ち
	ACK_Wait_timer = ACK_WAIT_TIME;				// Ack待ちタイムアウト時間Set

	usSioRcvCount = 0;
	memset( cSioRcvBuf, 0, RCV_BUF_SIZE );
	
}

/*==========================================================================*/
/**
 *  VA300s制御Boxへ初期時刻の設定要求コマンド(10)を送信する。
 */
/*==========================================================================*/
static void send_sio_init_time( void )
{
	int i;
	UB str1;
	UB fig1, fig2;
	int cnt;

#if(PCCTRL)	//20160930Miya PCからVA300Sを制御する
	MdCngSubMode( SUB_MD_IDLE );	//　サブモード・ステータスを、”アイドル”へ。
	return;
#endif
	
	/** コマンドデータの準備	**/
	cnt = 0;
	send_sio_buff[ cnt++ ] = '*';			// ヘッダ　1Byte　ASCII '*'
	send_sio_buff[ cnt++ ] = 0;				// データ長　２バイトBinary
	send_sio_buff[ cnt++ ] = 0;
	send_sio_buff[ cnt++ ] = 0x00;			// ブロック番号 ２バイトバイナリ
	send_sio_buff[ cnt++ ] = 0x00;			
	send_sio_buff[ cnt++ ] = '1';			// コマンド種別 ２桁ASCII
	send_sio_buff[ cnt++ ] = '0';		

	// 初期時刻の設定　ASCII ６桁
	if ( ( count_1hour > 0 ) && ( count_1hour < 24 ) ){	// 時
		fig2 = count_1hour / 10;
		fig1 = count_1hour - ( fig2 * 10 );
		send_sio_buff[ cnt++ ] = fig2 + '0';
		send_sio_buff[ cnt++ ] = fig1 + '0';
	} else {
		send_sio_buff[ cnt++ ] = '0';
		send_sio_buff[ cnt++ ] = '0';		
	}
	
	if ( count_1min > 0 && count_1min < 60 ){		// 分
		fig2 = count_1min / 10;
		fig1 = count_1min - ( fig2 * 10 );
		send_sio_buff[ cnt++ ] = fig2 + '0';
		send_sio_buff[ cnt++ ] = fig1 + '0';
	} else {
		send_sio_buff[ cnt++ ] = '0';
		send_sio_buff[ cnt++ ] = '0';		
	}

	if ( count_1sec > 0 && count_1sec < 60 ){		// 秒
		fig2 = count_1sec / 10;
		fig1 = count_1sec - ( fig2 * 10 );
		send_sio_buff[ cnt++ ] = fig2 + '0';
		send_sio_buff[ cnt++ ] = fig1 + '0';
	} else {
		send_sio_buff[ cnt++ ] = '0';
		send_sio_buff[ cnt++ ] = '0';		
	}

	sio_snd_comm_len = cnt;					// コマンド長
	send_sio_buff[ 2 ] = sio_snd_comm_len;
	
	// チェックSUMの準備。
	str1 = 0;
	for ( i = 0; i < sio_snd_comm_len; i++ ){	// コマンドのバイト分の総和を求める
		str1 = str1 + send_sio_buff[ i ];
	}
	send_sio_buff[ cnt++ ] = str1 ^ 0xff;	// チェックサムを反転
	
	send_sio_buff[ cnt++ ] = CODE_CR;		//　終端コード
	send_sio_buff[ cnt++ ] = CODE_LF;
	
	/** コマンド送信	**/
	SendSerialBinaryData( (UB*)send_sio_buff, ( sio_snd_comm_len + 3 ) );

	sio_mode = SIO_SEND_ACK_WAIT;				// シリアル送受信モード　→　Ack/Nack応答待ち
	ACK_Wait_timer = ACK_WAIT_TIME;				// Ack待ちタイムアウト時間Set

	usSioRcvCount = 0;
	memset( cSioRcvBuf, 0, RCV_BUF_SIZE );
	
}


/*==========================================================================*/
/**
 *  VA300s制御Boxへ強制解錠コマンド(11)を送信する。
 */
/*==========================================================================*/
static void send_sio_force_lock_close( void )
{
	int i;
	UB str1;
	int cnt;

#if(PCCTRL)	//20160930Miya PCからVA300Sを制御する
	MdCngSubMode( SUB_MD_IDLE );	//　サブモード・ステータスを、”アイドル”へ。
	return;
#endif
	
	/** コマンドデータの準備	**/
	cnt = 0;
	send_sio_buff[ cnt++ ] = '*';			// ヘッダ　1Byte　ASCII '*'
	send_sio_buff[ cnt++ ] = 0;				// データ長　２バイトBinary
	send_sio_buff[ cnt++ ] = 0;
	send_sio_buff[ cnt++ ] = 0x00;			// ブロック番号 ２バイトバイナリ
	send_sio_buff[ cnt++ ] = 0x00;			
	send_sio_buff[ cnt++ ] = '1';			// コマンド種別 ２桁ASCII
	send_sio_buff[ cnt++ ] = '1';

	send_sio_buff[ cnt++ ] = ' ';			//  伝送データ　２桁ASCII
	send_sio_buff[ cnt++ ] = ' ';			

	sio_snd_comm_len = cnt;					// コマンド長
	send_sio_buff[ 2 ] = sio_snd_comm_len;
		
	// チェックSUMの準備。
	str1 = 0;
	for ( i = 0; i < sio_snd_comm_len; i++ ){	// コマンドのバイト分の総和を求める
		str1 = str1 + send_sio_buff[ i ];
	}
	send_sio_buff[ cnt++ ] = str1 ^ 0xff;	// チェックサムを反転
	
	send_sio_buff[ cnt++ ] = CODE_CR;		//　終端コード
	send_sio_buff[ cnt++ ] = CODE_LF;
	
	/** コマンド送信	**/
	SendSerialBinaryData( (UB*)send_sio_buff, ( sio_snd_comm_len + 3 ) );

	sio_mode = SIO_SEND_ACK_WAIT;				// シリアル送受信モード　→　Ack/Nack応答待ち
	ACK_Wait_timer = ACK_WAIT_TIME;				// Ack待ちタイムアウト時間Set

	usSioRcvCount = 0;
	memset( cSioRcvBuf, 0, RCV_BUF_SIZE );
	
}

//20160905Miya B-PWR制御
/*==========================================================================*/
/**
 *  VA300s制御Boxへコマンド(22)を送信する。
 */
// B-PWR ON/OFF制御　0:OFF 1:ON
/*==========================================================================*/
static void send_sio_BPWR( int sw )
{
	int i;
	UB str1;
	int cnt;

#if(BPWRCTRL == 0)
	MdCngSubMode( SUB_MD_IDLE );	//　サブモード・ステータスを、”アイドル”へ。
	return;
#endif

#if(PCCTRL)	//20160930Miya PCからVA300Sを制御する
	MdCngSubMode( SUB_MD_IDLE );	//　サブモード・ステータスを、”アイドル”へ。
	return;
#endif

	/** コマンドデータの準備	**/
	cnt = 0;
	send_sio_buff[ cnt++ ] = '*';			// ヘッダ　1Byte　ASCII '*'
	send_sio_buff[ cnt++ ] = 0;				// データ長　２バイトBinary
	send_sio_buff[ cnt++ ] = 0;
	send_sio_buff[ cnt++ ] = 0x00;			// ブロック番号 ２バイトバイナリ
	send_sio_buff[ cnt++ ] = 0x00;			
	send_sio_buff[ cnt++ ] = '2';			// コマンド種別 ２桁ASCII
	send_sio_buff[ cnt++ ] = '2';

	if(sw == FLG_OFF){
		send_sio_buff[ cnt++ ] = 'O';			//  伝送データ　２桁ASCII
		send_sio_buff[ cnt++ ] = 'F';			
	}else{
		send_sio_buff[ cnt++ ] = 'O';			//  伝送データ　２桁ASCII
		send_sio_buff[ cnt++ ] = 'N';			
	}

	sio_snd_comm_len = cnt;					// コマンド長
	send_sio_buff[ 2 ] = sio_snd_comm_len;
		
	// チェックSUMの準備。
	str1 = 0;
	for ( i = 0; i < sio_snd_comm_len; i++ ){	// コマンドのバイト分の総和を求める
		str1 = str1 + send_sio_buff[ i ];
	}
	send_sio_buff[ cnt++ ] = str1 ^ 0xff;	// チェックサムを反転
	
	send_sio_buff[ cnt++ ] = CODE_CR;		//　終端コード
	send_sio_buff[ cnt++ ] = CODE_LF;
	
	/** コマンド送信	**/
	SendSerialBinaryData( (UB*)send_sio_buff, ( sio_snd_comm_len + 3 ) );

	sio_mode = SIO_SEND_ACK_WAIT;				// シリアル送受信モード　→　Ack/Nack応答待ち
	ACK_Wait_timer = ACK_WAIT_TIME;				// Ack待ちタイムアウト時間Set

	usSioRcvCount = 0;
	memset( cSioRcvBuf, 0, RCV_BUF_SIZE );
	
}

//20161031Miya Ver2204 端末リセット
/*==========================================================================*/
/**
 *  VA300s制御Boxへコマンド(23)を送信する。
 */
// 端末リセット
/*==========================================================================*/
static void send_sio_VA300Reset( void )	//20161031Miya Ver2204 端末リセット
{
	int i;
	UB str1;
	int cnt;

#if(BPWRCTRL == 0)
	MdCngSubMode( SUB_MD_IDLE );	//　サブモード・ステータスを、”アイドル”へ。
	return;
#endif

#if(PCCTRL)	//20160930Miya PCからVA300Sを制御する
	MdCngSubMode( SUB_MD_IDLE );	//　サブモード・ステータスを、”アイドル”へ。
	return;
#endif

	/** コマンドデータの準備	**/
	cnt = 0;
	send_sio_buff[ cnt++ ] = '*';			// ヘッダ　1Byte　ASCII '*'
	send_sio_buff[ cnt++ ] = 0;				// データ長　２バイトBinary
	send_sio_buff[ cnt++ ] = 0;
	send_sio_buff[ cnt++ ] = 0x00;			// ブロック番号 ２バイトバイナリ
	send_sio_buff[ cnt++ ] = 0x00;			
	send_sio_buff[ cnt++ ] = '2';			// コマンド種別 ２桁ASCII
	send_sio_buff[ cnt++ ] = '3';

	send_sio_buff[ cnt++ ] = 0x20;			//  伝送データ　２桁ASCII
	send_sio_buff[ cnt++ ] = 0x20;			

	sio_snd_comm_len = cnt;					// コマンド長
	send_sio_buff[ 2 ] = sio_snd_comm_len;
		
	// チェックSUMの準備。
	str1 = 0;
	for ( i = 0; i < sio_snd_comm_len; i++ ){	// コマンドのバイト分の総和を求める
		str1 = str1 + send_sio_buff[ i ];
	}
	send_sio_buff[ cnt++ ] = str1 ^ 0xff;	// チェックサムを反転
	
	send_sio_buff[ cnt++ ] = CODE_CR;		//　終端コード
	send_sio_buff[ cnt++ ] = CODE_LF;
	
	/** コマンド送信	**/
	SendSerialBinaryData( (UB*)send_sio_buff, ( sio_snd_comm_len + 3 ) );

	sio_mode = SIO_SEND_ACK_WAIT;				// シリアル送受信モード　→　Ack/Nack応答待ち
	ACK_Wait_timer = ACK_WAIT_TIME;				// Ack待ちタイムアウト時間Set

	usSioRcvCount = 0;
	memset( cSioRcvBuf, 0, RCV_BUF_SIZE );
	
}

//20160930Miya PCからVA300Sを制御する
/*==========================================================================*/
// VA300s制御Boxへコマンド(OK/NG)を送信する。
/*==========================================================================*/
static void send_sio_STANDARD( void )
{
	int i;
	UB str1;
	int cnt;
	int dat, datlen;

	/** コマンドデータの準備	**/
	cnt = 0;
	send_sio_buff[ cnt++ ] = '*';			// ヘッダ　1Byte　ASCII '*'
	send_sio_buff[ cnt++ ] = 1;				// 端末番号　1バイトBinary
	send_sio_buff[ cnt++ ] = 0;				// データ長　２バイトBinary
	send_sio_buff[ cnt++ ] = 0;
	send_sio_buff[ cnt++ ] = 0x00;			// ブロック番号 ２バイトバイナリ
	send_sio_buff[ cnt++ ] = 0x00;			
	send_sio_buff[ cnt++ ] = cSioRcvBuf[6];			// コマンド種別 ２桁ASCII
	send_sio_buff[ cnt++ ] = cSioRcvBuf[7];

	send_sio_buff[ cnt++ ] = 'O';
	send_sio_buff[ cnt++ ] = 'K';

	sio_snd_comm_len = cnt;					// コマンド長
	send_sio_buff[ 3 ] = sio_snd_comm_len;
		
	// チェックSUMの準備。
	str1 = 0;
	for ( i = 0; i < sio_snd_comm_len; i++ ){	// コマンドのバイト分の総和を求める
		str1 = str1 + send_sio_buff[ i ];
	}
	send_sio_buff[ cnt++ ] = str1 ^ 0xff;	// チェックサムを反転
	
	send_sio_buff[ cnt++ ] = CODE_CR;		//　終端コード
	send_sio_buff[ cnt++ ] = CODE_LF;
	
	/** コマンド送信	**/
	SendSerialBinaryData( (UB*)send_sio_buff, ( sio_snd_comm_len + 3 ) );

	usSioRcvCount = 0;
	memset( cSioRcvBuf, 0, RCV_BUF_SIZE );
}
/*==========================================================================*/
// VA300s制御Boxへコマンド(A0)を送信する。
/*==========================================================================*/
static void send_sio_TANMATU_INFO( void )
{
	int i;
	UB str1;
	int cnt;
	int dat, datlen;
	int keta1, keta2, keta3, keta4;

	/** コマンドデータの準備	**/
	cnt = 0;
	send_sio_buff[ cnt++ ] = '*';			// ヘッダ　1Byte　ASCII '*'
	send_sio_buff[ cnt++ ] = 1;				// 端末番号　1バイトBinary
	send_sio_buff[ cnt++ ] = 0;				// データ長　２バイトBinary
	send_sio_buff[ cnt++ ] = 0;
	send_sio_buff[ cnt++ ] = 0x00;			// ブロック番号 ２バイトバイナリ
	send_sio_buff[ cnt++ ] = 0x00;			
	send_sio_buff[ cnt++ ] = 'A';			// コマンド種別 ２桁ASCII
	send_sio_buff[ cnt++ ] = '0';

	//端末番号
	send_sio_buff[ cnt++ ] = '0';
	send_sio_buff[ cnt++ ] = '1';
	//端末識別ID
	dat = g_RegUserInfoData.UserId;
	keta4 = dat / 1000;
	dat = dat - 1000 * keta4;
	keta3 = dat / 100;
	dat = dat - 100 * keta3;
	keta2 = dat / 10;
	keta1 = dat % 10;
	send_sio_buff[ cnt++ ] = (unsigned char)keta4;
	send_sio_buff[ cnt++ ] = (unsigned char)keta3;
	send_sio_buff[ cnt++ ] = (unsigned char)keta2;
	send_sio_buff[ cnt++ ] = (unsigned char)keta1;
	//端末Ver
	send_sio_buff[ cnt++ ] = Senser_soft_VER[0];
	send_sio_buff[ cnt++ ] = Senser_soft_VER[2];
	send_sio_buff[ cnt++ ] = Senser_soft_VER[3];
	send_sio_buff[ cnt++ ] = Senser_soft_VER[4];
	//FPGA Ver
	dat = FpgaVerNum & 0xF000;
	dat = dat >> 12;
	send_sio_buff[ cnt++ ] = (UB)dat;
	dat = FpgaVerNum & 0x0F00;
	dat = dat >> 8;
	send_sio_buff[ cnt++ ] = (UB)dat;
	dat = FpgaVerNum & 0x00F0;
	dat = dat >> 4;
	send_sio_buff[ cnt++ ] = (UB)dat;
	send_sio_buff[ cnt++ ] = (UB)dat;
	dat = FpgaVerNum & 0x000F;
	//シリアル番号
	for(i = 0 ; i < 16 ; i++){ send_sio_buff[ cnt++ ] = i + 0x30; }

	sio_snd_comm_len = cnt;					// コマンド長
	send_sio_buff[ 3 ] = sio_snd_comm_len;
		
	// チェックSUMの準備。
	str1 = 0;
	for ( i = 0; i < sio_snd_comm_len; i++ ){	// コマンドのバイト分の総和を求める
		str1 = str1 + send_sio_buff[ i ];
	}
	send_sio_buff[ cnt++ ] = str1 ^ 0xff;	// チェックサムを反転
	
	send_sio_buff[ cnt++ ] = CODE_CR;		//　終端コード
	send_sio_buff[ cnt++ ] = CODE_LF;
	
	/** コマンド送信	**/
	SendSerialBinaryData( (UB*)send_sio_buff, ( sio_snd_comm_len + 3 ) );

	usSioRcvCount = 0;
	memset( cSioRcvBuf, 0, RCV_BUF_SIZE );
}
/*==========================================================================*/
// VA300s制御Boxへコマンド(A4)を送信する。
/*==========================================================================*/
static void send_sio_REGDATA_UPLD_STA( void )
{
	int i;
	UB str1;
	int cnt;
	int dat, datlen;

	/** コマンドデータの準備	**/
	cnt = 0;
	send_sio_buff[ cnt++ ] = '*';			// ヘッダ　1Byte　ASCII '*'
	send_sio_buff[ cnt++ ] = 1;				// 端末番号　1バイトBinary
	send_sio_buff[ cnt++ ] = 0;				// データ長　２バイトBinary
	send_sio_buff[ cnt++ ] = 0;
	send_sio_buff[ cnt++ ] = 0x00;			// ブロック番号 ２バイトバイナリ
	send_sio_buff[ cnt++ ] = 0x00;			
	send_sio_buff[ cnt++ ] = 'A';			// コマンド種別 ２桁ASCII
	send_sio_buff[ cnt++ ] = '4';

	datlen = 2 * 80 * 40 + 20 * 10;
	dat = (datlen & 0xFF00) >> 8;
	send_sio_buff[ cnt++ ] = (unsigned char)dat;
	dat = datlen & 0xFF;
	send_sio_buff[ cnt++ ] = (unsigned char)dat;

	sio_snd_comm_len = cnt;					// コマンド長
	send_sio_buff[ 3 ] = sio_snd_comm_len;
		
	// チェックSUMの準備。
	str1 = 0;
	for ( i = 0; i < sio_snd_comm_len; i++ ){	// コマンドのバイト分の総和を求める
		str1 = str1 + send_sio_buff[ i ];
	}
	send_sio_buff[ cnt++ ] = str1 ^ 0xff;	// チェックサムを反転
	
	send_sio_buff[ cnt++ ] = CODE_CR;		//　終端コード
	send_sio_buff[ cnt++ ] = CODE_LF;
	
	/** コマンド送信	**/
	SendSerialBinaryData( (UB*)send_sio_buff, ( sio_snd_comm_len + 3 ) );

	usSioRcvCount = 0;
	memset( cSioRcvBuf, 0, RCV_BUF_SIZE );
	
	//アップロード用の画像準備
	memcpy(&IfImageBuf[0], &RegImgBuf[0][0][0][0], 80 * 40);
	memcpy(&IfImageBuf[80*40], &RegImgBuf[0][0][1][0], 80 * 40);
	memcpy(&IfImageBuf[2*80*40], &g_RegBloodVesselTagData[0].MinAuthImg[0][0], 20 * 10);
}
/*==========================================================================*/
// VA300s制御Boxへコマンド(A5)を送信する。
/*==========================================================================*/
static void send_sio_REGDATA_UPLD_GET( void )
{
	int i;
	UB str1;
	int cnt;
	int dat, datlen, st, sndsz;

	/** コマンドデータの準備	**/
	cnt = 0;
	send_sio_buff[ cnt++ ] = '*';			// ヘッダ　1Byte　ASCII '*'
	send_sio_buff[ cnt++ ] = 1;				// 端末番号　1バイトBinary
	send_sio_buff[ cnt++ ] = 0;				// データ長　２バイトBinary
	send_sio_buff[ cnt++ ] = 0;

	dat = (usSioBunkatuNum & 0xff00) >> 8;
	send_sio_buff[ cnt++ ] = dat;			// ブロック番号 ２バイトバイナリ
	dat = usSioBunkatuNum & 0xff;
	send_sio_buff[ cnt++ ] = 0x00;			

	send_sio_buff[ cnt++ ] = 'A';			// コマンド種別 ２桁ASCII
	send_sio_buff[ cnt++ ] = '5';

	datlen = 2 * 80 * 40 + 20 * 10;
	
	st = 1000 * usSioBunkatuNum;
	if(st + 1000 > datlen){
		sndsz = datlen - st;
	}else{
		sndsz = 1000;
	}

	memcpy(&send_sio_buff[cnt], &IfImageBuf[st], sndsz);
	cnt += sndsz;

	sio_snd_comm_len = cnt;					// コマンド長
	send_sio_buff[ 2 ] = (unsigned char)((sio_snd_comm_len & 0xFF00) >> 8);
	send_sio_buff[ 3 ] = (unsigned char)(sio_snd_comm_len & 0xFF);
		
	// チェックSUMの準備。
	str1 = 0;
	for ( i = 0; i < sio_snd_comm_len; i++ ){	// コマンドのバイト分の総和を求める
		str1 = str1 + send_sio_buff[ i ];
	}
	send_sio_buff[ cnt++ ] = str1 ^ 0xff;	// チェックサムを反転
	
	send_sio_buff[ cnt++ ] = CODE_CR;		//　終端コード
	send_sio_buff[ cnt++ ] = CODE_LF;
	
	/** コマンド送信	**/
	SendSerialBinaryData( (UB*)send_sio_buff, ( sio_snd_comm_len + 3 ) );

	usSioRcvCount = 0;
	memset( cSioRcvBuf, 0, RCV_BUF_SIZE );

}
/*==========================================================================*/
// VA300s制御Boxへコマンド(A7)を送信する。
/*==========================================================================*/
static void send_sio_REGPROC( int rslt )
{
	int i;
	UB str1;
	int cnt;
	int dat, datlen;

	memset(send_sio_buff, 0, SND_SIO_BUF_SIZE);

	/** コマンドデータの準備	**/
	cnt = 0;
	send_sio_buff[ cnt++ ] = '*';			// ヘッダ　1Byte　ASCII '*'
	send_sio_buff[ cnt++ ] = 1;				// 端末番号　1バイトBinary
	send_sio_buff[ cnt++ ] = 0;				// データ長　２バイトBinary
	send_sio_buff[ cnt++ ] = 0;
	send_sio_buff[ cnt++ ] = 0x00;			// ブロック番号 ２バイトバイナリ
	send_sio_buff[ cnt++ ] = 0x00;			
	send_sio_buff[ cnt++ ] = 'A';			// コマンド種別 ２桁ASCII
	send_sio_buff[ cnt++ ] = '7';


	if(rslt == 0){
		send_sio_buff[ cnt++ ] = 'O';
		send_sio_buff[ cnt++ ] = 'K';
	}else{
		send_sio_buff[ cnt++ ] = 'N';
		send_sio_buff[ cnt++ ] = 'G';
	}

	dat = g_RegBloodVesselTagData[0].RegFinger / 10;
	send_sio_buff[ cnt++ ] = (unsigned char)dat;
	dat = g_RegBloodVesselTagData[0].RegFinger % 10;
	send_sio_buff[ cnt++ ] = (unsigned char)dat;

	sio_snd_comm_len = cnt;					// コマンド長
	send_sio_buff[ 3 ] = sio_snd_comm_len;
		
	// チェックSUMの準備。
	str1 = 0;
	for ( i = 0; i < sio_snd_comm_len; i++ ){	// コマンドのバイト分の総和を求める
		str1 = str1 + send_sio_buff[ i ];
	}
	send_sio_buff[ cnt++ ] = str1 ^ 0xff;	// チェックサムを反転
	
	send_sio_buff[ cnt++ ] = CODE_CR;		//　終端コード
	send_sio_buff[ cnt++ ] = CODE_LF;
	
	/** コマンド送信	**/
	SendSerialBinaryData( (UB*)send_sio_buff, ( sio_snd_comm_len + 3 ) );

	usSioRcvCount = 0;
	memset( cSioRcvBuf, 0, RCV_BUF_SIZE );
	
	g_capallow = 0;		//撮影許可禁止
}
/*==========================================================================*/
// VA300s制御Boxへコマンド(A8)を送信する。
/*==========================================================================*/
static void send_sio_AUTHPROC( int rslt )
{
	int i;
	UB str1;
	int cnt;
	int dat, datlen;

	/** コマンドデータの準備	**/
	cnt = 0;
	send_sio_buff[ cnt++ ] = '*';			// ヘッダ　1Byte　ASCII '*'
	send_sio_buff[ cnt++ ] = 1;				// 端末番号　1バイトBinary
	send_sio_buff[ cnt++ ] = 0;				// データ長　２バイトBinary
	send_sio_buff[ cnt++ ] = 0;
	send_sio_buff[ cnt++ ] = 0x00;			// ブロック番号 ２バイトバイナリ
	send_sio_buff[ cnt++ ] = 0x00;			
	send_sio_buff[ cnt++ ] = 'A';			// コマンド種別 ２桁ASCII
	send_sio_buff[ cnt++ ] = '8';


	if(rslt == 0){
		send_sio_buff[ cnt++ ] = 'O';
		send_sio_buff[ cnt++ ] = 'K';
	}else{
		send_sio_buff[ cnt++ ] = 'N';
		send_sio_buff[ cnt++ ] = 'G';
	}

	send_sio_buff[ cnt++ ] = cSioRcvBuf[14];
	send_sio_buff[ cnt++ ] = cSioRcvBuf[15];

	send_sio_buff[ cnt++ ] = cSioRcvBuf[16];
	send_sio_buff[ cnt++ ] = cSioRcvBuf[17];
	send_sio_buff[ cnt++ ] = cSioRcvBuf[18];
	send_sio_buff[ cnt++ ] = cSioRcvBuf[19];
	send_sio_buff[ cnt++ ] = cSioRcvBuf[20];
	send_sio_buff[ cnt++ ] = cSioRcvBuf[21];
	send_sio_buff[ cnt++ ] = cSioRcvBuf[22];
	send_sio_buff[ cnt++ ] = cSioRcvBuf[23];

	send_sio_buff[ cnt++ ] = cSioRcvBuf[24];
	send_sio_buff[ cnt++ ] = cSioRcvBuf[25];

	send_sio_buff[ cnt++ ] = cSioRcvBuf[26];
	send_sio_buff[ cnt++ ] = cSioRcvBuf[27];


	sio_snd_comm_len = cnt;					// コマンド長
	send_sio_buff[ 3 ] = sio_snd_comm_len;
		
	// チェックSUMの準備。
	str1 = 0;
	for ( i = 0; i < sio_snd_comm_len; i++ ){	// コマンドのバイト分の総和を求める
		str1 = str1 + send_sio_buff[ i ];
	}
	send_sio_buff[ cnt++ ] = str1 ^ 0xff;	// チェックサムを反転
	
	send_sio_buff[ cnt++ ] = CODE_CR;		//　終端コード
	send_sio_buff[ cnt++ ] = CODE_LF;
	
	/** コマンド送信	**/
	SendSerialBinaryData( (UB*)send_sio_buff, ( sio_snd_comm_len + 3 ) );

	usSioRcvCount = 0;
	memset( cSioRcvBuf, 0, RCV_BUF_SIZE );
	
	g_capallow = 0;		//撮影許可禁止
}
