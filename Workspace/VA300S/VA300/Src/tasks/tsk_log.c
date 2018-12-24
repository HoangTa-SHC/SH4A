/**
*	VA-300s�v���O����
*
*	@file tsk_log.c
*	@version 1.00
*
*	@author Bionics Co.Ltd T.Nagai
*	@date   2013/12/20
*	@brief  VA-300s ���M���O�������C���^�X�N
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
void SendLogData( char *data, int cnt );	// �f�[�^�̃^�X�N�ԑ��M�i�ėp�E���M���O������p�j

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
		ercd = rcv_mbx(MBX_LOG_DATA, &msg);	// ���M���O�����p�f�[�^�P�����̎�M�҂�
		if ( ercd == E_OK ){

			// ���M���O����
			nop();
			
			
			/* Release memory block */
			rel_mpf(MPF_LOG_DATA, msg);

			
    	} else {							/* �R�[�f�B���O�G���[ */
	    	ErrCodeSet( ercd );
    	}
    }
}



//=============================================================================
/**
 *	�f�[�^�̃^�X�N�ԑ��M�i�ėp�E���M���O������p�j
 *	@param	data�@���M�f�[�^
 *	@param	cnt�@���M�o�C�g��
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