//=============================================================================
/**
 *	�r�g-�Q �b�o�t���W���[�� ���j�^�v���O����
 *	
 *	@file Mon_ver.c
 *	@version 1.00
 *	
 *	@author	F.Saeki
 *	@date	2001/12/07
 *	@brief	�o�[�W�����\���R�}���h���W���[��
 *
 *	Copyright (C) 2007, OYO Electric Corporation
 */
//=============================================================================
#include "kernel.h"
#include "nonet.h"
#include "nonteln.h"
#include "mon.h"

//=============================================================================
/**
 * �u�d�q�R�}���h�̎��s
 */
//=============================================================================
void CmdVer(void)
{
	Ctrl.Proc = ReadCmd;						// �R�}���h���͂ɖ߂�

	SendText("*** VA-300 Driver test Program Version 0.91       ***" CRLF);
	SendText("*** Copyright (C)2012 OYO Electric Software Group ***" CRLF);
}

