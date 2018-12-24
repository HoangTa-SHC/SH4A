/**
*	VA-300�v���O����
*
*	@file tsk_ts.c
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/09/27
*	@brief  ���Ԍ��m�Z���T�^�X�N
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "kernel.h"
#include "sh7750.h"
#include "id.h"
#include "drv_fpga.h"
#include "err_ctrl.h"
#include "va300.h"
#include "drv_led.h"

// �ϐ���`
static ID s_idTsk;						///< �^�X�NID

#define	enable_ts_int()	fpga_setw(INT_CRL, (INT_CRL_SENS));	///< �^�b�`�p�l�������݋���
#define	clear_ts_int()	fpga_clrw(INT_CRL, (INT_CRL_SENS));	///< �^�b�`�p�l�������݃N���A

#define CHATA_WAIT_TIME 40

static UH chata_wait_time = CHATA_WAIT_TIME;// �`���^�����O�E�`�F�b�N��Wait����
static char s_cBuf[ 256 ];				// �����p�o�b�t�@

// �v���g�^�C�v�錾
static ER Wait_Brink_end( void );		// ���̌��m�Z���T�[�̃u�����N�ƁA���̏I���҂�
static void ts_tsk_finish( void );		// ���̌��m�����݂̋��ALED OFF
static ER check_touch_off( void );		// ���̌��m�Z���T�[�̃I�t�E�^�b�`���o����
static void wait_screen( void );		// ��ʂ��u�w�𔲂��ĉ������v�܂��́A���s��ʂɂȂ�̂�҂B

// �^�X�N�̒�`
const T_CTSK ctsk_ts = { TA_HLNG, NULL, TsTask, 5, 2048, NULL, (B *)"Ts task" };//

/*==========================================================================*/
/**
 * ���Ԍ��m�Z���T�^�X�N������
 */
/*==========================================================================*/
ER TsTaskInit(ID tskid)
{
	ER ercd;
	
	// �^�X�N�̐���
	if (tskid > 0) {
		ercd = cre_tsk(tskid, &ctsk_ts);
		if (ercd == E_OK) {
			s_idTsk = tskid;
		}
	} else {
		ercd = acre_tsk(&ctsk_ts);
		if (ercd > 0) {
			s_idTsk = ercd;
		}
	}
	if (ercd < 0) {
		return ercd;
	}
	
	// �^�X�N�̋N��
	ercd = sta_tsk(s_idTsk, 0);
	
	// ���Ԍ��m�Z���T�̏�����
	if (ercd == E_OK) {
		ercd = TsInit(s_idTsk);
	}
	return ercd;
}

/*==========================================================================*/
/**
 * ���Ԍ��m�Z���T����^�X�N
 */
/*==========================================================================*/
TASK TsTask(void)
{
	ER		ercd, stat;
	FLGPTN	flgptn;
	
	// �����J�n
	for ( ;; ){
//		ercd = tslp_tsk( TMO_FEVR );			// ���Ԍ��m�Z���TON�҂�
		ercd = tslp_tsk( 50/MSEC );				// ���Ԍ��m�Z���TON�҂�
		
		ts_TSK_wdt = FLG_ON;					// �^�b�`�Z���T�E�^�X�N�@WDT�J�E���^���Z�b�g�E���N�G�X�g�E�t���O Added T.N 2015.3.10

		if( GetScreenNo() == LCD_SCREEN105){	//20150928Miya �����1�x�A�E�E�E���30�b�����Ȃ��ꍇ
			if(Ninshou_wait_timer == 0){
				g_AuthCnt = 0;	//�F�؃��g���C ���Z�b�g
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN101 );	// �ʏ탂�[�h�̃��j���[��ʂցB
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN101 );		// ��ʔԍ��@<-�@���̉��
					if ( Pfail_mode_count == 0 ){ 
						MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h�ցi�K�v�������O�̂��߁j
					}	else	{
						MdCngMode( MD_PFAIL );			// ���u���[�h��	��d���[�h�ցi�K�v�������O�̂��߁j
					}
				}
				if(CmrWakFlg == 1){
					ercd = CmrCmdSleep();
				}
				
			}
		}
		//20160108Miya FinKeyS
		if( GetScreenNo() == LCD_SCREEN605){	//�����1�x�A�E�E�E���30�b�����Ȃ��ꍇ
			if(Ninshou_wait_timer == 0){
				g_AuthCnt = 0;	//�F�؃��g���C ���Z�b�g
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN601 );	// �ʏ탂�[�h�̃��j���[��ʂցB
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN601 );		// ��ʔԍ��@<-�@���̉��
					if ( Pfail_mode_count == 0 ){ 
						MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h�ցi�K�v�������O�̂��߁j
					}	else	{
						MdCngMode( MD_PFAIL );			// ���u���[�h��	��d���[�h�ցi�K�v�������O�̂��߁j
					}
				}
				if(CmrWakFlg == 1){
					ercd = CmrCmdSleep();
				}
				
			}
		}
		
		if ( ercd == E_TMOUT ) continue;
		
		if ( ercd == E_OK ) {
			clear_ts_int();						// �^�b�`�Z���T�����݃N���A&�s����

			dly_tsk( chata_wait_time );			// 40mSec�҂��B

			//if( g_CapTimes > 0 ){
			//	g_CapTimes = 3;
			//}


			if ( ( fpga_inw( SENS_MON ) & 0x0001 ) == 1 ){//�@�`���^�����O�E�`�F�b�N1���
			
				LedOut(LED_OK, LED_ON);			// �f�o�b�O�pLED��ON����i��j
				dly_tsk( chata_wait_time );		// 40mSec�҂��B
				
			} else	{
				ts_tsk_finish();				// �`���^�����O�Ƃ��āA�I���B
				continue;
			}
			
			if ( ( fpga_inw( SENS_MON ) & 0x0001 ) == 1 ){	//�@�`���^�����O�E�`�F�b�N�Q���
			
				LedOut(LED_POW, LED_ON);		// �f�o�b�O�pLED��ON����i�΁j
				dly_tsk( chata_wait_time );		// 40mSec�҂��B
				
			} else	{
				ts_tsk_finish();				// �`���^�����O�Ƃ��āA�I���B
				continue;
			}

			if ( ( fpga_inw( SENS_MON ) & 0x0001 ) == 1 ){	//�@�`���^�����O�E�`�F�b�N�R���
			
				LedOut(LED_ERR, LED_ON);		// �f�o�b�O�pLED��ON����i�ԁj						
				dly_tsk( chata_wait_time );		// 40mSec�҂��B
				
			} else {
				ts_tsk_finish();				// �`���^�����O�Ƃ��āA�I���B	
				continue;
			}

#if(PCCTRL)	//20160930Miya PC����VA300S�𐧌䂷��
			if(g_capallow == 0){	//�B�e���Ȃ�
				ts_tsk_finish();				// �`���^�����O�Ƃ��āA�I���B	
				continue;
			}
#endif

			/** �J�����B�e�J�n�̔��� **/
			if ( ( ( MdGetMode() == MD_INITIAL ) && ( ( GetScreenNo() == LCD_SCREEN6 ) 			// �}���V����(��L��)�d�l
												   || ( GetScreenNo() == LCD_SCREEN8 )			// �}���V����(��L��)�d�l
												   || ( GetScreenNo() == LCD_SCREEN405 )		// �P�΂P�d�l
												   || ( GetScreenNo() == LCD_SCREEN407 ) ) )	// �P�΂P�d�l
			  || ( ( ( MdGetMode() == MD_NORMAL ) || ( MdGetMode() == MD_PFAIL ) )
			  									&& ( ( GetScreenNo() == LCD_SCREEN101 ) 		// �}���V����(��L��)�d�l
								    		  	   || ( GetScreenNo() == LCD_SCREEN102 )
								    		  	   || ( GetScreenNo() == LCD_SCREEN105 )		//20140423Miya �F�؃��g���C�ǉ�
												   || ( GetScreenNo() == LCD_SCREEN121 )
												   || ( GetScreenNo() == LCD_SCREEN127 )
												   || ( GetScreenNo() == LCD_SCREEN129 )
								    			   || ( GetScreenNo() == LCD_SCREEN141 )
												   || ( GetScreenNo() == LCD_SCREEN161 ) 
												   || ( GetScreenNo() == LCD_SCREEN181 ) 
												   || ( GetScreenNo() == LCD_SCREEN601 ) 		//20160108Miya FinKeyS
												   || ( GetScreenNo() == LCD_SCREEN602 ) 		//20160108Miya FinKeyS
												   || ( GetScreenNo() == LCD_SCREEN605 ) 		//20160108Miya FinKeyS
												   || ( GetScreenNo() == LCD_SCREEN610 ) 		//20160108Miya FinKeyS
												   || ( GetScreenNo() == LCD_SCREEN611 ) 		//20160108Miya FinKeyS
												   || ( GetScreenNo() == LCD_SCREEN503 )		// �P�΂P�d�l
												   || ( GetScreenNo() == LCD_SCREEN523 )
												   || ( GetScreenNo() == LCD_SCREEN530 )
												   || ( GetScreenNo() == LCD_SCREEN532 )
												   || ( GetScreenNo() == LCD_SCREEN544 ) ) )
			  || ( ( MdGetMode() == MD_MAINTE )  && ( ( GetScreenNo() == LCD_SCREEN203 ) ) ) ){

			if( g_CapTimes == 0 && CmrReloadFlg == 0){
				dbg_ts_flg = 1;


				/**�@�����ɁA�J�����̎B�e�v�����������b�Z�[�W�E�L���[�ŋL�q	**/
				g_CapTimes = 1;	//20131210Miya add
							
				//20140423Miya �F�؃��g���C�ǉ� �F�؎����g���C�񐔃��Z�b�g ->
				if( GetScreenNo() == LCD_SCREEN105){
					//Ninshou_wait_timer = 0;	//20150928Miya �����1�x�A�E�E�E���30�b�����Ȃ��ꍇ
					Ninshou_wait_timer = NINSHOU_WAIT_TIME;	//20150928Miya �����1�x�A�E�E�E���30�b�����Ȃ��ꍇ
					
					//20171206Miya 20170915Miya LCD105��LCD106���d�Ȃ�s��΍�LCD106�\�����Ȃ�
					//ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN106 );	// �O���[��ʂցB	
					//if ( ercd == E_OK ){
					//	ChgScreenNo( LCD_SCREEN105 );			// ��ʔԍ��@<-�@���̉��
					//}
				}
				//20160108Miya FinKeyS
				if( GetScreenNo() == LCD_SCREEN605){
					Ninshou_wait_timer = 0;	//�����1�x�A�E�E�E���30�b�����Ȃ��ꍇ
					
					//20171206Miya 20170915Miya LCD105��LCD106���d�Ȃ�s��΍�LCD106�\�����Ȃ�
					//ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN606 );	// �O���[��ʂցB	
					//if ( ercd == E_OK ){
					//	ChgScreenNo( LCD_SCREEN605 );			// ��ʔԍ��@<-�@���̉��
					//}
				}
			
				//if( GetScreenNo() == LCD_SCREEN101 || GetScreenNo() == LCD_SCREEN121 ){
				if( GetScreenNo() == LCD_SCREEN101 || GetScreenNo() == LCD_SCREEN121
				|| GetScreenNo() == LCD_SCREEN141 || GetScreenNo() == LCD_SCREEN161 || GetScreenNo() == LCD_SCREEN181
				|| GetScreenNo() == LCD_SCREEN601 || GetScreenNo() == LCD_SCREEN610 || GetScreenNo() == LCD_SCREEN611 ){ //20160108Miya FinKeyS
					IrLedOnOffSet(1, irDuty2, irDuty3, irDuty4, irDuty5);	//20160312Miya �ɏ����xUP
					g_AuthCnt = 1;	//20140423Miya �F�؃��g���C��
#if(PCCTRL)	//20160930Miya PC����VA300S�𐧌䂷��
					g_AuthCnt = g_pc_authnum;
#endif

#if(AUTHTEST >= 2)	//20160613Miya
					g_AuthCnt = 3;	//20160613Miya �F�؃��g���C��
#endif
				}
				//20140423Miya �F�؃��g���C�ǉ� �F�؎����g���C�񐔃��Z�b�g <-

				ercd = set_flg( ID_FLG_MAIN, FPTN_START_CAP );
				if ( ercd != E_OK ){
					break;
				}
			
				dly_tsk( 1000/MSEC );
			
				/**�@�J�����B�e�v�������@�I���҂�	**/
				ercd = wai_flg( ID_FLG_TS, FPTN_CMR_TS, TWF_ORW, &flgptn );
				if ( ercd != E_OK ){
					break;
				}
			
				ercd = clr_flg( ID_FLG_TS, ~FPTN_CMR_TS );	// �t���O�̃N���A	
				if ( ercd != E_OK ){
					break;
				}

				ercd = Wait_Brink_end();			// ���̌��m�Z���T�[�̃u�����N�ƁA���̏I���҂�(�J�����摜�̑��M�I��)�B
				if ( ercd != E_OK ){
					break;
				}

				LedOut(LED_POW, LED_ON);			// �d���\��LED��ON����i��j�E�E�E�@��������^�C�~���O�����邽�߁B


#if ( VA300S == 1 || VA300S == 2 )
				if( s_CapResult == CAP_JUDGE_RT || s_CapResult == CAP_JUDGE_RT2 ){	// �ĎB�e�v���҂��Ȃ�
					SetReCap();
				}
#endif
													//�@��ʂ��u�w���Z�b�g���āv�Ȃ�								
				if ( ( GetScreenNo() == LCD_SCREEN6 ) || ( GetScreenNo() == LCD_SCREEN127 ) || ( GetScreenNo() == LCD_SCREEN405 ) || ( GetScreenNo() == LCD_SCREEN530 ) ){ 
					wait_screen();					// ��ʂ��u�w�𔲂��ĉ������v�܂��́A���s��ʂɂȂ�̂�҂B
					while(1){//20131210Miya add
						if ( ( GetScreenNo() == LCD_SCREEN7 ) || ( GetScreenNo() == LCD_SCREEN128 ) || ( GetScreenNo() == LCD_SCREEN406 ) || ( GetScreenNo() == LCD_SCREEN531 ) ){ 
							nop();
							dly_tsk( 1000/MSEC );				// �Œ�P�b�Ԃ́A�u�w�𔲂��ĉ������v��ʂ�ێ��B
							break;
						}
						if ( ( GetScreenNo() == LCD_SCREEN10 ) || ( GetScreenNo() == LCD_SCREEN131 ) || ( GetScreenNo() == LCD_SCREEN409 ) || ( GetScreenNo() == LCD_SCREEN534 ) ){ 
							nop();
							dly_tsk( 1000/MSEC );				// �Œ�P�b�Ԃ́A�u�w�𔲂��ĉ������v��ʂ�ێ��B
							break;
						}
						dly_tsk( 200/MSEC );
					}
				}

//				dly_tsk( 2000/MSEC );				//20131210Miya del // �Œ�2�b�Ԃ́A�u�w�𔲂��ĉ������v��ʂ�ێ��B
//				dly_tsk( 1000/MSEC );				// �Œ�P�b�Ԃ́A�u�w�𔲂��ĉ������v��ʂ�ێ��B

				/** �I�t�E�^�b�`�����C���E�^�X�N�֒ʒm	**/
				if ( GetSysSpec() == SYS_SPEC_MANTION ){	// �}���V����(��L��)�d�l�}���V����(��L��)�d�l�̏ꍇ�B
					if ( ( MdGetMode() == MD_INITIAL ) && ( GetScreenNo() == LCD_SCREEN7 ) ){ // �������[�h�i�o�^�j�u�w�𔲂��ĉ������v
						while ( check_touch_off() != E_OK ){ // ���̌��m�Z���T�[�̃I�t�E�^�b�`�����o����
				  			dly_tsk( 20/MSEC );
						}
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN8 );	// �u������x�A�w��...�v��ʂցB	
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN8 );		// ��ʔԍ��@<-�@���̉��
							MdCngMode( MD_INITIAL );		// ���u���[�h�������o�^���[�h��	
						}
											
					} else if ( ( ( MdGetMode() == MD_NORMAL ) || ( MdGetMode() == MD_PFAIL ) ) && ( GetScreenNo() == LCD_SCREEN128 ) ){ // �ʏ탂�[�h�i�o�^�j�u�w�𔲂��ĉ������v
						while ( check_touch_off() != E_OK ){ // ���̌��m�Z���T�[�̃I�t�E�^�b�`�����o����
					  		dly_tsk( 20/MSEC );
						}
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN129 );	// �u������x�A�w��...�v��ʂցB	
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN129 );		// ��ʔԍ��@<-�@���̉��
						}					
					}
					
				} else if ( GetSysSpec() == SYS_SPEC_OFFICE ){		// �P�΂P�d�l�̏ꍇ�B {
					if ( ( MdGetMode() == MD_INITIAL ) && ( GetScreenNo() == LCD_SCREEN406 ) ){ // �������[�h�i�o�^�j�u�w�𔲂��ĉ������v
						while ( check_touch_off() != E_OK ){ // ���̌��m�Z���T�[�̃I�t�E�^�b�`�����o����
				  			dly_tsk( 20/MSEC );
						}
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN407 );	// �u������x�A�w��...�v��ʂցB	
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN407 );		// ��ʔԍ��@<-�@���̉��
							MdCngMode( MD_INITIAL );		// ���u���[�h�������o�^���[�h��	
						}
											
					} else if ( ( ( MdGetMode() == MD_NORMAL ) || ( MdGetMode() == MD_PFAIL ) )  && ( GetScreenNo() == LCD_SCREEN531 ) ){ // �ʏ탂�[�h�i�o�^�j�u�w�𔲂��ĉ������v
						while ( check_touch_off() != E_OK ){ // ���̌��m�Z���T�[�̃I�t�E�^�b�`�����o����
					  		dly_tsk( 20/MSEC );
						}
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN532 );	// �u������x�A�w��...�v��ʂցB	
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN532 );		// ��ʔԍ��@<-�@���̉��
						}					
					}
					
				} else {
					nop();
				}
								
				ts_tsk_finish();					// �^�b�`�Z���T�����݋��A�R�FLED OFF
				dbg_ts_flg = 0;
			}
			}
			 
			// �Y����ʂłȂ����ɁA���̌��m�Z���T�[��ON���ꂽ�ꍇ�́APOW_LED�i��j�̂�ON���āA�c��͏����B
			nop();
			ts_tsk_finish();					// �^�b�`�Z���T�����݋��A�R�FLED OFF

		} else {
			// �����ɂ���͎̂����G���[
			PrgErrSet();
			slp_tsk();							// �G���[�̂Ƃ��̓^�X�N�I��
		}
	}
	PrgErrSet();								// �����ɂ���͎̂����G���[
	slp_tsk();
}

/*==========================================================================*/
/*
 * ���Ԍ��m�Z���T��p���[�e�B���e�B
 * ��ʂ��u�w�𔲂��ĉ������v�܂��́A���s��ʂɂȂ�̂�҂B
 */
/*==========================================================================*/
static void wait_screen( void )
{	
	ER ercd;
	FLGPTN	flgptn;
	
	ercd = twai_flg( ID_FLG_TS, FPTN_WAIT_CHG_SCRN, TWF_ORW, &flgptn, 3000/MSEC );
	if ( ercd == E_OK ){		
		ercd = clr_flg( ID_FLG_TS, ~FPTN_WAIT_CHG_SCRN );	// �t���O�̃N���A	
		nop();
	}	else	{
		nop();
	}
}

/*==========================================================================*/
/*
 * ���Ԍ��m�Z���T��p���[�e�B���e�B
 * ���̌��m�Z���T�[�̃I�t�E�^�b�`���o����
 */
/*==========================================================================*/
static ER check_touch_off( void )
{
	ER ercd = -1;
	
	dly_tsk( chata_wait_time );			// 40mSec�҂��B

	if ( ( fpga_inw( SENS_MON ) & 0x0001 ) == 0 ){ //�@�`���^�����O�E�`�F�b�N1���
	
		dly_tsk( chata_wait_time );		// 40mSec�҂��B
				
	} else	{
		return ercd;
	}
			
	if ( ( fpga_inw( SENS_MON ) & 0x0001 ) == 0 ){	//�@�`���^�����O�E�`�F�b�N�Q���
			
		dly_tsk( chata_wait_time );		// 40mSec�҂��B
				
	} else	{
		return ercd;
	}

	if ( ( fpga_inw( SENS_MON ) & 0x0001 ) == 0 ){	//�@�`���^�����O�E�`�F�b�N�R���
									
		ercd = E_OK;
				
	} else {
		return ercd;
	}

	return ercd;		
}

/*==========================================================================*/
/*
 * ���Ԍ��m�Z���T��p���[�e�B���e�B
 * ���̌��m�Z���T�[�̃u�����N�I���҂�����
 */
/*==========================================================================*/
static ER Wait_Brink_end( void ){
	
	ER ercd;
	FLGPTN	flgptn;

	while(1){								// LED�u�����N
		ercd = twai_flg( ID_FLG_TS, FPTN_END_TS, TWF_ORW, &flgptn, 250/MSEC );
		if ( ercd == E_OK ){		
			ercd = clr_flg( ID_FLG_TS, ~FPTN_END_TS );	// �t���O�̃N���A	
			break;
		} else if ( ercd != E_TMOUT ){
			break;
		}	else	{
			LedOut(LED_OK, LED_REVERSE);	// �f�o�b�O�pLED�𔽓]����i��j
			LedOut(LED_POW, LED_REVERSE);	// �f�o�b�O�pLED�𔽓]����i�΁j
			LedOut(LED_ERR, LED_REVERSE);	// �f�o�b�O�pLED�𔽓]����i�ԁj
		}
	}
	return ercd;
}

/*==========================================================================*/
/*
 * ���Ԍ��m�Z���T��p���[�e�B���e�B
 * ���̌��m�Z���T�[�̏I������
 */
/*==========================================================================*/
static void ts_tsk_finish( void ){
	
		clear_ts_int();					// �^�b�`�Z���T�����݃N���A
										// ���O�ɁA���̃Z���T�����݂��Ăѓ����Ă��Ă��A��U�����݂����������B
		
		LedOut(LED_POW, LED_ON);		// �d���\��LED��ON����i��j
		
		LedOut(LED_OK, LED_OFF);		// �f�o�b�O�pLED��OFF����i�΁j
		LedOut(LED_ERR, LED_OFF);		// �f�o�b�O�pLED��OFF����i�ԁj
		
		enable_ts_int();				// �^�b�`�Z���T�����݋���
}