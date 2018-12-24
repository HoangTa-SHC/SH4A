/**
*	VA-300�v���O����
*
*	@file drv_led.c
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/09/07
*	@brief  LED���W���[��(���Č����痬�p)
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
#define	_DRV_LED_C_
#include "kernel.h"
#include "drv_fpga.h"
#include "drv_led.h"
#include "sh7750.h"

// �}�N����`
#define	led_out(n)	fpga_outw(LED_CRL, n)	///< LED�o��(SIL)
#define	led_out2(n)	sfr_outw(BSC_PDTRA, ((sfr_inw(BSC_PDTRA) & 0xF0FF) | (n << 5)))	///< LED�o��(SIL)

// ���[�J���ϐ�
static UH s_uhLedStatus;				// ��ԕێ��p�ϐ�
static const UH s_uhLedList[] = {
	LED_POW,							///< �ʓdLED(��)
	LED_OK,								///< �F��OK LED(�I�����W)
	LED_ERR,							///< �G���[LED(��)
	
	LED_DBG1,							///< �f�o�b�O�pLED
	LED_DBG2,							///< �f�o�b�O�pLED
	LED_DBG3,							///< �f�o�b�O�pLED
	LED_DBG4,							///< �f�o�b�O�pLED
};
static const UH s_nLedList = sizeof s_uhLedList / sizeof s_uhLedList[ 0 ];
static ID s_idSem;						///< �Z�}�t�HID

const T_CSEM csem_led = { TA_TFIFO, 1, 1, (B *)"sem_led" };

// �v���g�^�C�v�錾
static void ledOut(int SelectLed, int OnOff);	///< LED�o�͐���(�{��)

/*==========================================================================*/
/**
 * LED������
 *
 *	@param idSem LED�p�Z�}�t�HID
 *	@return �G���[�R�[�h
 */
/*==========================================================================*/
ER LedInit(ID idSem)
{
	ER ercd;

	sfr_setl(BSC_PCTRA, 0x00550000);	// PA8-11�ݒ�

	s_uhLedStatus = 0;
	s_idSem = 0;
	ercd = cre_sem(idSem, &csem_led);	// Create semaphore
	
	if (ercd == E_OK) {
		s_idSem = idSem;
		ercd = LedOut(LED_ALL, LED_OFF);// LED OFF
	}
	
	return ercd;
}

/*==========================================================================*/
/**
 * LED��ON/OFF(GDIC)
 *
 * @param  SelectLed LED_POW(01)/LED_OK(02)/LED_ERR(04)���w��
 * @param  OnOff LED_OFF(0)/LED_ON(1)/LED_REVERSE(2)���w��
 * @retval E_OK �ݒ�OK
 */
/*==========================================================================*/
ER LedOut(int SelectLed, int OnOff)
{
	ER	ercd;
	
	ercd = twai_sem(s_idSem, (10/MSEC));
	if (ercd == E_OK) {
		ledOut(SelectLed, OnOff);
	} else {								// �Z�}�t�H�l�����s�̂Ƃ�
		return ercd;
	}

	if (sig_sem(s_idSem) != E_OK) {			// �����ŃG���[���ł�̂͐݌v�~�X
		return E_SYS;						// �G���[
	}
	
	return ercd;
}

/*==========================================================================*/
/**
 * LED��ON/OFF(PDIC)
 *
 * @param  SelectLed LED_POW(01)/LED_OK(02)/LED_ERR(04)���w��
 * @param  OnOff LED_OFF(0)/LED_ON(1)/LED_REVERSE(2)���w��
 * @retval E_OK �ݒ�OK
 */
/*==========================================================================*/
static void ledOut(int SelectLed, int OnOff)
{
	int i;
	UH	uhLed;
	
	for (i = 0;i < s_nLedList;i++) {
		uhLed = s_uhLedList[ i ];
		if (SelectLed & uhLed) {
			if (OnOff == LED_ON) {
				s_uhLedStatus |= uhLed;	// High�œ_��
			} else if (OnOff == LED_REVERSE){
				s_uhLedStatus ^= uhLed;
			} else {
				s_uhLedStatus &= ~uhLed;// Low�ŏ���
			}
		}
	}
	s_uhLedStatus &= LED_ALL;
	led_out((s_uhLedStatus & (LED_POW | LED_OK | LED_ERR)));	// LED�o��
	// LED�o��
	led_out2((s_uhLedStatus & (LED_DBG1 | LED_DBG2 | LED_DBG3 | LED_DBG4)));
}

/*==========================================================================*/
/**
 * LED�̏�Ԏ擾
 *
 * @param  SelectLed LED_POW(01)/LED_OK(02)/LED_ERR(04)���w��
 * @retval LED_ON(1)
 * @retval LED_OFF(0)
 */
/*==========================================================================*/
int LedStatusGet(int SelectLed)
{
	int	iStatus;
	int	i;
	UH	uhLed;

	iStatus = -1;						// �p�����[�^�G���[�ɂ��Ă���
	for (i = 0;i < s_nLedList;i++) {
		uhLed = s_uhLedList[ i ];
		if (SelectLed == uhLed) {
			iStatus = LED_OFF;
			if (s_uhLedStatus & uhLed) {
				iStatus = LED_ON;
			}
			break;
		}
	}
	
	return iStatus;
}
