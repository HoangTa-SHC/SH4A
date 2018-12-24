//=============================================================================
/**
 *
 * VA-300�v���O����
 * <<I/O����֘A���W���[��>>
 *
 *	@brief I/O���͐���@�\�B
 *	
 *	@file drv_io.c
 *	
 *	@date	2013/03/08
 *	@version 1.00 �V�K�쐬
 *	@author	F.Saeki
 *
 *	Copyright (C) 2012, OYO Electric Corporation
 */
//=============================================================================
#define	_DRV_IO_C_
#include "kernel.h"
#include "sh7750.h"
#include "drv_fpga.h"

// �}�N����`
#define	IO_IP			8				///< I/O���͊����݃��x��(8)
#define	IO_INT			INT_IRL8		///< I/O���͊����ݔԍ�
#define	enable_io_int()	fpga_setw(INT_CRL, (INT_CRL_AUXIN));	///< I/O���͊����݋���
#define	clear_io_int()	fpga_clrw(INT_CRL, (INT_CRL_AUXIN));	///< I/O���͊����݃N���A

// �v���g�^�C�v�錾
static INTHDR ioInt(void);				///< ���Ԍ��m�Z���T������
static void io_int(void);				///< ���Ԍ��m�Z���T������(�{��)


// �ϐ���`
static ID s_idTsk;						///< �҂��^�X�N
const T_DINH dinh_io = { TA_HLNG, ioInt, IO_IP};		// �����ݒ�`

//=============================================================================
/**
 * I/O���͊����ݏ�����
 *
 * @param idTsk �����ݎ��ɋN������^�X�N
 * @retval E_OK ����
 */
//=============================================================================
ER IoInit(ID idTsk)
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
	
	ercd = def_inh(IO_INT, &dinh_io);				// I/O���͊����ݐݒ�

	// �����݋���
	if (ercd == E_OK) {
		enable_io_int();
	}
	vset_psw(psw);

	return ercd;
}

#pragma interrupt(ioInt)
//=============================================================================
/**
 * I/O���͊�����
 */
//=============================================================================
static INTHDR ioInt(void)
{
	ent_int();
	io_int();						// I/O���A�����ݏ���(�{��)
	ret_int();
}

//=============================================================================
/**
 * I/O���͊�����(�{��)
 */
//=============================================================================
static void io_int(void)
{
	clear_io_int();					// I/O���͊����݃N���A&�s����
	if (s_idTsk) {
		iwup_tsk( s_idTsk );
	}
	enable_io_int();				// I/O���͊����݋���
}

