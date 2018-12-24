//=============================================================================
/**
 *	@brief �r�g-�S �b�o�t���W���[�� ���j�^�v���O����
 *	
 *	@file Mon_NS.c
 *	
 *	@date	2007/12/10
 *	@version 1.00 IP�A�h���X�A�|�[�g�ԍ��A�T�u�l�b�g�}�X�N�ݒ�R�}���h���W���[��
 *	@author	F.Saeki
 *
 *	@date	2011/4/7
 *	@version 1.20 OYO-105A�p�u�[�g�v���O�������痬�p
 *	@author	F.Saeki
 *
 *	Copyright (C) 2007, OYO Electric Corporation
 */
//=============================================================================
#include "kernel.h"
#include "nonet.h"
#include "nonteln.h"
#include "nonethw.h"
#include "mon.h"
#include "va300.h"
#include "drv_eep.h"

static UB ParsParm(void);

//=============================================================================
/**
 * �m�r�R�}���h�̎��s
 */
//=============================================================================
void CmdNS(void)
{
	BYTE	code;
	
	Ctrl.Proc = ReadCmd;						// �R�}���h���͂ɖ߂�
	
	if (code = ParsParm())						// �R�}���h�p�����[�^����
	{
		SendMsg(code);							// �p�����[�^�G���[
		return;
	}
//	dly_tsk( 1000/MSEC);
//	reset();									// ���Z�b�g
}

//-----------------------------------------------------------------------------
// �R�}���h�p�����[�^�̉���
//-----------------------------------------------------------------------------
static UB ParsParm(void)
{
	char*	Parm;
	LONG	tmp;
	UB		ip_addr[4];
	UB		mask[4];
	UH		portno;
	int		i;
	
	for(i=0;i<sizeof ip_addr;i++) {
		if (Parm = strtok(NULL," .\x0D\t"))		// �p�����[�^�w�肠��
		{
			if (!(Asc2Dec(Parm,&tmp,0))) return MSG_BADPARM;
			ip_addr[i] = (UB)tmp;
		}
		else return MSG_BADPARM;
	}
	for(i=0;i<sizeof mask;i++) {
		if (Parm = strtok(NULL," .\x0D\t"))		// �p�����[�^�w�肠��
		{
			if (!(Asc2Dec(Parm,&tmp,0))) return MSG_BADPARM;
			mask[i] = (UB)tmp;
		}
		else return MSG_BADPARM;
	}
	if (Parm = strtok(NULL," \x0D\t"))			// �p�����[�^�w�肠��
	{
		if (!(Asc2Dec(Parm,&tmp,0))) return MSG_BADPARM;
		portno = (UH)tmp;
	}
	else return MSG_BADPARM;
	
	lan_set_ip( ip_addr );						// IP�A�h���X�ۑ�
	lan_set_mask( mask );						// �T�u�l�b�g�}�X�N�ۑ�
	lan_set_port( portno );						// �ʐM�|�[�g�ԍ��ۑ�

	SendText("OK\r\n");

	return MSG_NONE;
}
