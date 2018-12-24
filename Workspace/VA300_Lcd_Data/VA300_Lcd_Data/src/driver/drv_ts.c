//=============================================================================
/**
 *
 * VA-300�v���O����
 * <<���Ԍ��m�Z���T����֘A���W���[��>>
 *
 *	@brief ���Ԍ��m�Z���T����@�\�B
 *	
 *	@file drv_ts.c
 *	
 *	@date	2012/09/26
 *	@version 1.00 �V�K�쐬
 *	@author	F.Saeki
 *
 *	Copyright (C) 2012, OYO Electric Corporation
 */
//=============================================================================
#define	_DRV_S_C_
#include "kernel.h"
#include "sh7750.h"
#include "drv_fpga.h"

// �}�N����`
#define	TS_IP			13				///< �^�b�`�p�l�������݃��x��(13)
#define	TS_INT			INT_IRL13		///< �^�b�`�p�l�������ݔԍ�
#define	enable_ts_int()	fpga_setw(INT_CRL, (INT_CRL_SENS));	///< �^�b�`�p�l�������݋���
#define	clear_ts_int()	fpga_clrw(INT_CRL, (INT_CRL_SENS));	///< �^�b�`�p�l�������݃N���A

// �v���g�^�C�v�錾
static INTHDR tsInt(void);				///< ���Ԍ��m�Z���T������
static void ts_int(void);				///< ���Ԍ��m�Z���T������(�{��)


// �ϐ���`
static ID s_idTsk;						///< �҂��^�X�N
const T_DINH dinh_ts = { TA_HLNG, tsInt,   TS_IP};		// �����ݒ�`

//=============================================================================
/**
 * ���Ԍ��m�Z���T������
 *
 * @param idTsk �����ݎ��ɋN������^�X�N
 * @retval E_OK ����
 */
//=============================================================================
ER TsInit(ID idTsk)
{
	ER ercd;
	UW psw;
	
	// �^�X�NID��ݒ�
	if (idTsk) {
		s_idTsk = idTsk;
	} else {
		return E_PAR;
	}

	//
	// �|�[�g�̐ݒ�(FPGA�Ȃ̂ŕs�v�Ǝv����)
	//
	
	
	//
	// �����ݐݒ�
	//
	psw = vdis_psw();
	
	ercd = def_inh(TS_INT, &dinh_ts);				// ���Ԍ��m�Z���T�����ݐݒ�

	// �����݋���
	if (ercd == E_OK) {
		enable_ts_int();
	}
	vset_psw(psw);

	return ercd;
}

#pragma interrupt(tsInt)
//=============================================================================
/**
 * ���Ԍ��m�Z���T������
 */
//=============================================================================
static INTHDR tsInt(void)
{
	ent_int();
	ts_int();						// ���Ԍ��m�Z���T�����ݏ���(�{��)
	ret_int();
}

//=============================================================================
/**
 * ���Ԍ��m�Z���T������(�{��)
 */
//=============================================================================
static void ts_int(void)
{
	clear_ts_int();					// �^�b�`�Z���T�����݃N���A&�s����
	if (s_idTsk) {
		iwup_tsk( s_idTsk );
	}
// By T.Nagai	enable_ts_int();				// �^�b�`�Z���T�����݋���
}

