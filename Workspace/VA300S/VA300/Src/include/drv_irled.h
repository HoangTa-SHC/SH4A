/**
*	VA-300�v���O����
*
*	@file drv_irled.h
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/09/25
*	@brief  �ԊO��LED��`���
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
#ifndef	_DRV_IRLED_H_
#define	_DRV_IRLED_H_
#include "drv_fpga.h"						///< FPGA�Ő���

// �}�N����`(�f�o�C�X�̊֐��ɂ��킹��)
#define	IrLedCrlOn(n)	fpga_setw(IRLED_CRL, n)	///< �ԊO��LED��ON����
#define	IrLedCrlOff(n)	fpga_clrw(IRLED_CRL, n)	///< �ԊO��LED��OFF����

#define	IR_LED_1	IRLED_CRL_LED1			///< �ԊO��LED1
#define	IR_LED_2	IRLED_CRL_LED2			///< �ԊO��LED2
#define	IR_LED_3	IRLED_CRL_LED3			///< �ԊO��LED3
#define	IR_LED_4	IRLED_CRL_LED4			///< �ԊO��LED4
#define	IR_LED_5	IRLED_CRL_LED5			///< �ԊO��LED5

// �^�錾


// EXTERN�錾�̒�`
#ifdef EXTERN
#undef EXTERN
#endif

#if defined(_DRV_IRLED_C_)
#define	EXTERN
#else
#define	EXTERN extern
#endif

// �v���g�^�C�v�錾
EXTERN ER IrLedInit(ID idSem);			///< �ԊO��LED������
EXTERN ER IrLedSet(UB ubLed, UB ubVal);	///< �ԊO��LED Duty�ݒ�
#endif									/* end of _DRV_IRLED_H_				*/
