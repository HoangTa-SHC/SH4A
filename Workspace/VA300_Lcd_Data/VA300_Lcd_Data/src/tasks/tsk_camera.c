/**
*	VA-300�v���O����
*
*	@file tsk_ctlmain.c
*	@version 1.00
*
*	@author Bionics Co.Ltd T.Nagai
*	@date   2013/04/20
*	@brief  VA-300 �J�����B�e�R���g���[���E�^�X�N
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

// �萔�錾
//#define CMR_WAIT_TIME	28				// �J������Wait����
#define CMR_WAIT_TIME	4				// �J������Wait����

#define CMR_SLEEP_CNTRL 1	// �J�����̃X���[�v���䂠��/�Ȃ��@1:����A0�F�Ȃ�
#define CMR_FREEZE_CNTRL 1	// �J�����̃t���[�Y���䂠��/�Ȃ��@1:����A0�F�Ȃ�
#define CAP_Debug_Test  0  // �摜�̃f�o�b�O�ƃe�X�g�@0:�e�X�g�Ȃ��@1:�e�X�g����

#define IR2_DUTY_DEFAULT	255			// �ԊO��LED��Duty�̃v���O���������l
#define IR3_DUTY_DEFAULT	255			// �ԊO��LED��Duty�̃v���O���������l
#define IR4_DUTY_DEFAULT	255			// �ԊO��LED��Duty�̃v���O���������l
#define IR5_DUTY_DEFAULT	0			// �ԊO��LED��Duty�̃v���O���������l

// �ϐ���`
static ID s_idTsk;

const UH cuhPrmShutter1 = 1;			// Fix Shutter Control�l�@�P��ځi�I�o�P�j �v���O�����Œ�`����Default�l
const UH cuhPrmShutter2 = 3;			// Fix Shutter Control�l�@�Q��ځi�I�o�Q�j �v���O�����Œ�`����Default�l
const UH cuhPrmShutter3 = 5;			// Fix Shutter Control�l�@�R��ځi�I�o�R�j �v���O�����Œ�`����Default�l

static UB g_ubCapBuf[ ( ( RE_SIZE_X * RE_SIZE_Y ) * 3 ) ];	///< �k���摜�擾�p�o�b�t�@
static UB g_ubCapBufRaw[ ( 1280 * 720 ) * 3 ];	///< �t���摜�擾�p�o�b�t�@
static UB g_ubCapBufTrm[ ( 800 * 400 ) * 3 ];	///< �g���~���O�摜�擾�p�o�b�t�@

static int iStartX, iStartY, iSizeX, iSizeY;// �L���v�`���摜�̃g���~���O�J�n���W�ƃT�C�Y
static int iReSizeX, iReSizeY;				// �k���摜��X�T�C�Y�AY�T�C�Y
static int iReSizeMode;						// �k���摜�̏k�����@0:��1/2�A1:��1/4�A2:��1/8

static RELTIM	dlytime;				// IR control wait�l

static UH cmrGain;						// �J�����E�Q�C���l�@PC����UDP�R�}���h�o�R�ŗ^����ꂽ�l���A�ʐM�̎w���ɏ]���ăJ�����ɐݒ肷��B
static UH cmrFixShutter1;				// Fix Shutter Control�l(�P���)�@���ۂ�FPGA�o�R�ŃJ�����R���g���[���ׂ̈ɎQ�Ƃ���l
static UH cmrFixShutter2;				// Fix Shutter Control�l(�Q���)			����
static UH cmrFixShutter3;				// Fix Shutter Control�l(�R��ځ��V���b�^�[�X�s�[�h�A�����l���󂯂��^�C�~���O�ŁA�J�����ɂ����ɐݒ肷��)�@����

static UH ini_cmrGain;					// �J�����E�Q�C���l�����l�@
static UH ini_cmrFixShutter1;			// Fix Shutter Control�l�����l(�P���)�@
static UH ini_cmrFixShutter2;			// Fix Shutter Control�l�����l(�Q���)			����
static UH ini_cmrFixShutter3;			// Fix Shutter Control�l�����l(�R��ځj

static UH cmr_wait_time = CMR_WAIT_TIME;// �J������Wait����

static UB irDuty2;						// IR Duty�lirDuty2;
static UB irDuty3;
static UB irDuty4;
static UB irDuty5;

static UB ini_irDuty2;					// IR Duty�lirDuty2�@�����l;
static UB ini_irDuty3;
static UB ini_irDuty4;
static UB ini_irDuty5;

static TMO trimTimeOut = 1000;			// �摜�̃g���~���O�̃^�C���A�E�g����
static TMO resizeTimeOut = 1000;		// �摜�̏k���E���k�̃^�C���A�E�g����

// �v���g�^�C�v�錾
static TASK CameraTask( void );		///< �J�����B�e�R���g���[���E�^�X�N

ER CmrCmdSleep(void);
ER CmrCmdWakeUp(void);
static ER CmrCmdFreezeOff(void);
static ER CmrCmdTest(void);
static ER CmrCmdFixShutterCtl( UB FSC_val );
static ER CmrCmdManualGainCtl( UB gain_val );
static ER CmrCmdLuminanceCtl( void );	//20130619_Miya �ǉ�
static ER SendCmd_204( UB *data, int len );
static ER SendCmd_210( UB *data, int len );
static ER SendCmd_141( UB *data, int len );
int Wait_Ack_and_Retry_forBlock( UB *snd_buff, int len );
static ER Wait_Ack_forBlock(void);
int	Cap_Resize_Picture( UINT command_no );
int	Cap_Raw_Picture( void );
void yb_init_capbuf( void );
void yb_init_all( void );		// �w�o�^���̃f�[�^������(all)

// �^�X�N�̒�`
const T_CTSK ctsk_camera    = { TA_HLNG, NULL, CameraTask,    5, 2048, NULL, (B *)"camera" };

static T_YBDATA yb_touroku_data;	// �w�o�^���i�P�w���j
static T_YBDATA20 yb_touroku_data20[21];	// �w�o�^���i20�w���j//�ǉ��@20130510 Miya

static UB kinkyuu_tel_no[17];		// �ً}�J���d�b�ԍ��P�U���i�z��ŏI�Ԗڂ͋�؂�L���h,�h�j
static UB kinkyuu_touroku_no[5];	// �ً}�J���ً̋}�o�^�ԍ��S���i�z��ŏI�Ԗڂ͋�؂�L���@NUL�@�j�@
static UB kinkyuu_hyouji_no[9];		// �ً}�J���ً̋}�ԍ��W���\���f�[�^�i�z��ŏI�Ԗڂ͋�؂�L���@NUL�@�j�@
static UB kinkyuu_kaijyo_no[9];		// �ً}�J���̊J���ԍ��W���f�[�^�i�z��ŏI�Ԗڂ͋�؂�L���@NUL�@�j
static UB kinkyuu_input_no[5];		// �ً}�J�����ɓ��͂��ꂽ�ԍ��S���i�z��ŏI�Ԗڂ͋�؂�L���@NUL�@�j

static UB mainte_password[5];		// �����e�i���X�E���[�h�ڍs���̊m�F�p�p�X���[�h�S���B�i�z��ŏI�Ԗڂ͋�؂�L���@NUL�@�j


/*==========================================================================*/
/*
 * �J�����B�e�R���g���[���E�^�X�N
 */
/*==========================================================================*/
TASK CameraTask( void )
{
	ER		ercd;
	FLGPTN	flgptn;
	int ercdStat;				// �G���[�R�[�h�L��

	cmrFixShutter1 = cuhPrmShutter1;	// Fix Shutter Control�l(�P���)
	cmrFixShutter2 = cuhPrmShutter2;	// Fix Shutter Control�l(�Q���)
	cmrFixShutter3 = cuhPrmShutter3;	// Fix Shutter Control�l(�R���)

	iStartX = TRIM_START_X;			// �L���v�`���摜�̃g���~���O�J�n���W�ƃT�C�Y
	iStartY = TRIM_START_Y;
	iSizeX = TRIM_SIZE_X;
	iSizeY = TRIM_SIZE_Y;
	
	iReSizeX = RE_SIZE_X;			// �g���~���O�摜�̏k���T�C�Y
	iReSizeY = RE_SIZE_Y;
	
	iReSizeMode = RSZ_MODE_1;		// ���k�摜�̏k����
	
	irDuty2 = IR2_DUTY_DEFAULT;		// IR LED�̓_�������l
	irDuty3 = IR3_DUTY_DEFAULT;
	irDuty4 = IR4_DUTY_DEFAULT;
	irDuty5 = IR5_DUTY_DEFAULT;

	// �����J�n
	
	
	/**	�J�����̃X���[�v�����A���̑�	***/
	ercd = CmrCmdTest();			// �J��������̉����`�F�b�N
	if ( ercd != E_OK ){
		ercdStat = 4;
		 ErrCodeSet( ercd );
	}
	
#if (CMR_SLEEP_CNTRL == 1)
	ercd = CmrCmdWakeUp();			// �J������WakeUP����
	if ( ercd != E_OK ){
		ercdStat = 5;
		 ErrCodeSet( ercd );
	}	
	dly_tsk( 1000/MSEC );

	ercd = CmrCmdSleep();			// �J�����̃X���[�v����
	if ( ercd != E_OK ){
		ercdStat = 5;
		 ErrCodeSet( ercd );
	}	
#endif
	
	CmrCmdLuminanceCtl();	//20130619_Miya �ǉ�

	
	for(;;) {
		ercd = wai_flg( ID_FLG_CAMERA, ( FPTN_START_CAP141
					  				   | FPTN_START_CAP204
					  				   | FPTN_START_CAP211
									   | FPTN_SETREQ_GAIN
									   | FPTN_SETREQ_SHUT1 ), TWF_ORW | TWF_CLR, &flgptn );	// �J�����B�e�v���̎�M�҂�
		if ( ercd != E_OK ){
			ercdStat = 6;
			break;
		}
		
		if ( flgptn == FPTN_START_CAP204 ){			// �F�ؗp�B�e�A�o�^�p�B�e�̏ꍇ
			
//			ercd = clr_flg( ID_FLG_CAMERA, ~( FPTN_START_CAP204 & 0x0fffffff ) );			// �t���O�̃N���A
//			if ( ercd != E_OK ){
//				ercdStat = 7;
//				break;
//			}
			
			MdCngMode( MD_CAP );					// ���u���[�h���L���v�`���[����
			ercdStat = Cap_Resize_Picture( COMMAND204 );		// �o�^�p�B�e����
			
			rxtid = rxtid_org;						// GET_UDP�ׂ̈ɖ{���̎�M�^�X�NID�itsk_rcv�j���ăZ�b�g
					
		
		} else if ( flgptn == FPTN_START_CAP211 ){			// �F�ؗp�B�e�̏ꍇ
			
//			ercd = clr_flg( ID_FLG_CAMERA, ~( FPTN_START_CAP211 & 0x0fffffff ) );			// �t���O�̃N���A
//			if ( ercd != E_OK ){
//				ercdStat = 7;
//				break;
//			}

			MdCngMode( MD_CAP );					// ���u���[�h���L���v�`���[����
			ercdStat = Cap_Resize_Picture( COMMAND210 );		// �F�ؗp�B�e����
			
			rxtid = rxtid_org;						// GET_UDP�ׂ̈ɖ{���̎�M�^�X�NID�itsk_rcv�j���ăZ�b�g

			
		} else if ( flgptn == FPTN_START_CAP141 ){	// ���摜�B�e�v���̏ꍇ
			
//			ercd = clr_flg( ID_FLG_CAMERA, ~( FPTN_START_CAP141 & 0x0fffffff ) );			// �t���O�̃N���A
//			if ( ercd != E_OK ){
//				ercdStat = 7;
//				break;
//			}

			ercdStat = Cap_Raw_Picture();			// ���摜�B�e����
			
			rxtid = rxtid_org;						// GET_UDP�ׂ̈ɖ{���̎�M�^�X�NID�itsk_rcv�j���ăZ�b�g

			
	    } else if ( flgptn == FPTN_SETREQ_GAIN ){			// �J�����E�Q�C���ݒ�v���̏ꍇ
			
//			ercd = clr_flg( ID_FLG_CAMERA, ~( FPTN_SETREQ_GAIN & 0x0fffffff ) );			// �t���O�̃N���A
//			if ( ercd != E_OK ){
//				ercdStat = 8;
//				break;
//			}
			
//			ercd = CmrCmdWakeUp();					// �J������WakeUP����
//			if ( ercd != E_OK ){
//		 		ErrCodeSet( ercd );
//			}	
//			dly_tsk( 1000/MSEC );

			ercd = CmrCmdManualGainCtl( ( UB )cmrGain );		// �J�����ɒ��ځA�Q�C���ݒ�l��ݒ�iFPGA���o�R���Ȃ��j
			if ( ercd != E_OK ){
			 	ErrCodeSet( ercd );
			}

//			ercd = CmrCmdSleep();					// �J�����̃X���[�v����
//			if ( ercd != E_OK ){
//				ErrCodeSet( ercd );
//			}	
			
		} else if ( flgptn == FPTN_SETREQ_SHUT1 ){			// �J�����E�V���b�^�[�X�s�[�h�P�ݒ�v���̏ꍇ
			
//			ercd = clr_flg( ID_FLG_CAMERA, ~( FPTN_SETREQ_SHUT1 & 0x0fffffff ) );			// �t���O�̃N���A
//			if ( ercd != E_OK ){
//				ercdStat = 8;
//				break;
//			}
			
//			ercd = CmrCmdWakeUp();					// �J������WakeUP����
//			if ( ercd != E_OK ){
//		 		ErrCodeSet( ercd );
//			}	
//			dly_tsk( 1000/MSEC );

			ercd = CmrCmdFixShutterCtl( ( UB )cmrFixShutter3 );	// �J�����ɒ��ځA�I�o�R�ݒ�l��ݒ�iFPGA���o�R���Ȃ��j
			if ( ercd != E_OK ){
				 ErrCodeSet( ercd );
			}
//			ercd = CmrCmdSleep();					// �J�����̃X���[�v����
//			if ( ercd != E_OK ){
//				ErrCodeSet( ercd );
//			}	
			
		}	 else {
		
		}
	}
}


//-----------------------------------------------------------------------------
// �F�ؗp�B�e�A�o�^�p�B�e����
//-----------------------------------------------------------------------------
int	Cap_Resize_Picture( UINT command_no ){
	
	int ercdStat = 0;
	ER ercd;
	T_YBDATA *ybdata;
		
//	ybdata = ( T_YBDATA * )yb_touroku_data;	
	ybdata = &yb_touroku_data;	
	
	yb_init_capbuf();					// �w���̏�����(�������ȊO)
		
	ercd = IrLedSet( IR_LED_2, irDuty2 );						// �ԊO��LED2�̊K���ݒ�
	if ( ercd != E_OK ){
		ercdStat = 8;
		return ercdStat;
	}
	ercd = IrLedSet( IR_LED_3, irDuty3 );						// �ԊO��LED3�̊K���ݒ�
	if ( ercd != E_OK ){
		ercdStat = 8;
		return ercdStat;
	}
	ercd = IrLedSet( IR_LED_4, irDuty4 );						// �ԊO��LED4�̊K���ݒ�
	if ( ercd != E_OK ){
		ercdStat = 8;
		return ercdStat;
	}
	ercd = IrLedSet( IR_LED_5, irDuty5 );						// �ԊO��LED5�̊K���ݒ�
	if ( ercd != E_OK ){
		ercdStat = 8;
		return ercdStat;
	}

//#if (CMR_SLEEP_CNTRL == 1)
//	ercd = CmrCmdWakeUp();			// �J������WakeUP����
//	if ( ercd != E_OK ){
//		ercdStat = 5;
//		 ErrCodeSet( ercd );
//	}		
//#endif

//	dlytime = ( 2000/MSEC );

	IrLedCrlOn( IR_LED_2 | IR_LED_3 | IR_LED_4 ); 				// �ԊO��LED�̑S�_��(LED2,3,4)
//	IrLedCrlOn( IR_LED_2 | IR_LED_3 | IR_LED_4 | IR_LED_5 ); 	// �ԊO��LED�̑S�_��
//	dly_tsk( dlytime );
				
	/** �J�����B�e����	**/
#if (CMR_FREEZE_CNTRL == 1)
	ercd = CmrCmdFreezeOff();									// �J�����E�t���[�Y�E�R���g���[���E�I�t
	if ( ercd != E_OK ){
		ercdStat = 9;
//		return ercdStat;
	}
#endif
	CmrPrmShutterSet( cmrFixShutter1 );							
	CmrWakeUpWaitCrl( cmr_wait_time );							// WakeUp���̃J������Wait���Ԑݒ�
		
	ercd = CmrCapture( CAP_MODE_1, CAP_BANK_0, (2000/MSEC) );	// �摜�̃L���v�`���[�A1����
	while( IsCmrCapStart() == TRUE ){							// �摜�L���v�`���[�̏I���҂�
		dly_tsk( 10/MSEC );
	}
	if ( ercd != E_OK ){
		while( ercd != E_OK ){
			ercd = CmrCapture( CAP_MODE_1, CAP_BANK_0, (2000/MSEC) );	// �摜�̃L���v�`���[�A
			while( IsCmrCapStart() == TRUE ){							// �摜�L���v�`���[�̏I���҂�
				dly_tsk( 10/MSEC );
			}
		}
	}
		
#if	(CAP_Debug_Test == 1)
	ercd = CmrCapGet( CAP_BANK_0, 0, 0, 1280, 720, &g_ubCapBufRaw[ 0 ] );	// �e�X�g�p�Ƀt���B�e�摜���擾
	if ( ercd != E_OK ){
		ercdStat = 16;
		return ercdStat;
	} 
#endif

	/** �L���v�`���摜�̃g���~���O	**/
	ercd = CmrTrim(CAP_BANK_0, TRIM_BANK_0, iStartX, iStartY, iSizeX, iSizeY, trimTimeOut ); // �P���ډ摜�̃g���~���O
	if ( ercd != E_OK ){
		ercdStat = 19;
		return ercdStat;
	}
	while ( IsCmrTrimStart() == TRUE ){
		dly_tsk( 10/MSEC );	
	}

#if	(CAP_Debug_Test == 1)
	ercd = CmrTrimGet( TRIM_BANK_0, 0, ( 640 * 320 ), &g_ubCapBufTrm[ 0 ] );	// �e�X�g�p�Ƀg���~���O�摜���擾�A�P����
	if ( ercd != E_OK ){
		ercdStat = 22;
		return ercdStat;
	}
#endif

	/** �g���~���O�摜�̏k���ƈ��k	**/
	ercd = CmrResize(TRIM_BANK_0, RSZ_BANK_0, RSZ_MODE_1, iReSizeX, iReSizeY, resizeTimeOut );	// �P���ډ摜�̏k���E���k
	if ( ercd != E_OK ){
		ercdStat = 25;
		return ercdStat;
	}
	while ( IsCmrResizeStart() == TRUE ){
		dly_tsk( 10/MSEC );
	}
		
	ercd = CmrResizeGet(RSZ_BANK_0, 0, ( 160 * 80 ), &g_ubCapBuf[ 0 ]);		// �P���ڏk���摜�̎擾
	if ( ercd != E_OK ){
		ercdStat = 28;
		return ercdStat;
	}


#if (CMR_FREEZE_CNTRL == 1)		
	ercd = CmrCmdFreezeOff();									// �J�����E�t���[�Y�E�R���g���[���E�I�t
	if ( ercd != E_OK ){
		ercdStat = 11;
//		return ercdStat;
	}
#endif		
	CmrPrmShutterSet( cmrFixShutter2 );
			
	ercd = CmrCapture( CAP_MODE_3, CAP_BANK_1, (2000/MSEC) );	// �摜�̃L���v�`���[�A2����
	while( IsCmrCapStart() == TRUE ){							// �摜�L���v�`���[�̏I���҂�
		dly_tsk( 10/MSEC );
	}
	if ( ercd != E_OK ){
		while( ercd != E_OK ){
			ercd = CmrCapture( CAP_MODE_3, CAP_BANK_1, (2000/MSEC) );	// �摜�̃L���v�`���[�A
			while( IsCmrCapStart() == TRUE ){							// �摜�L���v�`���[�̏I���҂�
				dly_tsk( 10/MSEC );
			}
		}
	}
		
#if	(CAP_Debug_Test == 1)
	ercd = CmrCapGet( CAP_BANK_1, 0, 0, 1280, 720, &g_ubCapBufRaw[ ( 1280 * 720 ) ] );	// �e�X�g�p�Ƀt���B�e�摜���擾
	if ( ercd != E_OK ){
		ercdStat = 17;
		return ercdStat;
	} 		
#endif

	ercd = CmrTrim(CAP_BANK_1, TRIM_BANK_1, iStartX, iStartY, iSizeX, iSizeY, trimTimeOut ); // �Q���ډ摜�̃g���~���O
	if ( ercd != E_OK ){
		ercdStat = 20;
		return ercdStat;
	}
	while ( IsCmrTrimStart() == TRUE ){
		dly_tsk( 10/MSEC );	
	}

#if	(CAP_Debug_Test == 1)
	ercd = CmrTrimGet( TRIM_BANK_1, 0, ( 640 * 320 ), &g_ubCapBufTrm[ ( 640 * 320 ) ] );	// �e�X�g�p�Ƀg���~���O�摜���擾�A�P����
	if ( ercd != E_OK ){
		ercdStat = 23;
		return ercdStat;
	}
#endif

	ercd = CmrResize(TRIM_BANK_1, RSZ_BANK_1, RSZ_MODE_1, iReSizeX, iReSizeY, resizeTimeOut );	// �Q���ډ摜�̏k���E���k
	if ( ercd != E_OK ){
		ercdStat = 26;
		return ercdStat;
	}
	while ( IsCmrResizeStart() == TRUE ){
		dly_tsk( 10/MSEC );
	}

	ercd = CmrResizeGet(RSZ_BANK_1, 0, ( 160 * 80 ), &g_ubCapBuf[( 160 * 80 )] );	// �Q���ڏk���摜�̎擾
	if ( ercd != E_OK ){
		ercdStat = 29;
		return ercdStat;
	}


#if (CMR_FREEZE_CNTRL == 1)
	ercd = CmrCmdFreezeOff();									// �J�����E�t���[�Y�E�R���g���[���E�I�t
	if ( ercd != E_OK ){
		ercdStat = 13;
//		return ercdStat;
	}
#endif
	CmrPrmShutterSet( cmrFixShutter3 );

	ercd = CmrCapture( CAP_MODE_3, CAP_BANK_2, (2000/MSEC) );	// �摜�̃L���v�`���[�A3����
	while( IsCmrCapStart() == TRUE ){							// �摜�L���v�`���[�̏I���҂�
		dly_tsk( 10/MSEC );
	}
	if ( ercd != E_OK ){
		while( ercd != E_OK ){
			ercd = CmrCapture( CAP_MODE_3, CAP_BANK_2, (2000/MSEC) );	// �摜�̃L���v�`���[�A
			while( IsCmrCapStart() == TRUE ){							// �摜�L���v�`���[�̏I���҂�
				dly_tsk( 10/MSEC );
			}
		}
	}
		
#if	(CAP_Debug_Test == 1)
	ercd = CmrCapGet( CAP_BANK_2, 0, 0, 1280, 720, &g_ubCapBufRaw[ ( 1280 * 720 ) * 2 ] );	// �e�X�g�p�Ƀt���B�e�摜���擾
	if ( ercd != E_OK ){
		ercdStat = 18;
		return ercdStat;
	} 
#endif

	ercd = CmrTrim(CAP_BANK_2, TRIM_BANK_2, iStartX, iStartY, iSizeX, iSizeY, trimTimeOut ); // �R���ډ摜�̃g���~���O
	if ( ercd != E_OK ){
		ercdStat = 21;
		return ercdStat;
	}
	while ( IsCmrTrimStart() == TRUE ){
		dly_tsk( 10/MSEC );	
	}

#if	(CAP_Debug_Test == 1)
	ercd = CmrTrimGet( TRIM_BANK_2, 0, ( 640 * 320 ), &g_ubCapBufTrm[ ( ( 640 * 320 ) * 2 ) ] );	// �e�X�g�p�Ƀg���~���O�摜���擾�A�P����
	if ( ercd != E_OK ){
		ercdStat = 24;
		return ercdStat;
	} 
#endif
	
	ercd = CmrResize(TRIM_BANK_2, RSZ_BANK_2, RSZ_MODE_1, iReSizeX, iReSizeY, resizeTimeOut );	// �R���ډ摜�̏k���E���k
	if ( ercd != E_OK ){
		ercdStat = 27;
		return ercdStat;
	}
	while ( IsCmrResizeStart() == TRUE ){
		dly_tsk( 10/MSEC );
	}

	ercd = CmrResizeGet(RSZ_BANK_2, 0, ( 160 * 80 ), &g_ubCapBuf[ ( ( 160 * 80 ) * 2 ) ] );	// �R���ڏk���摜�̎擾
	if ( ercd != E_OK ){
		ercdStat = 30;
		return ercdStat;
	}

	memcpy( &ybdata->ybCapBuf, &g_ubCapBuf, ( 160 * 80 * 3 ) );

#if (CMR_FREEZE_CNTRL == 1)	
	ercd = CmrCmdFreezeOff();									// �J�����E�t���[�Y�E�R���g���[���E�I�t
	if ( ercd != E_OK ){
		ercdStat = 14;
//		return ercdStat;
	}
#endif

#if (CMR_SLEEP_CNTRL == 1)		
	/**	�J�����̃X���[�v����	***/
	ercd = CmrCmdSleep();
	if ( ercd != E_OK ){
		ercdStat = 15;
		return ercdStat;
	}		
	/** 	��	�I��	**/
#endif

	/** IR LED�̏��������@**/
	IrLedCrlOff( IR_LED_2 | IR_LED_3 | IR_LED_4 | IR_LED_5 ); 	// �ԊO��LED�̑S����
		
	/** �J�����B�e�̏I�������@**/
	ercd = set_flg( ID_FLG_TS, FPTN_CMR_TS );					// ���̌��m�^�X�N�ցA�J�����B�e�I����ʒm
	if ( ercd != E_OK ){
		ercdStat = 31;
		return ercdStat;
	}

	/** �摜��UDP���M�^�X�N�֑���	**/
	if ( command_no == COMMAND204 ){
		ercd = SendCmd_204( ( UB* )ybdata, ( 42 + ( 160 * 80 * 3 ) + 2 ) );		// �R�}���h204�A�k���摜�i�������t���j
//		ercd = SendCmd_204( ( UB* )ybdata->ybCapBuf, ( ( 160 * 80 ) * 3 ) );		// �R�}���h204�A�k���摜�i�������Ȃ��j
		if ( ercd != E_OK ){
			ercdStat = 34;
//			return ercdStat;
		}
	} else if	( command_no == COMMAND210 ){
		ercd = SendCmd_210( ( UB* )ybdata->ybCapBuf, ( ( 160 * 80 ) * 3 ) + 2 );	// �R�}���h210�A�k���摜�R���i�������Ȃ��j
		if ( ercd != E_OK ){
			ercdStat = 34;
//			return ercdStat;
		}
	}

	/** �J�����B�e�̏I�������@**/
	ercd = set_flg( ID_FLG_TS, FPTN_END_TS );				// ���̌��m�^�X�N�ցA�J�����B�e�I����ʒm
	if ( ercd != E_OK ){
		ercdStat = 35;
		return ercdStat;
	}
	return ercdStat;
}

//-----------------------------------------------------------------------------
// ���摜�B�e����
//-----------------------------------------------------------------------------
int	Cap_Raw_Picture( void ){
	
	int ercdStat = 0;
	ER ercd;
		
	ercd = IrLedSet( IR_LED_2, irDuty2 );						// �ԊO��LED2�̊K���ݒ�
	if ( ercd != E_OK ){
		ercdStat = 8;
		return ercdStat;
	}
	ercd = IrLedSet( IR_LED_3, irDuty3 );						// �ԊO��LED3�̊K���ݒ�
	if ( ercd != E_OK ){
		ercdStat = 8;
		return ercdStat;
	}
	ercd = IrLedSet( IR_LED_4, irDuty4 );						// �ԊO��LED4�̊K���ݒ�
	if ( ercd != E_OK ){
		ercdStat = 8;
		return ercdStat;
	}
	ercd = IrLedSet( IR_LED_5, irDuty5 );						// �ԊO��LED5�̊K���ݒ�
	if ( ercd != E_OK ){
		ercdStat = 8;
		return ercdStat;
	}

#if (CMR_SLEEP_CNTRL == 1)
	ercd = CmrCmdWakeUp();			// �J������WakeUP����
	if ( ercd != E_OK ){
		ercdStat = 5;
		 ErrCodeSet( ercd );
	}		
#endif
	dlytime = ( 2000/MSEC );

	IrLedCrlOn( IR_LED_2 | IR_LED_3 | IR_LED_4 ); 				// �ԊO��LED�̑S�_��(LED2,3,4)
//	IrLedCrlOn( IR_LED_2 | IR_LED_3 | IR_LED_4 | IR_LED_5 ); 	// �ԊO��LED�̑S�_��
	dly_tsk( dlytime );
				
	/** �J�����B�e����	**/
#if (CMR_FREEZE_CNTRL == 1)
	ercd = CmrCmdFreezeOff();									// �J�����E�t���[�Y�E�R���g���[���E�I�t
	if ( ercd != E_OK ){
		ercdStat = 9;
//		return ercdStat;
	}
#endif
	CmrPrmShutterSet( cmrFixShutter1 );							
	CmrWakeUpWaitCrl( cmr_wait_time );							// WakeUp���̃J������Wait���Ԑݒ�
		
	ercd = CmrCapture( CAP_MODE_1, CAP_BANK_0, (2000/MSEC) );	// �摜�̃L���v�`���[�A1����
	while( IsCmrCapStart() == TRUE ){							// �摜�L���v�`���[�̏I���҂�
		dly_tsk( 10/MSEC );
	}
	if ( ercd != E_OK ){
		while( ercd != E_OK ){
			ercd = CmrCapture( CAP_MODE_1, CAP_BANK_0, (2000/MSEC) );	// �摜�̃L���v�`���[�A
			while( IsCmrCapStart() == TRUE ){							// �摜�L���v�`���[�̏I���҂�
				dly_tsk( 10/MSEC );
			}
		}
	}
		
	ercd = CmrCapGet( CAP_BANK_0, 0, 0, 1280, 720, &g_ubCapBufRaw[ 0 ] );	// �t���B�e�摜���擾
	if ( ercd != E_OK ){
		ercdStat = 16;
		return ercdStat;
	} 

#if (CMR_FREEZE_CNTRL == 1)		
	ercd = CmrCmdFreezeOff();									// �J�����E�t���[�Y�E�R���g���[���E�I�t
	if ( ercd != E_OK ){
		ercdStat = 11;
//		return ercdStat;
	}
#endif		

#if (CMR_SLEEP_CNTRL == 1)		
	/**	�J�����̃X���[�v����	***/
	ercd = CmrCmdSleep();
	if ( ercd != E_OK ){
		ercdStat = 15;
		return ercdStat;
	}		
	/** 	��	�I��	**/
#endif

Camera_End_Process:
	/** IR LED�̏��������@**/
	IrLedCrlOff( IR_LED_2 | IR_LED_3 | IR_LED_4 | IR_LED_5 ); 	// �ԊO��LED�̑S����
		
	/** �J�����B�e�̏I�������@**/
	ercd = set_flg( ID_FLG_TS, FPTN_CMR_TS );					// ���̌��m�^�X�N�ցA�J�����B�e�I����ʒm
	if ( ercd != E_OK ){
		ercdStat = 31;
		return ercdStat;
	}


	/** �摜��UDP���M�^�X�N�֑���	**/
	ercd = SendCmd_141( g_ubCapBufRaw, ( 1280 * 720 ) );		// �R�}���h141�A�t���摜 1����
	if ( ercd != E_OK ){
		ercdStat = 32;
//		return ercdStat;
	}

	/** �J�����B�e�̏I�������@**/
	ercd = set_flg( ID_FLG_TS, FPTN_END_TS );		// ���̌��m�^�X�N�ցA�J�����B�e�I����ʒm
	if ( ercd != E_OK ){
		ercdStat = 35;
		return ercdStat;
	}
	return ercdStat;
}


//-----------------------------------------------------------------------------
// �J�����R�}���h: �J���� Manual Gain Control
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
	
	ercd = CmrPktSend( cCmd, sizeof cCmd, iRecvSize );			// �R�}���h���M
	if (ercd == E_OK) {
		ercd = CmrPktRecv( cRetBuf, iRecvSize, (3000/MSEC));	// ������M
		if (ercd == E_OK) {
			if (((cRetBuf[ 0 ] & 0x7f) == cCmd[ 1 ]) && (cRetBuf[ 2 ] == cCmd[ 2 ]) && (cRetBuf[ 3 ] == 0x01)	// �������e�`�F�b�N
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
// �J�����R�}���h: �J���� Fix Shutter Control
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
	
	ercd = CmrPktSend( cCmd, sizeof cCmd, iRecvSize );				// �R�}���h���M
	if (ercd == E_OK) {
		ercd = CmrPktRecv( cRetBuf, iRecvSize, (3000/MSEC));	// ������M
		if (ercd == E_OK) {
			if (((cRetBuf[ 0 ] & 0x7f) == cCmd[ 1 ]) && (cRetBuf[ 2 ] == cCmd[ 2 ]) && (cRetBuf[ 3 ] == 0x01)	// �������e�`�F�b�N
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
// �J�����R�}���h: �J���� Sleep
//-----------------------------------------------------------------------------
ER CmrCmdSleep(void)
{
	const UB cCmd[] = { 0x00, 0x00, 0x18, 0x01, 0x00 };
	const int iRecvSize = 5;
	UB	cRetBuf[ 10 ];
	UB	*p;
	ER	ercd;
	
	ercd = CmrPktSend( cCmd, sizeof cCmd, iRecvSize );			// �R�}���h���M
	if (ercd == E_OK) {
		ercd = CmrPktRecv( cRetBuf, iRecvSize, (3000/MSEC));		// ������M
		if (ercd == E_OK) {
			if (((cRetBuf[ 0 ] & 0x7f) == cCmd[ 1 ]) && (cRetBuf[ 2 ] == cCmd[ 2 ]) && (cRetBuf[ 3 ] == cCmd[ 3 ])	// �������e�`�F�b�N
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
// �J�����R�}���h: �J���� WakeUp
//-----------------------------------------------------------------------------
ER CmrCmdWakeUp(void)
{
	const UB cCmd[] = { 0x00, 0x00, 0x18, 0x01, 0x01 };
	const int iRecvSize = 5;
	UB	cRetBuf[ 10 ];
	UB	*p;
	ER	ercd;
	
	ercd = CmrPktSend( cCmd, sizeof cCmd, iRecvSize );			// �R�}���h���M
	if (ercd == E_OK) {
		ercd = CmrPktRecv( cRetBuf, iRecvSize, (3000/MSEC));		// ������M
		if (ercd == E_OK) {
			if (((cRetBuf[ 0 ] & 0x7f) == cCmd[ 1 ]) && (cRetBuf[ 2 ] == cCmd[ 2 ]) && (cRetBuf[ 3 ] == cCmd[ 3 ])	// �������e�`�F�b�N
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
// �J�����R�}���h: �J���� Freeze Control OFF
//-----------------------------------------------------------------------------
static ER CmrCmdFreezeOff(void)
{
	const UB cCmd[] = { 0x00, 0x00, 0x28, 0x05, 0x08, 0x26, 0x00, 0x01, 0x02 };
	const int iRecvSize = 5;
	UB	cRetBuf[ 10 ];
	UB	*p;
	ER	ercd;
	
	ercd = CmrPktSend( cCmd, sizeof cCmd, iRecvSize );			// �R�}���h���M
	if (ercd == E_OK) {
		ercd = CmrPktRecv( cRetBuf, iRecvSize, (3000/MSEC));	// ������M
		if (ercd == E_OK) {
			if ((cRetBuf[ 0 ] & 0x7f) == cCmd[ 1 ] && (cRetBuf[ 2 ] == cCmd[ 2 ]) && (cRetBuf[ 3 ] == 0x01)	// �������e�`�F�b�N
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
// �J�����R�}���h�e�X�g : Request Response Command
//-----------------------------------------------------------------------------
static ER CmrCmdTest(void)
{
	char *cParm;
	const UB cCmd[] = { 0x00, 0x00, 0x10, 0x00 };
	const int iRecvSize = 5;
	UB	cRetBuf[ 10 ];
	UB	*p;
	ER	ercd;
	
	ercd = CmrPktSend( cCmd, sizeof cCmd, iRecvSize);	// �R�}���h���M
	if (ercd == E_OK) {
		ercd = CmrPktRecv( cRetBuf, iRecvSize, (5000 / MSEC));		// ������M
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
// �J�����R�}���h: �J���� Luminance Control
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
	
	ercd = CmrPktSend( cCmd, sizeof cCmd, iRecvSize );				// �R�}���h���M
	if (ercd == E_OK) {
		ercd = CmrPktRecv( cRetBuf, iRecvSize, (3000/MSEC));	// ������M
		if (ercd == E_OK) {
			if (((cRetBuf[ 0 ] & 0x7f) == cCmd[ 1 ]) && (cRetBuf[ 2 ] == cCmd[ 2 ]) && (cRetBuf[ 3 ] == 0x01)	// �������e�`�F�b�N
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
// �摜�f�[�^��UDP���M�i�w�摜�f�[�^�A�R�}���h�ԍ�204�j
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
	UB snd_data[ 1024 ];		// UDP ���M�f�[�^ �P�u���b�N
	int	send_cnt;
	ER ercd = E_OK;

	int escape;

	header[ 0 ] = 0x27;			//�@�w�b�_�@1Byte�@ASCII
	header[ 1 ] = 0;			//�@�f�[�^���@�Q�o�C�gBinary
	header[ 2 ] = 0;
	header[ 3 ] = 0x31;			//�@���M����ʁ@1Byte�@ASCII
	header[ 4 ] = '2';			//�@�R�}���h�ԍ��@�R��ASCII
	header[ 5 ] = '0';
	header[ 6 ] = '4';
	header[ 7 ] = '0';			//�@�u���b�N�ԍ� �S��ASCII
	header[ 8 ] = '0';
	header[ 9 ] = '0';
	header[ 10 ] = '0';
	header[ 11 ] = 0;	

/**		2013.6.27 Commented Out By T.N
	//���΍� 20130510 Miya  PC���ŉ����F��or�o�^�F�؂̋�ʂ����Ȃ�����
	if( (GetScreenNo() == LCD_SCREEN8) || (GetScreenNo() == LCD_SCREEN129) ){
		header[ 3 ] = 0x32;			//�@���M����ʁ@1Byte�@ASCII
	}
**/

	len_rest = len;

	// �ő�u���b�N���̌v�Z	
	if ( len > 1024 -13 )
	{
		max_blk_num = len / ( 1024 - 13 );
	}	else	{
		max_blk_num = 0;	
	}
	blk_num = max_blk_num;
	
	// �u���b�N���̏���	
	while( blk_num >= 0 )
	{
		// �f�[�^���̌v�Z�@�Ɗi�[	
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

		// �u���b�N�ԍ��̌v�Z�@�Ɗi�[	
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

		/** �u���b�N�̑��M����		**/
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
		
		// �����擪�u���b�N�Ȃ�B
		if ( blk_num == max_blk_num ){
		// ��x�ڂ̔F�؃f�[�^�^��x�ڂ̔F�؃f�[�^���̋敪�����r����
			if ( ( GetScreenNo() == LCD_SCREEN6 ) || ( GetScreenNo() == LCD_SCREEN121 )	// ���ځH
		      || ( GetScreenNo() == LCD_SCREEN127 ) || ( GetScreenNo() == LCD_SCREEN141 ) ){
			  
				snd_data[ 11 ] = '1';			//�@���M����ʁ@1Byte�@ASCII
			
			} else if ( ( GetScreenNo() == LCD_SCREEN8 ) || ( GetScreenNo() == LCD_SCREEN101 )	// ���ځH
		      || ( GetScreenNo() == LCD_SCREEN102 ) || ( GetScreenNo() == LCD_SCREEN129 ) ){
			  			  
				snd_data[ 11 ] = '2';			//�@1Byte�@ASCII
			
			} else {
				snd_data[ 11 ] = ' ';			//�@1Byte�@ASCII	�����ɗ���������ُ�B		
			}
			snd_data[ 12 ] = ',';				//�@���M����ʁ@1Byte�@ASCII
			
			for ( i=0; i<=cmd_len-13; i++ ){	//�@�摜�f�[�^�̃R�}���h�Z�b�g�B
			
				snd_data[ 13+i ] = data[ ( ( max_blk_num - blk_num ) * ( 1024 - 13 ) ) + i ];
			}
			
		}	else { // �����擪�u���b�N�łȂ��Ȃ�B
			for ( i=0; i<=cmd_len-11; i++ ){	//�@�摜�f�[�^�̃R�}���h�Z�b�g�B
			
				snd_data[ 11+i ] = data[ ( ( max_blk_num - blk_num ) * ( 1024 - 13 ) ) + i ];
			}
				
		}		  
		snd_data[ cmd_len ] = CODE_CR;
		snd_data[ cmd_len + 1 ] = CODE_LF;

		// �P�u���b�N�̃f�[�^���M	
		SendBinaryData( &snd_data, ( cmd_len + 2 ) );		// �P�u���b�N�̃f�[�^���M

		escape = Wait_Ack_and_Retry_forBlock( snd_data, ( cmd_len + 2 ) );// �u���b�N�f�[�^���M���Ack�����҂��ƁA���g���C���̃u���b�N�]��
		if ( escape != 0 ){
			ercd = E_OBJ;
			break;				// TimeOut�ȊO�̃V�X�e���G���[�i�u���b�N�]�����f
		}

		--blk_num;								// �c��̃u���b�N��
		len_rest = len_rest - ( 1024 - 13 );	// �c��̑��M�f�[�^��

		header[ 7 ] = '0';			//�@�u���b�N�ԍ� �S��ASCII�̍ď�����
		header[ 8 ] = '0';
		header[ 9 ] = '0';
		header[ 10 ] = '0';

	}
	
	return ercd;
}


//-----------------------------------------------------------------------------
// �摜�f�[�^��UDP���M�i�w�摜�f�[�^�A�R�}���h�ԍ�210�j
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
	UB snd_data[ 1024 ];		// UDP ���M�f�[�^ �P�u���b�N
	int	send_cnt;
	ER ercd = E_OK;

	int escape;

	header[ 0 ] = 0x27;			//�@�w�b�_�@1Byte�@ASCII
	header[ 1 ] = 0;			//�@�f�[�^���@�Q�o�C�gBinary
	header[ 2 ] = 0;
	header[ 3 ] = 0x31;			//�@���M����ʁ@1Byte�@ASCII
	header[ 4 ] = '2';			//�@�R�}���h�ԍ��@�R��ASCII
	header[ 5 ] = '1';
	header[ 6 ] = '0';
	header[ 7 ] = '0';			//�@�u���b�N�ԍ� �S��ASCII
	header[ 8 ] = '0';
	header[ 9 ] = '0';
	header[ 10 ] = '0';
	header[ 11 ] = 0;	

/**		2013.6.27 Commented Out By T.N
	//���΍� 20130510 Miya  PC���ŉ����F��or�o�^�F�؂̋�ʂ����Ȃ�����
	if(GetScreenNo() != LCD_SCREEN101){
		header[ 3 ] = 0x32;			//�@���M����ʁ@1Byte�@ASCII
	}
**/

	len_rest = len;

	// �ő�u���b�N���̌v�Z	
	if ( len > 1024 -13 )
	{
		max_blk_num = len / ( 1024 - 13 );
	}	else	{
		max_blk_num = 0;	
	}
	blk_num = max_blk_num;
	
	// �u���b�N���̏���	
	while( blk_num >= 0 )
	{
		// �f�[�^���̌v�Z�@�Ɗi�[	
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

		// �u���b�N�ԍ��̌v�Z�@�Ɗi�[	
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

		/** �u���b�N�̑��M����		**/
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

		// �����擪�u���b�N�Ȃ�B
		if ( blk_num == max_blk_num ){
		// �o�^�p�̔F�؃f�[�^�^�����p�̔F�؃f�[�^���̋敪�����r����
			if ( ( GetScreenNo() == LCD_SCREEN101 ) || ( GetScreenNo() == LCD_SCREEN102 ) ){	// �����p�H
			  
				snd_data[ 11 ] = '1';			//�@1Byte�@ASCII
			
			} else {		//�@��L�ȊO�́A�S�ēo�^�A�܂��͓o�^�m�F�p
				snd_data[ 11 ] = '0';			//�@1Byte�@ASCII		
			}
			snd_data[ 12 ] = ',';

			for ( i=0; i<=cmd_len-13; i++ ){	//�@�摜�f�[�^�̃R�}���h�Z�b�g�B
			
				snd_data[ 13+i ] = data[ ( ( max_blk_num - blk_num ) * ( 1024 - 13 ) ) + i ];
			}
			
		}	else { // �����擪�u���b�N�łȂ��Ȃ�B
			for ( i=0; i<=cmd_len-11; i++ ){	//�@�摜�f�[�^�̃R�}���h�Z�b�g�B
			
				snd_data[ 11+i ] = data[ ( ( max_blk_num - blk_num ) * ( 1024 - 13 ) ) + i ];
			}
				
		}
		snd_data[ cmd_len ] = CODE_CR;
		snd_data[ cmd_len + 1 ] = CODE_LF;

		// �P�u���b�N�̃f�[�^���M	
		SendBinaryData( &snd_data, ( cmd_len + 2 ) );		// �P�u���b�N�̃f�[�^���M

		escape = Wait_Ack_and_Retry_forBlock( snd_data, ( cmd_len + 2 ) );// �u���b�N�f�[�^���M���Ack�����҂��ƁA���g���C���̃u���b�N�]��
		if ( escape != 0 ){
			ercd = E_OBJ;
			break;				// TimeOut�ȊO�̃V�X�e���G���[�i�u���b�N�]�����f
		}

		--blk_num;								// �c��̃u���b�N��
		len_rest = len_rest - ( 1024 - 13 );	// �c��̑��M�f�[�^��

		header[ 7 ] = '0';			//�@�u���b�N�ԍ� �S��ASCII�̍ď�����
		header[ 8 ] = '0';
		header[ 9 ] = '0';
		header[ 10 ] = '0';

	}
	
	return ercd;
}


//-----------------------------------------------------------------------------
// �摜�f�[�^��UDP���M�i�w�摜�f�[�^�A�R�}���h�ԍ�210�j
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
	UB snd_data[ 1024 ];		// UDP ���M�f�[�^ �P�u���b�N
	int	send_cnt;
	ER ercd = E_OK;

	int escape;

	header[ 0 ] = 0x27;			//�@�w�b�_�@1Byte�@ASCII
	header[ 1 ] = 0;			//�@�f�[�^���@�Q�o�C�gBinary
	header[ 2 ] = 0;
	header[ 3 ] = 0x31;			//�@���M����ʁ@1Byte�@ASCII
	header[ 4 ] = '1';			//�@�R�}���h�ԍ��@�R��ASCII
	header[ 5 ] = '4';
	header[ 6 ] = '1';
	header[ 7 ] = '0';			//�@�u���b�N�ԍ� �S��ASCII
	header[ 8 ] = '0';
	header[ 9 ] = '0';
	header[ 10 ] = '0';
	header[ 11 ] = 0;	

	len_rest = len;

	// �ő�u���b�N���̌v�Z	
	if ( len > 1024 -13 )
	{
		max_blk_num = len / ( 1024 - 13 );
	}	else	{
		max_blk_num = 0;	
	}
	blk_num = max_blk_num;
	
	// �u���b�N���̏���	
	while( blk_num >= 0 )
	{
		// �f�[�^���̌v�Z�@�Ɗi�[	
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

		// �u���b�N�ԍ��̌v�Z�@�Ɗi�[	
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

		/** �u���b�N�̑��M����		**/
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

		// �P�u���b�N�̃f�[�^���M	
		SendBinaryData( &snd_data, ( cmd_len + 2 ) );		// �P�u���b�N�̃f�[�^���M

		escape = Wait_Ack_and_Retry_forBlock( snd_data, ( cmd_len + 2 ) );// �u���b�N�f�[�^���M���Ack�����҂��ƁA���g���C���̃u���b�N�]��
		if ( escape != 0 ){
			ercd = E_OBJ;
			break;				// TimeOut�ȊO�̃V�X�e���G���[�i�u���b�N�]�����f
		}

		--blk_num;								// �c��̃u���b�N��
		len_rest = len_rest - ( 1024 - 13 );	// �c��̑��M�f�[�^��

		header[ 7 ] = '0';			//�@�u���b�N�ԍ� �S��ASCII�̍ď�����
		header[ 8 ] = '0';
		header[ 9 ] = '0';
		header[ 10 ] = '0';

	}
	CmmSendEot();				// EOT�𑗐M���ău���b�N���M���I���B
	
	return ercd;
}


//-----------------------------------------------------------------------------
// �u���b�N�f�[�^���M���Ack�����҂��ƁA���g���C���̃u���b�N�]��
//-----------------------------------------------------------------------------
int Wait_Ack_and_Retry_forBlock( UB *snd_buff, int len ){
	
	int err = 0;
	int send_cnt = 3;
	ER ercd;
	
	while( send_cnt >= 0 ){
			
		ercd = Wait_Ack_forBlock();			// �u���b�N�f�[�^���M���Ack�����҂�

		if ( ercd == E_OK ){
			err = 0;
			break;							// Ack����������΁A���̃u���b�N�]���ցB
				
		}	else if ( ercd == E_TMOUT ){
			// �P�b�@Ack�����Ȃ��̏ꍇ�A�đ��R��
			if ( send_cnt >= 1 ){
				SendBinaryData( snd_buff, len );		// �P�u���b�N�̃f�[�^���đ��M
			}	else	{
				// EOT���M		
				//CmmSendEot();				// �đ��R��ŁA�������̏ꍇ�AEOT�𑗐M���ău���b�N���M���I���B
				err = 1;
				break;
			}

		} else if ( ercd == CODE_NACK ){	// Nack����������΁A�����u���b�N���đ��B
			if ( send_cnt >= 0 ){
				SendBinaryData( snd_buff, len );		// �P�u���b�N�̃f�[�^���đ��M
				send_cnt = 3;				// �đ��J�E���^���A�R��ɖ߂��B
				err = 0;
				continue;
			}	else	{
				// EOT���M		
				// CmmSendEot();				// �đ��R���ŁANack������M�̏ꍇ�AEOT�𑗐M���ău���b�N���M���I���B
				err = 1;					// ���蓾�Ȃ��H
				break;
			}
			
		} else	{
			err = 1;						// TimeOut�ȊO�̃V�X�e���G���[
			break;				
		}

		--send_cnt;							// �đ��J�E���^���f�N�������g�B
			
	}
	return err;
}


//-----------------------------------------------------------------------------
// �u���b�N�f�[�^���M���Ack�����҂�
//-----------------------------------------------------------------------------
static ER Wait_Ack_forBlock( void )
{
	int Fcnt = 0;
	ER ercd, err_code;
	UB buff[ 10 ], code;
	
	rxtid = vget_tid();		// get_udp �ׂ̈ɁA���́i���݂́j�����ݑ҂��^�X�NID���Z�b�g
	
	for(;;){
		err_code = get_udp( (UB*)&code, 1000/MSEC);// ��M�f�[�^�҂�
		if ( err_code == E_OK ){
			buff[ Fcnt ] = code;
			Fcnt++;
			if ( Fcnt >= 3){
				if ( ( buff[0] == CODE_ACK ) &&
					 ( buff[1] == CODE_CR ) &&
					 ( buff[2] == CODE_LF) ){
					ercd = E_OK;				// Ack������M
					Fcnt = 0;
					break;
					
				} else if 	 ( ( buff[0] == CODE_NACK ) &&
					 ( buff[1] == CODE_CR ) &&
					 ( buff[2] == CODE_LF) ){
					ercd = CODE_NACK;			// Nack������M
					Fcnt = 0;
					break;
				
				}	else {
					ercd = E_OBJ;				// ���̑��̎�M�f�[�^
					Fcnt = 0;
					//continue;
					break;
				}
				continue;
			}
			continue;
			
		}	else if ( err_code == E_TMOUT ){
			Fcnt = 0;
			ercd = E_TMOUT;						// Ack������M��TimeOut
			break;

		}	else	{
			Fcnt = 0;
			ercd = err_code;					// Ack�����ł��ATimeOut�ł��A���̑��̎�M�f�[�^�ł��Ȃ��ꍇ
			break;
		}
	}
	return ercd;
}

//-----------------------------------------------------------------------------
// �w�o�^���̃f�[�^������(�摜���)
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
// �w�o�^���̃f�[�^������(all)
//-----------------------------------------------------------------------------
void yb_init_all( void ){
	
	int i;
	
	for ( i=0; i<16; i++ ){
		kinkyuu_tel_no[ i ] 			= ' ';		// �ً}�J���d�b�ԍ�
	}
	kinkyuu_tel_no[ 16 ] 				= ',';
	
	memset( &yb_touroku_data, 0, ( 42 + ( 160 * 80 * 3 ) ) );
	
	yb_touroku_data.tou_no[ 0 ] 		= '0';		// ���ԍ�
	yb_touroku_data.tou_no[ 1 ] 		= '0';
	yb_touroku_data.tou_no[ 2 ] 		= ',';

	yb_touroku_data.user_id[ 0 ]		= '0';		// ���[�U�[ID
	yb_touroku_data.user_id[ 1 ]		= '0';
	yb_touroku_data.user_id[ 2 ]		= '0';
	yb_touroku_data.user_id[ 3 ]		= '0';
	yb_touroku_data.user_id[ 4 ]		= ',';

	yb_touroku_data.yubi_seq_no[ 0 ]	= '0';		// �ӔC��/��ʎ҂̓o�^�w���
	yb_touroku_data.yubi_seq_no[ 1 ]	= '0';
	yb_touroku_data.yubi_seq_no[ 2 ]	= '0';
	yb_touroku_data.yubi_seq_no[ 3 ]	= ',';

	yb_touroku_data.kubun[ 0 ]			= '2';		// �ӔC��/��ʎҋ敪�A�f�z���g�́h��ʎҁh
	yb_touroku_data.kubun[ 1 ]			= ',';

	yb_touroku_data.yubi_no[ 0 ]		= '0';		// �o�^�w�ԍ�
	yb_touroku_data.yubi_no[ 1 ]		= '0';
	yb_touroku_data.yubi_no[ 2 ]		= ',';

	yb_touroku_data.name[ 24 ]			= ',';		// �o�^�҂̖��O�A�f�z���g��NULL
	
	
	kinkyuu_touroku_no[0]				= ' ';		// �ً}�J���ً̋}�o�^�ԍ��S���i�z��ŏI�Ԗڂ͋�؂�L���@NUL�@�j
	kinkyuu_touroku_no[1]				= ' ';
	kinkyuu_touroku_no[2]				= ' ';
	kinkyuu_touroku_no[3]				= ' ';
	kinkyuu_touroku_no[4]				= 0;

	kinkyuu_hyouji_no[0]				= ' ';		// �ً}�J���ً̋}�ԍ��W���\���f�[�^�i�z��ŏI�Ԗڂ͋�؂�L���@NUL�@�j
	kinkyuu_hyouji_no[1]				= ' ';	
	kinkyuu_hyouji_no[2]				= ' ';	
	kinkyuu_hyouji_no[3]				= ' ';	
	kinkyuu_hyouji_no[4]				= ' ';	
	kinkyuu_hyouji_no[5]				= ' ';	
	kinkyuu_hyouji_no[6]				= ' ';	
	kinkyuu_hyouji_no[7]				= ' ';	
	kinkyuu_hyouji_no[8]				= 0;

	kinkyuu_kaijyo_no[0]				= ' ';		// �ً}�J���̊J���ԍ��W���f�[�^�i�z��ŏI�Ԗڂ͋�؂�L���@NUL�@�j
	kinkyuu_kaijyo_no[1]				= ' ';	
	kinkyuu_kaijyo_no[2]				= ' ';
	kinkyuu_kaijyo_no[3]				= ' ';	
	kinkyuu_kaijyo_no[4]				= ' ';	
	kinkyuu_kaijyo_no[5]				= ' ';	
	kinkyuu_kaijyo_no[6]				= ' ';	
	kinkyuu_kaijyo_no[7]				= ' ';		
	kinkyuu_kaijyo_no[8]				= 0;
	
	mainte_password[0]					= ' ';		// �����e�i���X�E���[�h�ڍs���̊m�F�p�p�X���[�h�S���B�i�z��ŏI�Ԗڂ͋�؂�L���@NUL�@�j
	mainte_password[1]					= ' ';
	mainte_password[2]					= ' ';
	mainte_password[3]					= ' ';
	mainte_password[4]					= 0;

}

/*==========================================================================*/
/**
 * �J�����B�e�R���g���[���E�^�X�N������
 *
 * @param idTsk �^�X�NID
 * @retval E_OK ����N��
 */
/*==========================================================================*/
ER CameraTaskInit(ID idTsk)
{
	ER ercd;
	
	// �^�X�N�̐���
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
	
	// �^�X�N�̋N��
	ercd = sta_tsk(s_idTsk, 0);
	
	return ercd;
}

