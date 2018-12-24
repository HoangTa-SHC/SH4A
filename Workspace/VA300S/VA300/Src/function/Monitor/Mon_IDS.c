//=============================================================================
/**
 *	@brief �r�g-�S �b�o�t���W���[�� ���j�^�v���O����
 *	
 *	@file Mon_IDS.c
 *	
 *	@date	2007/12/10
 *	@version 1.00 �@��ԍ��ݒ�R�}���h���W���[��
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
 * �h�c�r�R�}���h�̎��s
 */
//=============================================================================
void CmdIDS(void)
{
	BYTE	code;
	
	Ctrl.Proc = ReadCmd;						// �R�}���h���͂ɖ߂�
	
	if (code = ParsParm())						// �R�}���h�p�����[�^����
	{
		SendMsg(code);							// �p�����[�^�G���[
		return;
	}
}

//-----------------------------------------------------------------------------
// �R�}���h�p�����[�^�̉���
//-----------------------------------------------------------------------------
static UB ParsParm(void)
{
	char*	Parm;
	LONG	tmp;
	UH		idno;
	
	if (Parm = strtok(NULL," \x0D\t"))			// �p�����[�^�w�肠��
	{
		if (!(Asc2Dec(Parm,&tmp,0))) return MSG_BADPARM;
		idno = (UH)tmp;
	}
	else return MSG_BADPARM;
	
	lan_set_id( idno );					// ����Ղ�IP�A�h���X�ۑ�

	SendText("OK\r\n");

	return MSG_NONE;
}
