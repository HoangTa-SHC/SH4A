/**
*	VA-300�v���O����
*
*	@file tsk_snd_serial.c
*	@version 1.00
*
*	@author Bionics Co.Ltd T.Nagai
*	@date   2013/10/04
*	@brief  �V���A�����M�^�X�N
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


//#define SND_SIO_BUF_SIZE	1024 + 4 (va300.h �Œ�`)
static UB send_sio_buff[ SND_SIO_BUF_SIZE ];	 // �V���A�����M�R�}���h�f�[�^�E�o�b�t�@
static UINT sio_snd_comm_len;	 // ���M���R�}���h�̎w�背���O�X
static UINT sio_snd_block_num;	 // ���M���R�}���h�̃u���b�N�ԍ�

static UB IfImageBuf[2*80*40 + 20*10];


static void SendSerialBinaryData( UB *data, UINT cnt );	// Serial�f�[�^�̑��M�i�ėp�j
static void send_sio_WakeUp( void );			// VA300S����Box�փV���A����WakeUp�̖₢���킹���s���iTest	�R�}���h�A�d��ON���j
static void send_sio_Touroku( void );			// VA300S����Box�փV���A���œo�^�����R�}���h(01)�𑗐M�B
static void send_sio_Touroku_Init( int j );		// VA300S����Box�փV���A���œo�^�����R�}���h(01)�𑗐M(�d��ON���̈ꊇ���M)�B
static void send_sio_Touroku_InitAll( void );	// VA300S����Box�փV���A���œo�^�����R�}���h(01)�𑗐M���C��(�d��ON���̈ꊇ���M)�B
//static void send_sio_Ninshou( UB result );		// VA300S����Box�փV���A���ŔF�؊����R�}���h(03)�𑗐M�B
static void send_sio_Ninshou( UB result, UB auth_type, UB info_type );		//20160108Miya FinKeyS // VA300S����Box�փV���A���ŔF�؊����R�}���h(03)�𑗐M�B
static void send_sio_Touroku_Del( void );		// VA300S����Box�փV���A���œo�^�f�[�^�폜�R�}���h(04)�𑗐M�B
static void send_sio_Touroku_AllDel( void );	// VA300S����Box�փV���A���œo�^�f�[�^�������i�ꊇ�폜�j�R�}���h(05)�𑗐M�B
static void send_sio_ShutDown( void );			// VA300S����Box�փV���A���ŃV���b�g�E�_�E���v���R�}���h(08)�𑗐M�B
static void send_sio_Kakaihou_time( void );		// VA300s����Box�փV���A���ŉߊJ�����Ԃ̐ݒ�v���R�}���h(09)�𑗐M����B
static void send_sio_init_time( void );			// VA300s����Box�֏��������̐ݒ�v���R�}���h(10)�𑗐M����B
static void send_sio_force_lock_close( void );	// VA300s����Box�֋��������R�}���h(11)�𑗐M����B
static void send_sio_BPWR( int sw );		//20160905Miya B-PWR����
static void send_sio_VA300Reset( void );	//20161031Miya Ver2204 �[�����Z�b�g
static void send_sio_STANDARD( void );			//20160905Miya PC����VA300S�𐧌䂷��
static void send_sio_TANMATU_INFO( void );		//20160905Miya PC����VA300S�𐧌䂷��
static void send_sio_REGDATA_UPLD_STA( void );	//20160905Miya PC����VA300S�𐧌䂷��
static void send_sio_REGDATA_UPLD_GET( void );	//20160905Miya PC����VA300S�𐧌䂷��
static void send_sio_REGPROC( int rslt );		//20160905Miya PC����VA300S�𐧌䂷��
static void send_sio_AUTHPROC( int rslt );		//20160905Miya PC����VA300S�𐧌䂷��

/*******************************
 * Send Task
 *
 * @param ch �`�����l���ԍ�
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
			
    	} else {							/* �R�[�f�B���O�G���[ */
	    	ErrCodeSet( ercd );
    	}
    }
}


//=============================================================================
/**
 *	Serial�f�[�^�̑��M�i�ėp�j
 *	@param	data�@���M�f�[�^
 *	@param	cnt�@���M�o�C�g��
 */
//=============================================================================
void SendSerialBinaryData( UB *data, UINT cnt )
{
	T_COMMSG *msg;
	ER		ercd;
	
	ercd = tget_mpf( MPF_SND_SIO, ( VP* )&msg, ( 10/MSEC ) );
	
	sio_snd_TSK_wdt = FLG_ON;			// SIO���M�^�X�N�@WDT�J�E���^���Z�b�g�E���N�G�X�g�E�t���O
	
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
 *	VA300S����Box�փV���A����WakeUp�̖₢���킹���s���iTest	�R�}���h�A�d��ON���j
 */
/*==========================================================================*/
static void send_sio_WakeUp( void )
{
	int i;
	UB str1;

#if(PCCTRL)	//20160930Miya PC����VA300S�𐧌䂷��
	MdCngSubMode( SUB_MD_IDLE );	//�@�T�u���[�h�E�X�e�[�^�X���A�h�A�C�h���h�ցB
	return;
#endif
	
	/** �R�}���h�f�[�^�̏���	**/
	send_sio_buff[ 0 ] = '*';			// �w�b�_�@1Byte�@ASCII '*'
	send_sio_buff[ 1 ] = 0;				// �f�[�^���@�Q�o�C�gBinary
	send_sio_buff[ 2 ] = 0x0A;
	send_sio_buff[ 3 ] = 0x00;			// �u���b�N�ԍ� �Q�o�C�g�o�C�i��
	send_sio_buff[ 4 ] = 0x00;			
	send_sio_buff[ 5 ] = '0';			// �R�}���h��� �Q��ASCII
	send_sio_buff[ 6 ] = '0';

										//  �d�l��ʁ@�Q��ASCII
	if ( sys_demo_flg != SYS_SPEC_DEMO ){	// �f���d�l�̗L��
		send_sio_buff[ 7 ] = '0';			//�@�ʏ�d�l
	} else {
		send_sio_buff[ 7 ] = '1';			//�@�f���d�l	
	}
	
	if ( GetSysSpec() == SYS_SPEC_MANTION ){				// �}���V�����E��L���d�l
		send_sio_buff[ 8 ] = '0';

	} else if ( GetSysSpec() == SYS_SPEC_OFFICE ){			// �P�΂P�d�l�i�I�t�B�X�d�l�j
		send_sio_buff[ 8 ] = '1';
	
	} else if ( GetSysSpec() == SYS_SPEC_ENTRANCE ){		// �}���V�����E���p���d�l
		send_sio_buff[ 8 ] = '2';
	
	} else if ( GetSysSpec() == SYS_SPEC_OFFICE_NO_ID ){	// �P�Α��d�l�i�I�t�B�X�EID�ԍ������d�l�j
		send_sio_buff[ 8 ] = '3';
	
	} else {
		send_sio_buff[ 8 ] = '0';		// �d�l�ݒ肪�s��̏ꍇ�́A�}���V�����E��L���d�l
	}

	send_sio_buff[ 9 ] = (UINT)g_TechMenuData.HijyouRemocon + '0';			// �@��ݒ�i��펞�������������R���̗L���j

	sio_snd_comm_len = 10;				// �R�}���h��
	
	// �`�F�b�NSUM�̏����B
	str1 = 0;
	for ( i=0; i<sio_snd_comm_len; i++ ){	// �R�}���h�̃o�C�g���̑��a�����߂�
		str1 = str1 + send_sio_buff[ i ];
	}
	send_sio_buff[ 10 ] = str1 ^ 0xff;	// �`�F�b�N�T���𔽓]
	
	send_sio_buff[ 11 ] = CODE_CR;		//�@�I�[�R�[�h
	send_sio_buff[ 12 ] = CODE_LF;
	
	/** �R�}���h���M	**/
	SendSerialBinaryData( (UB*)send_sio_buff, ( sio_snd_comm_len + 3 ) );

	sio_mode = SIO_SEND_ACK_WAIT;				// �V���A������M���[�h�@���@Ack/Nack�����҂�
	ACK_Wait_timer = ACK_WAIT_TIME;				// Ack�҂��^�C���A�E�g����Set

	usSioRcvCount = 0;
	memset( cSioRcvBuf, 0, RCV_BUF_SIZE );

}


/*==========================================================================*/
/**
 *	�o�^�����R�}���h(01)
 */
/*==========================================================================*/
static void send_sio_Touroku( void )
{
	int i, ii, k;
	UB str1;
	int cnt;

#if(PCCTRL)	//20160930Miya PC����VA300S�𐧌䂷��
	MdCngSubMode( SUB_MD_IDLE );	//�@�T�u���[�h�E�X�e�[�^�X���A�h�A�C�h���h�ցB
	return;
#endif
	
	/** �R�}���h�f�[�^�̏���	**/
	cnt = 0;
	send_sio_buff[ cnt++ ] = '*';			// �w�b�_�@1Byte�@ASCII '*'
	send_sio_buff[ cnt++ ] = 0;				// �f�[�^���@�Q�o�C�gBinary
	send_sio_buff[ cnt++ ] = 0;
	send_sio_buff[ cnt++ ] = 0x00;			// �u���b�N�ԍ� �Q�o�C�g�o�C�i��
	send_sio_buff[ cnt++ ] = 0x00;			
	send_sio_buff[ cnt++ ] = '0';			// �R�}���h��� �Q��ASCII
	send_sio_buff[ cnt++ ] = '1';
										
	// ���ԍ��@ASCII �R��
	send_sio_buff[ cnt++ ] = ' ';
	send_sio_buff[ cnt++ ] = yb_touroku_data.tou_no[ 0 ];
	send_sio_buff[ cnt++ ] = yb_touroku_data.tou_no[ 1 ];		

	// ���[�U�[ID�@ASCII �S��
	send_sio_buff[ cnt++ ] = yb_touroku_data.user_id[ 0 ];
	send_sio_buff[ cnt++ ] = yb_touroku_data.user_id[ 1 ];
	send_sio_buff[ cnt++ ] = yb_touroku_data.user_id[ 2 ];
	send_sio_buff[ cnt++ ] = yb_touroku_data.user_id[ 3 ];
	
	// �o�^�wID�@ASCII �R��
	send_sio_buff[ cnt++ ] = yb_touroku_data.yubi_seq_no[ 0 ];
	send_sio_buff[ cnt++ ] = yb_touroku_data.yubi_seq_no[ 1 ];
	send_sio_buff[ cnt++ ] = yb_touroku_data.yubi_seq_no[ 2 ];
	
	// �ӔC��/��ʎҋ敪�@ASCII �P��
	send_sio_buff[ cnt++ ] = yb_touroku_data.kubun[ 0 ];

	// �o�^�w��ʁ@ASCII �Q��
	send_sio_buff[ cnt++ ] = yb_touroku_data.yubi_no[ 0 ];
	send_sio_buff[ cnt++ ] = yb_touroku_data.yubi_no[ 1 ];
	
	// ���O�@ASCII �Q�S��
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
			send_sio_buff[ cnt++ ] = 0xDE; //���_
			++i;
		}
		if(k == 3){
			send_sio_buff[ cnt++ ] = 0xDF; //�����_
			++i;
		}
		++ii;
	}

	sio_snd_comm_len = cnt;					// �R�}���h��
	send_sio_buff[ 2 ] = sio_snd_comm_len;
	
	// �`�F�b�NSUM�̏����B
	str1 = 0;
	for ( i = 0; i < sio_snd_comm_len; i++ ){	// �R�}���h�̃o�C�g���̑��a�����߂�
		str1 = str1 + send_sio_buff[ i ];
	}
	send_sio_buff[ cnt++ ] = str1 ^ 0xff;	// �`�F�b�N�T���𔽓]
	
	send_sio_buff[ cnt++ ] = CODE_CR;		//�@�I�[�R�[�h
	send_sio_buff[ cnt++ ] = CODE_LF;
	
	/** �R�}���h���M	**/
	SendSerialBinaryData( (UB*)send_sio_buff, ( sio_snd_comm_len + 3 ) );

	sio_mode = SIO_SEND_ACK_WAIT;				// �V���A������M���[�h�@���@Ack/Nack�����҂�
	ACK_Wait_timer = ACK_WAIT_TIME;				// Ack�҂��^�C���A�E�g����Set

	usSioRcvCount = 0;
	memset( cSioRcvBuf, 0, RCV_BUF_SIZE );

}

/*==========================================================================*/
/**
 *	�o�^�������M�R�}���h(01)
 */
/*==========================================================================*/
static void send_sio_Touroku_InitAll( void )
{
	int i;
	UB str1;
	int cnt;

#if(PCCTRL)	//20160930Miya PC����VA300S�𐧌䂷��
	MdCngSubMode( SUB_MD_IDLE );	//�@�T�u���[�h�E�X�e�[�^�X���A�h�A�C�h���h�ցB
	return;
#endif
	
	for ( i = 0; i < 20 ; i++ ){
		
		if ( ( ( yb_touroku_data20[ i + 1 ].yubi_seq_no[ 0 ] >= '0') && ( yb_touroku_data20[ i + 1 ].yubi_seq_no[ 0 ] <= '9') )
	 	  && ( ( yb_touroku_data20[ i + 1 ].yubi_seq_no[ 1 ] >= '0') && ( yb_touroku_data20[ i + 1 ].yubi_seq_no[ 1 ] <= '9') )
		  && ( ( yb_touroku_data20[ i + 1 ].yubi_seq_no[ 2 ] >= '0') && ( yb_touroku_data20[ i + 1 ].yubi_seq_no[ 2 ] <= '9') ) ){

			if ( ( yb_touroku_data20[ i + 1 ].yubi_seq_no[ 0 ] != '0' )
			  || ( yb_touroku_data20[ i + 1 ].yubi_seq_no[ 1 ] != '0' )
			  || ( yb_touroku_data20[ i + 1 ].yubi_seq_no[ 2 ] != '0' ) ){
				send_sio_Touroku_Init( i );			// �o�^�������M�R�}���h(01)���M�B
				MdCngSubMode( SUB_MD_TOUROKU );		// �T�u���[�h���A�o�^�f�[�^���M���ցB
			
				while ( MdGetSubMode() != SUB_MD_IDLE ){	// ���M���Ack��M�҂��B
					dly_tsk( 25/MSEC );
				}	
			}
		}
	}
}


/*==========================================================================*/
/**
 *	�o�^�������M�R�}���h(01)
 */
/*==========================================================================*/
static void send_sio_Touroku_Init( int j )
{
	int i, ii, k;
	UB str1;
	int cnt;

#if(PCCTRL)	//20160930Miya PC����VA300S�𐧌䂷��
	MdCngSubMode( SUB_MD_IDLE );	//�@�T�u���[�h�E�X�e�[�^�X���A�h�A�C�h���h�ցB
	return;
#endif
	
	/** �R�}���h�f�[�^�̏���	**/
	cnt = 0;
	send_sio_buff[ cnt++ ] = '*';			// �w�b�_�@1Byte�@ASCII '*'
	send_sio_buff[ cnt++ ] = 0;				// �f�[�^���@�Q�o�C�gBinary
	send_sio_buff[ cnt++ ] = 0;
	send_sio_buff[ cnt++ ] = 0x00;			// �u���b�N�ԍ� �Q�o�C�g�o�C�i��
	send_sio_buff[ cnt++ ] = 0x00;			
	send_sio_buff[ cnt++ ] = '0';			// �R�}���h��� �Q��ASCII
	send_sio_buff[ cnt++ ] = '1';
										
	// ���ԍ��@ASCII �R��
	send_sio_buff[ cnt++ ] = ' ';
	send_sio_buff[ cnt++ ] = yb_touroku_data.tou_no[ 0 ];
	send_sio_buff[ cnt++ ] = yb_touroku_data.tou_no[ 1 ];		

	// ���[�U�[ID�@ASCII �S��
	send_sio_buff[ cnt++ ] = yb_touroku_data.user_id[ 0 ];
	send_sio_buff[ cnt++ ] = yb_touroku_data.user_id[ 1 ];
	send_sio_buff[ cnt++ ] = yb_touroku_data.user_id[ 2 ];
	send_sio_buff[ cnt++ ] = yb_touroku_data.user_id[ 3 ];
	
	// �o�^�wID�@ASCII �R��
	send_sio_buff[ cnt++ ] = yb_touroku_data20[ j + 1 ].yubi_seq_no[ 0 ];
	send_sio_buff[ cnt++ ] = yb_touroku_data20[ j + 1 ].yubi_seq_no[ 1 ];
	send_sio_buff[ cnt++ ] = yb_touroku_data20[ j + 1 ].yubi_seq_no[ 2 ];
	
	// �ӔC��/��ʎҋ敪�@ASCII �P��
	send_sio_buff[ cnt++ ] = yb_touroku_data20[ j + 1 ].kubun[ 0 ];

	// �o�^�w��ʁ@ASCII �Q��
	send_sio_buff[ cnt++ ] = yb_touroku_data20[ j + 1 ].yubi_no[ 0 ];
	send_sio_buff[ cnt++ ] = yb_touroku_data20[ j + 1 ].yubi_no[ 1 ];
	
	// ���O�@ASCII �Q�S��
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
			send_sio_buff[ cnt++ ] = 0xDE; //���_
			++i;
		}
		if(k == 3){
			send_sio_buff[ cnt++ ] = 0xDF; //�����_
			++i;
		}
		++ii;
	}

	sio_snd_comm_len = cnt;					// �R�}���h��
	send_sio_buff[ 2 ] = sio_snd_comm_len;
	
	// �`�F�b�NSUM�̏����B
	str1 = 0;
	for ( i = 0; i < sio_snd_comm_len; i++ ){	// �R�}���h�̃o�C�g���̑��a�����߂�
		str1 = str1 + send_sio_buff[ i ];
	}
	send_sio_buff[ cnt++ ] = str1 ^ 0xff;	// �`�F�b�N�T���𔽓]
	
	send_sio_buff[ cnt++ ] = CODE_CR;		//�@�I�[�R�[�h
	send_sio_buff[ cnt++ ] = CODE_LF;
	
	/** �R�}���h���M	**/
	SendSerialBinaryData( (UB*)send_sio_buff, ( sio_snd_comm_len + 3 ) );

	sio_mode = SIO_SEND_ACK_WAIT;				// �V���A������M���[�h�@���@Ack/Nack�����҂�
	ACK_Wait_timer = ACK_WAIT_TIME;				// Ack�҂��^�C���A�E�g����Set

	usSioRcvCount = 0;
	memset( cSioRcvBuf, 0, RCV_BUF_SIZE );

}


/*==========================================================================*/
/**
 *	�F�؊����R�}���h(03)
 */
/*==========================================================================*/
//static void send_sio_Ninshou( UB result )
static void send_sio_Ninshou( UB result, UB auth_type, UB info_type )
{
	int i;
	UB str1;
	int cnt;

#if(PCCTRL)	//20160930Miya PC����VA300S�𐧌䂷��
	MdCngSubMode( SUB_MD_IDLE );	//�@�T�u���[�h�E�X�e�[�^�X���A�h�A�C�h���h�ցB
	return;
#endif
	
	/** �R�}���h�f�[�^�̏���	**/
	cnt = 0;
	send_sio_buff[ cnt++ ] = '*';			// �w�b�_�@1Byte�@ASCII '*'
	send_sio_buff[ cnt++ ] = 0;				// �f�[�^���@�Q�o�C�gBinary
	send_sio_buff[ cnt++ ] = 0x0b;
	send_sio_buff[ cnt++ ] = 0x00;			// �u���b�N�ԍ� �Q�o�C�g�o�C�i��
	send_sio_buff[ cnt++ ] = 0x00;			
	send_sio_buff[ cnt++ ] = '0';			// �R�}���h��� �Q��ASCII
	send_sio_buff[ cnt++ ] = '3';
										//  �d�l��ʁ@�Q��ASCII
	if ( result == 1 ){	// �F�،���
		send_sio_buff[ cnt++ ] = 'O';			//�@�F��OK
		send_sio_buff[ cnt++ ] = 'K';			//�@�F��OK
	}else{
		send_sio_buff[ cnt++ ] = 'N';			//�@�F��NG	
		send_sio_buff[ cnt++ ] = 'G';			//�@�F��OK
	}
	
	//20160108Miya FinKetS
	if( auth_type == 1 ){						//�p�X���[�h�J��
		i = g_PasswordOpen2.num / 10;
		send_sio_buff[ cnt++ ] = 0x30 + i;
		i = g_PasswordOpen2.num % 10;
		send_sio_buff[ cnt++ ] = 0x30 + i;
		
		//20160108Miya FinKetS
		send_sio_buff[ cnt++ ] = '1';
	}else if( auth_type == 2 ){					//�ً}�J��
		send_sio_buff[ cnt++ ] = 0x30;
		send_sio_buff[ cnt++ ] = 0x30;

		//20160108Miya FinKetS
		send_sio_buff[ cnt++ ] = '2';
	}else{										//�w�J��
		i = g_RegUserInfoData.RegNum / 10;
		send_sio_buff[ cnt++ ] = 0x30 + i;
		i = g_RegUserInfoData.RegNum % 10;
		send_sio_buff[ cnt++ ] = 0x30 + i;

		//20160108Miya FinKetS
		send_sio_buff[ cnt++ ] = '0';
	}

	//20160108Miya FinKetS
	if( info_type == 1 ){						//���o�����ݒ�
		send_sio_buff[ cnt++ ] = '1';
	}else if( info_type == 2 ){					//������Ԑݒ�
		send_sio_buff[ cnt++ ] = '2';
	}else{
		send_sio_buff[ cnt++ ] = '0';
	}
		
	sio_snd_comm_len = cnt;					// �R�}���h��
	send_sio_buff[ 2 ] = sio_snd_comm_len;
	
	// �`�F�b�NSUM�̏����B
	str1 = 0;
	for ( i=0; i<sio_snd_comm_len; i++ ){	// �R�}���h�̃o�C�g���̑��a�����߂�
		str1 = str1 + send_sio_buff[ i ];
	}
	send_sio_buff[ cnt++ ] = str1 ^ 0xff;	// �`�F�b�N�T���𔽓]
	
	send_sio_buff[ cnt++ ] = CODE_CR;		//�@�I�[�R�[�h
	send_sio_buff[ cnt++ ] = CODE_LF;
	
	/** �R�}���h���M	**/
	SendSerialBinaryData( (UB*)send_sio_buff, ( sio_snd_comm_len + 3 ) );

	sio_mode = SIO_SEND_ACK_WAIT;				// �V���A������M���[�h�@���@Ack/Nack�����҂�
	ACK_Wait_timer = ACK_WAIT_TIME;				// Ack�҂��^�C���A�E�g����Set

	usSioRcvCount = 0;
	memset( cSioRcvBuf, 0, RCV_BUF_SIZE );

}

/*==========================================================================*/
/**
 *	�o�^�f�[�^�폜�R�}���h(04)
 */
/*==========================================================================*/
static void send_sio_Touroku_Del( void )
{
	int i;
	UB str1;
	int cnt;

#if(PCCTRL)	//20160930Miya PC����VA300S�𐧌䂷��
	MdCngSubMode( SUB_MD_IDLE );	//�@�T�u���[�h�E�X�e�[�^�X���A�h�A�C�h���h�ցB
	return;
#endif
	
	/** �R�}���h�f�[�^�̏���	**/
	cnt = 0;
	send_sio_buff[ cnt++ ] = '*';			// �w�b�_�@1Byte�@ASCII '*'
	send_sio_buff[ cnt++ ] = 0;				// �f�[�^���@�Q�o�C�gBinary
	send_sio_buff[ cnt++ ] = 0;
	send_sio_buff[ cnt++ ] = 0x00;			// �u���b�N�ԍ� �Q�o�C�g�o�C�i��
	send_sio_buff[ cnt++ ] = 0x00;			
	send_sio_buff[ cnt++ ] = '0';			// �R�}���h��� �Q��ASCII
	send_sio_buff[ cnt++ ] = '4';		

	// ���[�U�[ID�@ASCII �S��
	send_sio_buff[ cnt++ ] = yb_touroku_data.user_id[ 0 ];
	send_sio_buff[ cnt++ ] = yb_touroku_data.user_id[ 1 ];
	send_sio_buff[ cnt++ ] = yb_touroku_data.user_id[ 2 ];
	send_sio_buff[ cnt++ ] = yb_touroku_data.user_id[ 3 ];
	
	// �o�^�wID�@ASCII �R��
	send_sio_buff[ cnt++ ] = yb_touroku_data.yubi_seq_no[ 0 ];
	send_sio_buff[ cnt++ ] = yb_touroku_data.yubi_seq_no[ 1 ];
	send_sio_buff[ cnt++ ] = yb_touroku_data.yubi_seq_no[ 2 ];

	sio_snd_comm_len = cnt;					// �R�}���h��
	send_sio_buff[ 2 ] = sio_snd_comm_len;
	
	// �`�F�b�NSUM�̏����B
	str1 = 0;
	for ( i = 0; i < sio_snd_comm_len; i++ ){	// �R�}���h�̃o�C�g���̑��a�����߂�
		str1 = str1 + send_sio_buff[ i ];
	}
	send_sio_buff[ cnt++ ] = str1 ^ 0xff;	// �`�F�b�N�T���𔽓]
	
	send_sio_buff[ cnt++ ] = CODE_CR;		//�@�I�[�R�[�h
	send_sio_buff[ cnt++ ] = CODE_LF;
	
	/** �R�}���h���M	**/
	SendSerialBinaryData( (UB*)send_sio_buff, ( sio_snd_comm_len + 3 ) );

	sio_mode = SIO_SEND_ACK_WAIT;				// �V���A������M���[�h�@���@Ack/Nack�����҂�
	ACK_Wait_timer = ACK_WAIT_TIME;				// Ack�҂��^�C���A�E�g����Set

	usSioRcvCount = 0;
	memset( cSioRcvBuf, 0, RCV_BUF_SIZE );

}


/*==========================================================================*/
/**
 *	�o�^�f�[�^�������i�ꊇ�폜�j�R�}���h(05)
 */
/*==========================================================================*/
static void send_sio_Touroku_AllDel( void )
{
	int i;
	UB str1;
	int cnt;

#if(PCCTRL)	//20160930Miya PC����VA300S�𐧌䂷��
	MdCngSubMode( SUB_MD_IDLE );	//�@�T�u���[�h�E�X�e�[�^�X���A�h�A�C�h���h�ցB
	return;
#endif
	
	/** �R�}���h�f�[�^�̏���	**/
	cnt = 0;
	send_sio_buff[ cnt++ ] = '*';			// �w�b�_�@1Byte�@ASCII '*'
	send_sio_buff[ cnt++ ] = 0;				// �f�[�^���@�Q�o�C�gBinary
	send_sio_buff[ cnt++ ] = 0;
	send_sio_buff[ cnt++ ] = 0x00;			// �u���b�N�ԍ� �Q�o�C�g�o�C�i��
	send_sio_buff[ cnt++ ] = 0x00;			
	send_sio_buff[ cnt++ ] = '0';			// �R�}���h��� �Q��ASCII
	send_sio_buff[ cnt++ ] = '5';		

	// �`���f�[�^�@ASCII �Q��
	send_sio_buff[ cnt++ ] = ' ';
	send_sio_buff[ cnt++ ] = ' ';

	sio_snd_comm_len = cnt;					// �R�}���h��
	send_sio_buff[ 2 ] = sio_snd_comm_len;
	
	// �`�F�b�NSUM�̏����B
	str1 = 0;
	for ( i = 0; i < sio_snd_comm_len; i++ ){	// �R�}���h�̃o�C�g���̑��a�����߂�
		str1 = str1 + send_sio_buff[ i ];
	}
	send_sio_buff[ cnt++ ] = str1 ^ 0xff;	// �`�F�b�N�T���𔽓]
	
	send_sio_buff[ cnt++ ] = CODE_CR;		//�@�I�[�R�[�h
	send_sio_buff[ cnt++ ] = CODE_LF;
	
	/** �R�}���h���M	**/
	SendSerialBinaryData( (UB*)send_sio_buff, ( sio_snd_comm_len + 3 ) );

	sio_mode = SIO_SEND_ACK_WAIT;				// �V���A������M���[�h�@���@Ack/Nack�����҂�
	ACK_Wait_timer = ACK_WAIT_TIME;				// Ack�҂��^�C���A�E�g����Set

	usSioRcvCount = 0;
	memset( cSioRcvBuf, 0, RCV_BUF_SIZE );

}


/*==========================================================================*/
/**
 *	VA300S����Box�փV���A���ŃV���b�g�E�_�E���v���R�}���h(08)�𑗐M����B
 */
/*==========================================================================*/
static void send_sio_ShutDown( void )
{
	int i;
	UB str1;

#if(PCCTRL)	//20160930Miya PC����VA300S�𐧌䂷��
	MdCngSubMode( SUB_MD_IDLE );	//�@�T�u���[�h�E�X�e�[�^�X���A�h�A�C�h���h�ցB
	return;
#endif

	/** �R�}���h�f�[�^�̏���	**/
	send_sio_buff[ 0 ] = '*';			// �w�b�_�@1Byte�@ASCII '*'
	send_sio_buff[ 1 ] = 0;				// �f�[�^���@�Q�o�C�gBinary
	send_sio_buff[ 2 ] = 0x09;
	send_sio_buff[ 3 ] = 0x00;			// �u���b�N�ԍ� �Q�o�C�g�o�C�i��
	send_sio_buff[ 4 ] = 0x00;			
	send_sio_buff[ 5 ] = '0';			// �R�}���h��� �Q��ASCII
	send_sio_buff[ 6 ] = '8';

	send_sio_buff[ 7 ] = ' ';			//  �`���f�[�^�@�Q��ASCII
	send_sio_buff[ 8 ] = ' ';			

	sio_snd_comm_len = 9;				// �R�}���h��
	
	// �`�F�b�NSUM�̏����B
	str1 = 0;
	for ( i=0; i<sio_snd_comm_len; i++ ){	// �R�}���h�̃o�C�g���̑��a�����߂�
		str1 = str1 + send_sio_buff[ i ];
	}
	send_sio_buff[ 9 ] = str1 ^ 0xff;	// �`�F�b�N�T���𔽓]
	
	send_sio_buff[ 10 ] = CODE_CR;		//�@�I�[�R�[�h
	send_sio_buff[ 11 ] = CODE_LF;
	
	/** �R�}���h���M	**/
	SendSerialBinaryData( (UB*)send_sio_buff, ( sio_snd_comm_len + 3 ) );

	sio_mode = SIO_SEND_ACK_WAIT;				// �V���A������M���[�h�@���@Ack/Nack�����҂�
	ACK_Wait_timer = ACK_WAIT_TIME;				// Ack�҂��^�C���A�E�g����Set

	usSioRcvCount = 0;
	memset( cSioRcvBuf, 0, RCV_BUF_SIZE );

}


/*==========================================================================*/
/**
 *  VA300s����Box�։ߊJ�����Ԃ̐ݒ�v���R�}���h(09)�𑗐M����B
 */
/*==========================================================================*/
static void send_sio_Kakaihou_time( void )
{
	int i;
	UB str1, str4[ 4 ];
	int cnt;

#if(PCCTRL)	//20160930Miya PC����VA300S�𐧌䂷��
	MdCngSubMode( SUB_MD_IDLE );	//�@�T�u���[�h�E�X�e�[�^�X���A�h�A�C�h���h�ցB
	return;
#endif
	
	/** �R�}���h�f�[�^�̏���	**/
	cnt = 0;
	send_sio_buff[ cnt++ ] = '*';			// �w�b�_�@1Byte�@ASCII '*'
	send_sio_buff[ cnt++ ] = 0;				// �f�[�^���@�Q�o�C�gBinary
	send_sio_buff[ cnt++ ] = 0;
	send_sio_buff[ cnt++ ] = 0x00;			// �u���b�N�ԍ� �Q�o�C�g�o�C�i��
	send_sio_buff[ cnt++ ] = 0x00;			
	send_sio_buff[ cnt++ ] = '0';			// �R�}���h��� �Q��ASCII
	send_sio_buff[ cnt++ ] = '9';		

	// �ߊJ���ݒ莞�ԁ@ASCII �S��
	if ( door_open_over_time >= 9999 ){
		send_sio_buff[ cnt++ ] = '9';
		send_sio_buff[ cnt++ ] = '9';
		send_sio_buff[ cnt++ ] = '9';
		send_sio_buff[ cnt++ ] = '9';
			
	}	else {
		util_i_to_char4( door_open_over_time, str4 );		// intger�������@char�S��(�E�l��)�ɕϊ�����.
		
		send_sio_buff[ cnt++ ] = str4[ 0 ];
		send_sio_buff[ cnt++ ] = str4[ 1 ];
		send_sio_buff[ cnt++ ] = str4[ 2 ];
		send_sio_buff[ cnt++ ] = str4[ 3 ];
	}

	sio_snd_comm_len = cnt;					// �R�}���h��
	send_sio_buff[ 2 ] = sio_snd_comm_len;
	
	// �`�F�b�NSUM�̏����B
	str1 = 0;
	for ( i = 0; i < sio_snd_comm_len; i++ ){	// �R�}���h�̃o�C�g���̑��a�����߂�
		str1 = str1 + send_sio_buff[ i ];
	}
	send_sio_buff[ cnt++ ] = str1 ^ 0xff;	// �`�F�b�N�T���𔽓]
	
	send_sio_buff[ cnt++ ] = CODE_CR;		//�@�I�[�R�[�h
	send_sio_buff[ cnt++ ] = CODE_LF;
	
	/** �R�}���h���M	**/
	SendSerialBinaryData( (UB*)send_sio_buff, ( sio_snd_comm_len + 3 ) );

	sio_mode = SIO_SEND_ACK_WAIT;				// �V���A������M���[�h�@���@Ack/Nack�����҂�
	ACK_Wait_timer = ACK_WAIT_TIME;				// Ack�҂��^�C���A�E�g����Set

	usSioRcvCount = 0;
	memset( cSioRcvBuf, 0, RCV_BUF_SIZE );
	
}

/*==========================================================================*/
/**
 *  VA300s����Box�֏��������̐ݒ�v���R�}���h(10)�𑗐M����B
 */
/*==========================================================================*/
static void send_sio_init_time( void )
{
	int i;
	UB str1;
	UB fig1, fig2;
	int cnt;

#if(PCCTRL)	//20160930Miya PC����VA300S�𐧌䂷��
	MdCngSubMode( SUB_MD_IDLE );	//�@�T�u���[�h�E�X�e�[�^�X���A�h�A�C�h���h�ցB
	return;
#endif
	
	/** �R�}���h�f�[�^�̏���	**/
	cnt = 0;
	send_sio_buff[ cnt++ ] = '*';			// �w�b�_�@1Byte�@ASCII '*'
	send_sio_buff[ cnt++ ] = 0;				// �f�[�^���@�Q�o�C�gBinary
	send_sio_buff[ cnt++ ] = 0;
	send_sio_buff[ cnt++ ] = 0x00;			// �u���b�N�ԍ� �Q�o�C�g�o�C�i��
	send_sio_buff[ cnt++ ] = 0x00;			
	send_sio_buff[ cnt++ ] = '1';			// �R�}���h��� �Q��ASCII
	send_sio_buff[ cnt++ ] = '0';		

	// ���������̐ݒ�@ASCII �U��
	if ( ( count_1hour > 0 ) && ( count_1hour < 24 ) ){	// ��
		fig2 = count_1hour / 10;
		fig1 = count_1hour - ( fig2 * 10 );
		send_sio_buff[ cnt++ ] = fig2 + '0';
		send_sio_buff[ cnt++ ] = fig1 + '0';
	} else {
		send_sio_buff[ cnt++ ] = '0';
		send_sio_buff[ cnt++ ] = '0';		
	}
	
	if ( count_1min > 0 && count_1min < 60 ){		// ��
		fig2 = count_1min / 10;
		fig1 = count_1min - ( fig2 * 10 );
		send_sio_buff[ cnt++ ] = fig2 + '0';
		send_sio_buff[ cnt++ ] = fig1 + '0';
	} else {
		send_sio_buff[ cnt++ ] = '0';
		send_sio_buff[ cnt++ ] = '0';		
	}

	if ( count_1sec > 0 && count_1sec < 60 ){		// �b
		fig2 = count_1sec / 10;
		fig1 = count_1sec - ( fig2 * 10 );
		send_sio_buff[ cnt++ ] = fig2 + '0';
		send_sio_buff[ cnt++ ] = fig1 + '0';
	} else {
		send_sio_buff[ cnt++ ] = '0';
		send_sio_buff[ cnt++ ] = '0';		
	}

	sio_snd_comm_len = cnt;					// �R�}���h��
	send_sio_buff[ 2 ] = sio_snd_comm_len;
	
	// �`�F�b�NSUM�̏����B
	str1 = 0;
	for ( i = 0; i < sio_snd_comm_len; i++ ){	// �R�}���h�̃o�C�g���̑��a�����߂�
		str1 = str1 + send_sio_buff[ i ];
	}
	send_sio_buff[ cnt++ ] = str1 ^ 0xff;	// �`�F�b�N�T���𔽓]
	
	send_sio_buff[ cnt++ ] = CODE_CR;		//�@�I�[�R�[�h
	send_sio_buff[ cnt++ ] = CODE_LF;
	
	/** �R�}���h���M	**/
	SendSerialBinaryData( (UB*)send_sio_buff, ( sio_snd_comm_len + 3 ) );

	sio_mode = SIO_SEND_ACK_WAIT;				// �V���A������M���[�h�@���@Ack/Nack�����҂�
	ACK_Wait_timer = ACK_WAIT_TIME;				// Ack�҂��^�C���A�E�g����Set

	usSioRcvCount = 0;
	memset( cSioRcvBuf, 0, RCV_BUF_SIZE );
	
}


/*==========================================================================*/
/**
 *  VA300s����Box�֋��������R�}���h(11)�𑗐M����B
 */
/*==========================================================================*/
static void send_sio_force_lock_close( void )
{
	int i;
	UB str1;
	int cnt;

#if(PCCTRL)	//20160930Miya PC����VA300S�𐧌䂷��
	MdCngSubMode( SUB_MD_IDLE );	//�@�T�u���[�h�E�X�e�[�^�X���A�h�A�C�h���h�ցB
	return;
#endif
	
	/** �R�}���h�f�[�^�̏���	**/
	cnt = 0;
	send_sio_buff[ cnt++ ] = '*';			// �w�b�_�@1Byte�@ASCII '*'
	send_sio_buff[ cnt++ ] = 0;				// �f�[�^���@�Q�o�C�gBinary
	send_sio_buff[ cnt++ ] = 0;
	send_sio_buff[ cnt++ ] = 0x00;			// �u���b�N�ԍ� �Q�o�C�g�o�C�i��
	send_sio_buff[ cnt++ ] = 0x00;			
	send_sio_buff[ cnt++ ] = '1';			// �R�}���h��� �Q��ASCII
	send_sio_buff[ cnt++ ] = '1';

	send_sio_buff[ cnt++ ] = ' ';			//  �`���f�[�^�@�Q��ASCII
	send_sio_buff[ cnt++ ] = ' ';			

	sio_snd_comm_len = cnt;					// �R�}���h��
	send_sio_buff[ 2 ] = sio_snd_comm_len;
		
	// �`�F�b�NSUM�̏����B
	str1 = 0;
	for ( i = 0; i < sio_snd_comm_len; i++ ){	// �R�}���h�̃o�C�g���̑��a�����߂�
		str1 = str1 + send_sio_buff[ i ];
	}
	send_sio_buff[ cnt++ ] = str1 ^ 0xff;	// �`�F�b�N�T���𔽓]
	
	send_sio_buff[ cnt++ ] = CODE_CR;		//�@�I�[�R�[�h
	send_sio_buff[ cnt++ ] = CODE_LF;
	
	/** �R�}���h���M	**/
	SendSerialBinaryData( (UB*)send_sio_buff, ( sio_snd_comm_len + 3 ) );

	sio_mode = SIO_SEND_ACK_WAIT;				// �V���A������M���[�h�@���@Ack/Nack�����҂�
	ACK_Wait_timer = ACK_WAIT_TIME;				// Ack�҂��^�C���A�E�g����Set

	usSioRcvCount = 0;
	memset( cSioRcvBuf, 0, RCV_BUF_SIZE );
	
}

//20160905Miya B-PWR����
/*==========================================================================*/
/**
 *  VA300s����Box�փR�}���h(22)�𑗐M����B
 */
// B-PWR ON/OFF����@0:OFF 1:ON
/*==========================================================================*/
static void send_sio_BPWR( int sw )
{
	int i;
	UB str1;
	int cnt;

#if(BPWRCTRL == 0)
	MdCngSubMode( SUB_MD_IDLE );	//�@�T�u���[�h�E�X�e�[�^�X���A�h�A�C�h���h�ցB
	return;
#endif

#if(PCCTRL)	//20160930Miya PC����VA300S�𐧌䂷��
	MdCngSubMode( SUB_MD_IDLE );	//�@�T�u���[�h�E�X�e�[�^�X���A�h�A�C�h���h�ցB
	return;
#endif

	/** �R�}���h�f�[�^�̏���	**/
	cnt = 0;
	send_sio_buff[ cnt++ ] = '*';			// �w�b�_�@1Byte�@ASCII '*'
	send_sio_buff[ cnt++ ] = 0;				// �f�[�^���@�Q�o�C�gBinary
	send_sio_buff[ cnt++ ] = 0;
	send_sio_buff[ cnt++ ] = 0x00;			// �u���b�N�ԍ� �Q�o�C�g�o�C�i��
	send_sio_buff[ cnt++ ] = 0x00;			
	send_sio_buff[ cnt++ ] = '2';			// �R�}���h��� �Q��ASCII
	send_sio_buff[ cnt++ ] = '2';

	if(sw == FLG_OFF){
		send_sio_buff[ cnt++ ] = 'O';			//  �`���f�[�^�@�Q��ASCII
		send_sio_buff[ cnt++ ] = 'F';			
	}else{
		send_sio_buff[ cnt++ ] = 'O';			//  �`���f�[�^�@�Q��ASCII
		send_sio_buff[ cnt++ ] = 'N';			
	}

	sio_snd_comm_len = cnt;					// �R�}���h��
	send_sio_buff[ 2 ] = sio_snd_comm_len;
		
	// �`�F�b�NSUM�̏����B
	str1 = 0;
	for ( i = 0; i < sio_snd_comm_len; i++ ){	// �R�}���h�̃o�C�g���̑��a�����߂�
		str1 = str1 + send_sio_buff[ i ];
	}
	send_sio_buff[ cnt++ ] = str1 ^ 0xff;	// �`�F�b�N�T���𔽓]
	
	send_sio_buff[ cnt++ ] = CODE_CR;		//�@�I�[�R�[�h
	send_sio_buff[ cnt++ ] = CODE_LF;
	
	/** �R�}���h���M	**/
	SendSerialBinaryData( (UB*)send_sio_buff, ( sio_snd_comm_len + 3 ) );

	sio_mode = SIO_SEND_ACK_WAIT;				// �V���A������M���[�h�@���@Ack/Nack�����҂�
	ACK_Wait_timer = ACK_WAIT_TIME;				// Ack�҂��^�C���A�E�g����Set

	usSioRcvCount = 0;
	memset( cSioRcvBuf, 0, RCV_BUF_SIZE );
	
}

//20161031Miya Ver2204 �[�����Z�b�g
/*==========================================================================*/
/**
 *  VA300s����Box�փR�}���h(23)�𑗐M����B
 */
// �[�����Z�b�g
/*==========================================================================*/
static void send_sio_VA300Reset( void )	//20161031Miya Ver2204 �[�����Z�b�g
{
	int i;
	UB str1;
	int cnt;

#if(BPWRCTRL == 0)
	MdCngSubMode( SUB_MD_IDLE );	//�@�T�u���[�h�E�X�e�[�^�X���A�h�A�C�h���h�ցB
	return;
#endif

#if(PCCTRL)	//20160930Miya PC����VA300S�𐧌䂷��
	MdCngSubMode( SUB_MD_IDLE );	//�@�T�u���[�h�E�X�e�[�^�X���A�h�A�C�h���h�ցB
	return;
#endif

	/** �R�}���h�f�[�^�̏���	**/
	cnt = 0;
	send_sio_buff[ cnt++ ] = '*';			// �w�b�_�@1Byte�@ASCII '*'
	send_sio_buff[ cnt++ ] = 0;				// �f�[�^���@�Q�o�C�gBinary
	send_sio_buff[ cnt++ ] = 0;
	send_sio_buff[ cnt++ ] = 0x00;			// �u���b�N�ԍ� �Q�o�C�g�o�C�i��
	send_sio_buff[ cnt++ ] = 0x00;			
	send_sio_buff[ cnt++ ] = '2';			// �R�}���h��� �Q��ASCII
	send_sio_buff[ cnt++ ] = '3';

	send_sio_buff[ cnt++ ] = 0x20;			//  �`���f�[�^�@�Q��ASCII
	send_sio_buff[ cnt++ ] = 0x20;			

	sio_snd_comm_len = cnt;					// �R�}���h��
	send_sio_buff[ 2 ] = sio_snd_comm_len;
		
	// �`�F�b�NSUM�̏����B
	str1 = 0;
	for ( i = 0; i < sio_snd_comm_len; i++ ){	// �R�}���h�̃o�C�g���̑��a�����߂�
		str1 = str1 + send_sio_buff[ i ];
	}
	send_sio_buff[ cnt++ ] = str1 ^ 0xff;	// �`�F�b�N�T���𔽓]
	
	send_sio_buff[ cnt++ ] = CODE_CR;		//�@�I�[�R�[�h
	send_sio_buff[ cnt++ ] = CODE_LF;
	
	/** �R�}���h���M	**/
	SendSerialBinaryData( (UB*)send_sio_buff, ( sio_snd_comm_len + 3 ) );

	sio_mode = SIO_SEND_ACK_WAIT;				// �V���A������M���[�h�@���@Ack/Nack�����҂�
	ACK_Wait_timer = ACK_WAIT_TIME;				// Ack�҂��^�C���A�E�g����Set

	usSioRcvCount = 0;
	memset( cSioRcvBuf, 0, RCV_BUF_SIZE );
	
}

//20160930Miya PC����VA300S�𐧌䂷��
/*==========================================================================*/
// VA300s����Box�փR�}���h(OK/NG)�𑗐M����B
/*==========================================================================*/
static void send_sio_STANDARD( void )
{
	int i;
	UB str1;
	int cnt;
	int dat, datlen;

	/** �R�}���h�f�[�^�̏���	**/
	cnt = 0;
	send_sio_buff[ cnt++ ] = '*';			// �w�b�_�@1Byte�@ASCII '*'
	send_sio_buff[ cnt++ ] = 1;				// �[���ԍ��@1�o�C�gBinary
	send_sio_buff[ cnt++ ] = 0;				// �f�[�^���@�Q�o�C�gBinary
	send_sio_buff[ cnt++ ] = 0;
	send_sio_buff[ cnt++ ] = 0x00;			// �u���b�N�ԍ� �Q�o�C�g�o�C�i��
	send_sio_buff[ cnt++ ] = 0x00;			
	send_sio_buff[ cnt++ ] = cSioRcvBuf[6];			// �R�}���h��� �Q��ASCII
	send_sio_buff[ cnt++ ] = cSioRcvBuf[7];

	send_sio_buff[ cnt++ ] = 'O';
	send_sio_buff[ cnt++ ] = 'K';

	sio_snd_comm_len = cnt;					// �R�}���h��
	send_sio_buff[ 3 ] = sio_snd_comm_len;
		
	// �`�F�b�NSUM�̏����B
	str1 = 0;
	for ( i = 0; i < sio_snd_comm_len; i++ ){	// �R�}���h�̃o�C�g���̑��a�����߂�
		str1 = str1 + send_sio_buff[ i ];
	}
	send_sio_buff[ cnt++ ] = str1 ^ 0xff;	// �`�F�b�N�T���𔽓]
	
	send_sio_buff[ cnt++ ] = CODE_CR;		//�@�I�[�R�[�h
	send_sio_buff[ cnt++ ] = CODE_LF;
	
	/** �R�}���h���M	**/
	SendSerialBinaryData( (UB*)send_sio_buff, ( sio_snd_comm_len + 3 ) );

	usSioRcvCount = 0;
	memset( cSioRcvBuf, 0, RCV_BUF_SIZE );
}
/*==========================================================================*/
// VA300s����Box�փR�}���h(A0)�𑗐M����B
/*==========================================================================*/
static void send_sio_TANMATU_INFO( void )
{
	int i;
	UB str1;
	int cnt;
	int dat, datlen;
	int keta1, keta2, keta3, keta4;

	/** �R�}���h�f�[�^�̏���	**/
	cnt = 0;
	send_sio_buff[ cnt++ ] = '*';			// �w�b�_�@1Byte�@ASCII '*'
	send_sio_buff[ cnt++ ] = 1;				// �[���ԍ��@1�o�C�gBinary
	send_sio_buff[ cnt++ ] = 0;				// �f�[�^���@�Q�o�C�gBinary
	send_sio_buff[ cnt++ ] = 0;
	send_sio_buff[ cnt++ ] = 0x00;			// �u���b�N�ԍ� �Q�o�C�g�o�C�i��
	send_sio_buff[ cnt++ ] = 0x00;			
	send_sio_buff[ cnt++ ] = 'A';			// �R�}���h��� �Q��ASCII
	send_sio_buff[ cnt++ ] = '0';

	//�[���ԍ�
	send_sio_buff[ cnt++ ] = '0';
	send_sio_buff[ cnt++ ] = '1';
	//�[������ID
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
	//�[��Ver
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
	//�V���A���ԍ�
	for(i = 0 ; i < 16 ; i++){ send_sio_buff[ cnt++ ] = i + 0x30; }

	sio_snd_comm_len = cnt;					// �R�}���h��
	send_sio_buff[ 3 ] = sio_snd_comm_len;
		
	// �`�F�b�NSUM�̏����B
	str1 = 0;
	for ( i = 0; i < sio_snd_comm_len; i++ ){	// �R�}���h�̃o�C�g���̑��a�����߂�
		str1 = str1 + send_sio_buff[ i ];
	}
	send_sio_buff[ cnt++ ] = str1 ^ 0xff;	// �`�F�b�N�T���𔽓]
	
	send_sio_buff[ cnt++ ] = CODE_CR;		//�@�I�[�R�[�h
	send_sio_buff[ cnt++ ] = CODE_LF;
	
	/** �R�}���h���M	**/
	SendSerialBinaryData( (UB*)send_sio_buff, ( sio_snd_comm_len + 3 ) );

	usSioRcvCount = 0;
	memset( cSioRcvBuf, 0, RCV_BUF_SIZE );
}
/*==========================================================================*/
// VA300s����Box�փR�}���h(A4)�𑗐M����B
/*==========================================================================*/
static void send_sio_REGDATA_UPLD_STA( void )
{
	int i;
	UB str1;
	int cnt;
	int dat, datlen;

	/** �R�}���h�f�[�^�̏���	**/
	cnt = 0;
	send_sio_buff[ cnt++ ] = '*';			// �w�b�_�@1Byte�@ASCII '*'
	send_sio_buff[ cnt++ ] = 1;				// �[���ԍ��@1�o�C�gBinary
	send_sio_buff[ cnt++ ] = 0;				// �f�[�^���@�Q�o�C�gBinary
	send_sio_buff[ cnt++ ] = 0;
	send_sio_buff[ cnt++ ] = 0x00;			// �u���b�N�ԍ� �Q�o�C�g�o�C�i��
	send_sio_buff[ cnt++ ] = 0x00;			
	send_sio_buff[ cnt++ ] = 'A';			// �R�}���h��� �Q��ASCII
	send_sio_buff[ cnt++ ] = '4';

	datlen = 2 * 80 * 40 + 20 * 10;
	dat = (datlen & 0xFF00) >> 8;
	send_sio_buff[ cnt++ ] = (unsigned char)dat;
	dat = datlen & 0xFF;
	send_sio_buff[ cnt++ ] = (unsigned char)dat;

	sio_snd_comm_len = cnt;					// �R�}���h��
	send_sio_buff[ 3 ] = sio_snd_comm_len;
		
	// �`�F�b�NSUM�̏����B
	str1 = 0;
	for ( i = 0; i < sio_snd_comm_len; i++ ){	// �R�}���h�̃o�C�g���̑��a�����߂�
		str1 = str1 + send_sio_buff[ i ];
	}
	send_sio_buff[ cnt++ ] = str1 ^ 0xff;	// �`�F�b�N�T���𔽓]
	
	send_sio_buff[ cnt++ ] = CODE_CR;		//�@�I�[�R�[�h
	send_sio_buff[ cnt++ ] = CODE_LF;
	
	/** �R�}���h���M	**/
	SendSerialBinaryData( (UB*)send_sio_buff, ( sio_snd_comm_len + 3 ) );

	usSioRcvCount = 0;
	memset( cSioRcvBuf, 0, RCV_BUF_SIZE );
	
	//�A�b�v���[�h�p�̉摜����
	memcpy(&IfImageBuf[0], &RegImgBuf[0][0][0][0], 80 * 40);
	memcpy(&IfImageBuf[80*40], &RegImgBuf[0][0][1][0], 80 * 40);
	memcpy(&IfImageBuf[2*80*40], &g_RegBloodVesselTagData[0].MinAuthImg[0][0], 20 * 10);
}
/*==========================================================================*/
// VA300s����Box�փR�}���h(A5)�𑗐M����B
/*==========================================================================*/
static void send_sio_REGDATA_UPLD_GET( void )
{
	int i;
	UB str1;
	int cnt;
	int dat, datlen, st, sndsz;

	/** �R�}���h�f�[�^�̏���	**/
	cnt = 0;
	send_sio_buff[ cnt++ ] = '*';			// �w�b�_�@1Byte�@ASCII '*'
	send_sio_buff[ cnt++ ] = 1;				// �[���ԍ��@1�o�C�gBinary
	send_sio_buff[ cnt++ ] = 0;				// �f�[�^���@�Q�o�C�gBinary
	send_sio_buff[ cnt++ ] = 0;

	dat = (usSioBunkatuNum & 0xff00) >> 8;
	send_sio_buff[ cnt++ ] = dat;			// �u���b�N�ԍ� �Q�o�C�g�o�C�i��
	dat = usSioBunkatuNum & 0xff;
	send_sio_buff[ cnt++ ] = 0x00;			

	send_sio_buff[ cnt++ ] = 'A';			// �R�}���h��� �Q��ASCII
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

	sio_snd_comm_len = cnt;					// �R�}���h��
	send_sio_buff[ 2 ] = (unsigned char)((sio_snd_comm_len & 0xFF00) >> 8);
	send_sio_buff[ 3 ] = (unsigned char)(sio_snd_comm_len & 0xFF);
		
	// �`�F�b�NSUM�̏����B
	str1 = 0;
	for ( i = 0; i < sio_snd_comm_len; i++ ){	// �R�}���h�̃o�C�g���̑��a�����߂�
		str1 = str1 + send_sio_buff[ i ];
	}
	send_sio_buff[ cnt++ ] = str1 ^ 0xff;	// �`�F�b�N�T���𔽓]
	
	send_sio_buff[ cnt++ ] = CODE_CR;		//�@�I�[�R�[�h
	send_sio_buff[ cnt++ ] = CODE_LF;
	
	/** �R�}���h���M	**/
	SendSerialBinaryData( (UB*)send_sio_buff, ( sio_snd_comm_len + 3 ) );

	usSioRcvCount = 0;
	memset( cSioRcvBuf, 0, RCV_BUF_SIZE );

}
/*==========================================================================*/
// VA300s����Box�փR�}���h(A7)�𑗐M����B
/*==========================================================================*/
static void send_sio_REGPROC( int rslt )
{
	int i;
	UB str1;
	int cnt;
	int dat, datlen;

	memset(send_sio_buff, 0, SND_SIO_BUF_SIZE);

	/** �R�}���h�f�[�^�̏���	**/
	cnt = 0;
	send_sio_buff[ cnt++ ] = '*';			// �w�b�_�@1Byte�@ASCII '*'
	send_sio_buff[ cnt++ ] = 1;				// �[���ԍ��@1�o�C�gBinary
	send_sio_buff[ cnt++ ] = 0;				// �f�[�^���@�Q�o�C�gBinary
	send_sio_buff[ cnt++ ] = 0;
	send_sio_buff[ cnt++ ] = 0x00;			// �u���b�N�ԍ� �Q�o�C�g�o�C�i��
	send_sio_buff[ cnt++ ] = 0x00;			
	send_sio_buff[ cnt++ ] = 'A';			// �R�}���h��� �Q��ASCII
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

	sio_snd_comm_len = cnt;					// �R�}���h��
	send_sio_buff[ 3 ] = sio_snd_comm_len;
		
	// �`�F�b�NSUM�̏����B
	str1 = 0;
	for ( i = 0; i < sio_snd_comm_len; i++ ){	// �R�}���h�̃o�C�g���̑��a�����߂�
		str1 = str1 + send_sio_buff[ i ];
	}
	send_sio_buff[ cnt++ ] = str1 ^ 0xff;	// �`�F�b�N�T���𔽓]
	
	send_sio_buff[ cnt++ ] = CODE_CR;		//�@�I�[�R�[�h
	send_sio_buff[ cnt++ ] = CODE_LF;
	
	/** �R�}���h���M	**/
	SendSerialBinaryData( (UB*)send_sio_buff, ( sio_snd_comm_len + 3 ) );

	usSioRcvCount = 0;
	memset( cSioRcvBuf, 0, RCV_BUF_SIZE );
	
	g_capallow = 0;		//�B�e���֎~
}
/*==========================================================================*/
// VA300s����Box�փR�}���h(A8)�𑗐M����B
/*==========================================================================*/
static void send_sio_AUTHPROC( int rslt )
{
	int i;
	UB str1;
	int cnt;
	int dat, datlen;

	/** �R�}���h�f�[�^�̏���	**/
	cnt = 0;
	send_sio_buff[ cnt++ ] = '*';			// �w�b�_�@1Byte�@ASCII '*'
	send_sio_buff[ cnt++ ] = 1;				// �[���ԍ��@1�o�C�gBinary
	send_sio_buff[ cnt++ ] = 0;				// �f�[�^���@�Q�o�C�gBinary
	send_sio_buff[ cnt++ ] = 0;
	send_sio_buff[ cnt++ ] = 0x00;			// �u���b�N�ԍ� �Q�o�C�g�o�C�i��
	send_sio_buff[ cnt++ ] = 0x00;			
	send_sio_buff[ cnt++ ] = 'A';			// �R�}���h��� �Q��ASCII
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


	sio_snd_comm_len = cnt;					// �R�}���h��
	send_sio_buff[ 3 ] = sio_snd_comm_len;
		
	// �`�F�b�NSUM�̏����B
	str1 = 0;
	for ( i = 0; i < sio_snd_comm_len; i++ ){	// �R�}���h�̃o�C�g���̑��a�����߂�
		str1 = str1 + send_sio_buff[ i ];
	}
	send_sio_buff[ cnt++ ] = str1 ^ 0xff;	// �`�F�b�N�T���𔽓]
	
	send_sio_buff[ cnt++ ] = CODE_CR;		//�@�I�[�R�[�h
	send_sio_buff[ cnt++ ] = CODE_LF;
	
	/** �R�}���h���M	**/
	SendSerialBinaryData( (UB*)send_sio_buff, ( sio_snd_comm_len + 3 ) );

	usSioRcvCount = 0;
	memset( cSioRcvBuf, 0, RCV_BUF_SIZE );
	
	g_capallow = 0;		//�B�e���֎~
}
