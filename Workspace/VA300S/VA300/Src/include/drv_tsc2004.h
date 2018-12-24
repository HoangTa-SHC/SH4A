/**
*	VA-300�v���O����
*
*	@file drv_tsc2004.h
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/09/10
*	@brief  TSC2004��`���
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
#ifndef	_DRV_TSC2004_H_
#define	_DRV_TSC2004_H_
#include "drv_fpga.h"

// �}�N����`
#define	tscSetCtrlByte(n)	fpga_outw(TPL_CBYTE, n)			///< CONTROL BYTE���M
#define	tscSetDataByte(n)	fpga_outw(TPL_DBYTE, n)			///< DATA BYTE���M
#define	tscGetDataByte()	fpga_inw(TPL_DBYTE)				///< DATA BYTE��Ǐo��
#define	tscSendCtrlByte()	fpga_setw(TPL_CRL, TPL_CRL_WRC)	///< CONTROL BYTE���M�݂̂̃V�[�P���X
#define	tscSendDataByte()	fpga_setw(TPL_CRL, TPL_CRL_WRD)	///< DATA BYTE���M�V�[�P���X
#define	tscReadReg()		fpga_setw(TPL_CRL, TPL_CRL_RD)	///< �Ǐo���V�[�P���X

#define	tscSendResetOn()	tsc2004Send(s_cCmdResetOn, sizeof s_cCmdResetOn)	///< ���Z�b�gON���M
#define	tscSendResetOff()	tsc2004Send(s_cCmdResetOff, sizeof s_cCmdResetOff)	///< ���Z�b�gOFF���M
#define	tscSetCFR0()		tsc2004Send(s_cCmdCFR0Set, sizeof s_cCmdCFR0Set)	///< CFR0���W�X�^�ɐݒ著�M
#define	tscSetCFR1()		tsc2004Send(s_cCmdCFR1Set, sizeof s_cCmdCFR1Set)	///< CFR1���W�X�^�ɐݒ著�M
#define	tscSetCFR2()		tsc2004Send(s_cCmdCFR2Set, sizeof s_cCmdCFR2Set)	///< CFR2���W�X�^�ɐݒ著�M

#define	tscSendPosXGet()	tsc2004Send(s_cCmdPosXGet, sizeof s_cCmdPosXGet)	///< X���W�擾�v��
#define	tscSendPosYGet()	tsc2004Send(s_cCmdPosYGet, sizeof s_cCmdPosYGet)	///< Y���W�擾�v��

#define	enable_tpl_int()		fpga_setw(INT_CRL, INT_CRL_TPL);				///< �^�b�`�p�l�������݋���
#define	enable_tplcom_int()		fpga_setw(INT_CRL, INT_CRL_TPL_CMD);			///< �^�b�`�p�l���ʐM�����݋���
#define	disable_tpl_int()		fpga_clrw(INT_CRL, INT_CRL_TPL);				///< �^�b�`�p�l�������݋���
#define	disable_tplcom_int()	fpga_clrw(INT_CRL, INT_CRL_TPL_CMD);			///< �^�b�`�p�l���ʐM�����݋���

// EXTERN�錾�̒�`
#ifdef EXTERN
#undef EXTERN
#endif

#if defined(_DRV_TSC2004_C_)
#define	EXTERN
#else
#define	EXTERN extern
#endif

// �v���g�^�C�v�錾
EXTERN ER Tsc2004Init(void);							///< �f�o�C�X������
EXTERN ER Tsc2004PosGet(int *piPosX, int *piPosY);		///< ���W�擾
EXTERN void Tsc2004ComInt(void);						///< �ʐM����������
#endif										/* end of _DRV_TSC2004_H_				*/
