//=============================================================================
/**
 *
 * VA-300�v���O����
 * <<�@�\���W���[��>>
 *
 *	@brief ��ʓI�ȋ@�\�B
 *	
 *	@file sub_func.c
 *	
 *	@date	2012/05/20
 *	@version 1.00 �V�K�쐬
 *	@author	F.Saeki
 *
 *	Copyright (C) 2011, OYO Electric Corporation
 */
//=============================================================================
#include <machine.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
//#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include "kernel.h"
#include "sh7750.h"
#include "id.h"
#include "err_ctrl.h"

//==========================================================================
/**
 * sprintf�̃Z�}�t�H��
 */
//==========================================================================
int _sprintf(char *s, const char *control, ...)
{
	ER ercd;
	int n;
	va_list	ap;
	
	ercd = twai_sem(SEM_SPF, (1000/MSEC));
	if (ercd != E_OK) {
		return 0;
	}
	
	va_start(ap, control);
	n = vsprintf(s, control, ap);
	va_end(ap);
	
	sig_sem(SEM_SPF);
	
	return n;
}

//=============================================================================
/**
 *	�������v�[���g�p�ʕ\���f�[�^�擾
 *	@param iLine �s�ԍ�
 *	@param pBuf �i�[��
 *	@retval TRUE �擾����
 */
//=============================================================================
BOOL MpfUseDisplayLine(int iLine, char *pBuf)
{
	T_RMPF rmpf;
	const char* ccMpfName[] = {
		"NONE",
		"MPF_COM",
		"MPF_MON",
		"MPF_DISP",
		"MPF_LRES",
	};

	if ((iLine < 1) && (iLine >= MPF_MAX)) {
		return FALSE;
	}
	if (ref_mpf(iLine, &rmpf) != E_OK) {
		return FALSE;
	}
	_sprintf(pBuf, "MPF : %s  COUNT : %d\r\n", ccMpfName[ iLine ], rmpf.fblkcnt);
	
	return TRUE;
}
