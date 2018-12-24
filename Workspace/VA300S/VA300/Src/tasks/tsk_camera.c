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
#include "drv_led.h"

#include "va300.h"

#define SAMEIMGCHK	0	//20130115Miya

// �萔�錾
//#define CMR_WAIT_TIME	28				// �J������Wait����
#define CMR_WAIT_TIME	8				// �J������Wait����

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

//static UB g_ubCapBuf[ ( ( RE_SIZE_X * RE_SIZE_Y ) * 3 ) ];	///< �k���摜�擾�p�o�b�t�@
//static UB g_ubCapBuf[ ( ( 160 * 140 ) * 3 ) ];//20131210Miya cng	///< �k���摜�擾�p�o�b�t�@
static UB g_ubCapBuf[ 640 * 480 ];	//20160601Miya

#if	(VA300S == 0)
static UB g_ubCapBufRaw[ ( 1280 * 720 ) ];	///< �t���摜�擾�p�o�b�t�@
#endif


static UB g_ubHdrBuf[ 160 * 80 ];
//static UB g_ubResizeBuf[ 80 * 40 ];
//static UB g_ubResizeSvBuf[ 80 * 40 ];	//20140905Miya LBP�ǉ�
static UB g_ubResizeBuf[ 100 * 40 ];	//20140910Miya XSFT
static UB g_ubResizeSvBuf[ 100 * 40 ];	//20140910Miya XSFT //20140905Miya LBP�ǉ�
static UB g_ubSobelR1Buf[ 80 * 40 ];
static UB g_ubSobelR2Buf[ 80 * 40 ];
static UB g_ubSobelR3Buf[ 20 * 10 ];
static UB g_ubSobelR1SvBuf[ 80 * 40 ];
static UB g_ubSobelR2SvBuf[ 80 * 40 ];
static UB g_ubSobelR3SvBuf[ 20 * 10 ];
static unsigned short g_hdr_blend[ 160 * 80 ];
static UB g_ubLbpBuf[ 80 * 40 ];	//20140905Miya LBP�ǉ�
static UB g_ubResizeSvBuf2[ 80 * 40 ];	//20140905Miya LBP�ǉ�
static UB g_ubSobelR3DbgBuf[ 20 * 10 ];	//20160312Miya �ɏ����xUP

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

//static UH cmr_wait_time = CMR_WAIT_TIME;// �J������Wait����
static UH cmr_wait_time;// �J������Wait����

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

static int g_CmrParaSet;				//�J�����p�����[�^�ݒ�t���O
static UB g_IrLed_SW;					//20160312Miya �ɏ����xUP


// �v���g�^�C�v�錾
static TASK CameraTask( void );		///< �J�����B�e�R���g���[���E�^�X�N

static unsigned short CmrCmdManualGetParameter( UB para );
ER CmrCmdSleep(void);
ER CmrCmdWakeUp(char sw);
static ER CmrCmdFreezeOff(void);
static ER CmrCmdTest(void);
static ER CmrCmdFixShutterCtl( UB FSC_val );
static ER CmrCmdManualGainCtl( UB gain_val );
static ER CmrCmdLuminanceCtl( void );	//20130619_Miya �ǉ�
static int CapMinimum(void);
static int CapBantam(int hantei);
static int CapMidle(void);
static int CapHeavy(void);
static int IrLedOnOffSet(int sw, UH duty2, UH duty3, UH duty4, UH duty5);	//20160711Miya IR-LED�_�������^�C�~���O�ύX


#if ( VA300S == 0 || VA300S == 2)
static ER SendCmd_204( UB *data, int len );
static ER SendCmd_210( UB *data, int len );
static ER SendCmd_141( UB *data, int len );
int Wait_Ack_and_Retry_forBlock( UB *snd_buff, int len );
static ER Wait_Ack_forBlock(void);
#endif

#if ( VA300S == 1 || VA300S == 2)
static ER SendNinshou_204( UB *data, int len );
static ER SendNinshou_210( UB *data, int len );
#endif

#if ( VA300S == 2 ) 
static void DebugSendCmd_210( void );
#endif

#if ( NEWCMR == 1 )
static ER NCmr_GenericWrt(unsigned char addr, unsigned char dat);
static ER NCmr_PowerOn( void );	// ���{�P�~�R���ЃJ�����@NCM03-V��power on����
static ER NCmr_WrtIniPara( void );
static ER NCmr_WrtExpPara( unsigned char datH, unsigned char datM, unsigned char datL );
static ER NCmr_WrtGmma10( void );
static ER NCmr_WrtAutoExpPara( void );
static ER NCmr_ReadIniPara( void );
static ER NCmr_ReadGmma( void );
static ER NCmr_ReadPara( int bnk );									// �J�����p�����[�^�m�F��I2C���M //20160601Miya
unsigned long NCmr_CalExpTime(int mode, unsigned long st);
static int g_cmr_dbgcnt;
unsigned long g_exptime;
#endif

int	Cap_Resize_Picture( UINT command_no );
int CapImgSeq(UINT command_no, UB hantei);	//20151118Miya ���摜�ĎB�e
//int	Cap_Raw_Picture( void );
int	Check_Cap_Raw_Picture( void );
void yb_init_capbuf( void );
void yb_init_all( void );		// �w�o�^���̃f�[�^������(all)

// �^�X�N�̒�`
//const T_CTSK ctsk_camera    = { TA_HLNG, NULL, CameraTask,    5, 2048, NULL, (B *)"camera" };
const T_CTSK ctsk_camera    = { TA_HLNG, NULL, CameraTask,    3, 2048, NULL, (B *)"camera" };

static T_YBDATA yb_touroku_data;	// �w�o�^���i�P�w���j
static T_YBDATA20 yb_touroku_data20[21];	// �w�o�^���i20�w���j//�ǉ��@20130510 Miya

static UB kinkyuu_tel_no[17];		// �ً}�J���d�b�ԍ��P�U���i�z��ŏI�Ԗڂ͋�؂�L���h,�h�j
static UB kinkyuu_touroku_no[5];	// �ً}�J���ً̋}�o�^�ԍ��S���i�z��ŏI�Ԗڂ͋�؂�L���@NUL�@�j�@
static UB kinkyuu_hyouji_no[9];		// �ً}�J���ً̋}�ԍ��W���\���f�[�^�i�z��ŏI�Ԗڂ͋�؂�L���@NUL�@�j�@
static UB kinkyuu_kaijyo_no[9];		// �ً}�J���̊J���ԍ��W���f�[�^�i�z��ŏI�Ԗڂ͋�؂�L���@NUL�@�j
static UB kinkyuu_input_no[5];		// �ً}�J�����ɓ��͂��ꂽ�ԍ��S���i�z��ŏI�Ԗڂ͋�؂�L���@NUL�@�j

static UB mainte_password[5];		// �����e�i���X�E���[�h�ڍs���̊m�F�p�p�X���[�h�S���B�i�z��ŏI�Ԗڂ͋�؂�L���@NUL�@�j
int Check_Cap_Raw_flg;				// ���摜�B�e�`�F�b�N���s���t���O�@Added T.Nagai 
int Cmr_Start=0;					//20140930Miya
int CmrDebugCnt;					//20140930Miya
char CmrCapNg;						//20140930Miya
static int	CmrWakFlg;				//20150930Miya
static int	CmrReloadFlg;			//20150930Miya

#if ( NEWCMR == 1 )
extern ER NcmrCapture( enum CAP_MODE eMode, enum CAP_BANK eBank );	///< �J�����摜�L���v�`���[(�{��)
extern ER NcmrCaptureWait( TMO tmout );	///< �J�����摜��荞�ݑ҂�
#endif

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
	unsigned short result, para, inpara;
	unsigned long exptime;
	unsigned char time1, time2, time3;


	Cmr_Start = 0;
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

	g_IrLed_SW = 0;					//20160312Miya �ɏ����xUP

	// �����J�n
	
#if ( NEWCMR == 0 )
	/**	�J�����̃X���[�v�����A���̑�	***/
	ercd = CmrCmdTest();			// �J��������̉����`�F�b�N
	if ( ercd != E_OK ){
		ercdStat = 4;
		 ErrCodeSet( ercd );
	}
	
#if (CMR_SLEEP_CNTRL == 1)
	//ercd = CmrCmdWakeUp(0);			// �J������WakeUP����
	//if ( ercd != E_OK ){
	//	ercdStat = 5;
	//	 ErrCodeSet( ercd );
	//}	
	//dly_tsk( 1000/MSEC );

	//ercd = CmrCmdSleep();			// �J�����̃X���[�v����
	ercd = CmrCmdWakeUp(0);		//20150930Miya
	if ( ercd != E_OK ){
		ercdStat = 5;
		 ErrCodeSet( ercd );
	}	
#endif
#endif
	
	//CmrCmdLuminanceCtl();	//20130619_Miya �ǉ�

	Cmr_Start = 1;	
	for(;;) {
//		ercd = wai_flg( ID_FLG_CAMERA, ( FPTN_START_CAP141
		ercd = twai_flg( ID_FLG_CAMERA, ( FPTN_START_CAP141			/* Modify T.N 2015.3.10 */
					  				   | FPTN_START_CAP204
					  				   | FPTN_START_CAP211
									   | FPTN_CMR_INIT
									   | FPTN_REROAD_PARA	//20150930Miya
									   | FPTN_CHECK_IMAGE
									   | FPTN_SETREQ_GAIN
//									   | FPTN_SETREQ_SHUT1 ), TWF_ORW | TWF_CLR, &flgptn,  );	// �J�����B�e�v���̎�M�҂�
									   | FPTN_SETREQ_SHUT1 ), TWF_ORW | TWF_CLR, &flgptn, 100/MSEC  );	// �J�����B�e�v���̎�M�҂�

		camera_TSK_wdt = FLG_ON;				// �J�����^�X�N�@WDT�J�E���^���Z�b�g�E���N�G�X�g�E�t���O Added T.N 2015.3.10

		if ( ercd == E_TMOUT ){
			continue;
		}

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
			
			CmrCapNg = 0;	//20140930Miya
			MdCngMode( MD_CAP );					// ���u���[�h���L���v�`���[����
			ercdStat = Cap_Resize_Picture( COMMAND204 );		// �o�^�p�B�e����
			if( ercdStat != 0 ){
				SetError(32);
			}				
			if( CmrCapNg == 1){
				SetError(39);
			}
			
			rxtid = rxtid_org;						// GET_UDP�ׂ̈ɖ{���̎�M�^�X�NID�itsk_rcv�j���ăZ�b�g
					
		
		} else if ( flgptn == FPTN_START_CAP211 ){			// �F�ؗp�B�e�̏ꍇ
			dbg_cam_flg = 1;
			if( dbg_nin_flg == 1 ){
				dbg_nin_flg = 2;
			}
	
			
			
//			ercd = clr_flg( ID_FLG_CAMERA, ~( FPTN_START_CAP211 & 0x0fffffff ) );			// �t���O�̃N���A
//			if ( ercd != E_OK ){
//				ercdStat = 7;
//				break;
//			}

			CmrCapNg = 0;	//20140930Miya
			MdCngMode( MD_CAP );					// ���u���[�h���L���v�`���[����
			ercdStat = Cap_Resize_Picture( COMMAND210 );		// �F�ؗp�B�e����
			if( ercdStat != 0 ){
				SetError(32);
			}				
			if( CmrCapNg == 1){
				SetError(39);
			}
			
			rxtid = rxtid_org;						// GET_UDP�ׂ̈ɖ{���̎�M�^�X�NID�itsk_rcv�j���ăZ�b�g

			dbg_cam_flg = 0;

			
		} else if ( flgptn == FPTN_START_CAP141 ){	// ���摜�B�e�v���̏ꍇ
			
//			ercd = clr_flg( ID_FLG_CAMERA, ~( FPTN_START_CAP141 & 0x0fffffff ) );			// �t���O�̃N���A
//			if ( ercd != E_OK ){
//				ercdStat = 7;
//				break;
//			}

			//ercdStat = Cap_Raw_Picture();			// ���摜�B�e����
			
			rxtid = rxtid_org;						// GET_UDP�ׂ̈ɖ{���̎�M�^�X�NID�itsk_rcv�j���ăZ�b�g

		} else if ( flgptn == FPTN_CHECK_IMAGE ){	// ���摜�B�e�v���̏ꍇ
			
//			ercd = clr_flg( ID_FLG_CAMERA, ~( FPTN_START_CAP141 & 0x0fffffff ) );			// �t���O�̃N���A
//			if ( ercd != E_OK ){
//				ercdStat = 7;
//				break;
//			}


#if ( NEWCMR == 1 )		//20160601Miya
			//if(GetSysSpec() == SYS_SPEC_KOUJIS){
			if(GetSysSpec() == SYS_SPEC_KOUJIS || g_LedCheck == 1){	//20161031Miya Ver2204
				g_cmr_dbgcnt = 4;
				NCmr_WrtExpPara(0x00, 0x01, 0x00);

				CmrCapNg = 0;
				ercdStat = Check_Cap_Raw_Picture();		// ���摜�B�e����
				if(ercdStat > 0)
					Check_Cap_Raw_flg = 2;					// Modify T.Nagai �B�e���t���O�̃N���A
				else
					Check_Cap_Raw_flg = 0;					// Modify T.Nagai �B�e���t���O�̃N���A

				if(g_cmr_dbgcnt >= 4){
					g_cmr_dbgcnt = 0;
				}
			}else{
				g_cmr_dbgcnt = 0;
				//�f�t�H���g�I�����Ԑݒ�
				NCmr_WrtExpPara(0x01, 0x80, 0x00);

				CmrCapNg = 0;	//20140930Miya
				ercdStat = Check_Cap_Raw_Picture();		// ���摜�B�e����
				if(ercdStat > 0)
					Check_Cap_Raw_flg = 2;					// Modify T.Nagai �B�e���t���O�̃N���A
				//else
				//	Check_Cap_Raw_flg = 0;					// Modify T.Nagai �B�e���t���O�̃N���A

				else{

					//�K���I�����Ԍv�Z�E�ݒ�
					if( g_TechMenuData.DebugHyouji == FLG_ON ){
						g_exptime = NCmr_CalExpTime(0, 0x018000);
					}else{
						g_exptime = NCmr_CalExpTime(2, 0x018000);
						if(g_exptime >= 0x400000)	g_exptime = 0x400000;
					}
					exptime = g_exptime & 0xFF0000;
					exptime = (exptime >> 16) & 0xFF;
					time1 = (unsigned char)exptime;
					exptime = g_exptime & 0x00FF00;
					exptime = (exptime >> 8) & 0xFF;
					time2 = (unsigned char)exptime;
					exptime = g_exptime & 0x0000FF;
					exptime = exptime & 0xFF;
					time3 = (unsigned char)exptime;
					NCmr_WrtExpPara(time1, time2, time3);

					CmrCapNg = 0;	//20140930Miya
					ercdStat = Check_Cap_Raw_Picture();		// ���摜�B�e����
					if(ercdStat > 0)
						Check_Cap_Raw_flg = 2;					// Modify T.Nagai �B�e���t���O�̃N���A
					else
						Check_Cap_Raw_flg = 0;					// Modify T.Nagai �B�e���t���O�̃N���A
				}

				NCmr_ReadPara(0);
				NCmr_ReadPara(1);
				NCmr_ReadPara(2);
				NCmr_ReadGmma();
				NCmr_ReadIniPara();
			}
#else
			CmrCapNg = 0;	//20140930Miya
			ercdStat = Check_Cap_Raw_Picture();		// ���摜�B�e����

			if(ercdStat > 0)
				Check_Cap_Raw_flg = 2;					// Modify T.Nagai �B�e���t���O�̃N���A
			else
				Check_Cap_Raw_flg = 0;					// Modify T.Nagai �B�e���t���O�̃N���A
#endif
			if( CmrCapNg == 1 ){
#if (CMR_SLEEP_CNTRL == 1)		
				/**	�J�����̃X���[�v����	***/
				ercd = CmrCmdSleep();
				if ( ercd != E_OK ){
					ercdStat = 15;
				}		
#endif
				/** IR LED�̏��������@**/
				IrLedCrlOff( IR_LED_2 | IR_LED_3 | IR_LED_4 | IR_LED_5 ); 	// �ԊO��LED�̑S����
			}
			
//			MdCngSubMode(SUB_MD_IDLE);				//   ��
			
			
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

			g_CmrParaSet = 0;
			
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
//			dly_tsk( 150/MSEC );


			ercd = CmrCmdFixShutterCtl( ( UB )cmrFixShutter1 );	// �J�����ɒ��ځA�I�o�R�ݒ�l��ݒ�iFPGA���o�R���Ȃ��j
			//ercd = CmrCmdFixShutterCtl( ( UB )cmrFixShutter3 );	// �J�����ɒ��ځA�I�o�R�ݒ�l��ݒ�iFPGA���o�R���Ȃ��j
			if ( ercd != E_OK ){
				 ErrCodeSet( ercd );
			}
//			ercd = CmrCmdSleep();					// �J�����̃X���[�v����
//			if ( ercd != E_OK ){
//				ErrCodeSet( ercd );
//			}	

			g_CmrParaSet = 0;
			
		} else if ( flgptn == FPTN_CMR_INIT ){	// �J����WeakUP //20140930Miya Bio FPGA
/* Added T.N 2016.3.8	*/
#if ( NEWCMR == 1 )	
			//dly_tsk( 1000/MSEC );

			ercd = IrLedSet( IR_LED_2, irDuty2 );						// �ԊO��LED2�̊K���ݒ�
			if ( ercd != E_OK ){
				nop();
			}
			ercd = IrLedSet( IR_LED_3, irDuty3 );						// �ԊO��LED3�̊K���ݒ�
			if ( ercd != E_OK ){
				nop();
			}
			ercd = IrLedSet( IR_LED_4, irDuty4 );						// �ԊO��LED4�̊K���ݒ�
			if ( ercd != E_OK ){
				nop();
			}
			ercd = IrLedSet( IR_LED_5, irDuty5 );						// �ԊO��LED5�̊K���ݒ�
			if ( ercd != E_OK ){
				nop();
			}
			
			IrLedCrlOn( IR_LED_2 | IR_LED_3 | IR_LED_4 | IR_LED_5 ); 	// �ԊO��LED�̑S�_��

			NCmr_PowerOn();						// ���{�P�~�R���ЃJ�����@NCM03-V��power on����
			NCmr_WrtGmma10();		//20160601Miya �K���}1.0�ݒ�
			NCmr_WrtIniPara();		//20160601Miya �J���������ݒ�
			dly_tsk( 1000/MSEC );

/*			
			dly_tsk( 3000/MSEC );
			
			CmrTrimStartX( 0 );					// �g���~���O�J�nX���W
			CmrTrimStartY( 0 );					// �g���~���O�J�nY���W
			CmrTrimSizeX( 640 );				// �g���~���OX�T�C�Y
			CmrTrimSizeY( 480 );				// �g���~���OY�T�C�Y
			
			ercd = NcmrCapture( CAP_MODE_0, CAP_BANK_0 );		// �摜��荞��(NC�ЃJ����NCM03-V�̏ꍇ)
			if (ercd == E_OK) {
				ercd = NcmrCaptureWait( 2000 );	// ��荞�ݑ҂�(NC�ЃJ����NCM03-V�̏ꍇ)
			}
*/			
			IrLedCrlOff( IR_LED_2 | IR_LED_3 | IR_LED_4 | IR_LED_5 ); 	// �ԊO��LED�̑S�_��
			g_cmr_dbgcnt = 0;	//20160601Miya forDebug
			Check_Cap_Raw_flg = 0;
#else
/* Added End T.N */
			if( Check_Cap_Raw_flg ){
				ercd = CmrCmdLuminanceCtl();	//20130619_Miya �ǉ�
				if(ercd != E_OK){
					Cmr_Start = 0;
				}	
				dly_tsk( 500/MSEC );
				ercd = CmrCmdManualGainCtl( ( UB )cmrGain );		// �J�����ɒ��ځA�Q�C���ݒ�l��ݒ�iFPGA���o�R���Ȃ��j
				if(ercd != E_OK){
					Cmr_Start = 0;
				}	
				dly_tsk( 500/MSEC );
				ercd = CmrCmdFixShutterCtl( ( UB )cmrFixShutter1 );	// �J�����ɒ��ځA�I�o�R�ݒ�l��ݒ�iFPGA���o�R���Ȃ��j
				if(ercd != E_OK){
					Cmr_Start = 0;
				}
				if( Cmr_Start == 1 )
					Cmr_Start = 2;
					
				dly_tsk( 500/MSEC );
				ercd = CmrCmdSleep();	//20150930Miya	

				Check_Cap_Raw_flg = 0;
			}else{
				para = CmrCmdManualGetParameter(2);
				if( (UH)para != cmrGain || para == 0xffff ){
					result = 1;
				}
				para = CmrCmdManualGetParameter(1);
				if( (UH)para != cmrFixShutter1 || para == 0xffff ){
					result = 1;
				}
				para = CmrCmdManualGetParameter(0);
				if( para != 0 || para == 0xffff ){
					result = 1;
				}
				if( result == 1 ){
					nop();
				}
/*
				inpara = 0;
				result = 0;
				while(1){
					dly_tsk( 500/MSEC );
					ercd = CmrCmdManualGainCtl( (UB)inpara );		// �J�����ɒ��ځA�Q�C���ݒ�l��ݒ�iFPGA���o�R���Ȃ��j
					if( ercd != E_OK ){
						if( ercd == E_OBJ )
							result = 1;
						else
							result = 2;
					}else{
						para = CmrCmdManualGetParameter(2);
						if( para != inpara || para == 0xffff ){
							result = 3;
						}
					}
					++inpara;
					if(inpara >= 10)	inpara = 0;
					if(result != 0)
						break;

					ercd = CmrCmdFixShutterCtl( (UB)inpara );		// �J�����ɒ��ځA�Q�C���ݒ�l��ݒ�iFPGA���o�R���Ȃ��j
					if( ercd != E_OK ){
						if( ercd == E_OBJ )
							result = 11;
						else
							result = 12;
					}else{
						para = CmrCmdManualGetParameter(1);
						if( para != inpara || para == 0xffff ){
							result = 13;
						}
					}
					++inpara;
					if(inpara >= 10)	inpara = 0;
					if(result != 0)
						break;
				}
				if(result != 0)
					nop();
*/

			}
#endif
		} else if ( flgptn == FPTN_REROAD_PARA ){	// �J�����p�����[�^�����[�h//20150930Miya
			ercd = CmrCmdManualGainCtl( ( UB )cmrGain );		// �J�����ɒ��ځA�Q�C���ݒ�l��ݒ�iFPGA���o�R���Ȃ��j
			if ( ercd != E_OK ){
			 	ErrCodeSet( ercd );
			}
			dly_tsk( 500/MSEC );
			ercd = CmrCmdFixShutterCtl( ( UB )cmrFixShutter1 );	// �J�����ɒ��ځA�I�o1�ݒ�l��ݒ�iFPGA���o�R���Ȃ��j
			if ( ercd != E_OK ){
				 ErrCodeSet( ercd );
			}
			
			dly_tsk( 500/MSEC );
			ercd = CmrCmdSleep();	//20150930Miya	
			if ( ercd != E_OK ){
				 ErrCodeSet( ercd );
			}

			CmrReloadFlg = 0;

		}	 else {
			nop();
		}
	}
}


//-----------------------------------------------------------------------------
// �F�ؗp�B�e�A�o�^�p�B�e����
//-----------------------------------------------------------------------------
int	Cap_Resize_Picture( UINT command_no ){
	
	int ercdStat = 0, rtn;
	ER ercd;
	T_YBDATA *ybdata;
	volatile unsigned short ustmp, cap_wait;
	UB hantei, rslt;

	int xx;

#if ( NEWCMR == 0 )	//20160711Miya
	ercd = CmrCmdWakeUp(0);
	if( ercd != E_OK){
		//return(1);
	}
#endif

	//ybdata = &yb_touroku_data;	
	//yb_init_capbuf();					// �w���̏�����(�������ȊO)

//20160312Miya �ɏ����xUP LED�_���^�C�~���O�ύX�@tsk_ts
/*
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

	IrLedCrlOn( IR_LED_2 | IR_LED_3 | IR_LED_4 | IR_LED_5 ); 	// �ԊO��LED�̑S�_��
*/

	hantei = 1;	//���邳���肠��
	// �o�^1��ڂ̎B�e
	if ( ( GetScreenNo() == LCD_SCREEN6 ) || ( GetScreenNo() == LCD_SCREEN121 )	// ���ځH
      || ( GetScreenNo() == LCD_SCREEN127 ) || ( GetScreenNo() == LCD_SCREEN141 )
	  || ( GetScreenNo() == LCD_SCREEN161 ) || ( GetScreenNo() == LCD_SCREEN181 )
	  || ( GetScreenNo() == LCD_SCREEN405 ) || ( GetScreenNo() == LCD_SCREEN523 ) // ���ځH(�P�΂P�F�؎d�l)
	  || ( GetScreenNo() == LCD_SCREEN530 ) || ( GetScreenNo() == LCD_SCREEN544 )){
			  
		hantei = 1;	//���邳���肠��
	}
	
	// �o�^2��ڂ̎B�e
	if ( ( GetScreenNo() == LCD_SCREEN8 ) || ( GetScreenNo() == LCD_SCREEN101 )	// ���ځH
      || ( GetScreenNo() == LCD_SCREEN102 ) || ( GetScreenNo() == LCD_SCREEN129 )
	  || ( GetScreenNo() == LCD_SCREEN407 ) || ( GetScreenNo() == LCD_SCREEN503 ) // ���ځH(�P�΂P�F�؎d�l)
	  || ( GetScreenNo() == LCD_SCREEN532 ) ){
			  			  
		hantei = 1;	//���邳����Ȃ�
	}

	// �F�؂̎B�e
	//if ( ( GetScreenNo() == LCD_SCREEN101 ) || ( GetScreenNo() == LCD_SCREEN102 ) || ( GetScreenNo() == LCD_SCREEN105)
	//  || ( GetScreenNo() == LCD_SCREEN503 )  ){
	//20160108Miya FinKeyS
	if ( ( GetScreenNo() == LCD_SCREEN101 ) || ( GetScreenNo() == LCD_SCREEN102 ) || ( GetScreenNo() == LCD_SCREEN105)
	  || ( GetScreenNo() == LCD_SCREEN601 ) || ( GetScreenNo() == LCD_SCREEN602 ) || ( GetScreenNo() == LCD_SCREEN605) 
	  || ( GetScreenNo() == LCD_SCREEN610 ) || ( GetScreenNo() == LCD_SCREEN611 ) || ( GetScreenNo() == LCD_SCREEN503 )  ){

		hantei = 1;	//���邳���肠��
	}



#if ( VA300S == 0 )
	ybdata = &yb_touroku_data;	
	yb_init_capbuf();					// �w���̏�����(�������ȊO)
	memset( &g_ubCapBuf, 0, ( iReSizeX * iReSizeY * 3 ) );
		
//1���ډ摜(M)�L���v�`���[�J�n		
	ercd = CmrCmdFixShutterCtl( cmrFixShutter2 );	// �J�����ɒ��ځA�I�o2�ݒ�l��ݒ�iFPGA���o�R���Ȃ��j
	if ( ercd != E_OK ){
		 ErrCodeSet( ercd );
	}
	cmr_wait_time = CMR_WAIT_TIME;
	CmrWakeUpWaitCrl( cmr_wait_time );							// WakeUp���̃J������Wait���Ԑݒ�
	dly_tsk( 140/MSEC );
	ercd = CmrCmdFreezeOff();									// �J�����E�t���[�Y�E�R���g���[���E�I�t
	if ( ercd != E_OK ){
		ercdStat = 9;
	}

	if(f_fpga_ctrl){	//20140915Miya FPGA����t���O 0:V1.001�p�@1:V1.002�p
		ercd = CmrCapture2( CAP_MODE_2, CAP_BANK_0, iStartX, iStartY, iSizeX, iSizeY, (2000/MSEC) );	// �摜�̃L���v�`���[�A1����
	}else{
		ercd = CmrCapture( CAP_MODE_1, CAP_BANK_0, (2000/MSEC) );	// �摜�̃L���v�`���[�A1����
	}

	cap_wait = 0;
	while( IsCmrCapStart() == TRUE ){							// �摜�L���v�`���[�̏I���҂�
		dly_tsk( 10/MSEC );
		++cap_wait;
		if(cap_wait >= 100){
			ustmp = *(volatile unsigned short *)(FPGA_BASE + 0x0100);
			*(volatile unsigned short *)(FPGA_BASE + 0x0100) = ustmp & 0xFFFE;
			ustmp = *(volatile unsigned short *)(FPGA_BASE + 0x0100);
			CmrCapNg = 1;
			break;
		}
	}

	if(	CmrCapNg == 0 ){	//�摜�L���v�`���[�ɐ������Ă���	
		if(f_fpga_ctrl == 0){	//20140915Miya FPGA����t���O 0:V1.001�p�@1:V1.002�p
			/** �L���v�`���摜�̃g���~���O	**/
			ercd = CmrTrim(CAP_BANK_0, TRIM_BANK_0, iStartX, iStartY, iSizeX, iSizeY, trimTimeOut ); // �P���ډ摜�̃g���~���O
			if ( ercd != E_OK ){
				ercdStat = 19;
				return ercdStat;
			}
			while ( IsCmrTrimStart() == TRUE ){
				dly_tsk( 10/MSEC );	
			}
		}

		ercd = CmrCmdFixShutterCtl( cmrFixShutter1 );	// �J�����ɒ��ځA�I�o1�ݒ�l��ݒ�iFPGA���o�R���Ȃ��j
		if ( ercd != E_OK ){
			 ErrCodeSet( ercd );
		}

		/** �g���~���O�摜�̏k���ƈ��k	**/
		ercd = CmrResize(TRIM_BANK_0, RSZ_BANK_0, RSZ_MODE_1, iSizeX, iSizeY, resizeTimeOut );//20131210Miya cng	// �P���ډ摜�̏k���E���k
		if ( ercd != E_OK ){
			ercdStat = 25;
			return ercdStat;
		}
		while ( IsCmrResizeStart() == TRUE ){
			dly_tsk( 10/MSEC );
		}
		
		ercd = CmrResizeGet(RSZ_BANK_0, 0, ( iReSizeX * iReSizeY ), &g_ubCapBuf[ ( iReSizeX * iReSizeY ) ]);//1���ڏk���摜�̎擾
		if ( ercd != E_OK ){
			ercdStat = 28;
			return ercdStat;
		}

		//M�摜����(���邢���ǂ���)
		rslt = MidImgHantei(hantei);
		if(rslt == 1){	//���邢
			IrLedOnOffSet(1, 255, 255, 128, 0);	//20160312Miya �ɏ����xUP
			rtn = CapMinimum();
			if(rtn != 0){
				ercdStat = 99;
			}
			IrLedOnOffSet(1, irDuty2, irDuty3, irDuty4, irDuty5);	//20160312Miya �ɏ����xUP
		}else{
			rtn = CapBantam(hantei);
			if(rtn != 0){
				ercdStat = 99;
			}
		}
	}

	memcpy( &ybdata->ybCapBuf, &g_ubCapBuf, ( iReSizeX * iReSizeY * 3 ) );//20131210Miya cng


	/** IR LED�̏��������@**/
	//20160312Miya �ɏ����xUP �����͔F�؏I�����ɍs��
	//IrLedCrlOff( IR_LED_2 | IR_LED_3 | IR_LED_4 | IR_LED_5 ); 	// �ԊO��LED�̑S����
		
	/** �J�����B�e�̏I�������@**/
	ercd = set_flg( ID_FLG_TS, FPTN_CMR_TS );					// ���̌��m�^�X�N�ցA�J�����B�e�I����ʒm
	if ( ercd != E_OK ){
		ercdStat = 31;
		return ercdStat;
	}

	/** �摜��UDP���M�^�X�N�֑���	**/
	if(	g_CapTimes == 2 ){	//20131210Miya add
		LedOut(LED_OK, LED_ON);
		LedOut(LED_ERR, LED_ON);
	}
	if ( command_no == COMMAND204 ){
		ercd = SendCmd_204( ( UB* )ybdata, ( 42 + ( 160 * 140 * 3 ) + 2 ) );//20131210Miya cng		// �R�}���h204�A�k���摜�i�������t���j
		if ( ercd != E_OK ){
			ercdStat = 34;
//			return ercdStat;
		}
	} else if	( command_no == COMMAND210 ){
		ercd = SendCmd_210( ( UB* )ybdata->ybCapBuf, ( ( 160 * 140 ) * 3 ) + 2 );//20131210Miya cng	// �R�}���h210�A�k���摜�R���i�������Ȃ��j
		if ( ercd != E_OK ){
			ercdStat = 34;
//			return ercdStat;
		}
	}
	if(	g_CapTimes == 2 ){	//20131210Miya add
		LedOut(LED_OK, LED_OFF);
		LedOut(LED_ERR, LED_OFF);
	}
#else
	g_SameImgGet = 0;	//20151118Miya ���摜�ĎB�e
	g_SameImgCnt = 0;	//20160115Miya
	CapImgSeq(command_no, hantei);

	if(g_SameImgGet > 0){	//20151118Miya ���摜�ĎB�e 
		ercd = CmrCmdFixShutterCtl( ( UB )cmrFixShutter1 );	// �J�����ɒ��ځA�I�o1�ݒ�l��ݒ�iFPGA���o�R���Ȃ��j
		if ( ercd != E_OK ){
			 ErrCodeSet( ercd );
		}
		dly_tsk( 500/MSEC );
		ercd = CmrCmdSleep();	//20150930Miya	
		if ( ercd != E_OK ){
			 ErrCodeSet( ercd );
		}
		dly_tsk( 500/MSEC );
		ercd = CmrCmdWakeUp(0);

		g_SameImgGet = 0;	//20160115Miya ���摜�ĎB�e
		CapImgSeq(command_no, hantei);
		g_SameImgGet = 0;	//20151118Miya ���摜�ĎB�e
		g_SameImgCnt = 0;	//20160115Miya
	}

	/** IR LED�̏��������@**/
	//20160312Miya �ɏ����xUP �����͔F�؏I�����ɍs��
	//IrLedCrlOff( IR_LED_2 | IR_LED_3 | IR_LED_4 | IR_LED_5 ); 	// �ԊO��LED�̑S����
		

	/** �J�����B�e�̏I�������@**/
	ercd = set_flg( ID_FLG_TS, FPTN_CMR_TS );					// ���̌��m�^�X�N�ցA�J�����B�e�I����ʒm
	if ( ercd != E_OK ){
		ercdStat = 31;
		return ercdStat;
	}

	if(	g_CapTimes == 2 ){	//20131210Miya add
		LedOut(LED_OK, LED_OFF);
		LedOut(LED_ERR, LED_OFF);
	}
#endif


//for debug
#if ( VA300S == 3 )
	if ( command_no == COMMAND204 ){
		//dly_tsk( 5000/MSEC );
		//memcpy( &ybdata->ybCapBuf, &g_ubCapBuf, ( iReSizeX * iReSizeY * 3 ) );//20131210Miya cng
		//ercd = SendCmd_204( ( UB* )ybdata, ( 42 + ( 160 * 140 * 3 ) + 2 ) );//20131210Miya cng		// �R�}���h204�A�k���摜�i�������t���j

		memcpy( &ybdata->ybCapBuf, &g_ubSobelR2Buf[0], 80 * 40 );//HDR
		//memcpy( &ybdata->ybCapBuf, &g_ubResizeBuf[0], 80 * 40 );//HDR
		ercd = SendCmd_204( ( UB* )ybdata, ( 42 + ( 80 * 40 * 1 ) + 2 ) );//20131210Miya cng		// �R�}���h204�A�k���摜�i�������t���j
	} else if	( command_no == COMMAND210 ){
		//memcpy( &g_ubCapBuf[0], &g_ubSobelR1Buf[0], 80 * 40);
		//memcpy( &g_ubCapBuf[80 * 40], &RegImgBuf[1][0][0][0], 80 * 40);
		//memcpy( &g_ubCapBuf[iReSizeX * iReSizeY], &g_ubSobelR2Buf[0], 80 * 40);
		//memcpy( &g_ubCapBuf[iReSizeX * iReSizeY + 80 * 40], &RegImgBuf[1][0][1][0], 80 * 40);
		
		//memcpy( &ybdata->ybCapBuf, &g_ubCapBuf, ( iReSizeX * iReSizeY * 3 ) );//20131210Miya cng
		//ercd = SendCmd_210( ( UB* )ybdata->ybCapBuf, ( ( 160 * 140 ) * 3 ) + 2 );	// �R�}���h210�A�k���摜�R���i�������Ȃ��j

		//memcpy( &ybdata->ybCapBuf, &g_ubResizeBuf[0], 80 * 40 );//HDR
		memcpy( &ybdata->ybCapBuf, &RegImgBuf[0][1][0], 80 * 40 );//HDR
		ercd = SendCmd_210( ( UB* )ybdata->ybCapBuf, ( ( 80 * 40 ) * 1 ) + 2 );	// �R�}���h210�A�k���摜�R���i�������Ȃ��j
	}

#endif

	/** �J�����B�e�̏I�������@**/
	//if(	g_CapTimes == 1 ){	//20131210Miya add
	if(	g_CapTimes < 2 ){	//20140523Miya cng �\���΍�
		ercd = set_flg( ID_FLG_TS, FPTN_END_TS );				// ���̌��m�^�X�N�ցA�J�����B�e�I����ʒm
		if ( ercd != E_OK ){
			ercdStat = 35;
			return ercdStat;
		}
	}
	return ercdStat;
}



#if (NEWCMR == 0)
int CapImgSeq(UINT command_no, UB hantei)
{
	int ercdStat = 0, rtn;
	ER ercd;
	T_YBDATA *ybdata;
	volatile unsigned short ustmp, cap_wait, i, cnt;
	UB rslt;


	ybdata = &yb_touroku_data;	
	yb_init_capbuf();					// �w���̏�����(�������ȊO)
	memset( &g_ubCapBuf, 0, ( iReSizeX * iReSizeY * 3 ) );

	if(SAMEIMGCHK == 1){	//20160115Miya 
		for(i = 0 ; i < 8 ; i++ ){
			*(volatile unsigned long *)(CAP_BASE + 0x400000 + 4 * (unsigned long)i) = 0;
		}
	}
		
//1���ډ摜(M)�L���v�`���[�J�n		
	ercd = CmrCmdFixShutterCtl( cmrFixShutter2 );	// �J�����ɒ��ځA�I�o2�ݒ�l��ݒ�iFPGA���o�R���Ȃ��j
	if ( ercd != E_OK ){
		 ErrCodeSet( ercd );
	}
	cmr_wait_time = CMR_WAIT_TIME;
	CmrWakeUpWaitCrl( cmr_wait_time );							// WakeUp���̃J������Wait���Ԑݒ�
	dly_tsk( 140/MSEC );
	ercd = CmrCmdFreezeOff();									// �J�����E�t���[�Y�E�R���g���[���E�I�t
	if ( ercd != E_OK ){
		ercdStat = 9;
	}

	if(f_fpga_ctrl){	//20140915Miya FPGA����t���O 0:V1.001�p�@1:V1.002�p
		ercd = CmrCapture2( CAP_MODE_2, CAP_BANK_0, iStartX, iStartY, iSizeX, iSizeY, (2000/MSEC) );	// �摜�̃L���v�`���[�A1����
	}else{
		ercd = CmrCapture( CAP_MODE_1, CAP_BANK_0, (2000/MSEC) );	// �摜�̃L���v�`���[�A1����
	}

	cap_wait = 0;
	while( IsCmrCapStart() == TRUE ){							// �摜�L���v�`���[�̏I���҂�
		dly_tsk( 10/MSEC );
		++cap_wait;
		if(cap_wait >= 100){
			ustmp = *(volatile unsigned short *)(FPGA_BASE + 0x0100);
			*(volatile unsigned short *)(FPGA_BASE + 0x0100) = ustmp & 0xFFFE;
			ustmp = *(volatile unsigned short *)(FPGA_BASE + 0x0100);
			CmrCapNg = 1;
			break;
		}
	}

	if(	CmrCapNg == 0 ){	//�摜�L���v�`���[�ɐ������Ă���	
		if(f_fpga_ctrl == 0){	//20140915Miya FPGA����t���O 0:V1.001�p�@1:V1.002�p
			/** �L���v�`���摜�̃g���~���O	**/
			ercd = CmrTrim(CAP_BANK_0, TRIM_BANK_0, iStartX, iStartY, iSizeX, iSizeY, trimTimeOut ); // �P���ډ摜�̃g���~���O
			if ( ercd != E_OK ){
				ercdStat = 19;
				return ercdStat;
			}
			while ( IsCmrTrimStart() == TRUE ){
				dly_tsk( 10/MSEC );	
			}
		}

		ercd = CmrCmdFixShutterCtl( cmrFixShutter1 );	// �J�����ɒ��ځA�I�o1�ݒ�l��ݒ�iFPGA���o�R���Ȃ��j
		if ( ercd != E_OK ){
			 ErrCodeSet( ercd );
		}

		/** �g���~���O�摜�̏k���ƈ��k	**/
		ercd = CmrResize(TRIM_BANK_0, RSZ_BANK_0, RSZ_MODE_1, iSizeX, iSizeY, resizeTimeOut );//20131210Miya cng	// �P���ډ摜�̏k���E���k
		if ( ercd != E_OK ){
			ercdStat = 25;
			return ercdStat;
		}
		while ( IsCmrResizeStart() == TRUE ){
			dly_tsk( 10/MSEC );
		}
		
		ercd = CmrResizeGet(RSZ_BANK_0, 0, ( iReSizeX * iReSizeY ), &g_ubCapBuf[ ( iReSizeX * iReSizeY ) ]);//1���ڏk���摜�̎擾
		if ( ercd != E_OK ){
			ercdStat = 28;
			return ercdStat;
		}

		if(SAMEIMGCHK == 1){	//20160115Miya 
			cnt = 0;
			for(i = 0 ; i < 8 ; i++ ){	//20151216Miya
				if( *(volatile unsigned long *)(CAP_BASE + 0x400000 + 4 * (unsigned long)i) == 0 ){
					++cnt;
				}
			}
			if(cnt >= 4){
				g_SameImgGet = g_SameImgGet | 0x01;
			}
		}

		//M�摜����(���邢���ǂ���)
		rslt = MidImgHantei(hantei);
		if(rslt == 1){	//���邢
			IrLedOnOffSet(1, 255, 255, 128, 0);	//20160312Miya �ɏ����xUP
			rtn = CapMinimum();
			if(rtn != 0){
				ercdStat = 99;
			}
			IrLedOnOffSet(1, irDuty2, irDuty3, irDuty4, irDuty5);	//20160312Miya �ɏ����xUP
		}else{
			rtn = CapBantam(hantei);
			if(rtn != 0){
				ercdStat = 99;
			}
		}
	}

	memcpy( &ybdata->ybCapBuf, &g_ubCapBuf, ( iReSizeX * iReSizeY * 3 ) );//20131210Miya cng


//	/** IR LED�̏��������@**/
//	IrLedCrlOff( IR_LED_2 | IR_LED_3 | IR_LED_4 | IR_LED_5 ); 	// �ԊO��LED�̑S����
		
//	/** �J�����B�e�̏I�������@**/
//	ercd = set_flg( ID_FLG_TS, FPTN_CMR_TS );					// ���̌��m�^�X�N�ցA�J�����B�e�I����ʒm
//	if ( ercd != E_OK ){
//		ercdStat = 31;
//		return ercdStat;
//	}

	if(SAMEIMGCHK == 1){	//20160115Miya 
		if( g_SameImgGet > 0 ){	//20151216Miya
			g_SameImgCnt++;
			//g_SameImgGet = 0;
			ercdStat = 99;
		}
	}

	if(	g_CapTimes == 2 ){	//20131210Miya add
		LedOut(LED_OK, LED_ON);
		LedOut(LED_ERR, LED_ON);
	}
	/** �摜��UDP���M�^�X�N�֑���	**/
	if ( command_no == COMMAND204 ){
		ercd = SendNinshou_204( ( UB* )ybdata, ( 42 + ( 160 * 80 * 3 ) + 2 ) );		// �R�}���h204�A�k���摜�i�������t���j
		if ( ercd != E_OK ){
			ercdStat = 34;
		}
	} else if	( command_no == COMMAND210 ){
		ercd = SendNinshou_210( ( UB* )ybdata->ybCapBuf, ( ( 160 * 80 ) * 3 ) + 2 );	// �R�}���h210�A�k���摜�R���i�������Ȃ��j
		if ( ercd != E_OK ){
			ercdStat = 34;
		}
	}
	
	return(ercdStat);
}
#else	//20160610Miya
int CapImgSeq(UINT command_no, UB hantei)
{
	int ercdStat = 0, rtn;
	ER ercd;
	T_YBDATA *ybdata;
	volatile unsigned short ustmp, ustmp2, cap_wait, i, cnt, capcnt;
	UB rslt;
	unsigned long exptime;
	unsigned char time1, time2, time3;
	int ii;
	UB cAddr[] =    { 0x0D, 0x0E, 0x0F };
	UB cRcvData[3];


	ybdata = &yb_touroku_data;	
	yb_init_capbuf();					// �w���̏�����(�������ȊO)
	//memset( &g_ubCapBuf, 0, ( iReSizeX * iReSizeY * 3 ) );
	memset( &g_ubCapBuf, 0, ( 200 * 80 ) );

#if (AUTHTEST == 3)
	for(capcnt = 0 ; capcnt <= 8 ; capcnt++){
#else
	for(capcnt = 0 ; capcnt < 2 ; capcnt++){
#endif
		if(SAMEIMGCHK == 1){	//20160115Miya 
			for(i = 0 ; i < 8 ; i++ ){
				*(volatile unsigned long *)(CAP_BASE + 0x400000 + 4 * (unsigned long)i) = 0;
			}
		}
		
		if(capcnt == 0){
			NCmr_WrtExpPara(0x01, 0x80, 0x00);	//�f�t�H���g�I������
		}else if(capcnt == 1){
			exptime = g_exptime & 0xFF0000;
			exptime = (exptime >> 16) & 0xFF;
			time1 = (unsigned char)exptime;
			exptime = g_exptime & 0x00FF00;
			exptime = (exptime >> 8) & 0xFF;
			time2 = (unsigned char)exptime;
			exptime = g_exptime & 0x0000FF;
			exptime = exptime & 0xFF;
			time3 = (unsigned char)exptime;
			NCmr_WrtExpPara(time1, time2, time3);
		}

		//ercd = CmrCapture2( CAP_MODE_2, CAP_BANK_0, iStartX, iStartY, iSizeX, iSizeY, (2000/MSEC) );	// �摜�̃L���v�`���[�A1����
		ercd = CmrCapture2( CAP_MODE_0, CAP_BANK_0, iStartX, iStartY, iSizeX, iSizeY, (2000/MSEC) );	// �摜�̃L���v�`���[�A1����

		if(ercd != E_OK){
			cap_wait = 0;
			while(1){
				dly_tsk( 10/MSEC );
				++cap_wait;
				ustmp = *(volatile unsigned short *)(FPGA_BASE + 0x0100);
				ustmp2 = *(volatile unsigned short *)(FPGA_BASE + 0x0210);
				if((ustmp2 & 0x0001) == 0x0000 && (ustmp & 0x0001) == 0x0000){
					break;
				}
				if(cap_wait >= 100){
					ustmp = *(volatile unsigned short *)(FPGA_BASE + 0x0100);
					*(volatile unsigned short *)(FPGA_BASE + 0x0100) = ustmp & 0xFFFE;
					ustmp = *(volatile unsigned short *)(FPGA_BASE + 0x0100);
					CmrCapNg = 1;	//�L���v�`���[���s
					break;
				}
			}
		}

		if(	CmrCapNg == 0 ){	//�摜�L���v�`���[�ɐ������Ă���	
			/** �g���~���O�摜�̏k���ƈ��k	**/
			//ercd = CmrResize(TRIM_BANK_0, RSZ_BANK_0, RSZ_MODE_1, iSizeX, iSizeY, resizeTimeOut );//20131210Miya cng	// �P���ډ摜�̏k���E���k
			ercd = CmrResize(TRIM_BANK_0, RSZ_BANK_0, RSZ_MODE_1, iSizeX, iSizeY, resizeTimeOut );//20131210Miya cng	// �P���ډ摜�̏k���E���k
			if ( ercd != E_OK ){
				ercdStat = 25;
				return ercdStat;
			}
			ustmp = *(volatile unsigned short *)(FPGA_BASE + 0x0220);
			if((ustmp & 0x0001) == 0x0000){
				nop();
			}else{
				while ( IsCmrResizeStart() == TRUE ){
					dly_tsk( 10/MSEC );
				}
			}
		
			//ercd = CmrResizeGet(RSZ_BANK_0, 0, ( iReSizeX * iReSizeY ), &g_ubCapBuf[ ( iReSizeX * iReSizeY ) ]);//1���ڏk���摜�̎擾
			ercd = CmrResizeGet(RSZ_BANK_0, 0, ( iReSizeX * iReSizeY ), &g_ubCapBuf[ 0 ]);//20131210Miya cng		// �P���ڏk���摜�̎擾
			if ( ercd != E_OK ){
				ercdStat = 28;
				return ercdStat;
			}

			if(SAMEIMGCHK == 1){	//20160115Miya 
				cnt = 0;
				for(i = 0 ; i < 8 ; i++ ){	//20151216Miya
					if( *(volatile unsigned long *)(CAP_BASE + 0x400000 + 4 * (unsigned long)i) == 0 ){
						++cnt;
					}
				}
				if(cnt >= 4){
					g_SameImgGet = g_SameImgGet | 0x01;
				}
			}

#if (AUTHTEST == 3)
			if(capcnt > 0)	memcpy(&TstAuthImgBuf[capcnt-1][0], &g_ubCapBuf[0], 100 * 40);
			dly_tsk( 200/MSEC );	//200msec

			ercd = NCmrI2C_Rcv( cAddr, cRcvData, 3 );		//   ���@I2C��M
			for(ii=0 ; ii<3 ; ii++){
				if(cRcvData[ii] == 1){
						nop();
				}
			}

			if(capcnt == 0){
				g_exptime = NCmr_CalExpTime(1, 0x018000);
				if(g_exptime == 0x018000){
						nop();
				}
			}
#else
			if(capcnt == 0){
				g_exptime = NCmr_CalExpTime(1, 0x018000);
				if(g_exptime == 0x018000){
						break;
				}
			}
#endif	
		}
	}

	//memcpy( &ybdata->ybCapBuf, &g_ubCapBuf, ( iReSizeX * iReSizeY * 3 ) );//20131210Miya cng
	memcpy( &ybdata->ybCapBuf, &g_ubCapBuf, ( iReSizeX * iReSizeY ) );//20131210Miya cng

	if(SAMEIMGCHK == 1){	//20160115Miya 
		if( g_SameImgGet > 0 ){	//20151216Miya
			g_SameImgCnt++;
			//g_SameImgGet = 0;
			ercdStat = 99;
		}
	}

	if(	g_CapTimes == 2 ){	//20131210Miya add
		LedOut(LED_OK, LED_ON);
		LedOut(LED_ERR, LED_ON);
	}
	/** �摜��UDP���M�^�X�N�֑���	**/
	if ( command_no == COMMAND204 ){
		//ercd = SendNinshou_204( ( UB* )ybdata, ( 42 + ( 160 * 80 * 3 ) + 2 ) );		// �R�}���h204�A�k���摜�i�������t���j
		ercd = SendNinshou_204( ( UB* )ybdata, ( 42 + ( iReSizeX * iReSizeY ) + 2 ) );		// �R�}���h204�A�k���摜�i�������t���j
		if ( ercd != E_OK ){
			ercdStat = 34;
		}
	} else if	( command_no == COMMAND210 ){
		//ercd = SendNinshou_210( ( UB* )ybdata->ybCapBuf, ( ( 160 * 80 ) * 3 ) + 2 );	// �R�}���h210�A�k���摜�R���i�������Ȃ��j
		ercd = SendNinshou_210( ( UB* )ybdata->ybCapBuf, ( ( iReSizeX * iReSizeY ) ) + 2 );	// �R�}���h210�A�k���摜�R���i�������Ȃ��j
		if ( ercd != E_OK ){
			ercdStat = 34;
		}
	}
	
	return(ercdStat);
}
#endif



//-----------------------------------------------------------------------------
// ���摜�B�e�����`�F�b�N
//-----------------------------------------------------------------------------
#if (NEWCMR == 0)
int	Check_Cap_Raw_Picture( void )
{
	int ercdStat = 0;
	ER ercd;
	volatile unsigned short ustmp, cap_wait, para;

	int xx;
		
	CmrDebugCnt = 0;	//################
	IrLedCrlOn( IR_LED_2 | IR_LED_3 | IR_LED_4 | IR_LED_5 ); 	// �ԊO��LED�̑S�_��
				
	/** �J�����B�e����	**/

	CmrDebugCnt = 1;	//################
	ercd = CmrCmdWakeUp(1);
	if( ercd != E_OK){
		//return(1);
	}
	dly_tsk( 10/MSEC );
	//CmrPrmShutterSet( cmrFixShutter2 );							
	//CmrPrmShutterSet( 6 );							

	CmrDebugCnt = 2;	//################
	ercd = CmrCmdFixShutterCtl( 12 );	// �J�����ɒ��ځA�I�o�R�ݒ�l��ݒ�iFPGA���o�R���Ȃ��j
	if ( ercd != E_OK ){
		 ErrCodeSet( ercd );
	}
	para = CmrCmdManualGetParameter(1);
	CmrWakeUpWaitCrl( cmr_wait_time );							// WakeUp���̃J������Wait���Ԑݒ�
	dly_tsk( 150/MSEC );
	CmrDebugCnt = 3;	//################
	ercd = CmrCmdFreezeOff();									// �J�����E�t���[�Y�E�R���g���[���E�I�t
	if ( ercd != E_OK ){
		ercdStat = 9;
	}
		
	if(f_fpga_ctrl){	//20140915Miya FPGA����t���O 0:V1.001�p�@1:V1.002�p
		CmrDebugCnt = 4;	//################
		ustmp = *(volatile unsigned short *)(FPGA_BASE + 0x0100);
		if(ustmp & 0x8001 > 0){
			nop();
		}
		ercd = CmrCapture2( CAP_MODE_2, CAP_BANK_0, iStartX, iStartY, iSizeX, iSizeY, (2000/MSEC) );	// �摜�̃L���v�`���[�A1����
	}else{
		ercd = CmrCapture( CAP_MODE_2, CAP_BANK_0, (2000/MSEC) );	// �摜�̃L���v�`���[�A1����
	}

	cap_wait = 0;
	while( IsCmrCapStart() == TRUE ){							// �摜�L���v�`���[�̏I���҂�
		dly_tsk( 10/MSEC );
		++cap_wait;
		if(cap_wait >= 100){
			ustmp = *(volatile unsigned short *)(FPGA_BASE + 0x0100);
			*(volatile unsigned short *)(FPGA_BASE + 0x0100) = ustmp & 0xFFFE;
			ustmp = *(volatile unsigned short *)(FPGA_BASE + 0x0100);
			CmrCapNg = 1;
			return(1);
		}
	}

	//CmrPrmShutterSet( 1 );							
	CmrDebugCnt = 5;	//################
	ercd = CmrCmdFixShutterCtl( 6 );	// �J�����ɒ��ځA�I�o�R�ݒ�l��ݒ�iFPGA���o�R���Ȃ��j
	if ( ercd != E_OK ){
		 ErrCodeSet( ercd );
	}
	para = CmrCmdManualGetParameter(1);


	if(f_fpga_ctrl == 0){	//20140915Miya FPGA����t���O 0:V1.001�p�@1:V1.002�p
		/** �L���v�`���摜�̃g���~���O	**/
		ercd = CmrTrim(CAP_BANK_0, TRIM_BANK_0, iStartX, iStartY, iSizeX, iSizeY, trimTimeOut ); // �P���ډ摜�̃g���~���O
		if ( ercd != E_OK ){
			ercdStat = 19;
			return ercdStat;
		}
		while ( IsCmrTrimStart() == TRUE ){
			dly_tsk( 10/MSEC );	
		}
	}


	/** �g���~���O�摜�̏k���ƈ��k	**/
	CmrDebugCnt = 6;	//################
	ercd = CmrResize(TRIM_BANK_0, RSZ_BANK_0, RSZ_MODE_1, iSizeX, iSizeY, resizeTimeOut );//20131210Miya cng	// �P���ډ摜�̏k���E���k
	if ( ercd != E_OK ){
		ercdStat = 25;
		return ercdStat;
	}
	while ( IsCmrResizeStart() == TRUE ){
		dly_tsk( 10/MSEC );
	}
		
	CmrDebugCnt = 7;	//################
	ercd = CmrResizeGet(RSZ_BANK_0, 0, ( iReSizeX * iReSizeY ), &g_ubCapBuf[ 0 ]);//20131210Miya cng		// �P���ڏk���摜�̎擾
	if ( ercd != E_OK ){
		ercdStat = 28;
		return ercdStat;
	}

	//CmrPrmShutterSet( cmrFixShutter3 );
	//CmrPrmShutterSet( 1 );
	dly_tsk( 100/MSEC );
	CmrDebugCnt = 8;	//################
	ercd = CmrCmdFreezeOff();									// �J�����E�t���[�Y�E�R���g���[���E�I�t
			
	if(f_fpga_ctrl){	//20140915Miya FPGA����t���O 0:V1.001�p�@1:V1.002�p
		CmrDebugCnt = 9;	//################
		ustmp = *(volatile unsigned short *)(FPGA_BASE + 0x0100);
		if(ustmp & 0x8001 > 0){
			nop();
		}
		ercd = CmrCapture2( CAP_MODE_2, CAP_BANK_0, iStartX, iStartY, iSizeX, iSizeY, (2000/MSEC) );	// �摜�̃L���v�`���[�A2����
	}else{
		ercd = CmrCapture( CAP_MODE_2, CAP_BANK_0, (2000/MSEC) );	// �摜�̃L���v�`���[�A2����
	}

	cap_wait = 0;
	while( IsCmrCapStart() == TRUE ){							// �摜�L���v�`���[�̏I���҂�
		dly_tsk( 10/MSEC );
		++cap_wait;
		if(cap_wait >= 100){
			ustmp = *(volatile unsigned short *)(FPGA_BASE + 0x0100);
			*(volatile unsigned short *)(FPGA_BASE + 0x0100) = ustmp & 0xFFFE;
			ustmp = *(volatile unsigned short *)(FPGA_BASE + 0x0100);
			CmrCapNg = 1;
			return(1);
		}
	}

	//CmrPrmShutterSet( 1 );							
	CmrDebugCnt = 10;	//################
	ercd = CmrCmdFixShutterCtl( 1 );	// �J�����ɒ��ځA�I�o�R�ݒ�l��ݒ�iFPGA���o�R���Ȃ��j
	if ( ercd != E_OK ){
		 ErrCodeSet( ercd );
	}
	para = CmrCmdManualGetParameter(1);
		
	if(f_fpga_ctrl == 0){	//20140915Miya FPGA����t���O 0:V1.001�p�@1:V1.002�p
		ercd = CmrTrim(CAP_BANK_0, TRIM_BANK_0, iStartX, iStartY, iSizeX, iSizeY, trimTimeOut ); // �Q���ډ摜�̃g���~���O
		if ( ercd != E_OK ){
			ercdStat = 20;
			return ercdStat;
		}
		while ( IsCmrTrimStart() == TRUE ){
			dly_tsk( 10/MSEC );	
		}
	}

	CmrDebugCnt = 11;	//################
	ercd = CmrResize(TRIM_BANK_0, RSZ_BANK_0, RSZ_MODE_1, iSizeX, iSizeY, resizeTimeOut );//20131210Miya cng	// �Q���ډ摜�̏k���E���k
	if ( ercd != E_OK ){
		ercdStat = 26;
		return ercdStat;
	}
	while ( IsCmrResizeStart() == TRUE ){
		dly_tsk( 10/MSEC );
	}

	CmrDebugCnt = 12;	//################
	ercd = CmrResizeGet(RSZ_BANK_0, 0, ( iReSizeX * iReSizeY ), &g_ubCapBuf[( iReSizeX * iReSizeY )] );//20131210Miya cng	// �Q���ڏk���摜�̎擾
	if ( ercd != E_OK ){
		ercdStat = 29;
		return ercdStat;
	}


	//CmrPrmShutterSet( cmrFixShutter1 );
	dly_tsk( 100/MSEC );
	CmrDebugCnt = 13;	//################
	ercd = CmrCmdFreezeOff();									// �J�����E�t���[�Y�E�R���g���[���E�I�t

	if(f_fpga_ctrl){	//20140915Miya FPGA����t���O 0:V1.001�p�@1:V1.002�p
		CmrDebugCnt = 14;	//################
		ustmp = *(volatile unsigned short *)(FPGA_BASE + 0x0100);
		if(ustmp & 0x8001 > 0){
			nop();
		}
		ercd = CmrCapture2( CAP_MODE_2, CAP_BANK_0, iStartX, iStartY, iSizeX, iSizeY, (2000/MSEC) );	// �摜�̃L���v�`���[�A2����
	}else{
		ercd = CmrCapture( CAP_MODE_2, CAP_BANK_0, (2000/MSEC) );	// �摜�̃L���v�`���[�A3����
	}

	cap_wait = 0;
	while( IsCmrCapStart() == TRUE ){							// �摜�L���v�`���[�̏I���҂�
		dly_tsk( 10/MSEC );
		++cap_wait;
		if(cap_wait >= 100){
			ustmp = *(volatile unsigned short *)(FPGA_BASE + 0x0100);
			*(volatile unsigned short *)(FPGA_BASE + 0x0100) = ustmp & 0xFFFE;
			ustmp = *(volatile unsigned short *)(FPGA_BASE + 0x0100);
			CmrCapNg = 1;
			return(1);
		}
	}
		
	if(f_fpga_ctrl == 0){	//20140915Miya FPGA����t���O 0:V1.001�p�@1:V1.002�p
		ercd = CmrTrim(CAP_BANK_0, TRIM_BANK_0, iStartX, iStartY, iSizeX, iSizeY, trimTimeOut ); // �R���ډ摜�̃g���~���O
		if ( ercd != E_OK ){
			ercdStat = 21;
			return ercdStat;
		}
		while ( IsCmrTrimStart() == TRUE ){
			dly_tsk( 10/MSEC );	
		}
	}

	CmrDebugCnt = 15;	//################
	ercd = CmrResize(TRIM_BANK_0, RSZ_BANK_0, RSZ_MODE_1, iSizeX, iSizeY, resizeTimeOut );//20131210Miya cng	// �R���ډ摜�̏k���E���k
	if ( ercd != E_OK ){
		ercdStat = 27;
		return ercdStat;
	}
	while ( IsCmrResizeStart() == TRUE ){
		dly_tsk( 10/MSEC );
	}

	CmrDebugCnt = 16;	//################
	ercd = CmrResizeGet(RSZ_BANK_0, 0, ( iReSizeX * iReSizeY ), &g_ubCapBuf[ ( ( iReSizeX * iReSizeY ) * 2 ) ] );//20131210Miya cng	// �R���ڏk���摜�̎擾
	if ( ercd != E_OK ){
		ercdStat = 30;
		return ercdStat;
	}
	

#if (CMR_SLEEP_CNTRL == 1)		
	/**	�J�����̃X���[�v����	***/
	CmrDebugCnt = 17;	//################
	ercd = CmrCmdSleep();
	if ( ercd != E_OK ){
		ercdStat = 15;
		return ercdStat;
	}		
	/** 	��	�I��	**/
#endif

	/** IR LED�̏��������@**/
	CmrDebugCnt = 18;	//################
	IrLedCrlOff( IR_LED_2 | IR_LED_3 | IR_LED_4 | IR_LED_5 ); 	// �ԊO��LED�̑S����
		
	CmrDebugCnt = 19;	//################
	return ercdStat;

}
#else
int	Check_Cap_Raw_Picture( void )	//20160601Miya
{
	int ercdStat = 0;
	ER ercd;
	volatile unsigned short ustmp, ustmp2, cap_wait, para;
	int xx;
	UB	led02, led03, led04, led05;	//20170418Miya LED
		
	CmrDebugCnt = 0;	//################

#if(CMRSENS) //20170424Miya 	�J�������x�Ή�
	if(g_TechMenuData.DebugHyouji == FLG_ON){
		led02 = led03 = led04 = 255;
		led05 = 0;
	}else{
		led02 = led03 = led04 = 160;
		led05 = 0;
	}
	ercd = IrLedSet( IR_LED_2, led02 );
	ercd = IrLedSet( IR_LED_3, led03 );
	ercd = IrLedSet( IR_LED_4, led04 );
	ercd = IrLedSet( IR_LED_5, led05 );
	IrLedCrlOn( IR_LED_2 | IR_LED_3 | IR_LED_4 | IR_LED_5 ); 	// �ԊO��LED�̑S�_��
#else
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

	if(g_cmr_dbgcnt == 1){
		IrLedCrlOn( IR_LED_2 );
	}else if(g_cmr_dbgcnt == 2){
		IrLedCrlOn( IR_LED_3 );
	}else if(g_cmr_dbgcnt == 3){
		IrLedCrlOn( IR_LED_4 );
	}else if(g_cmr_dbgcnt == 4){
		if(g_LedCheck == 0){	//20161031Miya Ver2204
			ercd = IrLedSet( IR_LED_5, 255 );						// �ԊO��LED5�̊K���ݒ�
		}
		IrLedCrlOn( IR_LED_2 | IR_LED_3 | IR_LED_4 | IR_LED_5 ); 	// �ԊO��LED�̑S�_��
	}else{
		IrLedCrlOn( IR_LED_2 | IR_LED_3 | IR_LED_4 | IR_LED_5 ); 	// �ԊO��LED�̑S�_��
	}
#endif
	dly_tsk( 100/MSEC );
				
	/** �J�����B�e����	**/
	CmrDebugCnt = 4;	//################
	if( g_TechMenuData.DebugHyouji == FLG_ON )
		//ercd = CmrCapture2( CAP_MODE_0, CAP_BANK_0, 180, 150, 400, 160, (2000/MSEC) );	// �摜�̃L���v�`���[�A1����
		ercd = CmrCapture2( CAP_MODE_0, CAP_BANK_0, iStartX, iStartY, iSizeX, iSizeY, (2000/MSEC) );	// �摜�̃L���v�`���[�A1����
	else
		ercd = CmrCapture2( CAP_MODE_0, CAP_BANK_0, 0, 0, 640, 480, (2000/MSEC) );	// �摜�̃L���v�`���[�A1����

	ustmp2 = *(volatile unsigned short *)(FPGA_BASE + 0x0210);
	if((ustmp2 & 0x0001) == 0x0001){
		nop();
	}
	ustmp = *(volatile unsigned short *)(FPGA_BASE + 0x0100);
	if((ustmp & 0x0001) == 0x0001){
		nop();
	}

	/** �g���~���O�摜�̏k���ƈ��k	**/
	CmrDebugCnt = 6;	//################
	if( g_TechMenuData.DebugHyouji == FLG_ON )
		ercd = CmrResize(TRIM_BANK_0, RSZ_BANK_0, RSZ_MODE_0, 400, 160, 1000 );//20131210Miya cng	// �P���ډ摜�̏k���E���k
		//ercd = CmrResize(TRIM_BANK_0, RSZ_BANK_0, RSZ_MODE_1, 400, 160, 1000 );//20131210Miya cng	// �P���ډ摜�̏k���E���k
	else
		ercd = CmrResize(TRIM_BANK_0, RSZ_BANK_0, RSZ_MODE_0, 640, 480, 1000 );//20131210Miya cng	// �P���ډ摜�̏k���E���k

	ustmp = *(volatile unsigned short *)(FPGA_BASE + 0x0220);
	if((ustmp & 0x0001) == 0x0001){
		nop();
	}

	if ( ercd != E_OK ){
		ercdStat = 25;
		return ercdStat;
	}
		
	CmrDebugCnt = 7;	//################
	if( g_TechMenuData.DebugHyouji == FLG_ON )
		ercd = CmrResizeGet(RSZ_BANK_0, 0, ( 200 * 80 ), &g_ubCapBuf[ 0 ]);//20131210Miya cng		// �P���ڏk���摜�̎擾
		//ercd = CmrResizeGet(RSZ_BANK_0, 0, ( 100 * 40 ), &g_ubCapBuf[ 0 ]);//20131210Miya cng		// �P���ڏk���摜�̎擾
	else
		ercd = CmrResizeGet(RSZ_BANK_0, 0, ( 320 * 240 ), &g_ubCapBuf[ 0 ]);//20131210Miya cng		// �P���ڏk���摜�̎擾

	if ( ercd != E_OK ){
		ercdStat = 28;
		return ercdStat;
	}
	
	/** IR LED�̏��������@**/
	CmrDebugCnt = 18;	//################
	IrLedCrlOff( IR_LED_2 | IR_LED_3 | IR_LED_4 | IR_LED_5 ); 	// �ԊO��LED�̑S����
		
	CmrDebugCnt = 19;	//################
	return ercdStat;

}
#endif

//-----------------------------------------------------------------------------
// �~�j�}�����B�e
//-----------------------------------------------------------------------------
static int CapMinimum( void )
{
	ER ercd;
	int ercdStat = 0;
	UH shutter1, shutter2;
	UH duty2, duty3, duty4, duty5;
	volatile unsigned short ustmp, cap_wait, i, cnt;

	duty2 = 255;		
	duty3 = 255;
	duty4 = 128;
	duty5 = 0;
	shutter1 = 5;
	shutter2 = 6;

//20160312Miya �ɏ����xUP �֐��R�[���O�ɓ_��
/*
	ercd = IrLedSet( IR_LED_2, duty2 );						// �ԊO��LED2�̊K���ݒ�
	if ( ercd != E_OK ){
		ercdStat = 1;
		return ercdStat;
	}
	ercd = IrLedSet( IR_LED_3, duty3 );						// �ԊO��LED3�̊K���ݒ�
	if ( ercd != E_OK ){
		ercdStat = 1;
		return ercdStat;
	}
	ercd = IrLedSet( IR_LED_4, duty4 );						// �ԊO��LED4�̊K���ݒ�
	if ( ercd != E_OK ){
		ercdStat = 1;
		return ercdStat;
	}
	ercd = IrLedSet( IR_LED_5, duty5 );						// �ԊO��LED5�̊K���ݒ�
	if ( ercd != E_OK ){
		ercdStat = 1;
		return ercdStat;
	}

	IrLedCrlOn( IR_LED_2 | IR_LED_3 | IR_LED_4 | IR_LED_5 ); 	// �ԊO��LED�̑S�_��
*/

//1���ډ摜(H-)�L���v�`���[�J�n		
	if(SAMEIMGCHK == 1){	//20160115Miya 
		for(i = 0 ; i < 8 ; i++ ){
			*(volatile unsigned long *)(CAP_BASE + 0x400000 + 4 * (unsigned long)i) = 0;
		}
	}

	ercd = CmrCmdFixShutterCtl( shutter1 );	// �J�����ɒ��ځA�I�o1�ݒ�l��ݒ�iFPGA���o�R���Ȃ��j
	if ( ercd != E_OK ){
		ercdStat = 2;
		return ercdStat;
	}
	dly_tsk( 140/MSEC );
	ercd = CmrCmdFreezeOff();									// �J�����E�t���[�Y�E�R���g���[���E�I�t
	if ( ercd != E_OK ){
		ercdStat = 3;
	}

	if(f_fpga_ctrl){	//20140915Miya FPGA����t���O 0:V1.001�p�@1:V1.002�p
		ercd = CmrCapture2( CAP_MODE_2, CAP_BANK_0, iStartX, iStartY, iSizeX, iSizeY, (2000/MSEC) );	// �摜�̃L���v�`���[�A1����
	}else{
		ercd = CmrCapture( CAP_MODE_1, CAP_BANK_0, (2000/MSEC) );	// �摜�̃L���v�`���[�A1����
	}

	cap_wait = 0;
	while( IsCmrCapStart() == TRUE ){							// �摜�L���v�`���[�̏I���҂�
		dly_tsk( 10/MSEC );
		++cap_wait;
		if(cap_wait >= 100){
			ustmp = *(volatile unsigned short *)(FPGA_BASE + 0x0100);
			*(volatile unsigned short *)(FPGA_BASE + 0x0100) = ustmp & 0xFFFE;
			ustmp = *(volatile unsigned short *)(FPGA_BASE + 0x0100);
			CmrCapNg = 1;
			ercdStat = 4;
			return(ercdStat);
		}
	}

	if(f_fpga_ctrl == 0){	//20140915Miya FPGA����t���O 0:V1.001�p�@1:V1.002�p
		/** �L���v�`���摜�̃g���~���O	**/
		ercd = CmrTrim(CAP_BANK_0, TRIM_BANK_0, iStartX, iStartY, iSizeX, iSizeY, trimTimeOut ); // �P���ډ摜�̃g���~���O
		if ( ercd != E_OK ){
			ercdStat = 5;
			return ercdStat;
		}
		while ( IsCmrTrimStart() == TRUE ){
			dly_tsk( 10/MSEC );	
		}
	}

	ercd = CmrCmdFixShutterCtl( shutter2 );	// �J�����ɒ��ځA�I�o1�ݒ�l��ݒ�iFPGA���o�R���Ȃ��j
	if ( ercd != E_OK ){
		ercdStat = 6;
		return ercdStat;
	}

	/** �g���~���O�摜�̏k���ƈ��k	**/
	ercd = CmrResize(TRIM_BANK_0, RSZ_BANK_0, RSZ_MODE_1, iSizeX, iSizeY, resizeTimeOut );//20131210Miya cng	// �P���ډ摜�̏k���E���k
	if ( ercd != E_OK ){
		ercdStat = 7;
		return ercdStat;
	}
	while ( IsCmrResizeStart() == TRUE ){
		dly_tsk( 10/MSEC );
	}
		
	ercd = CmrResizeGet(RSZ_BANK_0, 0, ( iReSizeX * iReSizeY ), &g_ubCapBuf[ 0 ]);//1���ڏk���摜�̎擾
	if ( ercd != E_OK ){
		ercdStat = 8;
		return ercdStat;
	}

	if(SAMEIMGCHK == 1){	//20160115Miya 
		cnt = 0;
		for(i = 0 ; i < 8 ; i++ ){	//20151216Miya
				if( *(volatile unsigned long *)(CAP_BASE + 0x400000 + 4 * (unsigned long)i) == 0 ){
				++cnt;
			}
		}
		if(cnt >= 4){
			g_SameImgGet = g_SameImgGet | 0x02;
		}
	}

	dly_tsk( 90/MSEC );
	ercd = CmrCmdFreezeOff();									// �J�����E�t���[�Y�E�R���g���[���E�I�t
	if ( ercd != E_OK ){
		ercdStat = 9;
	}



//2���ډ摜(M-)�L���v�`���[�J�n		
	if(SAMEIMGCHK == 1){	//20160115Miya 
		for(i = 0 ; i < 8 ; i++ ){
			*(volatile unsigned long *)(CAP_BASE + 0x500000 + 4 * (unsigned long)i) = 0;
		}
	}

	if(f_fpga_ctrl){	//20140915Miya FPGA����t���O 0:V1.001�p�@1:V1.002�p
		ercd = CmrCapture2( CAP_MODE_2, CAP_BANK_1, iStartX, iStartY, iSizeX, iSizeY, (2000/MSEC) );	// �摜�̃L���v�`���[�A2����
	}else{
		ercd = CmrCapture( CAP_MODE_3, CAP_BANK_1, (2000/MSEC) );	// �摜�̃L���v�`���[�A2����
	}

	cap_wait = 0;
	while( IsCmrCapStart() == TRUE ){							// �摜�L���v�`���[�̏I���҂�
		dly_tsk( 10/MSEC );
		++cap_wait;
		if(cap_wait >= 100){
			ustmp = *(volatile unsigned short *)(FPGA_BASE + 0x0100);
			*(volatile unsigned short *)(FPGA_BASE + 0x0100) = ustmp & 0xFFFE;
			ustmp = *(volatile unsigned short *)(FPGA_BASE + 0x0100);
			CmrCapNg = 1;
			ercdStat = 10;
			return(ercdStat);
		}
	}
		
	if(f_fpga_ctrl == 0){	//20140915Miya FPGA����t���O 0:V1.001�p�@1:V1.002�p
		ercd = CmrTrim(CAP_BANK_1, TRIM_BANK_1, iStartX, iStartY, iSizeX, iSizeY, trimTimeOut ); // �Q���ډ摜�̃g���~���O
		if ( ercd != E_OK ){
			ercdStat = 11;
			return ercdStat;
		}
		while ( IsCmrTrimStart() == TRUE ){
			dly_tsk( 10/MSEC );	
		}
	}

	ercd = CmrResize(TRIM_BANK_1, RSZ_BANK_1, RSZ_MODE_1, iSizeX, iSizeY, resizeTimeOut );//20131210Miya cng	// �Q���ډ摜�̏k���E���k
	if ( ercd != E_OK ){
		ercdStat = 12;
		return ercdStat;
	}
	while ( IsCmrResizeStart() == TRUE ){
		dly_tsk( 10/MSEC );
	}

	ercd = CmrResizeGet(RSZ_BANK_1, 0, ( iReSizeX * iReSizeY ), &g_ubCapBuf[( iReSizeX * iReSizeY )] );//20131210Miya cng	// �Q���ڏk���摜�̎擾
	if ( ercd != E_OK ){
		ercdStat = 13;
		return ercdStat;
	}

	if(SAMEIMGCHK == 1){	//20160115Miya 
		cnt = 0;
		for(i = 0 ; i < 8 ; i++ ){	//20151216Miya
			if(*(volatile unsigned long *)(CAP_BASE + 0x500000 + 4 * (unsigned long)i) == 0){
				++cnt;
			}
		}
		if(cnt >= 4){
			g_SameImgGet = g_SameImgGet | 0x04;
		}
	}

	g_AuthLog.now_seq_num = 1;
	return ercdStat;
}


//-----------------------------------------------------------------------------
// �o���^�����B�e
//-----------------------------------------------------------------------------
static int CapBantam( int hantei )
{
	ER ercd;
	int ercdStat = 0, rtn;
	UB rslt;
	volatile unsigned short ustmp, cap_wait, i, cnt;

	dly_tsk( 90/MSEC );
	ercd = CmrCmdFreezeOff();									// �J�����E�t���[�Y�E�R���g���[���E�I�t
	if ( ercd != E_OK ){
		ercdStat = 1;
	}

//2���ډ摜(H)�L���v�`���[�J�n		
	if(SAMEIMGCHK == 1){	//20160115Miya 
		for(i = 0 ; i < 8 ; i++ ){
			*(volatile unsigned long *)(CAP_BASE + 0x500000 + 4 * (unsigned long)i) = 0;
		}
	}

	if(f_fpga_ctrl){	//20140915Miya FPGA����t���O 0:V1.001�p�@1:V1.002�p
		ercd = CmrCapture2( CAP_MODE_2, CAP_BANK_1, iStartX, iStartY, iSizeX, iSizeY, (2000/MSEC) );	// �摜�̃L���v�`���[�A2����
	}else{
		ercd = CmrCapture( CAP_MODE_3, CAP_BANK_1, (2000/MSEC) );	// �摜�̃L���v�`���[�A2����
	}

	cap_wait = 0;
	while( IsCmrCapStart() == TRUE ){							// �摜�L���v�`���[�̏I���҂�
		dly_tsk( 10/MSEC );
		++cap_wait;
		if(cap_wait >= 100){
			ustmp = *(volatile unsigned short *)(FPGA_BASE + 0x0100);
			*(volatile unsigned short *)(FPGA_BASE + 0x0100) = ustmp & 0xFFFE;
			ustmp = *(volatile unsigned short *)(FPGA_BASE + 0x0100);
			CmrCapNg = 1;
			ercdStat = 2;
			return(ercdStat);
		}
	}
		
	if(f_fpga_ctrl == 0){	//20140915Miya FPGA����t���O 0:V1.001�p�@1:V1.002�p
		ercd = CmrTrim(CAP_BANK_1, TRIM_BANK_1, iStartX, iStartY, iSizeX, iSizeY, trimTimeOut ); // �Q���ډ摜�̃g���~���O
		if ( ercd != E_OK ){
			ercdStat = 3;
			return ercdStat;
		}
		while ( IsCmrTrimStart() == TRUE ){
			dly_tsk( 10/MSEC );	
		}
	}

	ercd = CmrCmdFixShutterCtl( cmrFixShutter3 );	// �J�����ɒ��ځA�I�o�R�ݒ�l��ݒ�iFPGA���o�R���Ȃ��j
	if ( ercd != E_OK ){
		 ErrCodeSet( ercd );
	}

	ercd = CmrResize(TRIM_BANK_1, RSZ_BANK_1, RSZ_MODE_1, iSizeX, iSizeY, resizeTimeOut );//20131210Miya cng	// �Q���ډ摜�̏k���E���k
	if ( ercd != E_OK ){
		ercdStat = 4;
		return ercdStat;
	}
	while ( IsCmrResizeStart() == TRUE ){
		dly_tsk( 10/MSEC );
	}

	ercd = CmrResizeGet(RSZ_BANK_1, 0, ( iReSizeX * iReSizeY ), &g_ubCapBuf[0] );//20131210Miya cng	// �Q���ڏk���摜�̎擾
	if ( ercd != E_OK ){
		ercdStat = 5;
		return ercdStat;
	}

	if(SAMEIMGCHK == 1){	//20160115Miya 
		cnt = 0;
		for(i = 0 ; i < 8 ; i++ ){	//20151216Miya
			if(*(volatile unsigned long *)(CAP_BASE + 0x500000 + 4 * (unsigned long)i) == 0){
				++cnt;
			}
		}
		if(cnt >= 4){
			g_SameImgGet = g_SameImgGet | 0x08;
		}
	}

	//H�摜����(�Â����ǂ���)
	rslt = HiImgHantei(hantei);
	if( rslt == 1 ){
		//memcpy(&g_ubCapBuf[0], &g_ubCapBuf[( iReSizeX * iReSizeY )], ( iReSizeX * iReSizeY ) );	//Mid->High
		rtn = CapMidle();
		g_AuthLog.now_seq_num = 3;
	}else if( rslt == 2 ){
		rtn = CapHeavy();
		g_AuthLog.now_seq_num = 4;
	}else{
		g_AuthLog.now_seq_num = 2;
	}

	return ercdStat;
}


//-----------------------------------------------------------------------------
// �~�h�����B�e
//-----------------------------------------------------------------------------
static int CapMidle( void )
{
	ER ercd;
	int ercdStat = 0;
	UH shutter1, shutter2;
	UH duty2, duty3, duty4, duty5;
	volatile unsigned short ustmp, cap_wait, i, cnt;

	dly_tsk( 90/MSEC );
	ercd = CmrCmdFreezeOff();									// �J�����E�t���[�Y�E�R���g���[���E�I�t
	if ( ercd != E_OK ){
		ercdStat = 1;
	}

//2���ډ摜(L)�L���v�`���[�J�n		
	if(SAMEIMGCHK == 1){	//20160115Miya 
		for(i = 0 ; i < 8 ; i++ ){
			*(volatile unsigned long *)(CAP_BASE + 0x500000 + 4 * (unsigned long)i) = 0;
		}
	}

	if(f_fpga_ctrl){	//20140915Miya FPGA����t���O 0:V1.001�p�@1:V1.002�p
		ercd = CmrCapture2( CAP_MODE_2, CAP_BANK_1, iStartX, iStartY, iSizeX, iSizeY, (2000/MSEC) );	// �摜�̃L���v�`���[�A2����
	}else{
		ercd = CmrCapture( CAP_MODE_3, CAP_BANK_1, (2000/MSEC) );	// �摜�̃L���v�`���[�A2����
	}

	cap_wait = 0;
	while( IsCmrCapStart() == TRUE ){							// �摜�L���v�`���[�̏I���҂�
		dly_tsk( 10/MSEC );
		++cap_wait;
		if(cap_wait >= 100){
			ustmp = *(volatile unsigned short *)(FPGA_BASE + 0x0100);
			*(volatile unsigned short *)(FPGA_BASE + 0x0100) = ustmp & 0xFFFE;
			ustmp = *(volatile unsigned short *)(FPGA_BASE + 0x0100);
			CmrCapNg = 1;
			ercdStat = 2;
			return(ercdStat);
		}
	}
		
	if(f_fpga_ctrl == 0){	//20140915Miya FPGA����t���O 0:V1.001�p�@1:V1.002�p
		ercd = CmrTrim(CAP_BANK_1, TRIM_BANK_1, iStartX, iStartY, iSizeX, iSizeY, trimTimeOut ); // �Q���ډ摜�̃g���~���O
		if ( ercd != E_OK ){
			ercdStat = 3;
			return ercdStat;
		}
		while ( IsCmrTrimStart() == TRUE ){
			dly_tsk( 10/MSEC );	
		}
	}

	ercd = CmrResize(TRIM_BANK_1, RSZ_BANK_1, RSZ_MODE_1, iSizeX, iSizeY, resizeTimeOut );//20131210Miya cng	// �Q���ډ摜�̏k���E���k
	if ( ercd != E_OK ){
		ercdStat = 4;
		return ercdStat;
	}
	while ( IsCmrResizeStart() == TRUE ){
		dly_tsk( 10/MSEC );
	}

	ercd = CmrResizeGet(RSZ_BANK_1, 0, ( iReSizeX * iReSizeY ), &g_ubCapBuf[2 * ( iReSizeX * iReSizeY )] );//20131210Miya cng	// �Q���ڏk���摜�̎擾
	if ( ercd != E_OK ){
		ercdStat = 5;
		return ercdStat;
	}

	if(SAMEIMGCHK == 1){	//20160115Miya 
		cnt = 0;
		for(i = 0 ; i < 8 ; i++ ){	//20151216Miya
			if(*(volatile unsigned long *)(CAP_BASE + 0x500000 + 4 * (unsigned long)i) == 0){
				++cnt;
			}
		}
		if(cnt >= 4){
			g_SameImgGet = g_SameImgGet | 0x10;
		}
	}
}


//-----------------------------------------------------------------------------
// �w�r�[���B�e
//-----------------------------------------------------------------------------
static int CapHeavy( void )
{
	ER ercd;
	int ercdStat = 0;
	UH shutter1, shutter2, gain;
	UH duty2, duty3, duty4, duty5;
	volatile unsigned short ustmp, cap_wait, i, cnt;

	duty2 = 255;		
	duty3 = 255;
	duty4 = 255;
	duty5 = 0;
	shutter1 = cmrFixShutter1;
	shutter2 = cmrFixShutter2;
	gain = ini_cmrGain + 2;

	ercd = CmrCmdManualGainCtl( ( UB )gain );		// �J�����ɒ��ځA�Q�C���ݒ�l��ݒ�iFPGA���o�R���Ȃ��j
	if ( ercd != E_OK ){
		ercdStat = 1;
		return ercdStat;
	}

//1���ډ摜(H+)�L���v�`���[�J�n		
	ercd = CmrCmdFixShutterCtl( shutter1 );	// �J�����ɒ��ځA�I�o1�ݒ�l��ݒ�iFPGA���o�R���Ȃ��j
	if ( ercd != E_OK ){
		ercdStat = 2;
		return ercdStat;
	}
	dly_tsk( 140/MSEC );
	ercd = CmrCmdFreezeOff();									// �J�����E�t���[�Y�E�R���g���[���E�I�t
	if ( ercd != E_OK ){
		ercdStat = 3;
	}

	if(SAMEIMGCHK == 1){	//20160115Miya 
		for(i = 0 ; i < 8 ; i++ ){
			*(volatile unsigned long *)(CAP_BASE + 0x400000 + 4 * (unsigned long)i) = 0;
		}
	}

	if(f_fpga_ctrl){	//20140915Miya FPGA����t���O 0:V1.001�p�@1:V1.002�p
		ercd = CmrCapture2( CAP_MODE_2, CAP_BANK_0, iStartX, iStartY, iSizeX, iSizeY, (2000/MSEC) );	// �摜�̃L���v�`���[�A1����
	}else{
		ercd = CmrCapture( CAP_MODE_1, CAP_BANK_0, (2000/MSEC) );	// �摜�̃L���v�`���[�A1����
	}

	cap_wait = 0;
	while( IsCmrCapStart() == TRUE ){							// �摜�L���v�`���[�̏I���҂�
		dly_tsk( 10/MSEC );
		++cap_wait;
		if(cap_wait >= 100){
			ustmp = *(volatile unsigned short *)(FPGA_BASE + 0x0100);
			*(volatile unsigned short *)(FPGA_BASE + 0x0100) = ustmp & 0xFFFE;
			ustmp = *(volatile unsigned short *)(FPGA_BASE + 0x0100);
			CmrCapNg = 1;
			ercdStat = 4;
			return(ercdStat);
		}
	}

	if(f_fpga_ctrl == 0){	//20140915Miya FPGA����t���O 0:V1.001�p�@1:V1.002�p
		/** �L���v�`���摜�̃g���~���O	**/
		ercd = CmrTrim(CAP_BANK_0, TRIM_BANK_0, iStartX, iStartY, iSizeX, iSizeY, trimTimeOut ); // �P���ډ摜�̃g���~���O
		if ( ercd != E_OK ){
			ercdStat = 5;
			return ercdStat;
		}
		while ( IsCmrTrimStart() == TRUE ){
			dly_tsk( 10/MSEC );	
		}
	}

	ercd = CmrCmdFixShutterCtl( shutter2 );	// �J�����ɒ��ځA�I�o1�ݒ�l��ݒ�iFPGA���o�R���Ȃ��j
	if ( ercd != E_OK ){
		 ErrCodeSet( ercd );
	}

	/** �g���~���O�摜�̏k���ƈ��k	**/
	ercd = CmrResize(TRIM_BANK_0, RSZ_BANK_0, RSZ_MODE_1, iSizeX, iSizeY, resizeTimeOut );//20131210Miya cng	// �P���ډ摜�̏k���E���k
	if ( ercd != E_OK ){
		ercdStat = 6;
		return ercdStat;
	}
	while ( IsCmrResizeStart() == TRUE ){
		dly_tsk( 10/MSEC );
	}
		
	ercd = CmrResizeGet(RSZ_BANK_0, 0, ( iReSizeX * iReSizeY ), &g_ubCapBuf[ 0 ]);//1���ڏk���摜�̎擾
	if ( ercd != E_OK ){
		ercdStat = 7;
		return ercdStat;
	}

	if(SAMEIMGCHK == 1){	//20160115Miya 
		cnt = 0;
		for(i = 0 ; i < 8 ; i++ ){	//20151216Miya
			if(*(volatile unsigned long *)(CAP_BASE + 0x400000 + 4 * (unsigned long)i) == 0){
				++cnt;
			}
		}
		if(cnt >= 4){
			g_SameImgGet = g_SameImgGet | 0x20;
		}
	}

	dly_tsk( 90/MSEC );
	ercd = CmrCmdFreezeOff();									// �J�����E�t���[�Y�E�R���g���[���E�I�t
	if ( ercd != E_OK ){
		ercdStat = 8;
	}


//2���ډ摜(M-)�L���v�`���[�J�n		
	if(SAMEIMGCHK == 1){	//20160115Miya 
		for(i = 0 ; i < 8 ; i++ ){
			*(volatile unsigned long *)(CAP_BASE + 0x500000 + 4 * (unsigned long)i) = 0;
		}
	}

	if(f_fpga_ctrl){	//20140915Miya FPGA����t���O 0:V1.001�p�@1:V1.002�p
		ercd = CmrCapture2( CAP_MODE_2, CAP_BANK_1, iStartX, iStartY, iSizeX, iSizeY, (2000/MSEC) );	// �摜�̃L���v�`���[�A2����
	}else{
		ercd = CmrCapture( CAP_MODE_3, CAP_BANK_1, (2000/MSEC) );	// �摜�̃L���v�`���[�A2����
	}

	cap_wait = 0;
	while( IsCmrCapStart() == TRUE ){							// �摜�L���v�`���[�̏I���҂�
		dly_tsk( 10/MSEC );
		++cap_wait;
		if(cap_wait >= 100){
			ustmp = *(volatile unsigned short *)(FPGA_BASE + 0x0100);
			*(volatile unsigned short *)(FPGA_BASE + 0x0100) = ustmp & 0xFFFE;
			ustmp = *(volatile unsigned short *)(FPGA_BASE + 0x0100);
			CmrCapNg = 1;
			ercdStat = 9;
			return(ercdStat);
		}
	}
		
	if(f_fpga_ctrl == 0){	//20140915Miya FPGA����t���O 0:V1.001�p�@1:V1.002�p
		ercd = CmrTrim(CAP_BANK_1, TRIM_BANK_1, iStartX, iStartY, iSizeX, iSizeY, trimTimeOut ); // �Q���ډ摜�̃g���~���O
		if ( ercd != E_OK ){
			ercdStat = 10;
			return ercdStat;
		}
		while ( IsCmrTrimStart() == TRUE ){
			dly_tsk( 10/MSEC );	
		}
	}

	ercd = CmrResize(TRIM_BANK_1, RSZ_BANK_1, RSZ_MODE_1, iSizeX, iSizeY, resizeTimeOut );//20131210Miya cng	// �Q���ډ摜�̏k���E���k
	if ( ercd != E_OK ){
		ercdStat = 11;
		return ercdStat;
	}
	while ( IsCmrResizeStart() == TRUE ){
		dly_tsk( 10/MSEC );
	}

	ercd = CmrResizeGet(RSZ_BANK_1, 0, ( iReSizeX * iReSizeY ), &g_ubCapBuf[( iReSizeX * iReSizeY )] );//20131210Miya cng	// �Q���ڏk���摜�̎擾
	if ( ercd != E_OK ){
		ercdStat = 12;
		return ercdStat;
	}

	if(SAMEIMGCHK == 1){	//20160115Miya 
		cnt = 0;
		for(i = 0 ; i < 8 ; i++ ){	//20151216Miya
			if(*(volatile unsigned long *)(CAP_BASE + 0x500000 + 4 * (unsigned long)i) == 0){
				++cnt;
			}
		}
		if(cnt >= 4){
			g_SameImgGet = g_SameImgGet | 0x80;
		}
	}
}

//20160312Miya �ɏ����xUP LED�̓_��/�����̃^�C�~���O������(���g���C���_���̂܂܁A�o�^���X���̂܂�)
static int IrLedOnOffSet(int sw, UH duty2, UH duty3, UH duty4, UH duty5)
{
	ER ercd;
	int ercdStat = 0;
	UB	led02, led03, led04, led05;	//20170418Miya LED

	if(sw == 1){	//ON

#if(CMRSENS) //20170424Miya 	�J�������x�Ή�
		if(g_TechMenuData.DebugHyouji == FLG_ON && (GetScreenNo() == LCD_SCREEN120 || GetScreenNo() == LCD_SCREEN121)){
			led02 = led03 = led04 = 255;
			led05 = 0;
		}else{
			led02 = led03 = led04 = 160;
			led05 = 0;
		}
		ercd = IrLedSet( IR_LED_2, led02 );
		ercd = IrLedSet( IR_LED_3, led03 );
		ercd = IrLedSet( IR_LED_4, led04 );
		ercd = IrLedSet( IR_LED_5, led05 );
#else
		ercd = IrLedSet( IR_LED_2, duty2 );						// �ԊO��LED2�̊K���ݒ�
		if ( ercd != E_OK ){
			ercdStat = 1;
			return ercdStat;
		}
		ercd = IrLedSet( IR_LED_3, duty3 );						// �ԊO��LED3�̊K���ݒ�
		if ( ercd != E_OK ){
			ercdStat = 1;
			return ercdStat;
		}
		ercd = IrLedSet( IR_LED_4, duty4 );						// �ԊO��LED4�̊K���ݒ�
		if ( ercd != E_OK ){
			ercdStat = 1;
			return ercdStat;
		}
		ercd = IrLedSet( IR_LED_5, duty5 );						// �ԊO��LED5�̊K���ݒ�
		if ( ercd != E_OK ){
			ercdStat = 1;
			return ercdStat;
		}
#endif

		IrLedCrlOn( IR_LED_2 | IR_LED_3 | IR_LED_4 | IR_LED_5 ); 	// �ԊO��LED�̑S�_��
		g_IrLed_SW = 1;
	}else{
		IrLedCrlOff( IR_LED_2 | IR_LED_3 | IR_LED_4 | IR_LED_5 ); 	// �ԊO��LED�̑S����
		g_IrLed_SW = 0;
	}		
	
}


#if ( NEWCMR == 1 )
//-----------------------------------------------------------------------------
// ���{�P�~�R���ЃJ�����@NCM03-V��power on����
//-----------------------------------------------------------------------------
// ���{�P�~�R���ЃJ���� ���W�X�^�������ݔėp�֐�
// 20160601Miya
static ER NCmr_GenericWrt(unsigned char addr, unsigned char dat)
{
	ER ercd;
	UB cAddr[1];
	UB cSndData[1];
	
	cAddr[0] = addr;
	cSndData[0] = dat;

	ercd = NCmrI2C_Send( cAddr, cSndData, 1 );		// �J�����E�R�}���h��I2C���M
	if ( ercd != E_OK ){
		nop();
		return ercd;
	}

	return ercd;
}
// ���{�P�~�R���ЃJ���� �p���[�I�����������ݔėp�֐�
static ER NCmr_PowerOn( void )
{
	UB cAddr[] =    { 0x03, 0x62, 0x59, 0x5e, 0x60, 0x63 };
	UB cSndData[] = { 0x00, 0x40, 0x00, 0x18, 0x40, 0x02 };
	UB cRcvData[ 7 ];
	ER ercd;
	int iSendSize = 6;
	int iRcvSize = 6;
	int i;
	
	ercd = NCmrI2C_Send( cAddr, cSndData, iSendSize );		// �J�����E�R�}���h��I2C���M
	if ( ercd != E_OK ){
		nop();
		return ercd;
	}
	memset( cRcvData, 0, 7 );
	ercd = NCmrI2C_Rcv( cAddr, cRcvData, iRcvSize );		//   ���@I2C��M
	if ( ercd != E_OK ){
		nop();
		return ercd;
	}
	for ( i=0; i<6; i++ ){									// ���M�f�[�^�ƁA��M�f�[�^��Verify Check
		if ( cSndData[ i ] != cRcvData[ i ] ){
			ercd = E_OBJ;
			return ercd;
		}
	}
	NCmrCapAreaSel( CAP_MODE_0 );	// �L���v�`���[���[�h�̏����ݒ�
	NCmrCapBnkSel( CAP_BANK_0 );	// �L���v�`���[����̈�̏����ݒ�
	NCmrCapWaitSet( 3 );			//	�摜�捞�ݎ��̐擪�p���t���[����:0�`7
	
	return ercd;
}
// ���{�P�~�R���ЃJ���� �}�j���A���G�N�X�|�[�W���[�ݒ�
// 20160601Miya
static ER NCmr_WrtIniPara( void )
{
	UB cAddr[] =    { 0x03, 0x04, 0x48, 0x49, 0x10, 0x11, 0x0D, 0x0E, 0x0F, 0x03, 0x1A };
	UB cSndData[] = { 0x02, 0x9A, 0x00, 0x00, 0x01, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00 };
	ER ercd;
	int iSendSize = 11;
	int i;

	ercd = NCmrI2C_Send( cAddr, cSndData, iSendSize );		// �J�����E�R�}���h��I2C���M
	if ( ercd != E_OK ){
		nop();
		return ercd;
	}
	
	return ercd;
}
// ���{�P�~�R���ЃJ���� �I�����Ԑݒ�
// 20160601Miya
static ER NCmr_WrtExpPara( unsigned char datH, unsigned char datM, unsigned char datL )
{
	UB cAddr[] =    { 0x0D, 0x0E, 0x0F };
	UB cSndData[3], cRcvData[3];
	ER ercd;
	int iSendSize = 3;
	int i;

	NCmr_GenericWrt(0x03, 0x02);	//BANK-C�w��
	
	cSndData[0] = datH;
	cSndData[1] = datM;
	cSndData[2] = datL;

	ercd = NCmrI2C_Send( cAddr, cSndData, iSendSize );		// �J�����E�R�}���h��I2C���M
	if ( ercd != E_OK ){
		nop();
		return ercd;
	}
/*
	ercd = NCmrI2C_Rcv( cAddr, cRcvData, iSendSize );		//   ���@I2C��M
	for(i=0 ; i<iSendSize ; i++){
		if(cSndData[i] != cRcvData[i]){
			ercd = 99;
		}
	}
*/
	
	return ercd;
}
// ���{�P�~�R���ЃJ���� �K���}1.0�ݒ�
// 20160601Miya
static ER NCmr_WrtGmma10( void )
{
	UB cAddr[] =    { 0x03, 0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B };
	UB cSndData[] = { 0x01, 0x00, 0x04, 0x08, 0x0C, 0x10, 0x18, 0x20, 0x30, 0x40, 0x60, 0x80, 0xA0, 0xC0, 0xE0, 0xFF };
	UB cRcvData[16];
	ER ercd;
	int iSendSize = 16;
	int iRcvSize = 16;
	int i;

	ercd = NCmrI2C_Send( cAddr, cSndData, iSendSize );		// �J�����E�R�}���h��I2C���M
	if ( ercd != E_OK ){
		nop();
		return ercd;
	}
	
	return ercd;
}
// ���{�P�~�R���ЃJ���� �I�[�g�G�N�X�|�[�W���[�ݒ�
// 20160601Miya
static ER NCmr_WrtAutoExpPara( void )
{
	UB cAddr[] =    { 0x03, 0x04, 0x48, 0x49, 0x10, 0x11, 0x0D, 0x0E, 0x0F };
	UB cSndData[] = { 0x02, 0x98, 0x08, 0x0C, 0x01, 0x00, 0x00, 0x80, 0x00 };
	ER ercd;
	int iSendSize = 9;
	int i;

	ercd = NCmrI2C_Send( cAddr, cSndData, iSendSize );		// �J�����E�R�}���h��I2C���M
	if ( ercd != E_OK ){
		nop();
		return ercd;
	}
	
	return ercd;
}
static ER NCmr_ReadIniPara( void )
{
	UB cAddr[] =    { 0x03, 0x04, 0x48, 0x49, 0x10, 0x11, 0x0D, 0x0E, 0x0F, 0x03, 0x1A };
	UB cRcvData[16];
	ER ercd;
	int iRcvSize = 11;
	int i;

	NCmr_GenericWrt(0x03, 0x02);	//BANK-C�w��

	memset( cRcvData, 0, 16 );
	ercd = NCmrI2C_Rcv( cAddr, cRcvData, iRcvSize );		//   ���@I2C��M
	if ( ercd != E_OK ){
		nop();
		return ercd;
	}
	
	return ercd;
}
static ER NCmr_ReadGmma( void )
{
	UB cAddr[] =    { 0x03, 0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B };

	UB cSndData[1];
	UB cRcvData[16];
	ER ercd;
	int iRcvSize = 16;
	int i;
	
	NCmr_GenericWrt(0x03, 0x01);	//BANK-B�w��
	
	memset( cRcvData, 0, 16 );
	ercd = NCmrI2C_Rcv( cAddr, cRcvData, iRcvSize );		//   ���@I2C��M
	if ( ercd != E_OK ){
		nop();
		return ercd;
	}
	
	return ercd;
}
static ER NCmr_ReadPara( int bnk )
{
	UB cAddr0[] =    { 0x03, 0x1A, 0x1B };
	UB cAddr1[] =    { 0x03, 0x5E };
	UB cAddr2[] =    { 0x03, 0x04, 0x12, 0x13, 0x14, 0x15 };
	UB cSndData[1];
	UB cRcvData[8];
	ER ercd;
	int iSendSize;
	int iRcvSize;
	int i;

	if(bnk == 0){
		cSndData[0] = 0x00;
		ercd = NCmrI2C_Send( cAddr0, cSndData, 1 );		// �J�����E�R�}���h��I2C���M BANK-A�ؑ�
		if ( ercd != E_OK ){
			nop();
			return ercd;
		}
		memset( cRcvData, 0, 8 );
		iRcvSize = 3;
		ercd = NCmrI2C_Rcv( cAddr0, cRcvData, iRcvSize );		//   ���@I2C��M
		if ( ercd != E_OK ){
			nop();
			return ercd;
		}
	}else if(bnk == 1){
		cSndData[0] = 0x01;
		ercd = NCmrI2C_Send( cAddr1, cSndData, 1 );		// �J�����E�R�}���h��I2C���M BANK-B�ؑ�
		if ( ercd != E_OK ){
			nop();
			return ercd;
		}
		memset( cRcvData, 0, 8 );
		iRcvSize = 2;
		ercd = NCmrI2C_Rcv( cAddr1, cRcvData, iRcvSize );		//   ���@I2C��M
		if ( ercd != E_OK ){
			nop();
			return ercd;
		}
		
	}else if(bnk == 2){
		cSndData[0] = 0x02;
		ercd = NCmrI2C_Send( cAddr2, cSndData, 1 );		// �J�����E�R�}���h��I2C���M BANK-C�ؑ�
		if ( ercd != E_OK ){
			nop();
			return ercd;
		}
		memset( cRcvData, 0, 8 );
		iRcvSize = 6;
		ercd = NCmrI2C_Rcv( cAddr2, cRcvData, iRcvSize );		//   ���@I2C��M
		if ( ercd != E_OK ){
			nop();
			return ercd;
		}
	}
	
	return ercd;
}
// �I�����Ԃ����Z����
unsigned long NCmr_CalExpTime(int mode, unsigned long st)
{
	unsigned long r_st;
	int x, y, cnt;
	int x1, x2, y1, y2, sizx;
	double sum, den, kei;

	if(mode == 0){
		x1 = 50;	x2 = 150;
		y1 = 10;	y2 = 70;
		sizx = 200;
	}else if(mode == 1){
		x1 = 25;	x2 = 75;
		y1 =  5;	y2 = 35;
		sizx = 100;
	}else{
		x1 = 200;	x2 = 300;
		y1 = 80;	y2 = 160;
		sizx = 320;
	}

	sum = 0.0;
	cnt = 0;
	for(y = y1 ; y < y2 ; y++){
		for(x = x1 ; x < x2 ; x++){
			sum += (double)g_ubCapBuf[ sizx * y + x];
			++cnt;
		}
	}
	den = sum / (double)cnt;
	if(den <= 120.0 && den >= 90.0){
		kei = 1.0;
	}else if(den > 250){
		kei = 0.1;
	}else{
		kei = 100.0 / den;
	}
	
	r_st = (unsigned long)(kei * (double)st);
	return(r_st);
}

#endif


//-----------------------------------------------------------------------------
// �J�����R�}���h: �J�����p�����[�^��M
//-----------------------------------------------------------------------------
static unsigned short CmrCmdManualGetParameter( UB para )
{
	UB cCmd[8];
	int iSendSize;
	int iRecvSize;
	unsigned short ret_para;
	UB	cRetBuf[ 10 ];
	UB	*p;
	ER	ercd;
	int i;
	
	cCmd[0] = 0x00;
	cCmd[1] = 0x00;
	cCmd[2] = 0x20;
	cCmd[3] = 0x04;
	iSendSize = sizeof cCmd;

	switch(para)
	{
		case 0:	//Luminance Control
			cCmd[4] = 0x02;		//Block
			cCmd[5] = 0x00;		//offset low
			cCmd[6] = 0x00;		//offset high
			cCmd[7] = 0x01;		//length
			iRecvSize = 10;		//Recive Buff Size
			break;
		case 1:	//Fix Shutter
			cCmd[4] = 0x02;		//Block
			cCmd[5] = 0x03;		//offset low
			cCmd[6] = 0x00;		//offset high
			cCmd[7] = 0x01;		//length
			iRecvSize = 10;		//Recive Buff Size
			break;
		case 2:	//Gain
			cCmd[4] = 0x02;		//Block
			cCmd[5] = 0x06;		//offset low
			cCmd[6] = 0x00;		//offset high
			cCmd[7] = 0x01;		//length
			iRecvSize = 10;		//Recive Buff Size
			break;
		default:
			break;
	}
	
	ercd = CmrPktSend( cCmd, iSendSize, iRecvSize );			// �R�}���h���M
	if (ercd == E_OK) {
		ercd = CmrPktRecv( cRetBuf, iRecvSize, (3000/MSEC));	// ������M
		if (ercd == E_OK) {
			if(cRetBuf[4] != 0){
				ret_para = 0xffff;
			}else{
				ret_para = (unsigned short)cRetBuf[9];
			}
		}	else if ( ercd == E_TMOUT ) {
			ret_para = 0xffff;
		}	else if ( ercd == E_RLWAI ) {
			ret_para = 0xffff;
		}	else if ( ercd == E_CTX ) {
			ret_para = 0xffff;
		}	else {
			ret_para = 0xffff;
		}
	}

	return ret_para;
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
	
	if(CmrWakFlg == 0){
		ercd = 1;
	}
	
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

	if(CmrWakFlg == 0){
		ercd = 1;
	}

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
	//const UB cCmd[] = { 0x00, 0x00, 0x18, 0x01, 0x00 };
	const UB cCmd[] = { 0x00, 0x00, 0x18, 0x01, 0x01 }; //20140930Miya BUG�C��(WakeUp�R�}���h�ԈႢ)
	const int iRecvSize = 5;
	UB	cRetBuf[ 10 ];
	UB	*p;
	ER	ercd;

	CmrWakFlg = 0;
	
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
ER CmrCmdWakeUp(char sw)
{
	//const UB cCmd[] = { 0x00, 0x00, 0x18, 0x01, 0x01 };
	const UB cCmd[] = { 0x00, 0x00, 0x18, 0x01, 0x00 };	//20140930Miya BUG�C��(WakeUp�R�}���h�ԈႢ)
	int iRecvSize;
	UB	cRetBuf[ 10 ];
	UB	*p;
	ER	ercd;
	const UB cCmdSts[] = { 0x00, 0x00, 0x11, 0x00 };	//20140930Miya 

	CmrWakFlg = 1;

	if(sw == 1){
		iRecvSize = 6;
		ercd = CmrPktSend( cCmdSts, sizeof cCmdSts, iRecvSize );			// �R�}���h���M
		if (ercd == E_OK) {
			ercd = CmrPktRecv( cRetBuf, iRecvSize, (3000/MSEC));		// ������M
			if (ercd == E_OK) {
				if(cRetBuf[4] != 0 || cRetBuf[5] == 0){
					SetError(39);
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

		if (ercd != E_OK) {
			SetError(32);
			//return(ercd);
		}
	}
	
	
	iRecvSize = 5;
	ercd = CmrPktSend( cCmd, sizeof cCmd, iRecvSize );			// �R�}���h���M
	if (ercd == E_OK) {
		ercd = CmrPktRecv( cRetBuf, iRecvSize, (3000/MSEC));		// ������M
		if (ercd == E_OK) {
			if (((cRetBuf[ 0 ] & 0x7f) == cCmd[ 1 ]) && (cRetBuf[ 2 ] == cCmd[ 2 ]) && (cRetBuf[ 3 ] == cCmd[ 3 ])	// �������e�`�F�b�N
			/* && ((cRetBuf[ 4 ] & 0x01) == 0x01)*/ ) {
			} else {
				ercd = E_OBJ;
			}

			if(cRetBuf[4] != 0 ){
				SetError(40);
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

	if (ercd != E_OK) {
		SetError(32);
	}

	return ercd;
}

//-----------------------------------------------------------------------------
// �J�����R�}���h: �J���� Freeze Control OFF
//-----------------------------------------------------------------------------
static ER CmrCmdFreezeOff(void)
{
	//const UB cCmd[] = { 0x00, 0x00, 0x28, 0x05, 0x08, 0x26, 0x00, 0x01, 0x02 };
	const UB cCmd[] = { 0x00, 0x00, 0x28, 0x05, 0x08, 0x26, 0x00, 0x01, 0x03 }; //20140930Miya BUG�C��(WakeUp�R�}���h�ԈႢ)
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


#if ( VA300S == 0 || VA300S == 2 )
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
	unsigned long rcnt;	//20140317Miya NagaiBug�C��

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
	rcnt = 0;
	
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
				//cmd_len = ( len_rest % ( 1024 - 13 ) ) + 11;	
				cmd_len = len_rest + 11;	//20140317Miya NagaiBug�C��
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
		      || ( GetScreenNo() == LCD_SCREEN127 ) || ( GetScreenNo() == LCD_SCREEN141 )
			  || ( GetScreenNo() == LCD_SCREEN161 ) || ( GetScreenNo() == LCD_SCREEN181 )
			  || ( GetScreenNo() == LCD_SCREEN405 ) || ( GetScreenNo() == LCD_SCREEN523 ) // ���ځH(�P�΂P�F�؎d�l)
			  || ( GetScreenNo() == LCD_SCREEN530 ) || ( GetScreenNo() == LCD_SCREEN544 )){
			  
				snd_data[ 11 ] = '1';			//�@���M����ʁ@1Byte�@ASCII
			
			} else if ( ( GetScreenNo() == LCD_SCREEN8 ) || ( GetScreenNo() == LCD_SCREEN101 )	// ���ځH
		      || ( GetScreenNo() == LCD_SCREEN102 ) || ( GetScreenNo() == LCD_SCREEN129 )
			  || ( GetScreenNo() == LCD_SCREEN407 ) || ( GetScreenNo() == LCD_SCREEN503 ) // ���ځH(�P�΂P�F�؎d�l)
			  || ( GetScreenNo() == LCD_SCREEN532 ) ){
			  			  
				snd_data[ 11 ] = '2';			//�@1Byte�@ASCII
			
			} else {
				snd_data[ 11 ] = ' ';			//�@1Byte�@ASCII	�����ɗ���������ُ�B		
			}
			snd_data[ 12 ] = ',';				//�@���M����ʁ@1Byte�@ASCII
			
			//for ( i=0; i<=cmd_len-13; i++ ){	//�@�摜�f�[�^�̃R�}���h�Z�b�g�B
			for ( i=0; i<cmd_len-13; i++ ){	//�@�摜�f�[�^�̃R�}���h�Z�b�g�B
			
				//snd_data[ 13+i ] = data[ ( ( max_blk_num - blk_num ) * ( 1024 - 13 ) ) + i ];
				snd_data[ 13+i ] = data[rcnt];	//20140317Miya NagaiBug�C��
				++rcnt;
			}
			
		}	else { // �����擪�u���b�N�łȂ��Ȃ�B
			//for ( i=0; i<=cmd_len-11; i++ ){	//�@�摜�f�[�^�̃R�}���h�Z�b�g�B
			for ( i=0; i<cmd_len-11; i++ ){	//�@�摜�f�[�^�̃R�}���h�Z�b�g�B
			
				//snd_data[ 11+i ] = data[ ( ( max_blk_num - blk_num ) * ( 1024 - 13 ) ) + i ];
				snd_data[ 11+i ] = data[rcnt];	//20140317Miya NagaiBug�C��
				++rcnt;
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
		//len_rest = len_rest - ( 1024 - 13 );	// �c��̑��M�f�[�^��
		len_rest = len - rcnt;	//20140317Miya NagaiBug�C�� // �c��̑��M�f�[�^��

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
	unsigned long rcnt;	//20140317Miya NagaiBug�C��

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
	rcnt = 0;
	
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
				//cmd_len = ( len_rest % ( 1024 - 13 ) ) + 11;	
				cmd_len = len_rest + 11;	//20140317Miya NagaiBug�C��
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
			if ( ( GetScreenNo() == LCD_SCREEN101 ) || ( GetScreenNo() == LCD_SCREEN102 )
			  || ( GetScreenNo() == LCD_SCREEN503 )  ){	// �����p�H
			  
				snd_data[ 11 ] = '1';			//�@1Byte�@ASCII
			
			} else {		//�@��L�ȊO�́A�S�ēo�^�A�܂��͓o�^�m�F�p
				snd_data[ 11 ] = '0';			//�@1Byte�@ASCII		
			}
			snd_data[ 12 ] = ',';
			
			// �����R�[�h�A���[�U�[ID�i�P�F�P�F�؎d�l�̏ꍇ�B�}���V����(��L��)�d�l�̏ꍇ�̓u�����N�l�߁j
			if ( GetSysSpec() == SYS_SPEC_OFFICE ){
				snd_data[ 13 ] = '0';			// �����R�[�h�A�P�F�P�F�؎d�l�̏ꍇ�B
				snd_data[ 14 ] = '0';
				snd_data[ 15 ] = ',';
				
				snd_data[ 16 ] = yb_touroku_data.user_id[ 0 ];	// ���[�U�[ID, �P�F�P�F�؎d�l�̏ꍇ�B
				snd_data[ 17 ] = yb_touroku_data.user_id[ 1 ];
				snd_data[ 18 ] = yb_touroku_data.user_id[ 2 ];
				snd_data[ 19 ] = yb_touroku_data.user_id[ 3 ];
				snd_data[ 20 ] = ',';
			
			} else {
				snd_data[ 13 ] = ' ';			// �����R�[�h�A�}���V����(��L��)�d�l�̏ꍇ
				snd_data[ 14 ] = ' ';
				snd_data[ 15 ] = ',';
				
				snd_data[ 16 ] = ' ';			// ���[�U�[ID, �}���V����(��L��)�d�l�̏ꍇ�B
				snd_data[ 17 ] = ' ';
				snd_data[ 18 ] = ' ';
				snd_data[ 19 ] = ' ';
				snd_data[ 20 ] = ',';				
			}
			
			//20140317Miya VA300������
			//snd_data[ 21 ] = yb_touroku_data.yubi_sens[ 0 ];
			snd_data[ 21 ] = g_RegUserInfoData.RegImageSens + 0x30;
			snd_data[ 22 ] = ',';
			//snd_data[ 23 ] = yb_touroku_data.yubi_size[ 0 ];
			//snd_data[ 24 ] = yb_touroku_data.yubi_size[ 1 ];
			//snd_data[ 25 ] = yb_touroku_data.yubi_size[ 2 ];
			snd_data[ 23 ] = 0x30;
			snd_data[ 24 ] = 0x30;
			snd_data[ 25 ] = 0x30;
			snd_data[ 26 ] = ',';

			//for ( i=0; i<cmd_len-21; i++ ){	//�@�摜�f�[�^�̃R�}���h�Z�b�g�B
			for ( i=0; i<cmd_len-27; i++ ){	//�@�摜�f�[�^�̃R�}���h�Z�b�g�B
			
				//snd_data[ 21+i ] = data[ ( ( max_blk_num - blk_num ) * ( 1024 - 13 ) ) + i ];
				snd_data[ 27+i ] = data[rcnt];	//20140317Miya NagaiBug�C��&VA300������
				++rcnt;
			}
			
		}	else { // �����擪�u���b�N�łȂ��Ȃ�B
			for ( i=0; i<cmd_len-11; i++ ){	//�@�摜�f�[�^�̃R�}���h�Z�b�g�B
			
				//snd_data[ 11+i ] = data[ ( ( max_blk_num - blk_num ) * ( 1024 - 13 ) ) + i ];
				snd_data[ 11+i ] = data[rcnt];	//20140317Miya NagaiBug�C��
				++rcnt;
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
		//len_rest = len_rest - ( 1024 - 13 );	// �c��̑��M�f�[�^��
		len_rest = len - rcnt;	//20140317Miya NagaiBug�C�� // �c��̑��M�f�[�^��

		header[ 7 ] = '0';			//�@�u���b�N�ԍ� �S��ASCII�̍ď�����
		header[ 8 ] = '0';
		header[ 9 ] = '0';
		header[ 10 ] = '0';

	}
	
	return ercd;
}



#if ( VA300S == 2 ) 
static void DebugSendCmd_210( void )
{
	ER ercd = E_OK;
	T_YBDATA *ybdata;
	volatile int xx, yy, cnt1, cnt2, tst;
	volatile int p1, p2, p3;
//	unsigned char buf1[40][80], buf2[10][20];

	ybdata = &yb_touroku_data;	

#if(AUTHTEST >= 1)	//20160613Miya
	if( g_TechMenuData.DebugHyouji == FLG_ON ){
		//memcpy( &ybdata->ybCapBuf, &g_ubResizeSvBuf[ 0 ], 80 * 40 );	//�B�e�摜
		cnt1 = cnt2 = 0;
		for(yy = 0 ; yy < 40 ; yy++){
			for(xx = 0 ; xx < 100 ;  xx++){
				if(xx < 10 || xx >= 90){
					ybdata->ybCapBuf[cnt1++] = 0;
				}else{
					ybdata->ybCapBuf[cnt1++] = g_ubResizeSvBuf[cnt2++];
				}
			}
		}
		ercd = SendCmd_210( ( UB* )ybdata->ybCapBuf, ( ( 100 * 40 ) * 1 ) + 2 );	// �R�}���h210
	}else{
		memcpy( &ybdata->ybCapBuf, &g_ubCapBuf[ 0 ], 100 * 40 );	//�B�e�摜
		ercd = SendCmd_210( ( UB* )ybdata->ybCapBuf, ( ( 100 * 40 ) * 1 ) + 2 );	// �R�}���h210
	}
	rxtid = rxtid_org;						// GET_UDP�ׂ̈ɖ{���̎�M�^�X�NID�itsk_rcv�j���ăZ�b�g
#else
	if( g_TechMenuData.DebugHyouji == FLG_ON ){
		//memcpy( &ybdata->ybCapBuf, &g_ubCapBuf[ 0 ], 200 * 80 );	//�B�e�摜
		//ercd = SendCmd_210( ( UB* )ybdata->ybCapBuf, ( ( 200 * 80 ) * 1 ) + 2 );	// �R�}���h210
		memcpy( &ybdata->ybCapBuf, &g_ubCapBuf[ 0 ], 100 * 40 );	//�B�e�摜
		ercd = SendCmd_210( ( UB* )ybdata->ybCapBuf, ( ( 100 * 40 ) * 1 ) + 2 );	// �R�}���h210
	}else{
		memcpy( &ybdata->ybCapBuf, &g_ubCapBuf[ 0 ], 320 * 240 );	//�B�e�摜
		ercd = SendCmd_210( ( UB* )ybdata->ybCapBuf, ( ( 320 * 240 ) * 1 ) + 2 );	// �R�}���h210
	}
	rxtid = rxtid_org;						// GET_UDP�ׂ̈ɖ{���̎�M�^�X�NID�itsk_rcv�j���ăZ�b�g
#endif

	while ( MdGetSubMode() != SUB_MD_IDLE ){	// PC����̉���������҂B
		dly_tsk( 25/MSEC );	
	}

/*
	tst = 3;
	if( tst == 0 ){
		memcpy( &ybdata->ybCapBuf, &g_ubResizeSvBuf[0], 80 * 40 );	//�B�e�摜
	}
	if( tst == 1 ){
		p1 = 1;
		p2 = 0;
		p3 = 0;
		memcpy( &ybdata->ybCapBuf, &RegImgBuf[p1][p2][p3][0], 80 * 40 );	//�o�^�摜[�o�^��][�w�K][�\�[�x��][�F�؃T�C�Y]
	}
	if( tst == 2 ){
		memcpy( &ybdata->ybCapBuf, &g_ubLbpBuf[0], 80 * 40 );	//LBP�摜
	}
	if( tst == 3 ){
		cnt1 = 0;
		for(yy = 0 ; yy < 40 ; yy++){
			for(xx = 0 ; xx < 80 ; xx++){
				g_ubResizeSvBuf2[cnt1++] = 0;
			}
		}

		cnt1 = 0;
		cnt2 = 0;
		for(yy = 0 ; yy < 10 ; yy++){
			cnt1 = 80 * yy;
			for(xx = 0 ; xx < 20 ; xx++){
				g_ubResizeSvBuf2[cnt1+xx] = g_XsftMiniBuf[0][cnt2++];
			}
		}

		cnt1 = 0;
		cnt2 = 0;
		for(yy = 0 ; yy < 10 ; yy++){
			cnt1 = 80 * yy + 20;
			for(xx = 0 ; xx < 20 ; xx++){
				g_ubResizeSvBuf2[cnt1+xx] = g_XsftMiniBuf[1][cnt2++];
			}
		}

		cnt1 = 0;
		cnt2 = 0;
		for(yy = 0 ; yy < 10 ; yy++){
			cnt1 = 80 * yy + 40;
			for(xx = 0 ; xx < 20 ; xx++){
				g_ubResizeSvBuf2[cnt1+xx] = g_XsftMiniBuf[2][cnt2++];
			}
		}

		cnt1 = 0;
		cnt2 = 0;
		for(yy = 0 ; yy < 10 ; yy++){
			cnt1 = 80 * 10 + 80 * yy;
			for(xx = 0 ; xx < 20 ; xx++){
				g_ubResizeSvBuf2[cnt1+xx] = g_XsftMiniBuf[3][cnt2++];
			}
		}

		cnt1 = 0;
		cnt2 = 0;
		for(yy = 0 ; yy < 10 ; yy++){
			cnt1 = 80 * 10 + 80 * yy + 20;
			for(xx = 0 ; xx < 20 ; xx++){
				g_ubResizeSvBuf2[cnt1+xx] = g_XsftMiniBuf[4][cnt2++];
			}
		}

		memcpy( &ybdata->ybCapBuf, &g_ubResizeSvBuf2[0], 80 * 40 );
		
		
	}

	ercd = SendCmd_210( ( UB* )ybdata->ybCapBuf, ( ( 80 * 40 ) * 1 ) + 2 );	// �R�}���h210�A�k���摜�R���i�������Ȃ��j
*/
}
#endif

//-----------------------------------------------------------------------------
// �摜�f�[�^��UDP���M�i�w�摜�f�[�^�A�R�}���h�ԍ�141�j
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

#endif



#if ( VA300S == 1 || VA300S == 2 )
//-----------------------------------------------------------------------------
// �摜�f�[�^��UDP���M�i�w�摜�f�[�^�A�R�}���h�ԍ�204�j
//-----------------------------------------------------------------------------
static ER SendNinshou_204( UB *data, int len )
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
	
	cmd_len = 57;

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

	len_rest = len;

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
		
	// ��x�ڂ̔F�؃f�[�^�^��x�ڂ̔F�؃f�[�^���̋敪�����r����
	if ( ( GetScreenNo() == LCD_SCREEN6 ) || ( GetScreenNo() == LCD_SCREEN121 )	// ���ځH
      || ( GetScreenNo() == LCD_SCREEN127 ) || ( GetScreenNo() == LCD_SCREEN141 )
	  || ( GetScreenNo() == LCD_SCREEN161 ) || ( GetScreenNo() == LCD_SCREEN181 )
	  || ( GetScreenNo() == LCD_SCREEN405 ) || ( GetScreenNo() == LCD_SCREEN523 ) // ���ځH(�P�΂P�F�؎d�l)
	  || ( GetScreenNo() == LCD_SCREEN530 ) || ( GetScreenNo() == LCD_SCREEN544 )){
			  
		snd_data[ 11 ] = '1';			//�@���M����ʁ@1Byte�@ASCII
		
	} else if ( ( GetScreenNo() == LCD_SCREEN8 ) || ( GetScreenNo() == LCD_SCREEN101 )	// ���ځH
      || ( GetScreenNo() == LCD_SCREEN102 ) || ( GetScreenNo() == LCD_SCREEN129 )
	  || ( GetScreenNo() == LCD_SCREEN407 ) || ( GetScreenNo() == LCD_SCREEN503 ) // ���ځH(�P�΂P�F�؎d�l)
	  || ( GetScreenNo() == LCD_SCREEN532 ) ){
			  			  
		snd_data[ 11 ] = '2';			//�@1Byte�@ASCII
			
	} else {
		snd_data[ 11 ] = ' ';			//�@1Byte�@ASCII	�����ɗ���������ُ�B		
	}
	snd_data[ 12 ] = ',';				//�@���M����ʁ@1Byte�@ASCII

	for ( i = 0; i < 42; i++ ){	//�@�摜�f�[�^�̃R�}���h�Z�b�g�B
		snd_data[ 13+i ] = data[ i ];
	}

	snd_data[ cmd_len - 2 ] = CODE_CR;
	snd_data[ cmd_len - 1 ] = CODE_LF;

	// �P�u���b�N�̃f�[�^���M	
	SendNinshouData( &snd_data, cmd_len );		// �P�u���b�N�̃f�[�^���M

	return ercd;
}

static ER SendNinshou_210( UB *data, int len )
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

	cmd_len = 22;

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

	len_rest = len;

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

			
	// �o�^�p�̔F�؃f�[�^�^�����p�̔F�؃f�[�^���̋敪�����r����
	//20140423Miya �F�؃��g���C�ǉ�
	//if ( ( GetScreenNo() == LCD_SCREEN101 ) || ( GetScreenNo() == LCD_SCREEN102 ) || ( GetScreenNo() == LCD_SCREEN105)
	//  || ( GetScreenNo() == LCD_SCREEN503 )  ){	// �����p�H
	//20160108Miya FinKeyS
	if ( ( GetScreenNo() == LCD_SCREEN101 ) || ( GetScreenNo() == LCD_SCREEN102 ) || ( GetScreenNo() == LCD_SCREEN105)
	  || ( GetScreenNo() == LCD_SCREEN601 ) || ( GetScreenNo() == LCD_SCREEN602 ) || ( GetScreenNo() == LCD_SCREEN605) 
	  || ( GetScreenNo() == LCD_SCREEN610 ) || ( GetScreenNo() == LCD_SCREEN611 ) || ( GetScreenNo() == LCD_SCREEN503 )  ){	// �����p�H
			  
		snd_data[ 11 ] = '1';			//�@1Byte�@ASCII
			
	} else {		//�@��L�ȊO�́A�S�ēo�^�A�܂��͓o�^�m�F�p
		snd_data[ 11 ] = '0';			//�@1Byte�@ASCII		
	}
	snd_data[ 12 ] = ',';
		
			// �����R�[�h�A���[�U�[ID�i�P�F�P�F�؎d�l�̏ꍇ�B�}���V����(��L��)�d�l�̏ꍇ�̓u�����N�l�߁j
	if ( GetSysSpec() == SYS_SPEC_OFFICE ){
		snd_data[ 13 ] = '0';			// �����R�[�h�A�P�F�P�F�؎d�l�̏ꍇ�B
		snd_data[ 14 ] = '0';
		snd_data[ 15 ] = ',';
				
		snd_data[ 16 ] = yb_touroku_data.user_id[ 0 ];	// ���[�U�[ID, �P�F�P�F�؎d�l�̏ꍇ�B
		snd_data[ 17 ] = yb_touroku_data.user_id[ 1 ];
		snd_data[ 18 ] = yb_touroku_data.user_id[ 2 ];
		snd_data[ 19 ] = yb_touroku_data.user_id[ 3 ];
		snd_data[ 20 ] = ',';
			
	} else {
		snd_data[ 13 ] = ' ';			// �����R�[�h�A�}���V����(��L��)�d�l�̏ꍇ
		snd_data[ 14 ] = ' ';
		snd_data[ 15 ] = ',';
				
		snd_data[ 16 ] = ' ';			// ���[�U�[ID, �}���V����(��L��)�d�l�̏ꍇ�B
		snd_data[ 17 ] = ' ';
		snd_data[ 18 ] = ' ';
		snd_data[ 19 ] = ' ';
		snd_data[ 20 ] = ',';				
	}

			
				
	snd_data[ cmd_len ] = CODE_CR;
	snd_data[ cmd_len + 1 ] = CODE_LF;

	// �P�u���b�N�̃f�[�^���M	
	SendNinshouData( &snd_data, cmd_len );		// �P�u���b�N�̃f�[�^���M


	
	return ercd;
}


#endif







//-----------------------------------------------------------------------------
// �w�o�^���̃f�[�^������(�摜���)
//-----------------------------------------------------------------------------
void yb_init_capbuf( void ){
	
	//memset( yb_touroku_data.ybCapBuf, 0, ( 160 * 80 * 3 ) );
	memset( yb_touroku_data.ybCapBuf, 0, ( 200 * 40 ) );	//20160610Miya forDebug
	
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

