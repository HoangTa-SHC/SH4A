/**
*	VA-300�v���O����
*
*	@file drv_buz.c
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/07/10
*	@brief  Buzzer���W���[��(���Č����痬�p)
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
#define	_DRV_BUZ_C_
#include <mathf.h>
#include "kernel.h"
#include "sh7750.h"
#include "drv_buz.h"

// ��`
#define	FREQ_MIN	50.0						///< �ŏ����g��
#define	FREQ_MAX	48000.0						///< �ő���g��

// ���[�J���ϐ�
static ID s_idSem;						// �Z�}�t�HID

const T_CSEM csem_buz  = { TA_TFIFO, 1, 1, (B *)"sem_buz" };
	// ���K�Ǝ��g�����X�g(�f���@������p)
const float	buzfrq[] = { 
	261.5,	///< �h
	277.0,	///< �h#
	293.5,	///< ��
	311.0,	///< ��#
	329.5,	///< �~
	349.0,	///< �t�@
	369.5,	///< �t�@#
	391.5,	///< �\
	415.0,	///< �\#
	440.0,	///< ��
	466.0,	///< ��#
	494.0,	///< �V
	523.0,
	554.0,
	587.0,
	662.0
};

// �v���g�^�C�v�錾
static void buzTplSet(int iScale, enum BUZ_CTRL eCtrl);	///< �u�U�[�ݒ�
static float freqCalc(int iScale);						///< ���g���̌v�Z

/*==========================================================================*/
/**
 * �u�U�[������
 * @param id �u�U�[�p�Z�}�t�HID
 */
/*==========================================================================*/
void BuzInit(ID id)
{
	ER ercd; 

	s_idSem = 0;
	
	// �Z�}�t�H�̐���
	if (id > 0) {
		ercd = cre_sem(id, &csem_buz);	
		if (ercd == E_OK) {
			s_idSem = id;
		}
	} else {
		ercd = acre_sem(&csem_buz);
		if (ercd > 0) {
			s_idSem = ercd;
		}
	}
	
	BuzTplSet(0, BUZ_OFF);				// �^�b�`�p�l���p�u�U�[OFF
	BuzEmgOff();						// �x��p�u�U�[OFF
}

/*==========================================================================*/
/**
 * �^�b�`�p�l���p�u�U�[�ݒ�(GDIC)
 *
 * @param  iScale ���K
 * @param  eCtrl BUZ_ON �u�U�[ON/BUZ_OFF �u�U�[OFF
 * @retval E_OK �ݒ�OK
 */
/*==========================================================================*/
ER BuzTplSet(int iScale, enum BUZ_CTRL eCtrl)
{
	ER ercd;
	
	ercd = twai_sem(s_idSem, (100/MSEC));
	if (ercd != E_OK) {
		return ercd;
	}
	buzTplSet(iScale, eCtrl);
		
	ercd = sig_sem(s_idSem);
	
	return ercd;
}

/*==========================================================================*/
/**
 * �u�U�[�ݒ�(PDIC)
 *
 * @param  iScale ���K
 * @param  eCtrl BUZ_ON �u�U�[ON/BUZ_OFF �u�U�[OFF
 */
/*==========================================================================*/
static void buzTplSet(int iScale, enum BUZ_CTRL eCtrl)
{
	float fFreq;
	
	fFreq = 0.0;
	
	if (eCtrl == BUZ_ON) {
		// �ݒ���g���Z�o
		fFreq = freqCalc(iScale);
		BuzCycSet((UH)fFreq);			// ���g���ݒ�
		BuzTplOn();						// �u�U�[ON
	
	} else {
		// ��~�ݒ�
		BuzTplOff();					// �u�U�[OFF
	}
}

/**
 * ���g���̌v�Z(�f���@�̃\�[�X�R�[�h�𗬗p)
 */
static float freqCalc(int iScale)
{
	float fFreq;
	
	fFreq = ldexpf(buzfrq[ iScale & 0xF ] , ((iScale >> 4) & 0xF) - 7); // The new period or frequency
		
	// ���g���͈͂����͈̔͂ŃJ�b�g
	if (fFreq < FREQ_MIN) {
		fFreq = FREQ_MIN;
	}
	if (fFreq > FREQ_MAX) {
		fFreq = FREQ_MAX;
	}

	return fFreq;
}
