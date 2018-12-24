/**
*	VA-300 �u�U�[����v���O����
*
*	@file drv_buz.h
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/07/10
*	@brief  �u�U�[��`���(�V�K�쐬)
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
#ifndef	_DRV_BUZ_H_
#define	_DRV_BUZ_H_
#include "drv_fpga.h"

// ��`
enum BUZ_CTRL {
	BUZ_OFF,							///< �u�U�[OFF
	BUZ_ON,								///< �u�U�[ON
};
// �}�N����`
#define	BuzTplOn()		fpga_setw(BUZ_CRL, BUZ_CRL_BZ1)	///< �^�b�`�p�l���u�U�[ON
#define	BuzTplOff()		fpga_clrw(BUZ_CRL, BUZ_CRL_BZ1)	///< �^�b�`�p�l���u�U�[OFF
#define	BuzCycSet(n)	fpga_outw(BUZ1_CYC, n)			///< �u�U�[���g���ݒ�(SIL)
#define	BuzEmgOn()		fpga_setw(BUZ_CRL, BUZ_CRL_BZ2)	///< �x��u�U�[ON
#define	BuzEmgOff()		fpga_clrw(BUZ_CRL, BUZ_CRL_BZ2)	///< �x��u�U�[OFF

// EXTERN�錾�̒�`
#if defined(EXTERN)
#undef EXTERN
#endif

#if defined(_DRV_BUZ_C_)
#define	EXTERN
#else
#define	EXTERN	extern
#endif

// �v���g�^�C�v�錾
EXTERN void BuzInit(ID id);								///< �u�U�[������
EXTERN ER BuzTplSet(int iScale, enum BUZ_CTRL eCtrl);	///< �u�U�[ON
#endif										/* end of _DRV_LED_H_				*/
