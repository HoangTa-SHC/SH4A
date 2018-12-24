/**
*	VA-300�v���O����
*
*	@file tsk_exkey.c
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/08/30
*	@brief  �O���L�[����^�X�N
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
#include "drv_7seg.h"
#include "err_ctrl.h"
#include "command.h"
#include "drv_exkey.h"

#include "va300.h"

#define	KEY_FIG		8					///< ����

// �ϐ���`
static ID s_idFlg;						///< �t���OID
static ID s_idTsk;						///< �^�X�NID
static B s_cId[ KEY_FIG + 1 ];			///< ID�ԍ�
static int s_iFig;						///< ���͌���

// �v���g�^�C�v�錾
static void idNumberClr(void);			///< ID�ԍ����N���A
static void dispSegOff(void);			///< 7SEG�\��OFF
static void dispSegNumber(B *p);		///< 7SEG�ɔԍ��\��

// 
const T_CTSK ctsk_key = { TA_HLNG, NULL, ExKeyTask, 5, 2048, NULL, (B *)"ExKey task" };//

/*==========================================================================*/
/**
 * �O���L�[�^�X�N������
 */
/*==========================================================================*/
ER ExKeyTaskInit(ID tskid)
{
	ER ercd;
	UW psw;
	
	// ID�ԍ��N���A
	idNumberClr();
	
	// �^�X�N�̐���
	if (tskid > 0) {
		ercd = cre_tsk(tskid, &ctsk_key);
		if (ercd == E_OK) {
			s_idTsk = tskid;
		}
	} else {
		ercd = acre_tsk(&ctsk_key);
		if (ercd > 0) {
			s_idTsk = ercd;
		}
	}
	if (ercd < 0) {
		return ercd;
	}
	
	// �^�X�N�̋N��
	ercd = sta_tsk(s_idTsk, 0);
	
	// �L�[�̏�����
	if (ercd == E_OK) {
		ercd = ExKeyInit(TRUE, s_idTsk);
	}
	return ercd;
}

/*==========================================================================*/
/**
 * �O���L�[����^�X�N
 */
/*==========================================================================*/
TASK ExKeyTask(void)
{
	ER		ercd;
	enum KEY_CODE eCode;
	
	// �����J�n
	for(;;) {
		ercd = ExtKeyGet(&eCode, TMO_FEVR);
		if (ercd == E_OK) {
			if ((eCode >= KEY_0 && eCode <= KEY_9) || (eCode == KEY_ASTERISK) || (eCode == KEY_UNDEF)) {
				if (s_iFig < KEY_FIG) {		// �����������ς��łȂ��Ƃ�
					s_cId[ s_iFig ] = eCode;
					s_iFig++;				// �����C���N�������g
				}
				dispSegNumber( s_cId );		// 7SEG�ɕ\��
			} else {
				switch (eCode) {
				case KEY_ENTER:
//					MdSendMsg(s_idTsk, MD_CMD_KEY_ON, 0, s_cId, s_iFig);	// ID�ԍ��̑��M
					idNumberClr();
					dispSegOff();			// �VSEG�\��������
					break;
				case KEY_DEL:				// [���]�L�[
					if (s_iFig) {			// �����f�N�������g
						s_iFig--;
						s_cId[ s_iFig ] = 0;	// 1�����폜
					}
					dispSegNumber( s_cId );	// 7SEG�ɕ\��
					break;
//				case KEY_ASTERISK:			// �������Ȃ�
//
//					break;
//				case KEY_UNDEF:				// �������Ȃ�
//
//					break;
				default:					// �����G���[
				
					break;
				}
			}
		} else {
			// �����ɂ���͎̂����G���[
			PrgErrSet();
			slp_tsk();							// �G���[�̂Ƃ��̓^�X�N�I��
		}
	}
}

/*==========================================================================*/
/**
 * ID�ԍ����N���A
 */
/*==========================================================================*/
static void idNumberClr(void)
{
	memset(s_cId, 0, (KEY_FIG + 1));
	s_iFig = 0;
}

/*==========================================================================*/
/**
 * �\��������
 */
/*==========================================================================*/
static void dispSegOff(void)
{
	const char seg[] = { 0, 0, 0, 0, 0, 0, 0, 0};
	
	dispSegNumber( seg );
}

/*==========================================================================*/
/**
 * 7SEG�\��
 * @param p �\���f�[�^
 */
/*==========================================================================*/
static void dispSegNumber(B *p)
{
	int i, iVal;
	
	for(i = 0;i < KEY_FIG;i++, p++) {
		iVal = 0;
		if (isdigit(*p)) {
			iVal = *p - '0';
		} else if (*p == KEY_ASTERISK) {	// �e�X�g�p��'A'�\��
			iVal = 10;
		} else if (*p == KEY_UNDEF) {		// �e�X�g�p��'B'�\��
			iVal = 11;
		} else if (*p == 0){
			iVal = SEG_PTN_OFF;
		}
		Disp7Seg((LED_7SEG_1 + i), iVal, CTRL7SEG_NORMAL);
	}
}
