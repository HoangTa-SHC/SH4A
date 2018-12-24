/**
*	VA-300�v���O����
*
*	@file drv_tim.h
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/07/31
*	@brief  �^�C�}��`���
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
#ifndef	_DRV_TIM_H_
#define	_DRV_TIM_H_

// �^�C�}�[��`
enum TIM_NAME {								///< �^�C�}��`
	TIM_FLASH,								///< �t���b�V���������h���C�o�p�^�C�}
	
	
	TIM_MAX									///< �^�C�}�ő吔
};

// EXTERN�錾�̒�`
#ifdef EXTERN
#undef EXTERN
#endif

#if defined(_DRV_TIM_C_)
#define	EXTERN
#else
#define	EXTERN extern
#endif

// �v���g�^�C�v�錾
EXTERN ER TmInit(ID idCyc);									///< �^�C�}�[������
EXTERN void TmSet(enum TIM_NAME eName, const long lTime);	///< �^�C�}�[�ݒ�
EXTERN long TmPol(enum TIM_NAME eName);						///< �^�C�}�[�|�[�����O
#endif										/* end of _DRV_TIM_H_				*/
