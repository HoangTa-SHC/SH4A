/**
*	VA-300�v���O����
*
*	@file tsk_io.c
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/8/2
*	@brief  I/O���m�^�X�N
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
#include <stdio.h>

#include "kernel.h"
#include "sh7750.h"
#include "id.h"
#include "drv_fpga.h"
#include "err_ctrl.h"

#include "va300.h"

// 

// �v���g�^�C�v�錾
static BOOL ioVecInit(void);				// IO�x�N�^������
static void dma_stop_int(void);				// DMA�]���I��������

// �ϐ���`
const T_DINH dinh_stop_dma  = { TA_HLNG, DmaStopInt,   5};

/*==========================================================================*/
/*	I/O�x�N�^������															*/
/*==========================================================================*/
static BOOL ioVecInit(void)
{
	ER ercd;

	return (ercd == E_OK);
}

/*==========================================================================*/
/**
 * I/O���m�^�X�N
 */
/*==========================================================================*/
TASK IoTask(void)
{
	ER		ercd;
	FLGPTN	flgptn;
	static const FLGPTN	waiptn = FPTN_INIT;
	UB		ubErrCode;
	
	// ������

	
	ioVecInit();
	
	// �����J�n
	for(;;) {
		ercd = wai_flg(ID_FLG_IO, waiptn, TWF_ORW, &flgptn);
		if (ercd == E_OK) {
			if (flgptn & FPTN_INIT) {
				
				// �t���O�̃N���A
				clr_flg(ID_FLG_IO, ~waiptn);
			}
			if (flgptn & FPTN_CAP_TR_END) {
				
				// �t���O�̃N���A
				clr_flg(ID_FLG_IO, ~FPTN_CAP_TR_END);
			}
		} else {
			// �����ɂ���͎̂����G���[
			PrgErrSet();
			slp_tsk();							// �G���[�̂Ƃ��̓^�X�N�I��
		}
	}
}

#pragma interrupt(DmaStopInt)
//=============================================================================
/**
 *	DMA�]������������
 */
//=============================================================================
INTHDR DmaStopInt(void)
{
	ent_int();							// �����݃n���h���̊J�n
	dma_stop_int();						// �����݃n���h���{��
	ret_int();							// �����݃n���h�����畜�A����
}

//=============================================================================
/**
 *	DMA�]������������
 */
//=============================================================================
static void dma_stop_int(void)
{
	
	iset_flg(ID_FLG_IO, FPTN_CAP_TR_END);
}