/**
	VA-300プログラム
	@file tsk_lrcv.c
	@version 1.00
	
	@author OYO Electric Co.Ltd
	@date 2012/08/02
	@brief UDPの送受信タスク
*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <kernel.h>
#include "sh7750.h"
#include "nonet.h"
#include "id.h"
#include "err_ctrl.h"
#include "udp.h"
#include "command.h"

#include "va300.h"

//ファイル内変数
	//受信タスク/受信中
static int Fstatus;			// 状態変数
static UINT Fcnt;			// 受信データ格納位置
static UINT	cmd_len;		// 受信コマンド長
//static UINT s_uiCntLen;		// データ長の受信バイト数

//ローカル関数
static void CmmChangeStatus( int newStatus );

static 	ID	rxtid_org;		// GET_UDPの為のタスクID　
/**
//通信の状態
enum EmfStatus {
	ST_COM_STANDBY,		// 待機(0)
	ST_COM_RECEIVE,		// 受信中
		//以下は受信中の中の分類
	ST_COM_WAIT_LEN,	// データ長待ち
	ST_COM_WAIT_DATA,	// データ待ち
	ST_COM_WAIT_CR,		// CR待ち
	ST_COM_WAIT_LF,		// LF待ち
	ST_COM_WAIT_EOT,	// EOT待ち
	ST_COM_WAIT_ACK_NAK,	// ACK/NAK待ち
	
	ST_COM_SLEEP,		// 受信停止
	
	ST_COM_MAX			// 状態数
};
**/
// プロトタイプ宣言
static void UdpRcv_Process( T_COMMSG *msg );
static UB CAPGetResult( void );

/*****************************************************/
/*****************************************************
 *
 *	受信タスク
 *	
 *		
 *****************************************************/
/*****************************************************/
TASK UdpRcvTask( INT ch )
{
    T_COMMSG *msg;
	ER err_code;		//受信関数からのエラーコード
	
	/* initialize */
	rxtid_org = vget_tid();						// GET_UDP関数の為に、オリジナルのタスクIDを保存。
	
	CmmChangeStatus( ST_COM_STANDBY );			// 受信状態=待機中に移行

	for ( ;; ){

		err_code = get_mpf( MPF_COM, ( VP* )&msg );	// メモリープール獲得
		if ( err_code != E_OK) {
			ErrCodeSet( err_code );
			for( ;; )
				slp_tsk();
		}
		
		memset( msg, 0, sizeof ( T_COMMSG ) );

		UdpRcv_Process( msg );						// 受信タスク処理　本体
				
		CmmChangeStatus( ST_COM_STANDBY );			// 受信状態=待機中に移行

	}
}


/*=========================================================
    受信タスク処理　本体
==========================================================*/
static void UdpRcv_Process( T_COMMSG *msg )
{
	char	code;
	ER		ercd;		// 受信関数からのエラーコード
	UINT	tmp_len1, tmp_len2;
	UINT	msg_size;
	T_COMMSG rcv_data;
	
//	msg = &rcv_data;
//	rcv_data = *( ( T_COMMSG * )msg );
	
	for(;;) {
		// 受信処理
		if ( Fstatus == ST_COM_STANDBY ){
			get_udp( (UB*)&code, TMO_FEVR );				// 受信状態=受信データ待ち
			nop();
		} else {
			ercd = get_udp( ( UB* )&code, ( 1000/MSEC ) );	// 受信状態=コマンド・レングス待ち
			if ( ercd == E_OK ){
				if ( Fstatus == ST_COM_WAIT_LEN ){
					msg->buf[ Fcnt++ ] = code;
//					rcv_data.buf[ Fcnt++ ] = code;
					
				}	else if ( ( Fstatus == ST_COM_WAIT_DATA ) && ( Fcnt <= cmd_len + 1 ) ){
					msg->buf[ Fcnt++ ] = code;
//					rcv_data.buf[ Fcnt++ ] = code;
					
				}	else if ( Fstatus == ST_COM_WAIT_EOT ){
					msg->buf[ Fcnt++ ] = code;
//					rcv_data.buf[ Fcnt++ ] = code;
					
				}	else if ( Fstatus == ST_COM_WAIT_ACK_NAK ){
					msg->buf[ Fcnt++ ] = code;
//					rcv_data.buf[ Fcnt++ ] = code;
				}
					
			} else {	// タイムアウト?
				CmmSendNak();
				CmmChangeStatus( ST_COM_STANDBY );
				
				// 追加　2013.7.12				  
				ercd = rel_mpf( MPF_COM, ( VP )msg );	// 処理後はメモリ解放
				if ( ercd != E_OK) {
					ErrCodeSet( ercd );
				}
				return;
				// 追加END

				// continue;
			}
		}
			
		//状態ごとの処理
		switch ( Fstatus ) {
		case ST_COM_STANDBY:	// 待機中（ヘッダー受信のチェック）
		
			if ( ( code == '#' )	//　送信手順A
			  || ( code == '$' )	//　送信手順B
			  || ( code == '%' )	//　送信手順C
			  || ( code == '&' )	//　送信手順D
			  || ( code == 0x27 ) ){ //　送信手順E
				msg->buf[ Fcnt++ ] = code;				// コマンド・ヘッダーを格納
//				rcv_data.buf[ Fcnt++ ] = code;
				CmmChangeStatus( ST_COM_WAIT_LEN );		// 受信状態=コマンド・レングス待ち　に移行
			}
			break;
				
		case ST_COM_WAIT_LEN:							// 受信状態=コマンド・レングス待ち
			if ( Fcnt == 3 ){
				tmp_len1 = msg->buf[ 1 ];				// コマンド長を求める
//				tmp_len1 = rcv_data.buf[ 1 ];
				tmp_len1 = tmp_len1 << 8;
				tmp_len2 = msg->buf[ 2 ];
//				tmp_len2 = rcv_data.buf[ 2 ];
				cmd_len = tmp_len1 + tmp_len2; 
				
				CmmChangeStatus( ST_COM_WAIT_DATA );	// 受信状態=コマンドデータ待ち　に移行
			} 
			break;
					
		case ST_COM_WAIT_DATA:							// 受信状態=コマンドデータ待ち
			if ( Fcnt >= cmd_len ){						// 受信バイト長のチェック
				CmmChangeStatus( ST_COM_WAIT_CR );		// 受信状態=CR待ち　に移行
			}
			break;
					
		case ST_COM_WAIT_CR:							// 受信状態=CR待ち
			if ( code == CODE_CR ){
				CmmChangeStatus( ST_COM_WAIT_LF );		// 受信状態=LF待ち　に移行
			} else	{
				CmmSendNak();							// Nack送信
				CmmChangeStatus( ST_COM_STANDBY );		// 受信状態=受信データ待ちへ戻る(受信電文を破棄)				
				// 追加　2013.7.12				  
				 ercd = rel_mpf( MPF_COM, ( VP )msg );	// 処理後はメモリ解放
				  if ( ercd != E_OK) {
					ErrCodeSet( ercd );
				 }
				// 追加END
			}
			break;
					
		case ST_COM_WAIT_LF:							// 受信状態=LF待ち
			if ( code == CODE_LF ){
				CmmSendAck();							// 受信成功、Ack送信	
											
				ercd = snd_mbx( MBX_CMD_LAN, (VP)msg );	// 受信データを、コマンド処理タスクへ送る
				if ( ( ercd != E_OK ) /* && ( ercd != E_TMOUT ) */ ){
					PrgErrSet();
				}
				
				if ( MdGetMode() == MD_CAP ){			// キャプチャー処理中なら
					CmmChangeStatus( ST_COM_WAIT_EOT );	// 受信状態=EOT待ち
				} else {
					// 追加　2013.7.12				  
				 	ercd = rel_mpf( MPF_COM, ( VP )msg );	// 処理後はメモリ解放
				  	if ( ercd != E_OK) {
						ErrCodeSet( ercd );
				 	}
					// 追加END
					
					return;
				}

			} else	{
				CmmSendNak();							// Nack送信
				CmmChangeStatus( ST_COM_STANDBY );		// 受信状態=受信データ待ちへ戻る(受信電文を破棄)
				// 追加　2013.7.12				  
				ercd = rel_mpf( MPF_COM, ( VP )msg );	// 処理後はメモリ解放
				if ( ercd != E_OK) {
					ErrCodeSet( ercd );
				}
				return;
				// 追加END				
			}
			break;
			
		case ST_COM_WAIT_EOT:							// 受信状態=EOT待ち
			if ( Fcnt >= 3 ){							// 受信バイト長のチェック
				if ( ( msg->buf[ 0 ] == CODE_EOT )  
				  && ( msg->buf[ 1 ] == CODE_CR ) 
				  && ( msg->buf[ 2 ] == CODE_LF ) ){
//			if ( Fcnt >= 3 ){							// 受信バイト長のチェック
//				if ( ( rcv_data.buf[ 0 ] == CODE_EOT )  
//				  && ( rcv_data.buf[ 1 ] == CODE_CR ) 
//				  && ( rcv_data.buf[ 2 ] == CODE_LF ) ){
					 
					ercd = rel_mpf( MPF_COM, ( VP )msg );	// 処理後はメモリ解放
					if ( ercd != E_OK) {
						ErrCodeSet( ercd );
					}
					 return; 
				  } else {
				  
					CmmSendNak();							// Nack送信
					ercd = rel_mpf( MPF_COM, ( VP )msg );	// 処理後はメモリ解放
					if ( ercd != E_OK) {
						ErrCodeSet( ercd );
				  	}
					 
				    return;
				  }
			}
			break;
			
		case ST_COM_WAIT_ACK_NAK:
			if ( Fcnt >= 3 ){							// 受信バイト長のチェック
				if ( ( msg->buf[ 0 ] == CODE_ACK )  
				  && ( msg->buf[ 1 ] == CODE_CR ) 
				  && ( msg->buf[ 2 ] == CODE_LF ) ){
//			if ( Fcnt >= 3 ){							// 受信バイト長のチェック
//				if ( ( rcv_data.buf[ 0 ] == CODE_ACK )  
//				  && ( rcv_data.buf[ 1 ] == CODE_CR ) 
//				  && ( rcv_data.buf[ 2 ] == CODE_LF ) ){

					 rcv_ack_nak = 1;
					  
				 } else if  ( ( msg->buf[ 0 ] == CODE_NACK )  
				  		    && ( msg->buf[ 1 ] == CODE_CR ) 
				  			&& ( msg->buf[ 2 ] == CODE_LF ) ){
//				  } else if  ( ( rcv_data.buf[ 0 ] == CODE_NACK )  
//				  		    && ( rcv_data.buf[ 1 ] == CODE_CR ) 
//				  			&& ( rcv_data.buf[ 2 ] == CODE_LF ) ){
					 
					 rcv_ack_nak = -1;

				 }	else {

					CmmSendNak();							// Nack送信

				 }
				  
				 ercd = rel_mpf( MPF_COM, ( VP )msg );	// 処理後はメモリ解放
				  if ( ercd != E_OK) {
						ErrCodeSet( ercd );
				 }
					 
				 return;
			}
			break;
			
		default:
			break;
		}	
	}
}

/*=========================================================
	受信タスク内の受信中の状態遷移処理
	・現在の状態はFstatus変数が保持
	[引数]
		newStatus	次の状態
===========================================================*/
static void CmmChangeStatus(int newStatus)
{
	//退場処理
	
	//入場処理
	switch( newStatus ) {
	case ST_COM_STANDBY:		// 受信開始
		Fcnt = 0;
		cmd_len = 0;
		break;
	case ST_COM_WAIT_EOT:		// ETX待ち=受信開始
		Fcnt = 0;
		cmd_len = 0;
		break;
	case ST_COM_WAIT_LEN:		// ETX待ち=受信開始
//		s_uiCntLen = 0;
		break;
	case ST_COM_WAIT_ACK_NAK:	// ACK/NAK待ち=受信開始
		Fcnt = 0;
		rcv_ack_nak = 0;
		break;
	}
	
	Fstatus = newStatus;//次の状態保持
}

/**********************************************************/
/**********************************************************
 *
 * UDP Send Task
 *
 * @param ch チャンネル番号(互換性のため)
 *
 **********************************************************/
 /*********************************************************/
TASK UdpSndTask( INT ch )
{
	T_COMMSG *msg;
	ER	ercd;

	for (;;)
	{
		/* Wait message */
		ercd = rcv_mbx( MBX_RESSND, &msg );

		if( ercd == E_OK ) {
	        /* Send 1 line */
			put_udp( (UB*)msg->buf, (UH)msg->cnt );

			/* Release memory block */
			rel_mpf( MPF_LRES, msg );

		} else {							/* コーディングエラー */
			ErrCodeSet( ercd );
		}
	}
}

//=============================================================================
/**
 *	データの送信（汎用）
 *	@param	data　送信データ
 *	@param	cnt　送信バイト数
 */
//=============================================================================
void SendBinaryData( char *data, int cnt )
{
	T_COMMSG *msg;
	ER		ercd;
	
	ercd = tget_mpf( MPF_LRES, ( VP* )&msg, ( 10/MSEC ) );
	if( ercd == E_OK ) {		
		memset( &(msg->buf), 0, sizeof( msg->buf ) );
	
		msg->cnt    = cnt;
		msg->msgpri = 1;
		memcpy( &( msg->buf ), data, cnt );
	
		snd_mbx( MBX_RESSND, ( T_MSG* )msg );
	}
	
}
//=============================================================================
/**	
 *	NAK送信
 *	・NACK＋CR LF
 *	・メモリブロックの解放
 */
//=============================================================================
void CmmSendNak( void )
{
	char buff[ 5 ];
	
	buff[ 0 ] = CODE_NACK;
	buff[ 1 ] = CODE_CR;
	buff[ 2 ] = CODE_LF;
	
	SendBinaryData( buff, 5 );
}

//=============================================================================
/**
 *	ACK送信
 *	・ACK＋CR LF
 */
//=============================================================================
void CmmSendAck( void )
{
	char buff[ 5 ];
	buff[ 0 ] = CODE_ACK;
	buff[ 1 ] = CODE_CR;
	buff[ 2 ] = CODE_LF;
	
	SendBinaryData(buff, 3);
}

//=============================================================================
/**
 *	EOT送信
 *	・EOT＋CR LF
 */
//=============================================================================
void CmmSendEot( void )
{
	char buff[ 5 ];
	buff[ 0 ] = CODE_EOT;
	buff[ 1 ] = CODE_CR;
	buff[ 2 ] = CODE_LF;
	
	SendBinaryData( buff, 3 );
}


/*==========================================================================*/
/**
 *	最新の指認証結果を返す（指認証結果参照）
 */
/*==========================================================================*/
static UB CAPGetResult(void)
{
	return s_CapResult;
}
