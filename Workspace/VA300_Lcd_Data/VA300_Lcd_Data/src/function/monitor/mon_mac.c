//=============================================================================
/**
 *	�r�g-�S �b�o�t���W���[�� ���j�^�v���O����
 *	
 *	@file Mon_MAC.c
 *	@version 1.00
 *	
 *	@author	F.Saeki
 *	@date	2007/12/10
 *	@brief	MAC�A�h���X�ݒ�R�}���h���W���[��
 *
 *	Copyright (C) 2007, OYO Electric Corporation
 */
//=============================================================================
#include "kernel.h"
#include "nonet.h"
#include "nonteln.h"
#include "mon.h"
#include "lan9118.h"

static UB ParsParm(void);

//=============================================================================
/**
 * �l�r�R�}���h�̎��s
 */
//=============================================================================
void CmdMS(void)
{
	BYTE	code;

	Ctrl.Proc = ReadCmd;						// �R�}���h���͂ɖ߂�
	
	if (code = ParsParm())						// �R�}���h�p�����[�^����
	{
		SendMsg(code);							// �p�����[�^�G���[
		return;
	}
//	dly_tsk( 1000/MSEC);
//	reset();
}

//-----------------------------------------------------------------------------
// �R�}���h�p�����[�^�̉���
//-----------------------------------------------------------------------------
static UB ParsParm(void)
{
	char*	Parm;
	LONG	tmp;
	UB		mac_addr[6];
	int		i;
	
	for(i=0;i<sizeof mac_addr;i++) {
		if (Parm = strtok(NULL," -\x0D\t"))			// �p�����[�^�w�肠��
		{
			if (!(Asc2Hex(Parm,&tmp,0))) return MSG_BADPARM;
			mac_addr[i] = (UB)tmp;
		}
		else return MSG_BADPARM;
	}
	lan_set_mac( mac_addr );
	
	SendText("OK\r\n");
	
	return MSG_NONE;
}
