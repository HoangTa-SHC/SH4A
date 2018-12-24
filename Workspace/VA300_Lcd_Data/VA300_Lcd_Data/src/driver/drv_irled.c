/**
*	VA-300 �ԊO��LED�v���O����
*
*	@file drv_irled.c
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/09/25
*	@brief  �ԊO��LED���W���[��(�V�K�쐬)
*			���Ԃ�s�v�Ǝv���̂ŃZ�}�t�H�͎g�p���Ȃ��悤�ɂ����B
*			�������A�K�v�ɂȂ����������悤�ȍ\���ɂ��Ă���B
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
#define	_DRV_IRLED_C_
#include "kernel.h"
#include "drv_fpga.h"
#include "drv_irled.h"

// �}�N����`
#define	irled_set(n,v)	FpgaIrLedSet(n,v)	///< �ԊO��LED�ݒ�(SIL)

// �v���g�^�C�v�錾
static void irLedSet(UB ubLed, UB ubVal);	///< �ԊO��LED��Duty���ݒ肷��

/*==========================================================================*/
/**
 * �ԊO��LED�֘A������
 *
 *	@param idSem LED�p�Z�}�t�HID
 *	@return �G���[�R�[�h
 */
/*==========================================================================*/
ER IrLedInit(ID idSem)
{
	return E_OK;
}

/*==========================================================================*/
/**
 * �ԊO��LED��Duty��ݒ肷��(GDIC)
 *
 * @param  ubLed �ԊO��LED���w��(IR_LED_1(01)/IR_LED_2(02)/IR_LED_3(04)...)
 * @param  ubVal Duty(0�`255)
 * @retval E_OK �ݒ�OK
 */
/*==========================================================================*/
ER IrLedSet(UB ubLed, UB ubVal)
{
	ER	ercd;
	
	ercd = E_OK;
	
	irLedSet(ubLed, ubVal);

	return ercd;
}

/*==========================================================================*/
/**
 * �ԊO��LED��Duty��ݒ肷��(PDIC)
 *
 * @param  ubLed �ԊO��LED���w��(IR_LED_1(01)/IR_LED_2(02)/IR_LED_3(04)...)
 * @param  ubVal Duty(0�`255)
 * @retval E_OK �ݒ�OK
 */
/*==========================================================================*/
static void irLedSet(UB ubLed, UB ubVal)
{
	irled_set(ubLed, ubVal);				// �ԊO��LED�ɒl��ݒ�
}
