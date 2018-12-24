/**
	VA-300�v���O����
	@file tsk_lrcv.c
	@version 1.00
	
	@author OYO Electric Co.Ltd
	@date 2012/08/02
	@brief UDP�̑���M�^�X�N
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

//�t�@�C�����ϐ�
	//��M�^�X�N/��M��
static int Fstatus;			// ��ԕϐ�
static UINT Fcnt;			// ��M�f�[�^�i�[�ʒu
static UINT	cmd_len;		// ��M�R�}���h��
//static UINT s_uiCntLen;		// �f�[�^���̎�M�o�C�g��

//���[�J���֐�
static void CmmChangeStatus( int newStatus );

static 	ID	rxtid_org;		// GET_UDP�ׂ̈̃^�X�NID�@
/**
//�ʐM�̏��
enum EmfStatus {
	ST_COM_STANDBY,		// �ҋ@(0)
	ST_COM_RECEIVE,		// ��M��
		//�ȉ��͎�M���̒��̕���
	ST_COM_WAIT_LEN,	// �f�[�^���҂�
	ST_COM_WAIT_DATA,	// �f�[�^�҂�
	ST_COM_WAIT_CR,		// CR�҂�
	ST_COM_WAIT_LF,		// LF�҂�
	ST_COM_WAIT_EOT,	// EOT�҂�
	ST_COM_WAIT_ACK_NAK,	// ACK/NAK�҂�
	
	ST_COM_SLEEP,		// ��M��~
	
	ST_COM_MAX			// ��Ԑ�
};
**/
// �v���g�^�C�v�錾
static void UdpRcv_Process( T_COMMSG *msg );
static UB CAPGetResult( void );

/*****************************************************/
/*****************************************************
 *
 *	��M�^�X�N
 *	
 *		
 *****************************************************/
/*****************************************************/
TASK UdpRcvTask( INT ch )
{
    T_COMMSG *msg;
	ER err_code;		//��M�֐�����̃G���[�R�[�h
	
	/* initialize */
	rxtid_org = vget_tid();						// GET_UDP�֐��ׂ̈ɁA�I���W�i���̃^�X�NID��ۑ��B
	
	CmmChangeStatus( ST_COM_STANDBY );			// ��M���=�ҋ@���Ɉڍs

	for ( ;; ){

		err_code = get_mpf( MPF_COM, ( VP* )&msg );	// �������[�v�[���l��
		if ( err_code != E_OK) {
			ErrCodeSet( err_code );
			for( ;; )
				slp_tsk();
		}
		
		memset( msg, 0, sizeof ( T_COMMSG ) );

		UdpRcv_Process( msg );						// ��M�^�X�N�����@�{��
				
		CmmChangeStatus( ST_COM_STANDBY );			// ��M���=�ҋ@���Ɉڍs

	}
}


/*=========================================================
    ��M�^�X�N�����@�{��
==========================================================*/
static void UdpRcv_Process( T_COMMSG *msg )
{
	char	code;
	ER		ercd;		// ��M�֐�����̃G���[�R�[�h
	UINT	tmp_len1, tmp_len2;
	UINT	msg_size;
	T_COMMSG rcv_data;
	
//	msg = &rcv_data;
//	rcv_data = *( ( T_COMMSG * )msg );
	
	for(;;) {
		// ��M����
		if ( Fstatus == ST_COM_STANDBY ){
			get_udp( (UB*)&code, TMO_FEVR );				// ��M���=��M�f�[�^�҂�
			nop();
		} else {
			ercd = get_udp( ( UB* )&code, ( 1000/MSEC ) );	// ��M���=�R�}���h�E�����O�X�҂�
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
					
			} else {	// �^�C���A�E�g?
				CmmSendNak();
				CmmChangeStatus( ST_COM_STANDBY );
				
				// �ǉ��@2013.7.12				  
				ercd = rel_mpf( MPF_COM, ( VP )msg );	// ������̓��������
				if ( ercd != E_OK) {
					ErrCodeSet( ercd );
				}
				return;
				// �ǉ�END

				// continue;
			}
		}
			
		//��Ԃ��Ƃ̏���
		switch ( Fstatus ) {
		case ST_COM_STANDBY:	// �ҋ@���i�w�b�_�[��M�̃`�F�b�N�j
		
			if ( ( code == '#' )	//�@���M�菇A
			  || ( code == '$' )	//�@���M�菇B
			  || ( code == '%' )	//�@���M�菇C
			  || ( code == '&' )	//�@���M�菇D
			  || ( code == 0x27 ) ){ //�@���M�菇E
				msg->buf[ Fcnt++ ] = code;				// �R�}���h�E�w�b�_�[���i�[
//				rcv_data.buf[ Fcnt++ ] = code;
				CmmChangeStatus( ST_COM_WAIT_LEN );		// ��M���=�R�}���h�E�����O�X�҂��@�Ɉڍs
			}
			break;
				
		case ST_COM_WAIT_LEN:							// ��M���=�R�}���h�E�����O�X�҂�
			if ( Fcnt == 3 ){
				tmp_len1 = msg->buf[ 1 ];				// �R�}���h�������߂�
//				tmp_len1 = rcv_data.buf[ 1 ];
				tmp_len1 = tmp_len1 << 8;
				tmp_len2 = msg->buf[ 2 ];
//				tmp_len2 = rcv_data.buf[ 2 ];
				cmd_len = tmp_len1 + tmp_len2; 
				
				CmmChangeStatus( ST_COM_WAIT_DATA );	// ��M���=�R�}���h�f�[�^�҂��@�Ɉڍs
			} 
			break;
					
		case ST_COM_WAIT_DATA:							// ��M���=�R�}���h�f�[�^�҂�
			if ( Fcnt >= cmd_len ){						// ��M�o�C�g���̃`�F�b�N
				CmmChangeStatus( ST_COM_WAIT_CR );		// ��M���=CR�҂��@�Ɉڍs
			}
			break;
					
		case ST_COM_WAIT_CR:							// ��M���=CR�҂�
			if ( code == CODE_CR ){
				CmmChangeStatus( ST_COM_WAIT_LF );		// ��M���=LF�҂��@�Ɉڍs
			} else	{
				CmmSendNak();							// Nack���M
				CmmChangeStatus( ST_COM_STANDBY );		// ��M���=��M�f�[�^�҂��֖߂�(��M�d����j��)				
				// �ǉ��@2013.7.12				  
				 ercd = rel_mpf( MPF_COM, ( VP )msg );	// ������̓��������
				  if ( ercd != E_OK) {
					ErrCodeSet( ercd );
				 }
				// �ǉ�END
			}
			break;
					
		case ST_COM_WAIT_LF:							// ��M���=LF�҂�
			if ( code == CODE_LF ){
				CmmSendAck();							// ��M�����AAck���M	
											
				ercd = snd_mbx( MBX_CMD_LAN, (VP)msg );	// ��M�f�[�^���A�R�}���h�����^�X�N�֑���
				if ( ( ercd != E_OK ) /* && ( ercd != E_TMOUT ) */ ){
					PrgErrSet();
				}
				
				if ( MdGetMode() == MD_CAP ){			// �L���v�`���[�������Ȃ�
					CmmChangeStatus( ST_COM_WAIT_EOT );	// ��M���=EOT�҂�
				} else {
					// �ǉ��@2013.7.12				  
				 	ercd = rel_mpf( MPF_COM, ( VP )msg );	// ������̓��������
				  	if ( ercd != E_OK) {
						ErrCodeSet( ercd );
				 	}
					// �ǉ�END
					
					return;
				}

			} else	{
				CmmSendNak();							// Nack���M
				CmmChangeStatus( ST_COM_STANDBY );		// ��M���=��M�f�[�^�҂��֖߂�(��M�d����j��)
				// �ǉ��@2013.7.12				  
				ercd = rel_mpf( MPF_COM, ( VP )msg );	// ������̓��������
				if ( ercd != E_OK) {
					ErrCodeSet( ercd );
				}
				return;
				// �ǉ�END				
			}
			break;
			
		case ST_COM_WAIT_EOT:							// ��M���=EOT�҂�
			if ( Fcnt >= 3 ){							// ��M�o�C�g���̃`�F�b�N
				if ( ( msg->buf[ 0 ] == CODE_EOT )  
				  && ( msg->buf[ 1 ] == CODE_CR ) 
				  && ( msg->buf[ 2 ] == CODE_LF ) ){
//			if ( Fcnt >= 3 ){							// ��M�o�C�g���̃`�F�b�N
//				if ( ( rcv_data.buf[ 0 ] == CODE_EOT )  
//				  && ( rcv_data.buf[ 1 ] == CODE_CR ) 
//				  && ( rcv_data.buf[ 2 ] == CODE_LF ) ){
					 
					ercd = rel_mpf( MPF_COM, ( VP )msg );	// ������̓��������
					if ( ercd != E_OK) {
						ErrCodeSet( ercd );
					}
					 return; 
				  } else {
				  
					CmmSendNak();							// Nack���M
					ercd = rel_mpf( MPF_COM, ( VP )msg );	// ������̓��������
					if ( ercd != E_OK) {
						ErrCodeSet( ercd );
				  	}
					 
				    return;
				  }
			}
			break;
			
		case ST_COM_WAIT_ACK_NAK:
			if ( Fcnt >= 3 ){							// ��M�o�C�g���̃`�F�b�N
				if ( ( msg->buf[ 0 ] == CODE_ACK )  
				  && ( msg->buf[ 1 ] == CODE_CR ) 
				  && ( msg->buf[ 2 ] == CODE_LF ) ){
//			if ( Fcnt >= 3 ){							// ��M�o�C�g���̃`�F�b�N
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

					CmmSendNak();							// Nack���M

				 }
				  
				 ercd = rel_mpf( MPF_COM, ( VP )msg );	// ������̓��������
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
	��M�^�X�N���̎�M���̏�ԑJ�ڏ���
	�E���݂̏�Ԃ�Fstatus�ϐ����ێ�
	[����]
		newStatus	���̏��
===========================================================*/
static void CmmChangeStatus(int newStatus)
{
	//�ޏꏈ��
	
	//���ꏈ��
	switch( newStatus ) {
	case ST_COM_STANDBY:		// ��M�J�n
		Fcnt = 0;
		cmd_len = 0;
		break;
	case ST_COM_WAIT_EOT:		// ETX�҂�=��M�J�n
		Fcnt = 0;
		cmd_len = 0;
		break;
	case ST_COM_WAIT_LEN:		// ETX�҂�=��M�J�n
//		s_uiCntLen = 0;
		break;
	case ST_COM_WAIT_ACK_NAK:	// ACK/NAK�҂�=��M�J�n
		Fcnt = 0;
		rcv_ack_nak = 0;
		break;
	}
	
	Fstatus = newStatus;//���̏�ԕێ�
}

/**********************************************************/
/**********************************************************
 *
 * UDP Send Task
 *
 * @param ch �`�����l���ԍ�(�݊����̂���)
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

		} else {							/* �R�[�f�B���O�G���[ */
			ErrCodeSet( ercd );
		}
	}
}

//=============================================================================
/**
 *	�f�[�^�̑��M�i�ėp�j
 *	@param	data�@���M�f�[�^
 *	@param	cnt�@���M�o�C�g��
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
 *	NAK���M
 *	�ENACK�{CR LF
 *	�E�������u���b�N�̉��
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
 *	ACK���M
 *	�EACK�{CR LF
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
 *	EOT���M
 *	�EEOT�{CR LF
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
 *	�ŐV�̎w�F�،��ʂ�Ԃ��i�w�F�،��ʎQ�Ɓj
 */
/*==========================================================================*/
static UB CAPGetResult(void)
{
	return s_CapResult;
}
