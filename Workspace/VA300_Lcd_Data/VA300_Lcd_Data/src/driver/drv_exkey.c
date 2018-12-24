/**
*	VA-300�v���O����
*
*	@file drv_exkey.c
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/08/30
*	@brief  �O���L�[����h���C�o
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
#define	_DRV_EXKEY_C_
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "kernel.h"
#include "sh7750.h"
#include "id.h"
#include "drv_fpga.h"
#include "drv_exkey.h"
#include "va300.h"


#define	EXKEY_IP	9					///< �O���L�[�̊����݃��x��
#define	EXKEY_INT	INT_IRL9			///< �O���L�[�̊����ݔԍ�

#define	enable_key_int()	fpga_setw(INT_CRL, INT_CRL_KEY);	///< �����݋��ݒ�
#define	clear_key_int()		fpga_clrw(INT_CRL, INT_CRL_KEY);	///< �����݃t���O�N���A
#define	ex_key_in()			fpga_inw(KEY_IN)					///< �L�[���͒�`

// �ϐ���`
static ID s_idTsk;						///< �^�X�NID

// �v���g�^�C�v�錾
static INTHDR exKeyInt(void);			///< 10�L�[���͊�����
static void exkey_int(void);			///< 10�L�[�����ݖ{��
static enum KEY_CODE key2Code(UH uhKey);///< �L�[�R�[�h
// 
const T_DINH dinh_key = { TA_HLNG, exKeyInt, EXKEY_IP};

/*==========================================================================*/
/**
 * �O���L�[������
 * 
 * @param bIntEnable �����݋���/�s����
 * @param idTsk �����ݎ��ɋN������^�X�NID
 */
/*==========================================================================*/
ER ExKeyInit(BOOL bIntEnable, ID idTsk)
{
	ER ercd;
	UW psw;
	
	ercd = E_OK;
	
	// �����ݎ��̋N���^�X�N��ݒ�
	if (idTsk > 0) {
		s_idTsk = idTsk;
	} else {
		return E_PAR;					// �p�����[�^�G���[
	}
	
	if (bIntEnable == TRUE) {
		psw = vdis_psw();
	
		// �x�N�^�o�^
		ercd = def_inh(EXKEY_INT, &dinh_key);		// �����ݐݒ�
		if (ercd == E_OK) {
			// �����ݐݒ�(�n�[�h�E�F�A��)
			enable_key_int();
		}
		vset_psw(psw);
	}
	
	return ercd;
}

#pragma interrupt(exKeyInt)
/*==========================================================================*/
/**
 * �����݃n���h��
 *
 */
/*==========================================================================*/
static INTHDR exKeyInt(void)
{
	ent_int();
	exkey_int();
	ret_int();
}

/*==========================================================================*/
/**
 * �����݃n���h��(�{��)
 *
 */
/*==========================================================================*/
static void exkey_int(void)
{
	clear_key_int();							/* �����ݗv�����N���A&�s���� */
	
	if (s_idTsk) {
		iwup_tsk(s_idTsk);
	}
	enable_key_int();							/* �����݋��� */
}

/*==========================================================================*/
/**
 * �L�[���͎擾�҂�
 *
 *	@param peKey �L�[�R�[�h�i�[��A�h���X
 *	@param tmout �^�C���A�E�g����
 *	@return OS�̃G���[�R�[�h
 */
/*==========================================================================*/
ER ExtKeyGet(enum KEY_CODE *peKey, TMO tmout)
{
	ER ercd;

	ercd = E_OK;
	
	// �L�[���Q��擾���A�ƍ�����
	if (tmout != TMO_POL) {
		
		ercd = tslp_tsk( tmout );
	}
	if (ercd == E_OK) {
		*peKey = ExtKeyPol();
	}
	return ercd;
}

/*==========================================================================*/
/**
 * �L�[���͂��擾
 *
 *	@retval KEY_ENTER �ďo
 *	@retval KEY_DEL ���
 *	@retval KEY_ASTERISK *
 *	@retval KEY_UNDEF ����`
 *	@retval 0�`9 �l
 */
/*==========================================================================*/
enum KEY_CODE ExtKeyPol(void)
{
	enum KEY_CODE eKey;
	UH uhKey;

	uhKey = ex_key_in();				// �L�[�̎�荞��
	eKey = key2Code(uhKey);

	return eKey;
}

/*==========================================================================*/
/**
 * �L�[���͂��R�[�h�ϊ�
 *
 *	@param uhKey �����ꂽ�L�[
 *	@retval KEY_ENTER �ďo
 *	@retval KEY_DEL ���
 *	@retval KEY_ASTERISK *
 *	@retval KEY_UNDEF ����`
 *	@retval 0�`9 �l
 */
/*==========================================================================*/
static enum KEY_CODE key2Code(UH uhKey)
{
	static const struct {
		UH uhInkey;						// ���̓L�[
		enum KEY_CODE eCode;			// �L�[�R�[�h
	} stCodeList[] = {
		{ KEY_IN_0, KEY_0},
		{ KEY_IN_1, KEY_1},
		{ KEY_IN_2, KEY_2},
		{ KEY_IN_3, KEY_3},
		{ KEY_IN_4, KEY_4},
		{ KEY_IN_5, KEY_5},
		{ KEY_IN_6, KEY_6},
		{ KEY_IN_7, KEY_7},
		{ KEY_IN_8, KEY_8},
		{ KEY_IN_9, KEY_9},
		{ KEY_IN_ENTER, KEY_ENTER},
		{ KEY_IN_ASTERISK, KEY_ASTERISK},
		{ KEY_IN_CLR,  KEY_DEL},
		{ KEY_IN_NONE, KEY_NONE},
		
	};
	static const int iCodeListCount = sizeof stCodeList / sizeof stCodeList[ 0 ];
	int i;
	
	// �L�[�R�[�h����
	for (i = 0;i < iCodeListCount;i++) {
		if (uhKey == stCodeList[ i ].uhInkey) {
			return stCodeList[ i ].eCode;
		}
	}
	return KEY_UNDEF;
}

