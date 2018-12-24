/**
*	VA-300�v���O����
*
*	@file drv_led.h
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/07/10
*	@brief  LED��`���(���Č����痬�p)
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
#ifndef	_DRV_LED_H_
#define	_DRV_LED_H_

//#define Test_Old_Version	// Added 2013.6.18 For Sanwa NewTec Products

// ��`
#if defined( Test_Old_Version )
#define	LED_OK			(0x01)				///< �ΐFLED
#define	LED_POW			(0x02)				///< �I�����W�FLED
#define	LED_ERR			(0x04)				///< �ԐFLED
#else
#define	LED_ERR			(0x01)				///< �ԐFLED
#define	LED_OK			(0x02)				///< �ΐFLED
#define	LED_POW			(0x04)				///< �I�����W�FLED
#endif


#define	LED_DBG1		(0x08)				///< �f�o�b�O�pLED1
#define	LED_DBG2		(0x10)				///< �f�o�b�O�pLED2
#define	LED_DBG3		(0x20)				///< �f�o�b�O�pLED3
#define	LED_DBG4		(0x40)				///< �f�o�b�O�pLED4
#define	LED_ALL			(0x7F)				///< �SLED

// LED��ON/OFF��`
#define	LED_OFF			0					///< LED OFF
#define	LED_ON			1					///< LED ON
#define	LED_REVERSE		2					///< LED ���]

// �v���g�^�C�v�錾
#if defined(_DRV_LED_C_)
ER LedInit(ID idSem);						// LED������
ER LedOut(int, int);						// LED�o��
int LedStatusGet(int);						// LED��Ԏ擾
#else
extern ER LedInit(ID idSem);
extern ER LedOut(int, int);
extern int LedStatusGet(int);
#endif
#endif										/* end of _DRV_LED_H_				*/
