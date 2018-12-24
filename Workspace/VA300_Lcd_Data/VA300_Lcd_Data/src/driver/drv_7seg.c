/**
*	VA-300�v���O����
*
*	@file drv_7seg.c
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/09/07
*	@brief  7SegLED���W���[��(���Č����痬�p)
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
#define	_DRV_7SEG_C_
#include <string.h>
#include "kernel.h"
#include "drv_7seg.h"

// �}�N����`
#define	seg_out(n,v)	fpga_outw((SEG_CRL + (n << 1)), v)	///< 7SEG�֏o��(SIL)
#define	seg_in(n)		fpga_inw((SEG_CRL + (n << 1)))		///< 7SEG�̏�Ԃ�Ǐo��(SIL)

// ���[�J���ϐ�
static unsigned char SegStatus[ LED_7SEG_MAX ];				///< ��ԕێ��p�ϐ�
static ID s_idSem;											///< �Z�}�t�HID

const T_CSEM csem_seg = { TA_TFIFO, 1, 1, (B *)"sem_7seg" };

// �v���g�^�C�v�錾
static BOOL SegOut(enum LED_7SEG_SEL SelectSeg, int Value);	///< 7SegLED�ւ̎��ۂ̏o�͊֐�

/*==========================================================================*/
/**
 * 7SegLED������
 *
 *	@param idSem LED�p�Z�}�t�HID
 *	@return �G���[�R�[�h
 */
/*==========================================================================*/
ER Led7SegInit(ID idSem)
{
	ER ercd;

	memset(SegStatus, 0, sizeof(SegStatus));
	s_idSem = 0;

	ercd = cre_sem(idSem, &csem_seg);		// Create semaphore
	if (ercd == E_OK) {
		s_idSem = idSem;
		ercd = Led7SegOut(LED_7SEG_ALL, 0);	// 7Seg������
	}
	
	return ercd;
}

/*==========================================================================*/
/**
 * 7SegLED��ON/OFF
 *
 * @param  eSelectSeg LED_7SEG_1(1)�`LED_7SEG_10(10)���w��
 * @param  Value 0�`H'0F���w��
 * @retval E_OK �ݒ�OK
 */
/*==========================================================================*/
ER Led7SegOut(enum LED_7SEG_SEL eSelectSeg, int Value)
{
	ER	ercd;
	
	ercd = twai_sem(s_idSem, (10/MSEC));
	if (ercd == E_OK) {
		if ( SegOut(eSelectSeg, Value) == FALSE) {
			ercd = E_PAR;				// �p�����[�^�G���[
		}
	} else {							// �Z�}�t�H�l�����s�̂Ƃ�
		return ercd;
	}
	if (sig_sem(s_idSem) != E_OK) {	// �����ŃG���[���ł�̂͐݌v�~�X
		return E_SYS;					// �G���[
	}
	return ercd;
}

/*==========================================================================*/
/* 7SegLED�̏o��															*/
/*==========================================================================*/
static BOOL SegOut(enum LED_7SEG_SEL eSelectSeg, int Value)
{
	int i, iSelSeg;
	unsigned short usOut;
	unsigned short usReg;

	if (eSelectSeg == LED_7SEG_ALL) {
		for( i = LED_7SEG_MIN;i <= LED_7SEG_MAX;i++) {
			SegOut( i, Value);
		}
	} else if (eSelectSeg >= LED_7SEG_MIN && eSelectSeg <= LED_7SEG_MAX) {
		if ((Value & ~SEG_PTN_DOT) >= s_nSegPtn) {
			return FALSE;
		}
		iSelSeg = (int)(eSelectSeg - LED_7SEG_1);
		SegStatus[ iSelSeg ] = Value;						// �ۑ��ϐ��ɒl���i�[
		if ((SegStatus[ iSelSeg ] & ~SEG_PTN_DOT) < s_nSegPtn) {
			usOut = s_SegPtn[ (SegStatus[ iSelSeg ]) ];		// �o�̓p�^�[��������ΐݒ�
			if (SegStatus[ iSelSeg ] & SEG_PTN_DOT) {		// '.'�\������ΐݒ�
				usOut |= SEG_PTN_DOT;
			}
			// 7SEG�o��
			seg_out(iSelSeg, usOut);
		}
	} else {
		return FALSE;
	}
	return TRUE;
}