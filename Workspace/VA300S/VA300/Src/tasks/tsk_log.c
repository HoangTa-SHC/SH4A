/**
*	VA-300sプログラム
*
*	@file tsk_log.c
*	@version 1.00
*
*	@author Bionics Co.Ltd T.Nagai
*	@date   2013/12/20
*	@brief  VA-300s ロギング処理メインタスク
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

#if ( VA300S == 1 || VA300S == 2 )	

TASK LogTask( void );
void SendLogData( char *data, int cnt );	// データのタスク間送信（汎用・ロギング処理専用）

/*******************************
 * Logging Task
 *
 * @param  NON
 *******************************/
TASK LogTask( viod )
{
	ER	ercd;
	T_COMMSG *msg;

	for (;;)
	{
		ercd = rcv_mbx(MBX_LOG_DATA, &msg);	// ロギング処理用データ１件分の受信待ち
		if ( ercd == E_OK ){

			// ロギング処理
			nop();
			
			
			/* Release memory block */
			rel_mpf(MPF_LOG_DATA, msg);

			
    	} else {							/* コーディングエラー */
	    	ErrCodeSet( ercd );
    	}
    }
}



//=============================================================================
/**
 *	データのタスク間送信（汎用・ロギング処理専用）
 *	@param	data　送信データ
 *	@param	cnt　送信バイト数
 */
//=============================================================================
void SendLogData( char *data, int cnt )
{
	T_COMMSG *msg;
	ER		ercd;
	
	ercd = tget_mpf( MPF_LOG_DATA, ( VP* )&msg, ( 10/MSEC ) );
	if( ercd == E_OK ) {		
		memset( &(msg->buf), 0, sizeof( msg->buf ) );
	
		msg->cnt    = cnt;
		msg->msgpri = 1;
		memcpy( &( msg->buf ), data, cnt );
	
		snd_mbx( MBX_LOG_DATA, ( T_MSG* )msg );
	}
	
}

#endif