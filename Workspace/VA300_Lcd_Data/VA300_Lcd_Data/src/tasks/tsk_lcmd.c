/**
	VA-300�v���O����
	@file tsk_lcmd.c
	@version 1.00
	
	@author OYO Electric Co.Ltd
	@date 2012/07/31
	@brief UDP����̃R�}���h�����^�X�N
*/
/*=========================================================
	�R�}���h����
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
	�L��ϐ�
		�}�N���ϐ�
-------------------*/

// �ϐ���`
static UINT s_SubMode;			// �����e�i���X���[�h�̃T�u���
static UINT s_NormalSubMode;	// �ʏ탂�[�h�̃T�u���
//static enum MD_LAN_CMD_STATUS s_LanCmdSubMode;	// LAN�R�}���h���[�h�̃T�u���

static UB MdGetMode(void);					// ���[�h�̎擾�i���[�h�E�p�����[�^�́A"va300.h"���Q�Ƃ̂��Ɓj
static void MdCngMode( UINT eNextMode );	// ���[�h�J��
static UB MdGetSubMode(void);				// �T�u���[�h�̎擾
static void MdCngSubMode( UINT eNextMode );	// �T�u���[�h�J��
static UB rcv_cmd_proc_nomal( T_COMMSG *rcv_msg );		// �ʏ탂�[�h����M�R�}���h�������C��
static UB rcv_cmd211( T_COMMSG *rcv_msg );	// ��M�R�}���h"211"�w�o�^�@OK/NG���菈��
static UB rcv_cmd205( T_COMMSG *rcv_msg );	// ��M�R�}���h"205"�w�F�؁@OK/NG���菈��
static UB rcv_cmd206( T_COMMSG *rcv_msg );	// ��M�R�}���h"206"�w�f�[�^�̍ĎB�e����
static UB rcv_cmd207( T_COMMSG *rcv_msg );	//	��M�R�}���h"207"�J�����E�p�����[�^�̕ύX����
static UB rcv_cmd208( T_COMMSG *rcv_msg );	//	��M�R�}���h"208"LED���ʂ̕ύX����
static UB rcv_cmd101( T_COMMSG *rcv_msg );	// ��M�R�}���h�@�����e�i���X�E���[�h�ڍs����(�R�}���h101�̏���)
static UB rcv_cmd_proc_power_on( T_COMMSG *rcv_msg );	// �p���[�I�����[�h����M�R�}���h�������C��
static UB rcv_cmd_proc_power_off( T_COMMSG *rcv_msg );	// �p���[�I�t���[�h����M�R�}���h�������C��
static UB rcv_cmd_proc_init( T_COMMSG *rcv_msg );		// �����o�^����M�R�}���h�������C��
static UB rcv_cmd_proc_meinte( T_COMMSG *rcv_msg );		// �����e�i���X���[�h����M�R�}���h�������C��
static UB rcv_cmd022( T_COMMSG *rcv_msg );	// �������[�h�ݒ�v�� �����i�R�}���h022�̏����j
static UB rcv_cmd012( T_COMMSG *rcv_msg );	// �J�����E�p�����[�^�̏����l���M �����i�R�}���h022�̏����j
static UB rcv_cmd015( T_COMMSG *rcv_msg );	// �摜�����̏����l���M �����i�R�}���h015�̏����j
static UB rcv_cmd013( T_COMMSG *rcv_msg );	// LED���ʐ��l�̏����l���M �����i�R�}���h013�̏����j
static UB rcv_cmd020( T_COMMSG *rcv_msg );	// �o�^���̏����l���M �����i�R�}���h020�̏����j
static UB rcv_cmd205_211_ok_proc( void );	// ��M�R�}���h"205"�w�F��,"211"�w�o�^��OK����̎�M����
static UB rcv_cmd205_211_ng_proc( void );	// ��M�R�}���h"205"�w�F��,"211"�w�o�^��NG����̎�M����
static void rcv_cmd205_211_rt_proc( void );	// ��M�R�}���h"205"�w�F��,"211"�w�o�^�̃��g���C�v���̎�M����
static UB rcv_cmd002( T_COMMSG *rcv_msg );  // �菇A�̏ꍇ��OK/NG�̉�����M����
static UB rcv_cmd272( T_COMMSG *rcv_msg );	// �ً}�ԍ�(�W��)�\���f�[�^��M�����i�R�}���h271�̉����R�}���h�j


/*=========================================================*/
/**
 *	LAN�R�}���h�����^�X�N
 *
 *	@param	iCh	�`�����l��
 *	
 */
/*=========================================================*/
TASK LanCmdTask(INT iCh)
{
    T_COMMSG *msg;
	ER_UINT	msg_size;
	ER	ercd;
	TMO	tmout;
	
	tmout = TMO_FEVR;				// = -1�A�����҂�(�^�C���A�E�g�Ȃ�)
	
	if (iCh > 0) {
		// �����ɂ���͎̂����G���[
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
		  		case MD_POWER_ON:			// �p���[ON
					// ��M�R�}���h�������C��
					ercd = rcv_cmd_proc_power_on( msg );
					break;
			
		  		case MD_POWER_OFF:			// �p���[OFF
					// ��M�R�}���h�������C��
					ercd = rcv_cmd_proc_power_off( msg );
					break;
			
		  		case MD_INITIAL:			// �����o�^��
					// ��M�R�}���h�������C��
					ercd = rcv_cmd_proc_init( msg );
					
					ercd = rcv_cmd_proc_power_on( msg );	// �p���[�I���E���[�h�̍ŏ��ŁA���[�h�؂�ւ��v��������,
															// �����o�^���[�h�ֈڍs�����ꍇ�ׁ̈B
					break;		

	  	  		case MD_MAINTE:				// �����e�i���X�E���[�h
					// ��M�R�}���h�������C��
					ercd = rcv_cmd_proc_meinte( msg );
					break;
		
		  		case MD_NORMAL:				// �ʏ탂�[�h
		  		case MD_CAP:				// �J�����B�e�����[�h
					// ��M�R�}���h�������C��
					ercd = rcv_cmd_proc_nomal( msg );		// �ʏ탂�[�h�A�J�����B�e�����[�h�̏ꍇ
					
					ercd = rcv_cmd_proc_power_on( msg );	// �p���[�I���E���[�h�̍ŏ��ŁA���[�h�؂�ւ��v��������,
															// �ʏ탂�[�h�ֈڍs�����ꍇ�ׁ̈B
					break;					
			
	      		default:
					break;
			}	
			
			rel_mpf( MPF_COM, ( VP )msg );		// ������̓��������
			
		} else if ( msg_size == E_RLWAI ){	
			// �����҂������̂Ƃ��̏���(�R�}���h���s)
			tmout = McnParamRcvTmoutGet() / MSEC;	// �^�C���A�E�g��ݒ�
			nop();
		
		} else if ( msg_size == E_TMOUT ) {	
			// �^�C���A�E�g�̂Ƃ��̏���(���̃R�[�f�B���O�ł́A�����֗��Ȃ�)
			nop();


		} else {
			ercd = msg_size;
			ErrCodeSet( ercd );
		}
	}
}


/*==========================================================================*/
/**
 *	�ʏ탂�[�h���@��M�R�}���h��͂Ə������C��
 *	@return ��͐���/���s
 */
/*==========================================================================*/
static UB rcv_cmd_proc_nomal( T_COMMSG *rcv_msg )
{
	UB ercd;
	
	nop();

	if ( ( rcv_msg->buf[ 4 ] == '2' )			// �R�}���h211�Ȃ�A�w�o�^��OK/NG�̔���
	  && ( rcv_msg->buf[ 5 ] == '1' )
	  && ( rcv_msg->buf[ 6 ] == '1' ) ){
					  
		ercd = rcv_cmd211( rcv_msg );			// �R�}���h211�̏���
	
	} else if ( ( rcv_msg->buf[ 4 ] == '2' )	// �R�}���h205�Ȃ�A�w�F�؂�OK/NG�̔���
	 		 && ( rcv_msg->buf[ 5 ] == '0' )
	  		 && ( rcv_msg->buf[ 6 ] == '5' ) ){
					  
		ercd = rcv_cmd205( rcv_msg );			// �R�}���h205�̏���
/**	
	} else if ( ( rcv_msg->buf[ 4 ] == '2' )	// �R�}���h201�Ȃ�A�ʏ탂�[�h�؂�ւ��̏���
	 		 && ( rcv_msg->buf[ 5 ] == '0' )
	  		 && ( rcv_msg->buf[ 6 ] == '1' ) ){
					  
		MdCngMode( MD_NORMAL );					// �ʏ탂�[�h�؂�ւ�
		
		MdCngSubMode( SUB_MD_IDLE );			// �T�u���[�h���AIDLE�֐ݒ�
**/	
	} else if ( ( rcv_msg->buf[ 4 ] == '2' )	// �R�}���h206�Ȃ�A�w�f�[�^�̍ĎB�e����
	 		 && ( rcv_msg->buf[ 5 ] == '0' )
	  		 && ( rcv_msg->buf[ 6 ] == '6' ) ){
					  
		ercd = rcv_cmd206( rcv_msg );			// �R�}���h206�̏���
		
	} else if ( ( rcv_msg->buf[ 4 ] == '2' )	// �R�}���h207�Ȃ�A�J�����E�p�����[�^�̕ύX����
	 		 && ( rcv_msg->buf[ 5 ] == '0' )
	  		 && ( rcv_msg->buf[ 6 ] == '7' ) ){
					  
		ercd = rcv_cmd207( rcv_msg );			// �R�}���h207�̏���
		
	} else if ( ( rcv_msg->buf[ 4 ] == '2' )	// �R�}���h208�Ȃ�ALED���ʂ̕ύX����
	 		 && ( rcv_msg->buf[ 5 ] == '0' )
	  		 && ( rcv_msg->buf[ 6 ] == '8' ) ){
					  
		ercd = rcv_cmd208( rcv_msg );			// �R�}���h208�̏���
		
	} else if ( ( rcv_msg->buf[ 4 ] == '1' )	// �R�}���h101�Ȃ�A�����e�i���X���[�h�ڍs����
	 		 && ( rcv_msg->buf[ 5 ] == '0' )
	  		 && ( rcv_msg->buf[ 6 ] == '1' ) ){
					  
		ercd = rcv_cmd101( rcv_msg );			// �R�}���h101�̏���
		
	} else if ( ( rcv_msg->buf[ 4 ] == '0' )	// �R�}���h002�Ȃ�A�菇A�̏ꍇ��OK/NG�̉�������
	 		 && ( rcv_msg->buf[ 5 ] == '0' )
	  		 && ( rcv_msg->buf[ 6 ] == '2' ) ){
					  
		if ( MdGetSubMode() != SUB_MD_IDLE ){
			ercd = rcv_cmd002( rcv_msg );			// �R�}���h002�̏���
		}
		
	} else if ( ( rcv_msg->buf[ 4 ] == '2' )	// �R�}���h272�Ȃ�A�ً}�ԍ�(�W��)�\���f�[�^��M�����i�R�}���h271�̉����R�}���h�j
	 		 && ( rcv_msg->buf[ 5 ] == '7' )
	  		 && ( rcv_msg->buf[ 6 ] == '2' ) ){
					  
		ercd = rcv_cmd272( rcv_msg );			// �R�}���h272�̏���
		
	}

	return ercd;				
}


/*==========================================================================*/
//	��M�R�}���h"205"�w�F�؂�OK/NG�̔���̏���
//	@return �����̐���/���s
/*==========================================================================*/
static UB rcv_cmd205( T_COMMSG *rcv_msg )
{
	UB ercd = 0xff;

	nop();
	
	 // ���茋�ʂ�OK�H
 	if ( ( ( rcv_msg->buf[ 11 ] == 'O' ) && ( rcv_msg->buf[ 12 ] == 'K' ) ) 
      || ( ( rcv_msg->buf[ 11 ] == 'o' ) && ( rcv_msg->buf[ 12 ] == 'k' ) )
	  || ( ( rcv_msg->buf[ 11 ] == 'O' ) && ( rcv_msg->buf[ 12 ] == 'k' ) )
	  || ( ( rcv_msg->buf[ 11 ] == 'o' ) && ( rcv_msg->buf[ 12 ] == 'K' ) ) ){
						  
		s_CapResult = CAP_JUDGE_OK;
		
		ercd = rcv_cmd205_211_ok_proc();	//�@OK����̎�M����
		
	 // ���茋�ʂ�NG�H
	} else  if ( ( ( rcv_msg->buf[ 11 ] == 'N' ) && ( rcv_msg->buf[ 12 ] == 'G' ) )
		      || ( ( rcv_msg->buf[ 11 ] == 'n' ) && ( rcv_msg->buf[ 12 ] == 'g' ) )
		      || ( ( rcv_msg->buf[ 11 ] == 'N' ) && ( rcv_msg->buf[ 12 ] == 'g' ) )
		      || ( ( rcv_msg->buf[ 11 ] == 'n' ) && ( rcv_msg->buf[ 12 ] == 'G' ) ) ){
						
	 	s_CapResult = CAP_JUDGE_NG;
		
		ercd = rcv_cmd205_211_ng_proc();	//�@NG����̎�M����

	} else if ( ( ( rcv_msg->buf[ 11 ] == 'R' ) && ( rcv_msg->buf[ 12 ] == 'T' ) ) 
	       || ( ( rcv_msg->buf[ 11 ] == 'r' ) && ( rcv_msg->buf[ 12 ] == 't' ) )
	       || ( ( rcv_msg->buf[ 11 ] == 'R' ) && ( rcv_msg->buf[ 12 ] == 't' ) )
	   	   || ( ( rcv_msg->buf[ 11 ] == 'r' ) && ( rcv_msg->buf[ 12 ] == 'T' ) ) ){
						  
		s_CapResult = CAP_JUDGE_RT;
		
		rcv_cmd205_211_rt_proc();			//�@���g���C�v���̏ꍇ�̎�M����

	} else if ( ( ( rcv_msg->buf[ 11 ] == 'E' ) && ( rcv_msg->buf[ 12 ] == '1' ) ) 
	       || ( ( rcv_msg->buf[ 11 ] == 'e' ) && ( rcv_msg->buf[ 12 ] == '1' ) ) ){
						  
		s_CapResult = CAP_JUDGE_E1;
		
		ercd = rcv_cmd205_211_ng_proc();	//�@NG����Ɠ�����M����
		
	} else if ( ( ( rcv_msg->buf[ 11 ] == 'E' ) && ( rcv_msg->buf[ 12 ] == '2' ) ) 
	       || ( ( rcv_msg->buf[ 11 ] == 'e' ) && ( rcv_msg->buf[ 12 ] == '2' ) ) ){
						  
		s_CapResult = CAP_JUDGE_E2;
		
		ercd = rcv_cmd205_211_ng_proc();	//�@NG����Ɠ�����M����
	}
	 
	return ercd;
} 


/*==========================================================================*/
//	��M�R�}���h"211"�w�o�^��OK/NG�̔���̏���
//	@return �����̐���/���s
/*==========================================================================*/
static UB rcv_cmd211( T_COMMSG *rcv_msg )
{
	UB ercd = 0xff;
	
	nop();

	 // ���茋�ʂ�OK�H
 	if ( ( ( rcv_msg->buf[ 11 ] == 'O' ) && ( rcv_msg->buf[ 12 ] == 'K' ) ) 
      || ( ( rcv_msg->buf[ 11 ] == 'o' ) && ( rcv_msg->buf[ 12 ] == 'k' ) )
	  || ( ( rcv_msg->buf[ 11 ] == 'O' ) && ( rcv_msg->buf[ 12 ] == 'k' ) )
	  || ( ( rcv_msg->buf[ 11 ] == 'o' ) && ( rcv_msg->buf[ 12 ] == 'K' ) ) ){
						  
		s_CapResult = CAP_JUDGE_OK;
		
		ercd = rcv_cmd205_211_ok_proc();	//�@OK����̎�M����
	
	 // ���茋�ʂ�NG�H
	} else  if ( ( ( rcv_msg->buf[ 11 ] == 'N' ) && ( rcv_msg->buf[ 12 ] == 'G' ) )
		      || ( ( rcv_msg->buf[ 11 ] == 'n' ) && ( rcv_msg->buf[ 12 ] == 'g' ) )
		      || ( ( rcv_msg->buf[ 11 ] == 'N' ) && ( rcv_msg->buf[ 12 ] == 'g' ) )
		      || ( ( rcv_msg->buf[ 11 ] == 'n' ) && ( rcv_msg->buf[ 12 ] == 'G' ) ) ){
				
		s_CapResult = CAP_JUDGE_NG;
		
		ercd = rcv_cmd205_211_ng_proc();	//�@NG����̎�M����

	} else if ( ( ( rcv_msg->buf[ 11 ] == 'R' ) && ( rcv_msg->buf[ 12 ] == 'T' ) ) 
	       || ( ( rcv_msg->buf[ 11 ] == 'r' ) && ( rcv_msg->buf[ 12 ] == 't' ) )
	       || ( ( rcv_msg->buf[ 11 ] == 'R' ) && ( rcv_msg->buf[ 12 ] == 't' ) )
	   	   || ( ( rcv_msg->buf[ 11 ] == 'r' ) && ( rcv_msg->buf[ 12 ] == 'T' ) ) ){
						  
		s_CapResult = CAP_JUDGE_RT;
		
		rcv_cmd205_211_rt_proc();			//�@���g���C�v���̏ꍇ�̎�M����

	} else if ( ( ( rcv_msg->buf[ 11 ] == 'E' ) && ( rcv_msg->buf[ 12 ] == '1' ) ) 
	       || ( ( rcv_msg->buf[ 11 ] == 'e' ) && ( rcv_msg->buf[ 12 ] == '1' ) ) ){
						  
		s_CapResult = CAP_JUDGE_E1;
		
		ercd = rcv_cmd205_211_ng_proc();	//�@NG����Ɠ�����M����
		
	} else if ( ( ( rcv_msg->buf[ 11 ] == 'E' ) && ( rcv_msg->buf[ 12 ] == '2' ) ) 
	       || ( ( rcv_msg->buf[ 11 ] == 'e' ) && ( rcv_msg->buf[ 12 ] == '2' ) ) ){
						  
		s_CapResult = CAP_JUDGE_E2;
		
		ercd = rcv_cmd205_211_ng_proc();	//�@NG����Ɠ�����M����
	}	
		
	return ercd;
} 


/*==========================================================================*/
//	��M�R�}���h"205"�w�F��,"211"�w�o�^��OK����̎�M����
/*==========================================================================*/
static UB rcv_cmd205_211_ok_proc( void )
{
	UB ercd = 0xff;
		
	// �����o�^���[�h�̏ꍇ
	if ( GetScreenNo() == LCD_SCREEN6 ){	// �F�؎��u�w���Z�b�g���ĉ�����..�v

		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN7 );	// 	�o�^�u�w�𔲂���..�v��ʂցB	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN7 );			// ��ʔԍ��@<-�@���̉��
		}
		MdCngMode( MD_INITIAL );			// ���u���[�h�������o�^���[�h��

		ercd = set_flg( ID_FLG_TS, FPTN_WAIT_CHG_SCRN );// TS�^�X�N�։�ʂ��u�w�𔲂��ĉ������v�܂��́A���s��ʂɂȂ�̂�ʒm�B
		if ( ercd != E_OK ){
			nop();			// �G���[�����̋L�q
		}						
	}
		
	if ( GetScreenNo() == LCD_SCREEN8 ){	// �F�؎��u������x�w���Z�b�g���ĉ�����..�v

		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN9 );	// �o�^������ʂցB	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN9 );			// ��ʔԍ��@<-�@���̉��
		}
		MdCngMode( MD_INITIAL );			// ���u���[�h�������o�^���[�h��
		
		reload_CAP_Param();			// Gain. FixShutter,IR_LED�̃p�����[�^���A�����v���l�ɖ߂��B�iCAP��Retry���ɕς�����\��������ׁB�j

	}

	// �ʏ탂�[�h�̏ꍇ		
	if ( ( GetScreenNo() == LCD_SCREEN101 ) || ( GetScreenNo() == LCD_SCREEN102 ) ){
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN103 );	// �F�؊�����ʂցB	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN103 );			// ��ʔԍ��@<-�@���̉��
		}
		MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h��
					
		reload_CAP_Param();			// Gain. FixShutter,IR_LED�̃p�����[�^���A�����v���l�ɖ߂��B�iCAP��Retry���ɕς�����\��������ׁB�j
			
	}
		
	if ( GetScreenNo() == LCD_SCREEN121 ){
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN122 );	// �o�^���̐ӔC�ҔF�؊�����ʂցB	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN122 );			// ��ʔԍ��@<-�@���̉��
		}
		MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h��	
		
		reload_CAP_Param();			// Gain. FixShutter,IR_LED�̃p�����[�^���A�����v���l�ɖ߂��B�iCAP��Retry���ɕς�����\��������ׁB�j
			
	}
		
	if ( GetScreenNo() == LCD_SCREEN127 ){	// �u�w���Z�b�g����...�v���
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN128 );	// �u�w�𔲂��ĉ������v��ʂցB	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN128 );			// ��ʔԍ��@<-�@���̉��
		}
		MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h��	
			
		ercd = set_flg( ID_FLG_TS, FPTN_WAIT_CHG_SCRN );// TS�^�X�N�։�ʂ��u�w�𔲂��ĉ������v�܂��́A���s��ʂɂȂ�̂�ʒm�B
		if ( ercd != E_OK ){
			nop();			// �G���[�����̋L�q
		}			
	}
		
	if ( GetScreenNo() == LCD_SCREEN129 ){	// �u������x�w���Z�b�g����...�v���
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN130 );	// �F�؊�����ʂցB	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN130 );			// ��ʔԍ��@<-�@���̉��
		}
		MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h��	
		
		reload_CAP_Param();			// Gain. FixShutter,IR_LED�̃p�����[�^���A�����v���l�ɖ߂��B�iCAP��Retry���ɕς�����\��������ׁB�j

	}
		
	if ( GetScreenNo() == LCD_SCREEN141 ){	// �폜���̐ӔC�ҔF�؊�����ʂցB
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN142 );	// �F�؊�����ʂցB	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN142 );			// ��ʔԍ��@<-�@���̉��
		}
		MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h��	
		
		reload_CAP_Param();			// Gain. FixShutter,IR_LED�̃p�����[�^���A�����v���l�ɖ߂��B�iCAP��Retry���ɕς�����\��������ׁB�j

	}
		
	if ( GetScreenNo() == LCD_SCREEN161 ){	// �ً}�J���ԍ��o�^���̐ӔC�ҔF�؊�����ʂցB
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN162 );	// �F�؊�������ʂցB	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN162 );			// ��ʔԍ��@<-�@���̉��
		}
		MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h��	
		
		reload_CAP_Param();			// Gain. FixShutter,IR_LED�̃p�����[�^���A�����v���l�ɖ߂��B�iCAP��Retry���ɕς�����\��������ׁB�j

	}
	
	// �����e�i���X�E���[�h�̏ꍇ
	if ( GetScreenNo() == LCD_SCREEN203 ){	// �����e�i���X�E�t���摜���M���u�w���Z�b�g���ĉ�����..�v

		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN204 );	// 	�B�e����ʂցB	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN204 );			// ��ʔԍ��@<-�@���̉��
		}
		MdCngMode( MD_MAINTE );			// ���u���[�h�������e�i���X���[�h��
	}
	
	return ercd;

}

/*==========================================================================*/
//	��M�R�}���h"205"�w�F��,"211"�w�o�^��NG����̎�M����
/*==========================================================================*/
static UB rcv_cmd205_211_ng_proc( void )
{
	UB ercd = 0xff;;
							
	// �����o�^���[�h�̏ꍇ
	if ( ( GetScreenNo() == LCD_SCREEN6 ) ){
		ercd = set_flg( ID_FLG_TS, FPTN_WAIT_CHG_SCRN );// TS�^�X�N�։�ʂ��u�w�𔲂��ĉ������v�܂��́A���s��ʂɂȂ�̂�ʒm�B
		if ( ercd != E_OK ){
			nop();			// �G���[�����̋L�q
		}
	}

	if ( ( GetScreenNo() == LCD_SCREEN6 ) || ( GetScreenNo() == LCD_SCREEN8 ) ){	// �F�؎��u�w���Z�b�g���ĉ�����..�v

		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN10 );	// �F�؎��s��ʂցB	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN10 );			// ��ʔԍ��@<-�@���̉��
		}
		MdCngMode( MD_INITIAL );			// ���u���[�h�������o�^���[�h��

		reload_CAP_Param();			// Gain. FixShutter,IR_LED�̃p�����[�^���A�����v���l�ɖ߂��B�iCAP��Retry���ɕς�����\��������ׁB�j

	}		

	// �ʏ탂�[�h�̏ꍇ					
	if ( ( GetScreenNo() == LCD_SCREEN101 ) || ( GetScreenNo() == LCD_SCREEN102 ) ){
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN104 );	// �F�؎��s��ʂցB	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN104 );			// ��ʔԍ��@<-�@���̉��
		}
		MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h��

		reload_CAP_Param();			// Gain. FixShutter,IR_LED�̃p�����[�^���A�����v���l�ɖ߂��B�iCAP��Retry���ɕς�����\��������ׁB�j			
			
	}
		
	if ( GetScreenNo() == LCD_SCREEN121 ){
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN123 );	// �F�؎��s��ʂցB	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN123 );			// ��ʔԍ��@<-�@���̉��
		}
		MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h��	
			
		reload_CAP_Param();			// Gain. FixShutter,IR_LED�̃p�����[�^���A�����v���l�ɖ߂��B�iCAP��Retry���ɕς�����\��������ׁB�j

	}

	if ( GetScreenNo() == LCD_SCREEN127 ){
		ercd = set_flg( ID_FLG_TS, FPTN_WAIT_CHG_SCRN );// TS�^�X�N�։�ʂ��u�w�𔲂��ĉ������v�܂��́A���s��ʂɂȂ�̂�ʒm�B
		if ( ercd != E_OK ){
			nop();			// �G���[�����̋L�q
		}
	}
				
	if ( ( GetScreenNo() == LCD_SCREEN127 ) || ( GetScreenNo() == LCD_SCREEN129 ) ){
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN131 );	// �F�؎��s��ʂցB	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN131 );			// ��ʔԍ��@<-�@���̉��
		}
		MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h��	

		reload_CAP_Param();			// Gain. FixShutter,IR_LED�̃p�����[�^���A�����v���l�ɖ߂��B�iCAP��Retry���ɕς�����\��������ׁB�j			
			
	}
		
	if ( GetScreenNo() == LCD_SCREEN141 ){
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN143 );	// �F�؎��s��ʂցB	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN143 );			// ��ʔԍ��@<-�@���̉��
		}
		MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h��	
			
		reload_CAP_Param();			// Gain. FixShutter,IR_LED�̃p�����[�^���A�����v���l�ɖ߂��B�iCAP��Retry���ɕς�����\��������ׁB�j
			
	}
		
		
	if ( GetScreenNo() == LCD_SCREEN161 ){	// �ً}�J���ԍ��o�^���̐ӔC�ҔF�؎��s��ʂցB
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN163 );	// �F�؎��s�~��ʂցB	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN163 );			// ��ʔԍ��@<-�@���̉��
		}
		MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h��	
		
		reload_CAP_Param();			// Gain. FixShutter,IR_LED�̃p�����[�^���A�����v���l�ɖ߂��B�iCAP��Retry���ɕς�����\��������ׁB�j

	}

	// �����e�i���X�E���[�h�̏ꍇ
	if ( GetScreenNo() == LCD_SCREEN203 ){	// �����e�i���X�E�t���摜���M�u�w���Z�b�g���ĉ�����..�v

		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN205 );	// 	�B�e�~��ʂցB	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN205 );			// ��ʔԍ��@<-�@���̉��
		}
		MdCngMode( MD_MAINTE );			// ���u���[�h�������e�i���X�E���[�h��
	}
	
	return ercd;
	
}



/*==========================================================================*/
//	��M�R�}���h"205"�w�F��,"211"�w�o�^�̃��g���C�v���̎�M����
/*==========================================================================*/
static void rcv_cmd205_211_rt_proc( void )
{
		
	CmmChangeStatus( ST_COM_STANDBY );		// ��M���=��M�f�[�^�҂��֖߂�(EOT�҂��������)

	// �����o�^���[�h�̏ꍇ
	if ( ( GetScreenNo() == LCD_SCREEN6 ) || ( GetScreenNo() == LCD_SCREEN8 ) ){	// �F�؎��u�w���Z�b�g���ĉ�����..�v

		MdCngMode( MD_INITIAL );		// ���u���[�h�������o�^���[�h��
	}		
		
	// �ʏ탂�[�h�̏ꍇ					
	if ( ( GetScreenNo() == LCD_SCREEN101 ) || ( GetScreenNo() == LCD_SCREEN102 ) ){

		MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h��
	}
		
	if ( GetScreenNo() == LCD_SCREEN121 ){

		MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h��	
	}
		
	if ( ( GetScreenNo() == LCD_SCREEN127 ) || ( GetScreenNo() == LCD_SCREEN129 ) ){

		MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h��	
	}
		
	if ( GetScreenNo() == LCD_SCREEN141 ){

		MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h��	
	}
	
	if ( GetScreenNo() == LCD_SCREEN161 ){

		MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h��	
	}
		
/**	
	// �����e�i���X�E���[�h�̏ꍇ
	if ( GetScreenNo() == LCD_SCREEN204 ){	// �����e�i���X�F�؎��u�w���Z�b�g���ĉ�����..�v

		MdCngMode( MD_MAINTE );			// ���u���[�h�������e�i���X�E���[�h��
	}
**/	
}

/*==========================================================================*/
//	��M�R�}���h"206"�w�f�[�^�̍ĎB�e����
//	@return �����̐���/���s
/*==========================================================================*/
static UB rcv_cmd206( T_COMMSG *rcv_msg )			// �R�}���h206�̏���
{
	UB ercd;

	nop();
	
 	if ( ( ( rcv_msg->buf[ 11 ] == 'T' ) && ( rcv_msg->buf[ 12 ] == 'O' ) ) 
      || ( ( rcv_msg->buf[ 11 ] == 't' ) && ( rcv_msg->buf[ 12 ] == 'o' ) )
	  || ( ( rcv_msg->buf[ 11 ] == 'T' ) && ( rcv_msg->buf[ 12 ] == 'o' ) )
	  || ( ( rcv_msg->buf[ 11 ] == 't' ) && ( rcv_msg->buf[ 12 ] == 'O' ) ) ){	

		if ( ( MdGetMode() == MD_NORMAL ) || ( MdGetMode() == MD_INITIAL ) ){	// �m�[�}�����[�h�A�C�j�V�������[�h�Ȃ�
						
			if ( s_CapResult == CAP_JUDGE_RT ){		// �ĎB�e�v���҂��Ȃ�
			
				ercd = set_flg( ID_FLG_CAMERA, FPTN_START_CAP204 );			// �J�����B�e+�o�^�����i�R�}���h204�j�ցB
			}
		}
		
	} else if ( ( ( rcv_msg->buf[ 11 ] == 'N' ) && ( rcv_msg->buf[ 12 ] == 'I' ) ) 
      	     || ( ( rcv_msg->buf[ 11 ] == 'n' ) && ( rcv_msg->buf[ 12 ] == 'i' ) )
	         || ( ( rcv_msg->buf[ 11 ] == 'N' ) && ( rcv_msg->buf[ 12 ] == 'i' ) )
	         || ( ( rcv_msg->buf[ 11 ] == 'n' ) && ( rcv_msg->buf[ 12 ] == 'I' ) ) ){
		  
		if ( ( MdGetMode() == MD_NORMAL ) || ( MdGetMode() == MD_INITIAL ) ){	// �m�[�}�����[�h�A�C�j�V�������[�h�Ȃ�

			if ( s_CapResult == CAP_JUDGE_RT ){		// �ĎB�e�v���҂��Ȃ�
									
				ercd = set_flg( ID_FLG_CAMERA, FPTN_START_CAP211 );			// �F�ؗp�B�e�����i�R�}���h210�j�ցB
			}
		}		  
	}
	 
	return ercd;
}


/*==========================================================================*/
//	��M�R�}���h"207"�J�����E�p�����[�^�̕ύX����
//	@return �����̐���/���s
/*==========================================================================*/
static UB rcv_cmd207( T_COMMSG *rcv_msg )			// �R�}���h207�̏���
{
	UB ercd = E_OK;

	nop();
	
	if ( rcv_msg->buf[ 11 ] == 0 ){		// �摜�擾���@�F�}�j���A���擾

		if ( ( rcv_msg->buf[ 12 ] >= 0 ) && ( rcv_msg->buf[ 12 ] <= 15 ) ){	//�@�J�����Q�C���ݒ�
			cmrGain = rcv_msg->buf[ 12 ];
			ercd = set_flg( ID_FLG_CAMERA, FPTN_SETREQ_GAIN );	// �J�����^�X�N�ɁA���ڂ̃Q�C���ݒ�l��ݒ���˗��iFPGA���o�R���Ȃ��j
		}
	
		if ( ( rcv_msg->buf[ 13 ] >= 0 ) && ( rcv_msg->buf[ 13 ] <= 15 ) ){	//�@�I�o�P�ݒ�
			cmrFixShutter1 = rcv_msg->buf[ 13 ];
		}
	
		if ( ( rcv_msg->buf[ 14 ] >= 0 ) && ( rcv_msg->buf[ 14 ] <= 15 ) ){	//�@�I�o�Q�ݒ�
			cmrFixShutter2 = rcv_msg->buf[ 14 ];
		}
	
		if ( ( rcv_msg->buf[ 15 ] >= 0 ) && ( rcv_msg->buf[ 15 ] <= 15 ) ){	//�@�I�o�R�ݒ�
			cmrFixShutter3 = rcv_msg->buf[ 15 ];
			ercd = set_flg( ID_FLG_CAMERA, FPTN_SETREQ_SHUT1 );	// �J�����^�X�N�ɁA���ڂ̘I�o�R�ݒ�l��ݒ���˗��iFPGA���o�R���Ȃ��j
		}
				
	} else if ( rcv_msg->buf[ 11 ] == 1 ){	// �摜�擾���@�FAES+�}�j���A���擾
	
		nop();
	}
	
	return ercd;
}


/*==========================================================================*/
//	��M�R�}���h"208"LED���ʂ̕ύX����
//	@return �����̐���/���s
/*==========================================================================*/
static UB rcv_cmd208( T_COMMSG *rcv_msg )			// �R�}���h208�̏���
{
	UB ercd = E_OK;

	nop();
	
	irDuty2 = rcv_msg->buf[ 11 ];		// IR LED�̓_�������l
	irDuty3 = rcv_msg->buf[ 12 ];
	irDuty4 = rcv_msg->buf[ 13 ];
	irDuty5 = rcv_msg->buf[ 14 ];
	
	return ercd;

}

/*==========================================================================*/
//	�菇A�̏ꍇ��OK/NG�̉�����M����
//	@return �����̐���/���s
/*==========================================================================*/
static UB rcv_cmd002( T_COMMSG *rcv_msg )			// �R�}���h002�̏���
{
	UB ercd = E_OK;

	nop();

	 // ��M���ʂ�OK�H
 	if ( ( ( rcv_msg->buf[ 11 ] == 'O' ) && ( rcv_msg->buf[ 12 ] == 'K' ) ) 
      || ( ( rcv_msg->buf[ 11 ] == 'o' ) && ( rcv_msg->buf[ 12 ] == 'k' ) )
	  || ( ( rcv_msg->buf[ 11 ] == 'O' ) && ( rcv_msg->buf[ 12 ] == 'k' ) )
	  || ( ( rcv_msg->buf[ 11 ] == 'o' ) && ( rcv_msg->buf[ 12 ] == 'K' ) ) ){
						  
		nop();					//�@OK����̎�M����
		if ( MdGetSubMode() == SUB_MD_DONGURU_CHK ){	// PC�փh���O���̗L���m�F�̊m�F�ʐM���Ȃ�
			s_DongleResult = DONGURU_JUDGE_OK;			// �h���O���̗L���m�F�̌���
			
			if ( GetScreenNo() == LCD_SCREEN101 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN200 );	// �����e�i���X���[�h��ʂցB
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN200 );		// ��ʔԍ��@<-�@���̉��
					
				} else {
					nop();			// �G���[�����̋L�q
				}
			}			
			MdCngSubMode( SUB_MD_IDLE );	//�@�T�u���[�h�E�X�e�[�^�X���A�h�A�C�h���h�ցB
			
		} else if ( MdGetSubMode() == SUB_MD_PASSWORD_CHK ){
			s_PasswordResult = PASSWORD_JUDGE_OK;		// �p�X���[�h�m�F�̌���
			
			if ( GetScreenNo() == LCD_SCREEN201 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN202 );	// �����e�i���X�E���j���[��ʂցB
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN202 );		// ��ʔԍ��@<-�@���̉��
					
					MdCngMode( MD_MAINTE );				// ���u���[�h�������e�i���X���[�h��
//					ercd = SndCmdCngMode( MD_MAINTE );	// PC��	�����e�i���X���[�h�ؑւ��ʒm�𑗐M
					if ( ercd != E_OK ){
						nop();		// �G���[�����̋L�q	
					}				
				} else {
					nop();			// �G���[�����̋L�q
				}
			}			
			MdCngSubMode( SUB_MD_IDLE );	//�@�T�u���[�h�E�X�e�[�^�X���A�h�A�C�h���h�ցB
			
		} else if ( MdGetSubMode() == SUB_MD_KINKYU_TOUROKU ){
			s_KinkyuuTourokuResult = KINKYU_TOUROKU_JUDGE_OK;		// �ً}�J���ԍ��o�^���M�̌���
			
			if ( GetScreenNo() == LCD_SCREEN164 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// �ʏ탂�[�h�E������ʂցB
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN100 );		// ��ʔԍ��@<-�@���̉��
					MdCngMode( MD_NORMAL );				// ���u���[�h��ʏ탂�[�h�ցi�s�v�̂͂������O�̂��߁j
				
				} else {
					nop();			// �G���[�����̋L�q
				}
			}			
			MdCngSubMode( SUB_MD_IDLE );	//�@�T�u���[�h�E�X�e�[�^�X���A�h�A�C�h���h�ցB
						
		} else if ( MdGetSubMode() == SUB_MD_KINKYU_KAIJYO_SEND ){	// �ً}�J���ԍ��ʒm�ʐM��
			
			if ( GetScreenNo() == LCD_SCREEN184 ){		// �u�W���ԍ�����͂��ĉ������v��ʂȂ�
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN185 );	// �ً}�ԍ����͉�ʂցB
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN185 );		// ��ʔԍ��@<-�@���̉��
					MdCngMode( MD_NORMAL );				// ���u���[�h��ʏ탂�[�h�ցi�s�v�̂͂������O�̂��߁j
				
				} else {
					nop();			// �G���[�����̋L�q
				}
			}			
			MdCngSubMode( SUB_MD_IDLE );	//�@�T�u���[�h�E�X�e�[�^�X���A�h�A�C�h���h�ցB
						
		} else if ( MdGetSubMode() == SUB_MD_KINKYU_NO_CHK_REQ ){	// ���͂��ꂽ�ً}�J���ԍ��Ó����m�F�̒ʐM��
			
			if ( GetScreenNo() == LCD_SCREEN185 ){		// �u�ً}�ԍ�����͂��ĉ�����...�v��ʂȂ�
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN186 );	// �J��OK�́���ʂցB
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN186 );		// ��ʔԍ��@<-�@���̉��
					MdCngMode( MD_NORMAL );				// ���u���[�h��ʏ탂�[�h�ցi�s�v�̂͂������O�̂��߁j
				
				} else {
					nop();			// �G���[�����̋L�q
				}
			}			
			MdCngSubMode( SUB_MD_IDLE );	//�@�T�u���[�h�E�X�e�[�^�X���A�h�A�C�h���h�ցB
						
		} else if ( MdGetSubMode() == SUB_MD_CHG_NORMAL ){	// �m�[�}�����[�h�ֈڍs��
						
			MdCngSubMode( SUB_MD_IDLE );	//�@�T�u���[�h�E�X�e�[�^�X���A�h�A�C�h���h�ցB
						
		} else if ( MdGetSubMode() == SUB_MD_CHG_MAINTE ){	// �����e�i���X���[�h�ֈڍs��
						
			MdCngSubMode( SUB_MD_IDLE );	//�@�T�u���[�h�E�X�e�[�^�X���A�h�A�C�h���h�ցB
						
		}


			
	 // ��M���ʂ�NG�H
	} else  if ( ( ( rcv_msg->buf[ 11 ] == 'N' ) && ( rcv_msg->buf[ 12 ] == 'G' ) )
		      || ( ( rcv_msg->buf[ 11 ] == 'n' ) && ( rcv_msg->buf[ 12 ] == 'g' ) )
		      || ( ( rcv_msg->buf[ 11 ] == 'N' ) && ( rcv_msg->buf[ 12 ] == 'g' ) )
		      || ( ( rcv_msg->buf[ 11 ] == 'n' ) && ( rcv_msg->buf[ 12 ] == 'G' ) ) ){
				
		nop();					//�@NG����̎�M����
		if ( MdGetSubMode() == SUB_MD_DONGURU_CHK ){	// PC�փh���O���̗L���m�F�̊m�F�ʐM���Ȃ�
			s_DongleResult = DONGURU_JUDGE_NG;			// �h���O���̗L���m�F�̌���
			
			// �h���O�������̏ꍇ�́A��ʑJ�ڏ����͂��Ȃ��B
			if ( GetScreenNo() == LCD_SCREEN101 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN101 );	// �ʏ탂�[�h�E���j���[��ʂցB
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN101 );		// ��ʔԍ��@<-�@���̉��
					
				} else {
					nop();			// �G���[�����̋L�q
				}
			}			
			MdCngSubMode( SUB_MD_IDLE );	//�@�T�u���[�h�E�X�e�[�^�X���A�h�A�C�h���h�ցB
			
		} else if ( MdGetSubMode() == SUB_MD_PASSWORD_CHK ){
			s_PasswordResult = PASSWORD_JUDGE_NG;		// �p�X���[�h�m�F�̌���
			
			// �p�X���[�hNG�̏ꍇ�́A�ʏ탂�[�h�E���j���[��ʂ�
			if ( GetScreenNo() == LCD_SCREEN201 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// �ʏ탂�[�h�E������ʂցB
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN100 );		// ��ʔԍ��@<-�@���̉��
					
				} else {
					nop();			// �G���[�����̋L�q
				}
			}			
			MdCngSubMode( SUB_MD_IDLE );	//�@�T�u���[�h�E�X�e�[�^�X���A�h�A�C�h���h�ցB
						
		} else if ( MdGetSubMode() == SUB_MD_KINKYU_TOUROKU ){
			s_KinkyuuTourokuResult = KINKYU_TOUROKU_JUDGE_NG;		// �ً}�J���ԍ��o�^���M�̌���
			
			if ( GetScreenNo() == LCD_SCREEN164 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN161 );	// �ʏ탂�[�h�E�ً}�J���ԍ��ݒ�u�w������...�v�ցB
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN161 );		// ��ʔԍ��@<-�@���̉��
					MdCngMode( MD_NORMAL );				// ���u���[�h��ʏ탂�[�h�ցi�s�v�̂͂������O�̂��߁j
				
				} else {
					nop();			// �G���[�����̋L�q
				}
			}			
			MdCngSubMode( SUB_MD_IDLE );	//�@�T�u���[�h�E�X�e�[�^�X���A�h�A�C�h���h�ցB
						
		} else if ( MdGetSubMode() == SUB_MD_KINKYU_KAIJYO_SEND ){	// �ً}�J���ԍ��ʒm�ʐM��
			
			if ( GetScreenNo() == LCD_SCREEN184 ){		// �u�W���ԍ�����͂��ĉ������v��ʂȂ�
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN187 );	// ���s�~��ʂցB
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN187 );		// ��ʔԍ��@<-�@���̉��
					MdCngMode( MD_NORMAL );				// ���u���[�h��ʏ탂�[�h�ցi�s�v�̂͂������O�̂��߁j
				
				} else {
					nop();			// �G���[�����̋L�q
				}
			}			
			MdCngSubMode( SUB_MD_IDLE );	//�@�T�u���[�h�E�X�e�[�^�X���A�h�A�C�h���h�ցB
						
		} else if ( MdGetSubMode() == SUB_MD_KINKYU_NO_CHK_REQ ){	// ���͂��ꂽ�ً}�J���ԍ��Ó����m�F�̒ʐM��
			
			if ( GetScreenNo() == LCD_SCREEN185 ){		// �u�ً}�ԍ�����͂��ĉ�����...�v��ʂȂ�
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN187 );	// �J��NG�́~��ʂցB
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN187 );		// ��ʔԍ��@<-�@���̉��
					MdCngMode( MD_NORMAL );				// ���u���[�h��ʏ탂�[�h�ցi�s�v�̂͂������O�̂��߁j
				
				} else {
					nop();			// �G���[�����̋L�q
				}
			}			
			MdCngSubMode( SUB_MD_IDLE );	//�@�T�u���[�h�E�X�e�[�^�X���A�h�A�C�h���h�ցB
						
		} else if ( MdGetSubMode() == SUB_MD_CHG_NORMAL ){	// �m�[�}�����[�h�ֈڍs��
						
			MdCngSubMode( SUB_MD_IDLE );	//�@�T�u���[�h�E�X�e�[�^�X���A�h�A�C�h���h�ցB
						
		} else if ( MdGetSubMode() == SUB_MD_CHG_MAINTE ){	// �����e�i���X���[�h�ֈڍs��
						
			MdCngSubMode( SUB_MD_IDLE );	//�@�T�u���[�h�E�X�e�[�^�X���A�h�A�C�h���h�ցB
						
		}
			
	}						  

	return ercd;
}

/*==========================================================================*/
//	�ً}�ԍ�(�W��)�\���f�[�^��M�����i�R�}���h271�̉����R�}���h�j
//	@return �����̐���/���s
/*==========================================================================*/
static UB rcv_cmd272( T_COMMSG *rcv_msg )			// �R�}���h272�̏���
{
	UB ercd = E_OK;

	nop();
	
	if ( ( rcv_msg->buf[ 11 ] >= '0') && ( rcv_msg->buf[ 11 ] <= '9') ){
		kinkyuu_hyouji_no[0] = rcv_msg->buf[ 11 ];		// �ً}�J���ً̋}�ԍ��W���\���f�[�^�i�z��ŏI�Ԗڂ͋�؂�L���@NUL�@�j
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
	
	if ( MdGetSubMode() == SUB_MD_KINKYU_8KETA_REQ ){	// �ً}�W���ԍ��v���ʐM���Ȃ�A	
		if ( GetScreenNo() == LCD_SCREEN182 ){  	// �uID�ԍ�����͂��ĉ�����...�v��ʂ̎�
		
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN183 );	// �u�ԍ����R�[���Z���^�[��...�v��ʂ��ALCD�^�X�N�֕\���v������B
			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN183 );		// ��ʔԍ��@<-�@���̉��
				MdCngMode( MD_NORMAL );				// ���u���[�h��ʏ탂�[�h�ցi�s�v�̂͂������O�̂��߁j
			}
		}
		MdCngSubMode( SUB_MD_IDLE );	//�@�T�u���[�h�E�X�e�[�^�X���A�h�A�C�h���h�ցB						  
	}
	
	return ercd;
}		

/*==========================================================================*/
//	��M�R�}���h"101"�����e�i���X�E���[�h�ڍs����
//	@return �����̐���/���s
/*==========================================================================*/
static UB rcv_cmd101( T_COMMSG *rcv_msg )			// �R�}���h101�̏���
{
	UB ercd = E_OK;

	nop();
	
	MdCngMode( MD_MAINTE );					// ���u���[�h���A�����e�i���X�E���[�h�֐ݒ�
	
	MdCngSubMode( SUB_MD_IDLE );			// �T�u���[�h���AIDLE�֐ݒ�

	return ercd;
}

/*==========================================================================*/
/**
 *	�p���[�I�����[�h����M�R�}���h�������C��
 *	@return ��͐���/���s
 */
/*==========================================================================*/
static UB rcv_cmd_proc_power_on( T_COMMSG *rcv_msg )
{
	UB ercd;
	
	nop();

	if ( ( rcv_msg->buf[ 4 ] == '0' )			// �R�}���h022�Ȃ�A�������[�h�ݒ�v��
	  && ( rcv_msg->buf[ 5 ] == '2' )
	  && ( rcv_msg->buf[ 6 ] == '2' ) ){
					  
		ercd = rcv_cmd022( rcv_msg );			// �R�}���h022�̏���

		MdCngSubMode( SUB_MD_IDLE );			// �T�u���[�h���AIDLE�֐ݒ�
	
	} else if ( ( rcv_msg->buf[ 4 ] == '0' )	// �R�}���h012�Ȃ�A�J�����E�p�����[�^�̏����l���M
	 		 && ( rcv_msg->buf[ 5 ] == '1' )
	  		 && ( rcv_msg->buf[ 6 ] == '2' ) ){
					  
		ercd = rcv_cmd012( rcv_msg );			// �R�}���h012�̏���
		
		MdCngSubMode( SUB_MD_IDLE );			// �T�u���[�h���AIDLE�֐ݒ�
	
	} else if ( ( rcv_msg->buf[ 4 ] == '0' )	// �R�}���h015�Ȃ�A�摜�����̏����l���M
	 		 && ( rcv_msg->buf[ 5 ] == '1' )
	  		 && ( rcv_msg->buf[ 6 ] == '5' ) ){
					  
		ercd = rcv_cmd015( rcv_msg );			// �R�}���h015�̏���
		
		MdCngSubMode( SUB_MD_IDLE );			// �T�u���[�h���AIDLE�֐ݒ�
	
	} else if ( ( rcv_msg->buf[ 4 ] == '0' )	// �R�}���h013�Ȃ�ALED���ʐ��l�̏����l���M
	 		 && ( rcv_msg->buf[ 5 ] == '1' )
	  		 && ( rcv_msg->buf[ 6 ] == '3' ) ){
					  
		ercd = rcv_cmd013( rcv_msg );			// �R�}���h013�̏���
		
		MdCngSubMode( SUB_MD_IDLE );			// �T�u���[�h���AIDLE�֐ݒ�
	
	} else if ( ( rcv_msg->buf[ 4 ] == '0' )	// �R�}���h020�Ȃ�A�o�^���̏����l���M
	 		 && ( rcv_msg->buf[ 5 ] == '2' )
	  		 && ( rcv_msg->buf[ 6 ] == '0' ) ){
					  
		ercd = rcv_cmd020( rcv_msg );			// �R�}���h020�̏���
		
		MdCngSubMode( SUB_MD_IDLE );			// �T�u���[�h���AIDLE�֐ݒ�
		
//		MdCngMode( MD_NORMAL );					// ���u���[�h��ʏ탂�[�h��

	
	} else if ( ( rcv_msg->buf[ 4 ] == '2' )	// �R�}���h201�Ȃ�A�ʏ탂�[�h�؂�ւ��̏���
	 		 && ( rcv_msg->buf[ 5 ] == '0' )
	  		 && ( rcv_msg->buf[ 6 ] == '1' ) ){
					  
		MdCngMode( MD_NORMAL );					// �ʏ탂�[�h�؂�ւ�
		
		MdCngSubMode( SUB_MD_IDLE );			// �T�u���[�h���AIDLE�֐ݒ�

	} else if ( ( rcv_msg->buf[ 4 ] == '0' )	// �R�}���h002�Ȃ�A�菇A�̏ꍇ��OK/NG�̉�������
	 		 && ( rcv_msg->buf[ 5 ] == '0' )
	  		 && ( rcv_msg->buf[ 6 ] == '2' ) ){
					  
		if ( MdGetSubMode() != SUB_MD_IDLE ){
			ercd = rcv_cmd002( rcv_msg );			// �R�}���h002�̏���
		}
	}
	 
	 
	 return ercd;

}



/*==========================================================================*/
//	�������[�h�ݒ�v�� �����i�R�}���h022�̏����j
//	@return ��������/���s
/*==========================================================================*/
static UB rcv_cmd022( T_COMMSG *rcv_msg )
{
	UB ercd = E_OK;
	
	nop();
	
	if ( rcv_msg->buf[ 11 ] == '0' ){
		
		switch  ( rcv_msg->buf[ 12 ] ){
		  	case '1':			// �����o�^���[�h��
				MdCngMode( MD_INITIAL );	
				break;
			
		  	case '2':			// �ʏ탂�[�h��
				MdCngMode( MD_NORMAL );	
				break;
			
		  	case '3':			// �����e�i���X�E���[�h��
				MdCngMode( MD_MAINTE );	
				break;		

	  	  	case '4':			// ��d���[�h��
				MdCngMode( MD_POWER_FAIL );	
				break;
		
		  	case '5':			// ��펞�������[�h��
				MdCngMode( MD_PANIC );	
				break;
			
	      	default:
				ercd = -1;		// �f�[�^�ُ�
				break;
		}

	} else {
		
		ercd = -1;		// �f�[�^�ُ�
	}
	
	return ercd;
}

/*==========================================================================*/
//	�J�����E�p�����[�^�̏����l���M �����i�R�}���h012�̏����j
//	@return ��������/���s
/*==========================================================================*/
static UB rcv_cmd012( T_COMMSG *rcv_msg ) 
{
	UB ercd = E_OK;
	
	nop();
	
	if ( rcv_msg->buf[ 13 ] == 0 ){		// �摜�擾���@�F�}�j���A���擾

		if ( ( rcv_msg->buf[ 14 ] >= 0 ) && ( rcv_msg->buf[ 14 ] <= 15 ) ){	//�@�J�����Q�C���ݒ�
			cmrGain = rcv_msg->buf[ 14 ];
			ini_cmrGain = rcv_msg->buf[ 14 ];					//�@�����ݒ�l���L���B
			
			ercd = set_flg( ID_FLG_CAMERA, FPTN_SETREQ_GAIN );	// �J�����^�X�N�ɁA���ڂ̃Q�C���ݒ�l��ݒ���˗��iFPGA���o�R���Ȃ��j
		}
	
		if ( ( rcv_msg->buf[ 15 ] >= 0 ) && ( rcv_msg->buf[ 15 ] <= 15 ) ){	//�@�I�o�P�ݒ�
			cmrFixShutter1 = rcv_msg->buf[ 15 ];
			ini_cmrFixShutter1 = rcv_msg->buf[ 15 ];			//�@�����ݒ�l���L���B
		}
	
		if ( ( rcv_msg->buf[ 16 ] >= 0 ) && ( rcv_msg->buf[ 16 ] <= 15 ) ){	//�@�I�o�Q�ݒ�
			cmrFixShutter2 = rcv_msg->buf[ 16 ];
			ini_cmrFixShutter2 = rcv_msg->buf[ 16 ];			//�@�����ݒ�l���L���B
		}
	
		if ( ( rcv_msg->buf[ 17 ] >= 0 ) && ( rcv_msg->buf[ 17 ] <= 15 ) ){	//�@�I�o�R�ݒ�
			cmrFixShutter3 = rcv_msg->buf[ 17 ];
			ini_cmrFixShutter3 = rcv_msg->buf[ 17 ];			//�@�����ݒ�l���L���B

			ercd = set_flg( ID_FLG_CAMERA, FPTN_SETREQ_SHUT1 );	// �J�����^�X�N�ɁA���ڂ̘I�o�R�ݒ�l��ݒ���˗��iFPGA���o�R���Ȃ��j
		}
				
	} else if ( rcv_msg->buf[ 13 ] == 1 ){	// �摜�擾���@�FAES+�}�j���A���擾
	
	
	}	
	return ercd;
}


/*==========================================================================*/
//	�摜�����̏����l���M �����i�R�}���h015�̏����j
//	@return ��������/���s
/*==========================================================================*/
static UB rcv_cmd015( T_COMMSG *rcv_msg )
{
	UB ercd = E_OK;
	unsigned short tmp1, tmp2;
	unsigned short posX = 0, posY = 0;
	
	nop();
	
	// 	�摜�؂�o���T�C�Y��ݒ肷��
	switch  ( rcv_msg->buf[ 11 ] ){		// �摜�؂�o���T�C�Y�w��
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
			ercd = -1;		// �f�[�^�ُ�
			break;
	}

	// 	�g���~���O�̍��W��ݒ肷��
	tmp1 = rcv_msg->buf[ 12 ];
	tmp1 = tmp1 << 8;
	posX = tmp1 + rcv_msg->buf[ 13 ]; //�@X���W
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
	
	// ���T�C�Y�̏k������ݒ肷��
	switch  ( rcv_msg->buf[ 16 ] ){		// �摜�؂�o���T�C�Y�w��
		  case '1':	
			iReSizeMode = RSZ_MODE_0;	//< ��1/2
			break;
			
		  case '2':	
			iReSizeMode = RSZ_MODE_1;	//< ��1/4
			break;
			
		 case '3':
			iReSizeMode = RSZ_MODE_2;	//< ��1/8
			break;		

	      default:
			ercd = -1;		// �f�[�^�ُ�
			break;
	}	
		
	return ercd;
}


/*==========================================================================*/
//	LED���ʐ��l�̏����l���M �����i�R�}���h013�̏����j
//	@return ��������/���s
/*==========================================================================*/
static UB rcv_cmd013( T_COMMSG *rcv_msg )
{
	UB ercd = E_OK;
	
	nop();

	
	irDuty2 = rcv_msg->buf[ 11 ];		// IR LED�̓_�������l
	irDuty3 = rcv_msg->buf[ 12 ];
	irDuty4 = rcv_msg->buf[ 13 ];
	irDuty5 = rcv_msg->buf[ 14 ];
	
	ini_irDuty2 = rcv_msg->buf[ 11 ];		// IR LED�̓_�������l���L��
	ini_irDuty3 = rcv_msg->buf[ 12 ];
	ini_irDuty4 = rcv_msg->buf[ 13 ];
	ini_irDuty5 = rcv_msg->buf[ 14 ];
	
	return ercd;
}


/*==========================================================================*/
//	�o�^���̏����l���M �����i�R�}���h020�̏����j
//	@return ��������/���s
/*==========================================================================*/
static UB rcv_cmd020( T_COMMSG *rcv_msg )
{
	UB ercd = E_OK;
	int i, j, cnt;
	
	nop();
	
	cnt = 11;
	
	for ( i=0; i<17; i++ ){
		kinkyuu_tel_no[ i ] 			= rcv_msg->buf[ cnt++ ];		// �ً}�J���d�b�ԍ�
	}

	yb_touroku_data.tou_no[ 0 ] 		= rcv_msg->buf[ cnt++ ];	// ���ԍ�
	yb_touroku_data.tou_no[ 1 ] 		= rcv_msg->buf[ cnt++ ];
	yb_touroku_data.tou_no[ 2 ]			= rcv_msg->buf[ cnt++ ];

	yb_touroku_data.user_id[ 0 ]		= rcv_msg->buf[ cnt++ ];	// ���[�U�[ID
	yb_touroku_data.user_id[ 1 ]		= rcv_msg->buf[ cnt++ ];
	yb_touroku_data.user_id[ 2 ]		= rcv_msg->buf[ cnt++ ];
	yb_touroku_data.user_id[ 3 ]		= rcv_msg->buf[ cnt++ ];
	yb_touroku_data.user_id[ 4 ]		= rcv_msg->buf[ cnt++ ];

/**	
	yb_touroku_data.tou_no[ 0 ] 		= rcv_msg->buf[ 11 ];	// ���ԍ�
	yb_touroku_data.tou_no[ 1 ] 		= rcv_msg->buf[ 12 ];
	yb_touroku_data.tou_no[ 2 ]			= ',';

	yb_touroku_data.user_id[ 0 ]		= rcv_msg->buf[ 14 ];	// ���[�U�[ID
	yb_touroku_data.user_id[ 1 ]		= rcv_msg->buf[ 15 ];
	yb_touroku_data.user_id[ 2 ]		= rcv_msg->buf[ 16 ];
	yb_touroku_data.user_id[ 3 ]		= rcv_msg->buf[ 17 ];
	yb_touroku_data.user_id[ 4 ]		= ',';

	yb_touroku_data.yubi_seq_no[ 0 ]	= rcv_msg->buf[ 19 ];	// �ӔC��/��ʎ҂̓o�^�w���
	yb_touroku_data.yubi_seq_no[ 1 ]	= rcv_msg->buf[ 20 ];
	yb_touroku_data.yubi_seq_no[ 2 ]	= rcv_msg->buf[ 21 ];
	yb_touroku_data.yubi_seq_no[ 3 ]	= ',';

	yb_touroku_data.kubun[ 0 ]			= rcv_msg->buf[ 23 ];	// �ӔC��/��ʎҋ敪�A�f�z���g�́h��ʎҁh
	yb_touroku_data.kubun[ 1 ]			= ',';

	yb_touroku_data.yubi_no[ 0 ]		= rcv_msg->buf[ 25 ];	// �o�^�w�ԍ��i�w��ʁj
	yb_touroku_data.yubi_no[ 1 ]		= rcv_msg->buf[ 26 ];
	yb_touroku_data.yubi_no[ 2 ]		= ',';


	for ( i=0; i<24; i++ ){
		yb_touroku_data.name[ i ] = rcv_msg->buf[ 28 + i ];	
	}
**/

//	cnt = 19;
	for( j = 0 ; j < 21 ; j++ ){	//�@��M�z���񂪁A�Q�O�񕪁��Q�P�񕪂ɕύX�B2013.7.15
		yb_touroku_data20[j].yubi_seq_no[ 0 ]	= rcv_msg->buf[ cnt++ ];	// �ӔC��/��ʎ҂̓o�^�w���
		yb_touroku_data20[j].yubi_seq_no[ 1 ]	= rcv_msg->buf[ cnt++ ];
		yb_touroku_data20[j].yubi_seq_no[ 2 ]	= rcv_msg->buf[ cnt++ ];
		yb_touroku_data20[j].yubi_seq_no[ 3 ]	= rcv_msg->buf[ cnt++ ];

		yb_touroku_data20[j].kubun[ 0 ]			= rcv_msg->buf[ cnt++ ];	// �ӔC��/��ʎҋ敪�A�f�z���g�́h��ʎҁh
		yb_touroku_data20[j].kubun[ 1 ]			= rcv_msg->buf[ cnt++ ];

		yb_touroku_data20[j].yubi_no[ 0 ]		= rcv_msg->buf[ cnt++ ];	// �o�^�w�ԍ��i�w��ʁj
		yb_touroku_data20[j].yubi_no[ 1 ]		= rcv_msg->buf[ cnt++ ];
		yb_touroku_data20[j].yubi_no[ 2 ]		= rcv_msg->buf[ cnt++ ];
		
		for ( i=0; i<24; i++ ){
			yb_touroku_data20[j].name[ i ] = rcv_msg->buf[ cnt++ ];	
		}
		++cnt;
	}

	// 2013.7.15�@�̎d�l�ύX�ɂ��A��M�����o�^�f�[�^�̂����A�Q�ԖڈȌ�̔z���񂩂�L���Ƃ��邱�ƂɂȂ����B
	yb_touroku_data.yubi_seq_no[ 0 ]	= yb_touroku_data20[ 1 ].yubi_seq_no[ 0 ];	// �ӔC��/��ʎ҂̓o�^�w���
	yb_touroku_data.yubi_seq_no[ 1 ]	= yb_touroku_data20[ 1 ].yubi_seq_no[ 1 ];
	yb_touroku_data.yubi_seq_no[ 2 ]	= yb_touroku_data20[ 1 ].yubi_seq_no[ 2 ];
	yb_touroku_data.yubi_seq_no[ 3 ]	= ',';

	yb_touroku_data.kubun[ 0 ]			= yb_touroku_data20[ 1 ].kubun[ 0 ];	// �ӔC��/��ʎҋ敪�A�f�z���g�́h��ʎҁh
	yb_touroku_data.kubun[ 1 ]			= ',';

	yb_touroku_data.yubi_no[ 0 ]		= yb_touroku_data20[ 1 ].yubi_no[ 0 ];	// �o�^�w�ԍ��i�w��ʁj
	yb_touroku_data.yubi_no[ 1 ]		= yb_touroku_data20[ 1 ].yubi_no[ 1 ];
	yb_touroku_data.yubi_no[ 2 ]		= ',';


	for ( i=0; i<24; i++ ){
		yb_touroku_data.name[ i ] = yb_touroku_data20[ 1 ].name[ i ];	
	}


	
	return ercd;
}


/*==========================================================================*/
/**
 *	�����o�^����M�R�}���h�������C��
 *	@return ��͐���/���s
 */
/*==========================================================================*/
static UB rcv_cmd_proc_init( T_COMMSG *rcv_msg )
{
	UB ercd;
	
	nop();

	if ( ( rcv_msg->buf[ 4 ] == '2' )			// �R�}���h211�Ȃ�A�w�o�^��OK/NG�̔���
	  && ( rcv_msg->buf[ 5 ] == '1' )
	  && ( rcv_msg->buf[ 6 ] == '1' ) ){
					  
		ercd = rcv_cmd211( rcv_msg );			// �R�}���h211�̏���
	
//	} else if ( ( rcv_msg->buf[ 4 ] == '2' )	// �R�}���h205�Ȃ�A�w�F�؂�OK/NG�̔���
//	 		 && ( rcv_msg->buf[ 5 ] == '0' )
//	  		 && ( rcv_msg->buf[ 6 ] == '5' ) ){
					  
//		ercd = rcv_cmd205( rcv_msg );			// �R�}���h205�̏���
	
	} else if ( ( rcv_msg->buf[ 4 ] == '2' )	// �R�}���h201�Ȃ�A�ʏ탂�[�h�؂�ւ��̏���
	 		 && ( rcv_msg->buf[ 5 ] == '0' )
	  		 && ( rcv_msg->buf[ 6 ] == '1' ) ){
					  
		MdCngMode( MD_NORMAL );					// �ʏ탂�[�h�؂�ւ�
		
		MdCngSubMode( SUB_MD_IDLE );			// �T�u���[�h���AIDLE�֐ݒ�
	
	} else if ( ( rcv_msg->buf[ 4 ] == '2' )	// �R�}���h206�Ȃ�A�w�f�[�^�̍ĎB�e����
	 		 && ( rcv_msg->buf[ 5 ] == '0' )
	  		 && ( rcv_msg->buf[ 6 ] == '6' ) ){
					  
		ercd = rcv_cmd206( rcv_msg );			// �R�}���h206�̏���
		
	} else if ( ( rcv_msg->buf[ 4 ] == '2' )	// �R�}���h207�Ȃ�A�J�����E�p�����[�^�̕ύX����
	 		 && ( rcv_msg->buf[ 5 ] == '0' )
	  		 && ( rcv_msg->buf[ 6 ] == '7' ) ){
					  
		ercd = rcv_cmd207( rcv_msg );			// �R�}���h207�̏���
		
	} else if ( ( rcv_msg->buf[ 4 ] == '2' )	// �R�}���h208�Ȃ�ALED���ʂ̕ύX����
	 		 && ( rcv_msg->buf[ 5 ] == '0' )
	  		 && ( rcv_msg->buf[ 6 ] == '8' ) ){
					  
		ercd = rcv_cmd208( rcv_msg );			// �R�}���h208�̏���
		
	} else if ( ( rcv_msg->buf[ 4 ] == '0' )	// �R�}���h002�Ȃ�A�菇A�̏ꍇ��OK/NG�̉�������
	 		 && ( rcv_msg->buf[ 5 ] == '0' )
	  		 && ( rcv_msg->buf[ 6 ] == '2' ) ){
					  
		if ( MdGetSubMode() != SUB_MD_IDLE ){
			ercd = rcv_cmd002( rcv_msg );			// �R�}���h002�̏���
		}

	}

	return ercd;
}


/*==========================================================================*/
/**
 *	�����e�i���X���[�h����M�R�}���h�������C��
 *	@return ��͐���/���s
 */
/*==========================================================================*/
static UB rcv_cmd_proc_meinte( T_COMMSG *rcv_msg )
{
	UB ercd;
	
	nop();
	
	if ( ( rcv_msg->buf[ 4 ] == '1' )			// �R�}���h101�Ȃ�A�����e�i���X�E���[�h�؂�ւ�����
	  && ( rcv_msg->buf[ 5 ] == '0' )
	  && ( rcv_msg->buf[ 6 ] == '1' ) ){
	  
		MdCngMode( MD_MAINTE );					// �����e�i���X�E���[�h�؂�ւ�
		
		MdCngSubMode( SUB_MD_IDLE );			// �T�u���[�h���AIDLE�֐ݒ�
		
	
	} else if ( ( rcv_msg->buf[ 4 ] == '2' )	// �R�}���h201�Ȃ�A�ʏ탂�[�h�؂�ւ��̏���
	 		 && ( rcv_msg->buf[ 5 ] == '0' )
	  		 && ( rcv_msg->buf[ 6 ] == '1' ) ){
					  
		MdCngMode( MD_NORMAL );					// �ʏ탂�[�h�؂�ւ�
		
		MdCngSubMode( SUB_MD_IDLE );			// �T�u���[�h���AIDLE�֐ݒ�
		
	} else if ( ( rcv_msg->buf[ 4 ] == '2' )	// �R�}���h211�Ȃ�A�w�t���摜���M���ʂ�OK/NG�̔���
	  		 && ( rcv_msg->buf[ 5 ] == '1' )	// �R�}���h211/�R�}���h205�̂ǂ���ŕԐM�����邩������Ȃ��ׁB
	  		 && ( rcv_msg->buf[ 6 ] == '1' ) ){
					  
		ercd = rcv_cmd211( rcv_msg );			// �R�}���h211�̏���
	
	} else if ( ( rcv_msg->buf[ 4 ] == '2' )	// �R�}���h205�Ȃ�A�w�t���摜���M���ʂ�OK/NG�̔���
	 		 && ( rcv_msg->buf[ 5 ] == '0' )	// �R�}���h211/�R�}���h205�̂ǂ���ŕԐM�����邩������Ȃ��ׁB
	  		 && ( rcv_msg->buf[ 6 ] == '5' ) ){
					  
		ercd = rcv_cmd205( rcv_msg );			// �R�}���h205�̏���

	} else if ( ( rcv_msg->buf[ 4 ] == '0' )	// �R�}���h002�Ȃ�A�菇A�̏ꍇ��OK/NG�̉�������
	 		 && ( rcv_msg->buf[ 5 ] == '0' )
	  		 && ( rcv_msg->buf[ 6 ] == '2' ) ){
					  
		if ( MdGetSubMode() != SUB_MD_IDLE ){
			ercd = rcv_cmd002( rcv_msg );			// �R�}���h002�̏���
		}

	}
	 
	return ercd;
}

/*==========================================================================*/
/**
 *	�p���[�I�t���[�h����M�R�}���h�������C��
 *	@return ��͐���/���s
 */
/*==========================================================================*/
static UB rcv_cmd_proc_power_off( T_COMMSG *rcv_msg )
{
	UB ercd;
	
	nop();

	if ( ( rcv_msg->buf[ 4 ] == '0' )	// �R�}���h002�Ȃ�A�菇A�̏ꍇ��OK/NG�̉�������
	 		 && ( rcv_msg->buf[ 5 ] == '0' )
	  		 && ( rcv_msg->buf[ 6 ] == '2' ) ){
					  
		if ( MdGetSubMode() != SUB_MD_IDLE ){
			ercd = rcv_cmd002( rcv_msg );			// �R�}���h002�̏���
		}

	}

	return ercd;
}


/*==========================================================================*/
/**
 *	�}�V����Ԃ�Ԃ��i���[�h�Q�Ɓj
 *	@return �}�V���̏��
 */
/*==========================================================================*/
static UB MdGetMode(void)
{
	return sys_Mode;
}

/*==========================================================================*/
/**
 *	���[�h�J��
 *	@param eNextMode ���̏��
 */
/*==========================================================================*/
static void MdCngMode( UINT eNextMode )
{
	// ���[�h�ύX�̑O����
	switch ( sys_Mode ){
	  case MD_POWER_ON:

	  	break;
	  case MD_NORMAL:				// �ʏ탂�[�h

		break;
	}
	
	// ���̃��[�h�ݒ�
	sys_Mode = eNextMode;				
	
	// ���[�h�ύX�̌㏈��
	switch ( sys_Mode ){
	  case MD_INITIAL:				// �����o�^���[�h
		break;		

	  case MD_MAINTE:				// �����e�i���X���[�h
		break;
		
	  case MD_NORMAL:				// �ʏ탂�[�h
		break;
	}
	
	// LED�\��
	LedOut( LED_ERR, LED_OFF );
	switch ( sys_Mode ) {
	case MD_PANIC:
		LedOut( LED_ERR,  LED_ON );
		break;
	}
}


/*==========================================================================*/
/**
 *	�}�V���T�u��Ԃ�Ԃ��i���[�h�Q�Ɓj
 *	@return �}�V���̏��
 */
/*==========================================================================*/
static UB MdGetSubMode(void)
{
	return sys_SubMode;
}

/*==========================================================================*/
/**
 *	�T�u���[�h�J��
 *	@param eNextMode ���̏��
 */
/*==========================================================================*/
static void MdCngSubMode( UINT eNextMode )
{
	// ���̃��[�h�ݒ�
	sys_SubMode = eNextMode;				
}

