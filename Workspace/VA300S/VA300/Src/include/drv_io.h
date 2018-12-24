/**
*	VA-300�v���O����
*
*	@file drv_io.h
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2013/03/08
*	@brief  I/O��`���
*
*	Copyright (C) 2013, OYO Electric Corporation
*/
#ifndef	_DRV_IO_H_
#define	_DRV_IO_H_
#include "drv_fpga.h"

// ��`
#define	AuxOut1On()				fpga_setw(AUX_OUT, AUX_OUT_1)	///< AUX OUT1 �o��ON
#define	AuxOut1Off()			fpga_clrw(AUX_OUT, AUX_OUT_1)	///< AUX OUT1 �o��OFF
#define	AuxOut2On()				fpga_setw(AUX_OUT, AUX_OUT_2)	///< AUX OUT2 �o��ON
#define	AuxOut2Off()			fpga_clrw(AUX_OUT, AUX_OUT_2)	///< AUX OUT2 �o��OFF

#define	IsAuxInA0()				((fpga_inw(AUX_IN) & AUX_IN_A0) == AUX_IN_A0)	///< AUX IN A0����
#define	IsAuxInA1()				((fpga_inw(AUX_IN) & AUX_IN_A1) == AUX_IN_A1)	///< AUX IN A1����
#define	IsAuxInB0()				((fpga_inw(AUX_IN) & AUX_IN_B0) == AUX_IN_B0)	///< AUX IN B0����
#define	IsAuxInB1()				((fpga_inw(AUX_IN) & AUX_IN_B1) == AUX_IN_B1)	///< AUX IN B1����

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
EXTERN ER IoInit(ID idTsk);					// I/O�����ݏ�����

#endif										/* end of _DRV_IO_H_				*/
