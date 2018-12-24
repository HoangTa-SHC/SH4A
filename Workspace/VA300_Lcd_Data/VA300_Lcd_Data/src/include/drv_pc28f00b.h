/**
*	VA-300�v���O����
*
*	@file drv_PC28F00B.h
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/09/13
*	@brief  Micron��PC28F00BM29EWHA��`���
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
#ifndef	_DRV_PC28F00B_H_
#define	_DRV_PC28F00B_H_

// ��`
#define	ERS_TMOUT	(5000 * 1000)
#define	PRG_TMOUT	(1000 * 1000)
#ifndef FL_ADDR
	#define	FL_ADDR		0xA2000000L				// �t���b�V�������J�n�A�h���X
#endif
#define	FL_AREA_SZ	0x02000000					// �o���N�ؑւŃA�N�Z�X�ł���̈�T�C�Y

// EXTERN�錾�̒�`
#ifdef EXTERN
#undef EXTERN
#endif

#if defined(_DRV_PC28F00B_C_)
#define	EXTERN
#else
#define	EXTERN extern
#endif

// �v���g�^�C�v�錾
EXTERN ER FdErase(UW uwAddr);						///< �t���b�V������������
EXTERN UW FdWrite(UW uwFp, UH *puhBp, UW n);		///< �t���b�V��������������
EXTERN ER FdRead(UW uwFp, UH *puhBp, UW n);			///< �t���b�V���������Ǎ���
EXTERN UW FdFlAllSize(void);						///< �t���b�V���������T�C�Y��Ԃ�
#endif										/* end of _DRV_FLASH_H_				*/
