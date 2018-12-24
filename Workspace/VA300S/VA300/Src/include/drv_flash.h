/**
*	VA-300�v���O����
*
*	@file drv_flash.h
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/08/01
*	@brief  �t���b�V����������`���
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
#ifndef	_DRV_FLASH_H_
#define	_DRV_FLASH_H_
#include "drv_fpga.h"

// ��`
#ifndef FL_ADDR
	#define	FL_ADDR		0xA2000000L				// �t���b�V�������J�n�A�h���X
#endif
#ifndef FL_AREA_SZ
	#define	FL_AREA_SZ	0x02000000				// �o���N�ؑւŃA�N�Z�X�ł���̈�T�C�Y
#endif

#define	FlBankCalc(n)	(n / FL_AREA_SZ)			///< �t���b�V���������̃A�N�Z�X�E�B���h�E�ԍ������߂�
#define	FlBankSet(n)	fpga_outw(FROM_BANK, n)		///< �t���b�V���������̃A�N�Z�X�E�B���h�E�ؑ�
#define	FlBankGet()		fpga_inw(FROM_BANK)			///< �t���b�V���������̃A�N�Z�X�E�B���h�E�ԍ��Ǎ���
#define	IsFlBank(n)		(fpga_inw(FROM_BANK) == n)	///< ���݂̃A�N�Z�X�E�B���h�E�ԍ��Ɣ�r

// EXTERN�錾�̒�`
#ifdef EXTERN
#undef EXTERN
#endif

#if defined(_DRV_FLASH_C_)
#define	EXTERN
#else
#define	EXTERN extern
#endif

// �v���g�^�C�v�錾
EXTERN void FlInit(ID id);							///< �t���b�V���������h���C�o������
EXTERN ER FlErase(UW uwAddr);						///< �t���b�V������������
EXTERN UW FlWrite(UW uwFp, UH *puhBp, UW n);		///< �t���b�V��������������
EXTERN ER FlRead(UW uwFp, UH *puhBp, UW n);			///< �t���b�V���������Ǎ���

#endif										/* end of _DRV_FLASH_H_				*/
