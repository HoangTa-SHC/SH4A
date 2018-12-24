/**
*	VA-300�v���O����
*
*	@file drv_7seg.h
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/08/31
*	@brief  7Seg��`���(���Č����痬�p)
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
#ifndef	_DRV_7SEG_H_
#define	_DRV_7SEG_H_
#include "drv_fpga.h"

enum LED_7SEG_SEL {						///< 7SEG�̒�`
	LED_7SEG_ALL,						///< ALL
	LED_7SEG_1,							///< 7Seg LED(�ŏ�ʌ�?)
	LED_7SEG_2,							///< 7Seg LED
	LED_7SEG_3,							///< 7Seg LED
	LED_7SEG_4,							///< 7Seg LED
	LED_7SEG_5,							///< 7Seg LED
	LED_7SEG_6,							///< 7Seg LED
	LED_7SEG_7,							///< 7Seg LED
	LED_7SEG_8,							///< 7Seg LED(�ŉ��ʌ�?)

};

// �ŉ��ʁA�ŏ�ʂɂ��Ă�
// FPGA�̎d�l���ɂ���ĕύX�ɂȂ�\������
// �������A���̒�`���͔͈̂͂Ȃ̂Ŗ��Ȃ�
#define	LED_7SEG_MIN	LED_7SEG_1		///< 7SEG��ʌ�
#define	LED_7SEG_MAX	LED_7SEG_8		///< 7SEG���ʌ�

enum CTRL7SEG {							//=== 7SEG ���� ===
	CTRL7SEG_NORMAL,					///< �ʏ�\��
};

enum DISP_SELECT {						//=== �\���I�� ===
	DISP_LED,							///< LED�I��
	DISP_7SEG							///< 7SEG�I��
};

typedef struct {
	ID		idOrg;						///< �^�X�NID
	enum DISP_SELECT eDispSelect;		///< �\���I��
	union {
		struct {
			int iSelect;				///< LED�̑I��
			int iCtrl;					///< ����
			int dummy;					///< �\��
		} Led;
		struct {
			enum LED_7SEG_SEL eSelect;	///< 7SEG�̑I��
			int iValue;					///< �ݒ�l
			enum CTRL7SEG eCtrl;		///< �VSEG����
		} Seg;
	}Dev;
}ST_DISPMSG;

// �\����`
#if defined(_DRV_7SEG_C_)
static const UH s_SegPtn[] = {
	0x3f, 0x06, 0x5b, 0x4f, 0x66, 	//0-4
	0x6d, 0x7d, 0x07, 0x7f, 0x6F,	//5-9
	0x77, 0x7c, 0x58, 0x5e, 0x79,   //A,b,c,d,E
	0x71, 0x50, 0x80, 0x00			//F,r,-,off	
};
static const s_nSegPtn = sizeof s_SegPtn / sizeof s_SegPtn[ 0 ];
#endif
#define	SEG_PTN_r		0x10		///< r
#define	SEG_PTN_MINUS	0x11		///< -
#define	SEG_PTN_OFF		0x12		///< ��\��
#define	SEG_PTN_DOT		0x80		///< .(�l��OR���Ďg�p)

// �v���g�^�C�v�錾
#if defined(_DRV_7SEG_C_)
ER Led7SegInit(ID);							// 7SegLED������
ER Led7SegOut(enum LED_7SEG_SEL, int);		// 7SegLED�o��
#else
extern ER Led7SegInit(ID);
extern ER Led7SegOut(enum LED_7SEG_SEL, int);
#endif
#endif										/* end of _DRV_7SEG_H_				*/
