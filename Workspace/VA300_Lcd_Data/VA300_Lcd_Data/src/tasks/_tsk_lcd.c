/**
*	VA-300�e�X�g�v���O����
*
*	@file tsk_lcd.c
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/09/28
*	@brief  LCD�\���^�X�N
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

//#define LCDPROCON	//LCD����p�錾�@LCD����̃e�X�g�����Ȃ��ꍇ�́A�{�錾���R�����g�A�E�g���Ă�������
#define LCDX	480
#define LCDY	272

// �ϐ���`
static ID s_idTsk;

// �v���g�^�C�v�錾
static TASK LcdTask( void );		///< LCD�\���^�X�N

// �^�X�N�̒�`
const T_CTSK ctsk_lcd = { TA_HLNG, NULL, LcdTask, 7, 2048, NULL, (B *)"lcd task" };//

UB WaitKeyProc(int sousa, int tflg, UB *msg);
void SetGamen01(int buf, UB perm, UB perm_r, UB perm_g, UB perm_b, UH stx, UH sty, UH edx, UH edy);
void SetGamen02(int buf, UB perm, UB perm_r, UB perm_g, UB perm_b, UH stx, UH sty, UH edx, UH edy);
UH GetPixDataFromGamen01(int buf, unsigned long cnt);
UH GetPixDataFromGamen02(int buf, unsigned long cnt);
UINT DbgSendMessageToMain(int num, int sousa, UB *msg);

//extern const unsigned short LcdGmn01[];
//extern const unsigned short LcdGmn02[];
extern const unsigned short btn_NEXT[];
extern const unsigned short dbgmn001[];
extern const unsigned short dbgmn002[];
extern const unsigned short dbgmn003[];
extern const unsigned short dbgmn004[];
extern const unsigned short dbgmn005[];
extern const unsigned short dbgmn006[];
extern const unsigned short dbgmn007[];
extern const unsigned short dbgmn008[];
extern const unsigned short dbgmn009[];
extern const unsigned short dbgmn010[];
extern const unsigned short dbgmn011[];
extern const unsigned short dbgmn012[];
extern const unsigned short dbgmn101[];
extern const unsigned short dbgmn102[];
extern const unsigned short dbgmn103[];
extern const unsigned short dbgmn104[];
extern const unsigned short dbgmn121[];
extern const unsigned short dbgmn122[];
extern const unsigned short dbgmn123[];
extern const unsigned short dbgmn124[];
extern const unsigned short dbgmn125[];
extern const unsigned short dbgmn126[];
extern const unsigned short dbgmn127[];
extern const unsigned short dbgmn128[];
extern const unsigned short dbgmn129[];
extern const unsigned short dbgmn130[];
extern const unsigned short dbgmn131[];
extern const unsigned short dbgmn132[];
extern const unsigned short dbgmn141[];
extern const unsigned short dbgmn142[];
extern const unsigned short dbgmn143[];
extern const unsigned short dbgmn144[];
extern const unsigned short dbgmn145[];
extern const unsigned short dbgmn146[];
extern const unsigned short dbgmn201[];
extern const unsigned short dbgmn202[];
extern const unsigned short dbgmn203[];
extern const unsigned short dbgmn204[];

//extern const UINT g_LcdMsgSize;
//extern const UB g_LcdMsgBuff[1024];
extern struct{
	unsigned int LcdMsgSize;
	UB LcdMsgBuff[1024];
}g_LcdmsgData;


/*==========================================================================*/
/**
 * LCD�\���^�X�N������
 *
 * @param idTsk �^�X�NID
 * @retval E_OK ����N��
 */
/*==========================================================================*/
ER LcdTaskInit(ID idTsk)
{
	ER ercd;
	
	// �^�X�N�̐���
	if (idTsk > 0) {
		ercd = cre_tsk(idTsk, &ctsk_lcd);
		if (ercd == E_OK) {
			s_idTsk = idTsk;
		}
	} else {
		ercd = acre_tsk(&ctsk_lcd);
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

/*==========================================================================*/
/**
 * LCD�\���^�X�N
 * 
 */
/*==========================================================================*/
static TASK LcdTask( void )
{
	FLGPTN	flgptn;
	ER		ercd;
	UB	sts_key;
	UB	buf_num, perm_enb, perm_r, perm_g, perm_b;
	UH	st_x, st_y, ed_x, ed_y;
	
	int cnt, num;
	int posx, posy;
	
	/** ���b�Z�[�W�E�o�b�t�@�p�̐錾  **/
	UB *msg; 		// <-- �T�C�Y��120���炢�܂ł͈̔͂œK�X�ς��ĉ������B
	UINT MsgSize;


	// LCD�̋@��Initialize �́A�����ɋL���B
	buf_num = 0;	//��ʃo�b�t�@(0�`15)
	perm_enb = 0;	//���ߐF�ݒ� 0:���g�p 1:�g�p
	perm_r = 0;		//R���ߐF
	perm_g = 0;		//G���ߐF
	perm_b = 0;		//B���ߐF
	st_x = 0;
	st_y = 0;
	ed_x = 479;
	ed_y = 271;
	
	msg = &g_LcdmsgData.LcdMsgBuff[0];
	
	// �ʏ폈���J�n
	for(;;) {
		// ���C���^�X�N����̂̎�M�҂�
		ercd = wai_flg( ID_FLG_LCD, ( FPTN_LCD_INIT				// LCD������ʕ\���v��(���C����LCD�A�m�[�}�����[�h�ڍs�̎�)
									| FPTN_LCD_SCREEN1			// ��ʂP�\���v��(���C����LCD)
									| FPTN_LCD_SCREEN2			// ��ʂQ�\���v��(���C����LCD)
									| FPTN_LCD_SCREEN3			// ��ʂR�\���v��(���C����LCD)
									| FPTN_LCD_SCREEN4			// ��ʂS�\���v��(���C����LCD)
									| FPTN_LCD_SCREEN5			// ��ʂT�\���v��(���C����LCD)
									| FPTN_LCD_SCREEN6			// ��ʂU�\���v��(���C����LCD)
									| FPTN_LCD_SCREEN7			// ��ʂV�\���v��(���C����LCD)
									| FPTN_LCD_SCREEN8			// ��ʂW�\���v��(���C����LCD)
									| FPTN_LCD_SCREEN9			// ��ʂX�\���v��(���C����LCD)
									| FPTN_LCD_SCREEN10			// ��ʂP�O�\���v��(���C����LCD)
									| FPTN_LCD_SCREEN11			// ��ʂP�P�\���v��(���C����LCD)
									| FPTN_LCD_SCREEN12			// ��ʂP�Q�\���v��(���C����LCD)
									| FPTN_LCD_SCREEN100 		// ��ʂP�O�O�\���v��(���C����LCD)�@�ʏ탂�[�h
									| FPTN_LCD_SCREEN101 		// ��ʂP�O�P�\���v��(���C����LCD)
									| FPTN_LCD_SCREEN102 		// ��ʂP�O�Q�\���v��(���C����LCD)
									| FPTN_LCD_SCREEN103 		// ��ʂP�O�R�\���v��(���C����LCD)
									| FPTN_LCD_SCREEN104 		// ��ʂP�O�S�\���v��(���C����LCD)
									| FPTN_LCD_SCREEN120		// ��ʂP�Q�O�\���v��(���C����LCD)�@�ʏ탂�[�h�i�o�^�j
									| FPTN_LCD_SCREEN121		// ��ʂP�Q�P�\���v��(���C����LCD)
									| FPTN_LCD_SCREEN122		// ��ʂP�Q�Q�\���v��(���C����LCD)
									| FPTN_LCD_SCREEN123		// ��ʂP�Q�R�\���v��(���C����LCD)
									| FPTN_LCD_SCREEN124		// ��ʂP�Q�S�\���v��(���C����LCD)
									| FPTN_LCD_SCREEN125		// ��ʂP�Q�T�\���v��(���C����LCD)
									| FPTN_LCD_SCREEN126		// ��ʂP�Q�U�\���v��(���C����LCD)
									| FPTN_LCD_SCREEN127		// ��ʂP�Q�V�\���v��(���C����LCD)
									| FPTN_LCD_SCREEN128		// ��ʂP�Q�W�\���v��(���C����LCD)
									| FPTN_LCD_SCREEN129		// ��ʂP�Q�X�\���v��(���C����LCD)
									| FPTN_LCD_SCREEN130		// ��ʂP�R�O�\���v��(���C����LCD)
									| FPTN_LCD_SCREEN131		// ��ʂP�R�P�\���v��(���C����LCD)
									| FPTN_LCD_SCREEN132		// ��ʂP�R�Q�\���v��(���C����LCD)
									| FPTN_LCD_SCREEN140		// ��ʂP�S�O�\���v��(���C����LCD)�@�ʏ탂�[�h�i�폜�j
									| FPTN_LCD_SCREEN141		// ��ʂP�S�P�\���v��(���C����LCD)
									| FPTN_LCD_SCREEN142		// ��ʂP�S�Q�\���v��(���C����LCD)
									| FPTN_LCD_SCREEN143		// ��ʂP�S�R�\���v��(���C����LCD)
									| FPTN_LCD_SCREEN144		// ��ʂP�S�S�\���v��(���C����LCD)
									| FPTN_LCD_SCREEN145		// ��ʂP�S�T�\���v��(���C����LCD)
									| FPTN_LCD_SCREEN146		// ��ʂP�S�U�\���v��(���C����LCD)
									| FPTN_LCD_SCREEN200		// ��ʂQ�O�O�\���v��(���C����LCD)�@�����e�i���X�E���[�h
									| FPTN_LCD_SCREEN201		// ��ʂQ�O�P�\���v��(���C����LCD)
									| FPTN_LCD_SCREEN202		// ��ʂQ�O�Q�\���v��(���C����LCD)
									| FPTN_LCD_SCREEN203		// ��ʂQ�O�R�\���v��(���C����LCD)
									| FPTN_LCD_SCREEN204		// ��ʂQ�O�S�\���v��(���C����LCD)
									| FPTN_LCD_SCREEN205 ), TWF_ORW, &flgptn );	// ��ʂQ�O�T�\���v��(���C����LCD)
		if ( ercd != E_OK ){
			break;
		}
														
		// �ȉ��ɁASwitch���ȂǂŎ�M���e�ɉ������������L�q
		// ���̎��A�������e�̍ŏ��ɁA
		//		ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_INIT );		// �t���O�̃N���A
		// 		if ( ercd != E_OK ){
		//			break;
		//		}
		//  �ȂǂŁA��M�����t���O���ɁA�N���A�������s���B
		//  wai_flg() �Ŏg�p����t���O�p�^�[���́Aid.h�@���Q�Ƃ̂��ƁB
		// 

		memset( msg, 0x20, sizeof(msg) );

		switch(flgptn)
		{
// �C�j�V�����C�Y ------------------------------------------------------------------------------------//
			case FPTN_LCD_INIT:
				num = 0;
				//////// �t���O�̃N���A ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_INIT );
				if ( ercd != E_OK ){
					break;
				}

				LcdcBackLightOff();
				for(cnt = 0 ; cnt < 2 ; cnt++){	//��ʂ���ʃo�b�t�@�ɓ]������
					SetGamen01(cnt, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				}
				SetGamen02(0, perm_enb, perm_r, perm_g, perm_b, 208, 104, 271, 167);
				LcdcDisplayModeSet(0, 0);
				LcdcBackLightOn();
				break;
// �������[�h ------------------------------------------------------------------------------------//
			case FPTN_LCD_SCREEN1:
				num = 1;
				//////// �t���O�̃N���A ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN1 );
				if ( ercd != E_OK ){
					break;
				}

				//////// ��ʕ\�� ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);

				///////// �L�[���͑҂� ////////
				sts_key = WaitKeyProc(99, 0, msg);
				if(sts_key == 1){	//�L�[��������

					//////// ���b�Z�[�W���M ////////
					g_LcdmsgData.LcdMsgSize = DbgSendMessageToMain(num, 0, msg);
					//g_LcdmsgData.LcdMsgSize = MsgSize;
					//ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
					//if(ercd != E_OK){
					//	//�G���|�����v
					//}					
					///////// �t���O�Z�b�g ////////
					ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
					if(ercd != E_OK){
						//�G���|�����v
					}
				}
				break;
			case FPTN_LCD_SCREEN2:
				num = 2;
				//////// �t���O�̃N���A ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN2 );
				if ( ercd != E_OK ){
					break;
				}


				//////// ��ʕ\�� ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				///////// �L�[���͑҂� ////////
				sts_key = WaitKeyProc(99, 0, msg);
				if(sts_key == 1){	//�L�[��������

					//////// ���b�Z�[�W���M ////////
					g_LcdmsgData.LcdMsgSize = DbgSendMessageToMain(num, 0, msg);
					//ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
					//if(ercd != E_OK){
						//�G���|�����v
					//}					
					///////// �t���O�Z�b�g ////////
					ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
					if(ercd != E_OK){
						//�G���|�����v
					}
				}
				break;
			case FPTN_LCD_SCREEN3:
				num = 3;
				//////// �t���O�̃N���A ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN3 );
				if ( ercd != E_OK ){
					break;
				}

				//////// ��ʕ\�� ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				///////// �L�[���͑҂� ////////
				sts_key = WaitKeyProc(99, 0, msg);
				if(sts_key == 1){	//�L�[��������

					//////// ���b�Z�[�W���M ////////
					g_LcdmsgData.LcdMsgSize = DbgSendMessageToMain(num, 0, msg);
					//ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
					//if(ercd != E_OK){
						//�G���|�����v
					//}					
					///////// �t���O�Z�b�g ////////
					ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
					if(ercd != E_OK){
						//�G���|�����v
					}
				}
				break;
			case FPTN_LCD_SCREEN4:
				num = 4;
				//////// �t���O�̃N���A ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN4 );
				if ( ercd != E_OK ){
					break;
				}

				//////// ��ʕ\�� ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				///////// �L�[���͑҂� ////////
				sts_key = WaitKeyProc(99, 0, msg);
				if(sts_key == 1){	//�L�[��������

					//////// ���b�Z�[�W���M ////////
					g_LcdmsgData.LcdMsgSize = DbgSendMessageToMain(num, 0, msg);
					//ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
					//if(ercd != E_OK){
						//�G���|�����v
					//}					
					///////// �t���O�Z�b�g ////////
					ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
					if(ercd != E_OK){
						//�G���|�����v
					}
				}
				break;
			case FPTN_LCD_SCREEN5:
				num = 5;
				//////// �t���O�̃N���A ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN5 );
				if ( ercd != E_OK ){
					break;
				}

				//////// ��ʕ\�� ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				///////// �L�[���͑҂� ////////
				sts_key = WaitKeyProc(99, 0, msg);
				if(sts_key == 1){	//�L�[��������

					//////// ���b�Z�[�W���M ////////
					g_LcdmsgData.LcdMsgSize = DbgSendMessageToMain(num, 0, msg);
					//ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
					//if(ercd != E_OK){
						//�G���|�����v
					//}
					///////// �t���O�Z�b�g ////////
					ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
					if(ercd != E_OK){
						//�G���|�����v
					}
				}				
				break;
			case FPTN_LCD_SCREEN6:
				num = 6;
				//////// �t���O�̃N���A ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN6 );
				if ( ercd != E_OK ){
					break;
				}

				//////// ��ʕ\�� ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				break;
			case FPTN_LCD_SCREEN7:
				num = 7;
				//////// �t���O�̃N���A ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN7 );
				if ( ercd != E_OK ){
					break;
				}

				//////// ��ʕ\�� ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				break;
			case FPTN_LCD_SCREEN8:
				num = 8;
				//////// �t���O�̃N���A ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN8 );
				if ( ercd != E_OK ){
					break;
				}

				//////// ��ʕ\�� ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				break;
			case FPTN_LCD_SCREEN9:
				num = 9;
				//////// �t���O�̃N���A ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN9 );
				if ( ercd != E_OK ){
					break;
				}

				//////// ��ʕ\�� ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);

				dly_tsk((3000/MSEC));	//�^�C�}�[3sec

				//////// ���b�Z�[�W���M ////////
				g_LcdmsgData.LcdMsgSize = DbgSendMessageToMain(num, 0, msg);
				//ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
				//if(ercd != E_OK){
					//�G���|�����v
				//}					

				///////// �t���O�Z�b�g ////////
				ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
				if(ercd != E_OK){
					//�G���|�����v
				}
				break;
			case FPTN_LCD_SCREEN10:
				num = 10;
				//////// �t���O�̃N���A ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN10 );
				if ( ercd != E_OK ){
					break;
				}
			
				//////// ��ʕ\�� ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);

				dly_tsk((3000/MSEC));	//�^�C�}�[3sec

				//////// ���b�Z�[�W���M ////////
				g_LcdmsgData.LcdMsgSize = DbgSendMessageToMain(num, 0, msg);
				//ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
				//if(ercd != E_OK){
					//�G���|�����v
				//}					

				///////// �t���O�Z�b�g ////////
				ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
				if(ercd != E_OK){
					//�G���|�����v
				}
				break;
			case FPTN_LCD_SCREEN11:
				num = 11;
				//////// �t���O�̃N���A ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN11 );
				if ( ercd != E_OK ){
					break;
				}

				//////// ��ʕ\�� ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				///////// �L�[���͑҂� ////////
				sts_key = WaitKeyProc(99, 0, msg);
				if(sts_key == 1){	//�L�[��������
					//////// ���b�Z�[�W���M ////////
					g_LcdmsgData.LcdMsgSize = DbgSendMessageToMain(num, 0, msg);
					//ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
					//if(ercd != E_OK){
						//�G���|�����v
					//}	

					///////// �t���O�Z�b�g ////////
					ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
					if(ercd != E_OK){
						//�G���|�����v
					}

					//////// ���b�Z�[�W���M ////////
					//MsgSize = DbgSendMessageToMain(num, 0, msg);
					//ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
					//if(ercd != E_OK){
						//�G���|�����v
					//}	
				}				
				break;
			case FPTN_LCD_SCREEN12:
				num = 12;
				//////// �t���O�̃N���A ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN12 );
				if ( ercd != E_OK ){
					break;
				}

				//////// ��ʕ\�� ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				///////// �L�[���͑҂� ////////
				sts_key = WaitKeyProc(99, 0, msg);
				if(sts_key == 1){	//�L�[��������
					///////// �t���O�Z�b�g ////////
					ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
					if(ercd != E_OK){
						//�G���|�����v
					}

					//////// ���b�Z�[�W���M ////////
					MsgSize = DbgSendMessageToMain(num, 0, msg);
					ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
					if(ercd != E_OK){
						//�G���|�����v
					}
				}					
				break;
// �ʏ탂�[�h ------------------------------------------------------------------------------------//
			case FPTN_LCD_SCREEN100:
				num = 100;
				//////// �t���O�̃N���A ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN100 );
				if ( ercd != E_OK ){
					break;
				}

				//////// ��ʕ\�� ////////
				SetGamen01(1, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				LcdcDisplayModeSet(1, 0);
				
				//////// ��ʃo�b�t�@�ɓ]�� ////////
				for(cnt = 0 ; cnt < 2 ; cnt++){	//��ʂ���ʃo�b�t�@�ɓ]������
					SetGamen01(cnt, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				}
				SetGamen02(0, perm_enb, perm_r, perm_g, perm_b, 208, 104, 271, 167);	//NEXT�{�^��

				///////// �t���O�Z�b�g ////////
				ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
				if(ercd != E_OK){
					//�G���|�����v
				}

				//////// ���b�Z�[�W���M ////////
				MsgSize = DbgSendMessageToMain(num, 0, msg);
				ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
				if(ercd != E_OK){
					//�G���|�����v
				}
				break;
			case FPTN_LCD_SCREEN101:
				num = 101;
				//////// �t���O�̃N���A ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN101 );
				if ( ercd != E_OK ){
					break;
				}

				//////// ��ʕ\�� ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				///////// �L�[���͑҂� ////////
				sts_key = WaitKeyProc(99, 1, msg);
				if(sts_key == 1){	//�L�[��������
					///////// �t���O�Z�b�g ////////
					ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
					if(ercd != E_OK){
						//�G���|�����v
					}

					//////// ���b�Z�[�W���M ////////
					MsgSize = DbgSendMessageToMain(num, 0, msg);
					ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
					if(ercd != E_OK){
						//�G���|�����v
					}
				}					
				break;
			case FPTN_LCD_SCREEN102:
				num = 101;	//���Sleep��� (���101�̏���������)
				//////// �t���O�̃N���A ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN102 );
				if ( ercd != E_OK ){
					break;
				}

				//////// ��ʕ\�� ////////
				LcdcBackLightOff();	//���Sleep���
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				///////// �L�[���͑҂� ////////
				sts_key = WaitKeyProc(99, 2, msg);	//���Sleep���
				if(sts_key == 1){	//�L�[��������
					///////// �t���O�Z�b�g ////////
					ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
					if(ercd != E_OK){
						//�G���|�����v
					}

					//////// ���b�Z�[�W���M ////////
					MsgSize = DbgSendMessageToMain(num, 0, msg);
					ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
					if(ercd != E_OK){
						//�G���|�����v
					}
				}					
				break;
			case FPTN_LCD_SCREEN103:
				num = 103;
				//////// �t���O�̃N���A ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN103 );
				if ( ercd != E_OK ){
					break;
				}

				//////// ��ʕ\�� ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);

				dly_tsk((3000/MSEC));	//�^�C�}�[3sec

				///////// �t���O�Z�b�g ////////
				ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
				if(ercd != E_OK){
					//�G���|�����v
				}

				//////// ���b�Z�[�W���M ////////
				MsgSize = DbgSendMessageToMain(num, 0, msg);
				ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
				if(ercd != E_OK){
					//�G���|�����v
				}					
				break;
			case FPTN_LCD_SCREEN104:
				num = 104;
				//////// �t���O�̃N���A ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN104 );
				if ( ercd != E_OK ){
					break;
				}
			
				//////// ��ʕ\�� ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);

				dly_tsk((3000/MSEC));	//�^�C�}�[3sec

				///////// �t���O�Z�b�g ////////
				ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
				if(ercd != E_OK){
					//�G���|�����v
				}

				//////// ���b�Z�[�W���M ////////
				MsgSize = DbgSendMessageToMain(num, 0, msg);
				ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
				if(ercd != E_OK){
					//�G���|�����v
				}					
				break;
// �ʏ탂�[�h(�o�^) ------------------------------------------------------------------------------------//
			case FPTN_LCD_SCREEN120:
				num = 120;
				//////// �t���O�̃N���A ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN120 );
				if ( ercd != E_OK ){
					break;
				}

				//////// ��ʕ\�� ////////
				SetGamen01(1, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				LcdcDisplayModeSet(1, 0);
				
				//////// ��ʃo�b�t�@�ɓ]�� ////////
				for(cnt = 0 ; cnt < 2 ; cnt++){	//��ʂ���ʃo�b�t�@�ɓ]������
					SetGamen01(cnt, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				}
				SetGamen02(0, perm_enb, perm_r, perm_g, perm_b, 208, 104, 271, 167);	//NEXT�{�^��

				///////// �t���O�Z�b�g ////////
				ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
				if(ercd != E_OK){
					//�G���|�����v
				}

				//////// ���b�Z�[�W���M ////////
				MsgSize = DbgSendMessageToMain(num, 0, msg);
				ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
				if(ercd != E_OK){
					//�G���|�����v
				}
				break;
			case FPTN_LCD_SCREEN121:
				num = 121;
				//////// �t���O�̃N���A ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN121 );
				if ( ercd != E_OK ){
					break;
				}

				//////// ��ʕ\�� ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				break;
			case FPTN_LCD_SCREEN122:
				num = 122;
				//////// �t���O�̃N���A ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN122 );
				if ( ercd != E_OK ){
					break;
				}

				//////// ��ʕ\�� ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);

				dly_tsk((3000/MSEC));	//�^�C�}�[3sec

				///////// �t���O�Z�b�g ////////
				ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
				if(ercd != E_OK){
					//�G���|�����v
				}

				//////// ���b�Z�[�W���M ////////
				MsgSize = DbgSendMessageToMain(num, 0, msg);
				ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
				if(ercd != E_OK){
					//�G���|�����v
				}					
				break;
			case FPTN_LCD_SCREEN123:
				num = 123;
				//////// �t���O�̃N���A ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN123 );
				if ( ercd != E_OK ){
					break;
				}
			
				//////// ��ʕ\�� ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);

				dly_tsk((3000/MSEC));	//�^�C�}�[3sec

				///////// �t���O�Z�b�g ////////
				ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
				if(ercd != E_OK){
					//�G���|�����v
				}

				//////// ���b�Z�[�W���M ////////
				MsgSize = DbgSendMessageToMain(num, 0, msg);
				ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
				if(ercd != E_OK){
					//�G���|�����v
				}					
				break;
			case FPTN_LCD_SCREEN124:
				num = 124;
				//////// �t���O�̃N���A ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN124 );
				if ( ercd != E_OK ){
					break;
				}

				//////// ��ʕ\�� ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				///////// �L�[���͑҂� ////////
				sts_key = WaitKeyProc(99, 0, msg);
				if(sts_key == 1){	//�L�[��������
					///////// �t���O�Z�b�g ////////
					ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
					if(ercd != E_OK){
						//�G���|�����v
					}

					//////// ���b�Z�[�W���M ////////
					MsgSize = DbgSendMessageToMain(num, 0, msg);
					ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
					if(ercd != E_OK){
						//�G���|�����v
					}					
				}
				break;
			case FPTN_LCD_SCREEN125:
				num = 125;
				//////// �t���O�̃N���A ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN125 );
				if ( ercd != E_OK ){
					break;
				}

				//////// ��ʕ\�� ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				///////// �L�[���͑҂� ////////
				sts_key = WaitKeyProc(99, 0, msg);
				if(sts_key == 1){	//�L�[��������
					///////// �t���O�Z�b�g ////////
					ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
					if(ercd != E_OK){
						//�G���|�����v
					}

					//////// ���b�Z�[�W���M ////////
					MsgSize = DbgSendMessageToMain(num, 0, msg);
					ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
					if(ercd != E_OK){
						//�G���|�����v
					}					
				}
				break;
			case FPTN_LCD_SCREEN126:
				num = 126;
				//////// �t���O�̃N���A ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN126 );
				if ( ercd != E_OK ){
					break;
				}

				//////// ��ʕ\�� ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				///////// �L�[���͑҂� ////////
				sts_key = WaitKeyProc(99, 0, msg);
				if(sts_key == 1){	//�L�[��������
					///////// �t���O�Z�b�g ////////
					ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
					if(ercd != E_OK){
						//�G���|�����v
					}

					//////// ���b�Z�[�W���M ////////
					MsgSize = DbgSendMessageToMain(num, 0, msg);
					ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
					if(ercd != E_OK){
						//�G���|�����v
					}
				}				
				break;
			case FPTN_LCD_SCREEN127:
				num = 127;
				//////// �t���O�̃N���A ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN127 );
				if ( ercd != E_OK ){
					break;
				}

				//////// ��ʕ\�� ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				break;
			case FPTN_LCD_SCREEN128:
				num = 128;
				//////// �t���O�̃N���A ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN128 );
				if ( ercd != E_OK ){
					break;
				}

				//////// ��ʕ\�� ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				break;
			case FPTN_LCD_SCREEN129:
				num = 129;
				//////// �t���O�̃N���A ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN129 );
				if ( ercd != E_OK ){
					break;
				}

				//////// ��ʕ\�� ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				break;
			case FPTN_LCD_SCREEN130:
				num = 130;
				//////// �t���O�̃N���A ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN130 );
				if ( ercd != E_OK ){
					break;
				}

				//////// ��ʕ\�� ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);

				dly_tsk((3000/MSEC));	//�^�C�}�[3sec

				///////// �t���O�Z�b�g ////////
				ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
				if(ercd != E_OK){
					//�G���|�����v
				}

				//////// ���b�Z�[�W���M ////////
				MsgSize = DbgSendMessageToMain(num, 0, msg);
				ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
				if(ercd != E_OK){
					//�G���|�����v
				}					
				break;
			case FPTN_LCD_SCREEN131:
				num = 131;
				//////// �t���O�̃N���A ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN131 );
				if ( ercd != E_OK ){
					break;
				}
			
				//////// ��ʕ\�� ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);

				dly_tsk((3000/MSEC));	//�^�C�}�[3sec

				///////// �t���O�Z�b�g ////////
				ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
				if(ercd != E_OK){
					//�G���|�����v
				}

				//////// ���b�Z�[�W���M ////////
				MsgSize = DbgSendMessageToMain(num, 0, msg);
				ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
				if(ercd != E_OK){
					//�G���|�����v
				}					
				break;
			case FPTN_LCD_SCREEN132:
				num = 132;
				//////// �t���O�̃N���A ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN132 );
				if ( ercd != E_OK ){
					break;
				}

				//////// ��ʕ\�� ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				///////// �L�[���͑҂� ////////
				sts_key = WaitKeyProc(99, 0, msg);
				if(sts_key == 1){	//�L�[��������
					///////// �t���O�Z�b�g ////////
					ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
					if(ercd != E_OK){
						//�G���|�����v
					}

					//////// ���b�Z�[�W���M ////////
					MsgSize = DbgSendMessageToMain(num, 0, msg);
					ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
					if(ercd != E_OK){
						//�G���|�����v
					}
				}					
				break;
// �ʏ탂�[�h(�폜) ------------------------------------------------------------------------------------//
			case FPTN_LCD_SCREEN140:
				num = 140;
				//////// �t���O�̃N���A ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN140 );
				if ( ercd != E_OK ){
					break;
				}

				//////// ��ʕ\�� ////////
				SetGamen01(1, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				LcdcDisplayModeSet(1, 0);
				
				//////// ��ʃo�b�t�@�ɓ]�� ////////
				for(cnt = 0 ; cnt < 2 ; cnt++){	//��ʂ���ʃo�b�t�@�ɓ]������
					SetGamen01(cnt, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				}
				SetGamen02(0, perm_enb, perm_r, perm_g, perm_b, 208, 104, 271, 167);	//NEXT�{�^��

				///////// �t���O�Z�b�g ////////
				ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
				if(ercd != E_OK){
					//�G���|�����v
				}

				//////// ���b�Z�[�W���M ////////
				MsgSize = DbgSendMessageToMain(num, 0, msg);
				ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
				if(ercd != E_OK){
					//�G���|�����v
				}
				break;
			case FPTN_LCD_SCREEN141:
				num = 141;
				//////// �t���O�̃N���A ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN141 );
				if ( ercd != E_OK ){
					break;
				}

				//////// ��ʕ\�� ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				break;
			case FPTN_LCD_SCREEN142:
				num = 142;
				//////// �t���O�̃N���A ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN142 );
				if ( ercd != E_OK ){
					break;
				}

				//////// ��ʕ\�� ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);

				dly_tsk((3000/MSEC));	//�^�C�}�[3sec

				///////// �t���O�Z�b�g ////////
				ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
				if(ercd != E_OK){
					//�G���|�����v
				}

				//////// ���b�Z�[�W���M ////////
				MsgSize = DbgSendMessageToMain(num, 0, msg);
				ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
				if(ercd != E_OK){
					//�G���|�����v
				}					
				break;
			case FPTN_LCD_SCREEN143:
				num = 143;
				//////// �t���O�̃N���A ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN143 );
				if ( ercd != E_OK ){
					break;
				}
			
				//////// ��ʕ\�� ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);

				dly_tsk((3000/MSEC));	//�^�C�}�[3sec

				///////// �t���O�Z�b�g ////////
				ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
				if(ercd != E_OK){
					//�G���|�����v
				}

				//////// ���b�Z�[�W���M ////////
				MsgSize = DbgSendMessageToMain(num, 0, msg);
				ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
				if(ercd != E_OK){
					//�G���|�����v
				}					
				break;
			case FPTN_LCD_SCREEN144:
				num = 144;
				//////// �t���O�̃N���A ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN144 );
				if ( ercd != E_OK ){
					break;
				}

				//////// ��ʕ\�� ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				///////// �L�[���͑҂� ////////
				sts_key = WaitKeyProc(99, 0, msg);
				if(sts_key == 1){	//�L�[��������
					///////// �t���O�Z�b�g ////////
					ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
					if(ercd != E_OK){
						//�G���|�����v
					}

					//////// ���b�Z�[�W���M ////////
					MsgSize = DbgSendMessageToMain(num, 0, msg);
					ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
					if(ercd != E_OK){
						//�G���|�����v
					}					
				}
				break;
			case FPTN_LCD_SCREEN145:
				num = 145;
				//////// �t���O�̃N���A ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN145 );
				if ( ercd != E_OK ){
					break;
				}

				//////// ��ʕ\�� ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				///////// �L�[���͑҂� ////////
				sts_key = WaitKeyProc(99, 0, msg);
				if(sts_key == 1){	//�L�[��������
					///////// �t���O�Z�b�g ////////
					ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
					if(ercd != E_OK){
						//�G���|�����v
					}

					//////// ���b�Z�[�W���M ////////
					MsgSize = DbgSendMessageToMain(num, 1, msg);
					ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
					if(ercd != E_OK){
						//�G���|�����v
					}
				}					
				break;
			case FPTN_LCD_SCREEN146:
				num = 146;
				//////// �t���O�̃N���A ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN146 );
				if ( ercd != E_OK ){
					break;
				}

				//////// ��ʕ\�� ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				///////// �L�[���͑҂� ////////
				sts_key = WaitKeyProc(99, 0, msg);
				if(sts_key == 1){	//�L�[��������
					///////// �t���O�Z�b�g ////////
					ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
					if(ercd != E_OK){
						//�G���|�����v
					}

					//////// ���b�Z�[�W���M ////////
					MsgSize = DbgSendMessageToMain(num, 0, msg);
					ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
					if(ercd != E_OK){
						//�G���|�����v
					}
				}					
				break;
// �����e���[�h ------------------------------------------------------------------------------------//
			case FPTN_LCD_SCREEN200:
				num = 200;
				//////// �t���O�̃N���A ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN200 );
				if ( ercd != E_OK ){
					break;
				}

				//////// ��ʕ\�� ////////
				SetGamen01(1, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				LcdcDisplayModeSet(1, 0);
				
				//////// ��ʃo�b�t�@�ɓ]�� ////////
				for(cnt = 0 ; cnt < 2 ; cnt++){	//��ʂ���ʃo�b�t�@�ɓ]������
					SetGamen01(cnt, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				}
				SetGamen02(0, perm_enb, perm_r, perm_g, perm_b, 208, 104, 271, 167);	//NEXT�{�^��

				///////// �t���O�Z�b�g ////////
				ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
				if(ercd != E_OK){
					//�G���|�����v
				}

				//////// ���b�Z�[�W���M ////////
				MsgSize = DbgSendMessageToMain(num, 0, msg);
				ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
				if(ercd != E_OK){
					//�G���|�����v
				}
				break;
			case FPTN_LCD_SCREEN201:
				num = 201;
				//////// �t���O�̃N���A ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN201 );
				if ( ercd != E_OK ){
					break;
				}


//��ʕ\���e�X�g
				SetGamen02(201, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				dly_tsk((3000/MSEC));	//�^�C�}�[3sec
				SetGamen02(202, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				dly_tsk((3000/MSEC));	//�^�C�}�[3sec
				SetGamen02(203, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				dly_tsk((3000/MSEC));	//�^�C�}�[3sec
				SetGamen02(204, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				dly_tsk((3000/MSEC));	//�^�C�}�[3sec
				SetGamen02(205, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				dly_tsk((3000/MSEC));	//�^�C�}�[3sec
//��ʕ\���e�X�g

				//////// ��ʕ\�� ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				///////// �L�[���͑҂� ////////
				sts_key = WaitKeyProc(99, 0, msg);
				if(sts_key == 1){	//�L�[��������
					///////// �t���O�Z�b�g ////////
					ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
					if(ercd != E_OK){
						//�G���|�����v
					}

					//////// ���b�Z�[�W���M ////////
					MsgSize = DbgSendMessageToMain(num, 0, msg);
					ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
					if(ercd != E_OK){
						//�G���|�����v
					}					
				}
				break;
			case FPTN_LCD_SCREEN202:
				num = 202;
				//////// �t���O�̃N���A ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN202 );
				if ( ercd != E_OK ){
					break;
				}

				//////// ��ʕ\�� ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				///////// �L�[���͑҂� ////////
				sts_key = WaitKeyProc(99, 0, msg);
				if(sts_key == 1){	//�L�[��������
					///////// �t���O�Z�b�g ////////
					ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
					if(ercd != E_OK){
						//�G���|�����v
					}

					//////// ���b�Z�[�W���M ////////
					MsgSize = DbgSendMessageToMain(num, 0, msg);
					ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
					if(ercd != E_OK){
						//�G���|�����v
					}					
				}
				break;
			case FPTN_LCD_SCREEN203:
				num = 203;
				//////// �t���O�̃N���A ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN203 );
				if ( ercd != E_OK ){
					break;
				}

				//////// ��ʕ\�� ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);
				break;
			case FPTN_LCD_SCREEN204:
				num = 204;
				//////// �t���O�̃N���A ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN204 );
				if ( ercd != E_OK ){
					break;
				}

				//////// ��ʕ\�� ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);

				dly_tsk((3000/MSEC));	//�^�C�}�[3sec

				///////// �t���O�Z�b�g ////////
				ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
				if(ercd != E_OK){
					//�G���|�����v
				}

				//////// ���b�Z�[�W���M ////////
				MsgSize = DbgSendMessageToMain(num, 0, msg);
				ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
				if(ercd != E_OK){
					//�G���|�����v
				}					
				break;
			case FPTN_LCD_SCREEN205:
				num = 205;
				//////// �t���O�̃N���A ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN205 );
				if ( ercd != E_OK ){
					break;
				}
			
				//////// ��ʕ\�� ////////
				SetGamen02(num, perm_enb, perm_r, perm_g, perm_b, 0, 104, 63, 167);
				LcdcDisplayModeSet(1, 0);

				dly_tsk((3000/MSEC));	//�^�C�}�[3sec

				///////// �t���O�Z�b�g ////////
				ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
				if(ercd != E_OK){
					//�G���|�����v
				}

				//////// ���b�Z�[�W���M ////////
				MsgSize = DbgSendMessageToMain(num, 0, msg);
				ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, MsgSize );
				if(ercd != E_OK){
					//�G���|�����v
				}					
				break;

			default:
				break;
		}
	}

	PrgErrSet();
	slp_tsk();					//�����ɗ���̂́A�V�X�e���E�G���[
}


UB WaitKeyProc(int sousa, int tflg, UB *msg)
{
	UB	rtn=0;
	UB	rtnsts;
	ER	ercd;
	int posx, posy;
	int x1, x2, y1, y2;
	int tcnt;
	int dbg_cnt;
	UB *buf;

	buf = msg;
	tcnt = 0;
	dbg_cnt = 0;
	while(1){
		ercd = TplPosGet(&posx, &posy, (500/MSEC));	//�L�[�҂�500msec
		if(	ercd == E_OK ){
			dly_tsk((500/MSEC));	//500msec�̃E�G�C�g(�^�b�`�L�[�̔��������p)
			rtn = 1;	//CHG_REQ�v��

			if(tflg == 2){	//���Sleep��Ԃ̏ꍇ(LCD�o�b�N���C�g��OFF�ɂ��Ă���)
				LcdcBackLightOn();
				tflg = 1;
				tcnt = 0;
			}else{
				if( sousa == 99 ){
					x1 = 208;
					x2 = 272;
					y1 = 104;
					y2 = 167;
					if( (posx > x1 && posx < x2) && (posy > y1 && posy < y2) ){
						break;
					}
				}else{
					break;
				}
			}
			
		}

		if(tflg == 1){
			++tcnt;
			if(tcnt >= 120){	//���Sleep��Ԃɂ���
				LcdcBackLightOff();
				tflg = 2;
				tcnt = 0;
			}
		}

		rtnsts = MdGetMode();
		if( rtnsts == MD_CAP ){
			rtn = 2;	//CHG_REQ�Ȃ�
			LcdcBackLightOn();
			break;
		}
	}

	return(rtn);
}


void SetGamen01(int buf, UB perm, UB perm_r, UB perm_g, UB perm_b, UH stx, UH sty, UH edx, UH edy)
{
	ER rtn;
	int x, y, x1, x2, y1, y2, size;
	UH val1, num;
	//UH val2;
	unsigned long cnt;

	num = buf;
	LcdcDisplayWindowSet(num, perm, perm_r, perm_g, perm_b, stx, sty, edx, edy);
	
	x1 = stx;
	x2 = edx;
	y1 = sty;
	y2 = edy;

	size = (x2 - x1 + 1) * (y2 - y1 + 1) * 2;
	for(cnt = 0 ; cnt < size ; cnt++ ){
		val1 = GetPixDataFromGamen01(num, cnt);
		rtn = LcdImageWrite(val1);	
		if(rtn != E_OK ){
			break;
		}
	}
	
	LcdcDisplayWindowFinish();
}


UH GetPixDataFromGamen01(int buf, unsigned long cnt)
{
	UH val;

	switch(buf){
		case 0:
			//val = LcdGmn01[cnt];
			val = 0x00;
			break;
		case 1:
			val = 0x00;
			break;
		default:
			val = 0x00;
			break;
	}
	return(val);
}



void SetGamen02(int buf, UB perm, UB perm_r, UB perm_g, UB perm_b, UH stx, UH sty, UH edx, UH edy)
{
	ER rtn;
	int x, y, x1, x2, y1, y2, size;
	UH val1, num;
	//UH val2;
	unsigned long cnt;

	num = buf;
	LcdcDisplayWindowSet(1, perm, perm_r, perm_g, perm_b, stx, sty, edx, edy);
	
	x1 = stx;
	x2 = edx;
	y1 = sty;
	y2 = edy;

	size = (x2 - x1 + 1) * (y2 - y1 + 1) * 2;
	for(cnt = 0 ; cnt < size ; cnt++ ){
		val1 = GetPixDataFromGamen02(num, cnt);
		rtn = LcdImageWrite(val1);	
		if(rtn != E_OK ){
			break;
		}
	}
	
	LcdcDisplayWindowFinish();
}



UH GetPixDataFromGamen02(int buf, unsigned long cnt)
{
	UH val;
	
	switch(buf){
		case 0:
			val = btn_NEXT[cnt];
			break;
		case 1:
			val = dbgmn001[cnt];
			break;
		case 2:
			val = dbgmn002[cnt];
			break;
		case 3:
			val = dbgmn003[cnt];
			break;
		case 4:
			val = dbgmn004[cnt];
			break;
		case 5:
			val = dbgmn005[cnt];
			break;
		case 6:
			val = dbgmn006[cnt];
			break;
		case 7:
			val = dbgmn007[cnt];
			break;
		case 8:
			val = dbgmn008[cnt];
			break;
		case 9:
			val = dbgmn009[cnt];
			break;
		case 10:
			val = dbgmn010[cnt];
			break;
		case 11:
			val = dbgmn011[cnt];
			break;
		case 12:
			val = dbgmn012[cnt];
			break;
		case 101:
			val = dbgmn101[cnt];
			break;
		case 102:
			val = dbgmn102[cnt];
			break;
		case 103:
			val = dbgmn103[cnt];
			break;
		case 104:
			val = dbgmn104[cnt];
			break;
		case 121:
			val = dbgmn121[cnt];
			break;
		case 122:
			val = dbgmn122[cnt];
			break;
		case 123:
			val = dbgmn123[cnt];
			break;
		case 124:
			val = dbgmn124[cnt];
			break;
		case 125:
			val = dbgmn125[cnt];
			break;
		case 126:
			val = dbgmn126[cnt];
			break;
		case 127:
			val = dbgmn127[cnt];
			break;
		case 128:
			val = dbgmn128[cnt];
			break;
		case 129:
			val = dbgmn129[cnt];
			break;
		case 130:
			val = dbgmn130[cnt];
			break;
		case 131:
			val = dbgmn131[cnt];
			break;
		case 132:
			val = dbgmn132[cnt];
			break;
		case 141:
			val = dbgmn141[cnt];
			break;
		case 142:
			val = dbgmn142[cnt];
			break;
		case 143:
			val = dbgmn143[cnt];
			break;
		case 144:
			val = dbgmn144[cnt];
			break;
		case 145:
			val = dbgmn145[cnt];
			break;
		case 146:
			val = dbgmn146[cnt];
			break;
		case 201:
			val = dbgmn201[cnt];
			break;
		case 202:
			val = dbgmn202[cnt];
			break;
		case 203:
			val = dbgmn203[cnt];
			break;
		case 204:
			val = dbgmn204[cnt];
			break;
		default:
			val = 0x00;
			break;
	}
	
	return(val);
}


UINT DbgSendMessageToMain(int num, int sousa, UB *msg)
{
	UB *buf;
	int cnt, i;
	UINT msize;

	msize = 0;	
	buf = msg;

	cnt = 0;
	switch(num)
	{
		case 1:
			*(buf + cnt++) = ( UB )LCD_USER_ID;
			*(buf + cnt++) = '0';
			*(buf + cnt++) = '9';
			*(buf + cnt++) = '8';
			*(buf + cnt++) = '7';
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 2:
			*(buf + cnt++) = ( UB )LCD_INIT_INPUT;
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 3:
			*(buf + cnt++) = ( UB )LCD_YUBI_ID;
			*(buf + cnt++) = '0';
			*(buf + cnt++) = '0';
			*(buf + cnt++) = '1';
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 4:
			for(i = 0 ; i < 24 ; i++ ){
				*(buf + i) = 0x20;
			}
			*(buf + 24) = 0;
				
			*(buf + cnt++) = ( UB )LCD_NAME;
			*(buf + cnt++) = '�';
			*(buf + cnt++) = '�';
			*(buf + cnt++) = '�';
			msize = 26;
			break;
		case 5:
			*(buf + cnt++) = ( UB )LCD_YUBI_SHUBETU;
			*(buf + cnt++) = '1';
			*(buf + cnt++) = '2';
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 6:
			//*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 7:
			//*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 8:
			//*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 9:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 10:
			*(buf + cnt++) = ( UB )LCD_BACK;
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 11:
			if(sousa == 0){
				*(buf + cnt++) = ( UB )LCD_YES;
				*(buf + cnt++) = 0;
				msize = cnt;
			}else{
				*(buf + cnt++) = ( UB )LCD_NO;
				*(buf + cnt++) = 0;
				msize = cnt;
			}
			break;
		case 12:
			if(sousa == 0){
				*(buf + cnt++) = ( UB )LCD_YES;
				*(buf + cnt++) = 0;
				msize = cnt;
			}else{
				*(buf + cnt++) = ( UB )LCD_NO;
				*(buf + cnt++) = 0;
				msize = cnt;
			}
			break;
		case 100:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 101:
		case 102:
			if(sousa == 0){
				*(buf + cnt++) = ( UB )LCD_TOUROKU;
				*(buf + cnt++) = 0;
				msize = cnt;
			}else if(sousa == 1){
				*(buf + cnt++) = ( UB )LCD_SAKUJYO;
				*(buf + cnt++) = 0;
				msize = cnt;
			}else{
				*(buf + cnt++) = ( UB )LCD_MAINTE;
				*(buf + cnt++) = 0;
				msize = cnt;
			}
			break;
		case 103:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 104:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 120:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 121:
			//*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 122:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 123:
			*(buf + cnt++) = ( UB )LCD_BACK;
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 124:
			*(buf + cnt++) = ( UB )LCD_YUBI_ID;
			*(buf + cnt++) = '0';
			*(buf + cnt++) = '0';
			*(buf + cnt++) = '1';
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 125:
			for(i = 0 ; i < 24 ; i++ ){
				*(buf + i) = 0x20;
			}
			*(buf + 24) = 0;
				
			*(buf + cnt++) = ( UB )LCD_NAME;
			*(buf + cnt++) = '�';
			*(buf + cnt++) = '�';
			*(buf + cnt++) = '�';
			msize = 26;
			break;
		case 126:
			*(buf + cnt++) = ( UB )LCD_YUBI_SHUBETU;
			*(buf + cnt++) = '1';
			*(buf + cnt++) = '2';
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 127:
			//*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 128:
			//*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 129:
			//*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 130:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 131:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 132:
			if(sousa == 0){
				*(buf + cnt++) = ( UB )LCD_YES;
				*(buf + cnt++) = 0;
				msize = cnt;
			}else{
				*(buf + cnt++) = ( UB )LCD_NO;
				*(buf + cnt++) = 0;
				msize = cnt;
			}
			break;
		case 140:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 141:
			//*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 142:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 143:
			*(buf + cnt++) = ( UB )LCD_BACK;
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 144:
			*(buf + cnt++) = ( UB )LCD_YUBI_ID;
			*(buf + cnt++) = '0';
			*(buf + cnt++) = '0';
			*(buf + cnt++) = '1';
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 145:
			if(sousa == 0){
				*(buf + cnt++) = ( UB )LCD_YES;
				*(buf + cnt++) = 0;
				msize = cnt;
			}else{
				*(buf + cnt++) = ( UB )LCD_NO;
				*(buf + cnt++) = 0;
				msize = cnt;
			}
			break;
		case 146:
			if(sousa == 0){
				*(buf + cnt++) = ( UB )LCD_YES;
				*(buf + cnt++) = 0;
				msize = cnt;
			}else{
				*(buf + cnt++) = ( UB )LCD_NO;
				*(buf + cnt++) = 0;
				msize = cnt;
			}
			break;
		case 200:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 201:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = '9';
			*(buf + cnt++) = '9';
			*(buf + cnt++) = '9';
			*(buf + cnt++) = '9';
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 202:
			*(buf + cnt++) = ( UB )LCD_FULL_PIC_SEND_REQ;
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 203:
			//*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 204:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		case 205:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			msize = cnt;
			break;
		default:
			break;
	}


	return(msize);
}
