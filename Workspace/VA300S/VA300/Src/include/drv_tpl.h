/**
*	VA-300�v���O����
*
*	@file drv_tpl.h
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/09/12
*	@brief  �^�b�`�p�l����`���
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
#ifndef	_DRV_TPL_H_
#define	_DRV_TPL_H_
#include "drv_tsc2004.h"						///< �f�o�C�X��TSC2004���g�p

// �}�N����`(�f�o�C�X�̊֐��ɂ��킹��)
#define	tpl_reset()			Tsc2004Init()		///< ���Z�b�gON���M
#define	tpl_pos_get(x,y)	Tsc2004PosGet(x,y)	///< ���W�擾
#define	tpl_com_int() 		Tsc2004ComInt()		///< �ʐM�����݊֐�

// �^�錾
typedef struct {						///< �^�b�`�p�l���̕␳�l
	float fXx;							///< X���W�p X���W�l�ɂ�����l
	float fXy;							///<         Y���W�n�ɂ�����l
	float fXofs;						///<         �I�t�Z�b�g
	float fYx;							///< Y���W�p X���W�l�ɂ�����l
	float fYy;							///<         Y���W�n�ɂ�����l
	float fYofs;						///<         �I�t�Z�b�g
} ST_TPL_REV;

typedef struct {						///< �|�W�V������`
	int iX;								///< X���W
	int iY;								///< Y���W
} ST_POS;

typedef struct {						///< �␳�p�|�W�V�����i�[�\���̒�`
	ST_POS	tPos[ 3 ];					///< �|�W�V����3�_
} ST_POS_REV;

// EXTERN�錾�̒�`
#ifdef EXTERN
#undef EXTERN
#endif

#if defined(_DRV_TPL_C_)
#define	EXTERN
#else
#define	EXTERN extern
#endif

// �v���g�^�C�v�錾
EXTERN ER TplInit(ID idSem);						///< �^�b�`�p�l��������
EXTERN ER TplPosGet(int *piPosX, int *piPosY, TMO tmout);		///< ���W�擾
EXTERN void TplRevSet(float fXx, float fXy, float fXofs, float fYx, float fYy, float fYofs);				///< �␳�l�̐ݒ�
EXTERN void TplRevGet(float *pfXx, float *pfXy, float *pfXofs, float *pfYx, float *pfYy, float *pfYofs);	///< ���݂̕␳�l�̎擾
EXTERN void TplRevInit(void);						///< �␳�l�̏�����
EXTERN void TplRevCalc(ST_POS_REV *pBase, ST_POS_REV *pInput, ST_TPL_REV *pRev);	///< �␳�l�̌v�Z
#endif										/* end of _DRV_TPL_H_				*/
