/**
*	VA-300�e�X�g�v���O����
*
*	@file tsk_ctlmain.c
*	@version 1.00
*
*	@author Bionics Co.Ltd T.Nagai
*	@date   2013/04/20
*	@brief  VA-300 �[������main
*
*	Copyright (C) 2013, Bionics Corporation
*/
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "kernel.h"
#include "sh7750.h"
#include "id.h"
#include "drv_fpga.h"
#include "err_ctrl.h"

#include "va300.h"

// �ϐ���`
static ID s_idTsk;

// �v���g�^�C�v�錾
static TASK CtlMain( void );		///< �[���R���g���[���E���C���E�^�X�N

// �^�X�N�̒�`
const T_CTSK ctsk_ctlmain    = { TA_HLNG, NULL, CtlMain,    8, 4096, NULL, (B *)"ctlmain" };


/*==========================================================================*/
/**
 * �[���R���g���[���E���C���E�^�X�N
 * 
 */
/*==========================================================================*/
TASK CtlMain( void )
{
	ER		ercd;
	
	// �����J�n
	for(;;) {
		// �v�ύX
		slp_tsk();
	}
}


/*==========================================================================*/
/**
 * �[���R���g���[���E���C���E�^�X�N������
 *
 * @param idTsk �^�X�NID
 * @retval E_OK ����N��
 */
/*==========================================================================*/
ER CtlMainTaskInit(ID idTsk)
{
	ER ercd;
	
	// �^�X�N�̐���
	if (idTsk > 0) {
		ercd = cre_tsk(idTsk, &ctsk_ctlmain);
		if (ercd == E_OK) {
			s_idTsk = idTsk;
		}
	} else {
		ercd = acre_tsk(&ctsk_ctlmain);
		if (ercd > 0) {
			s_idTsk = ercd;
		}
	}
	if (ercd < 0) {
		return ercd;
	}
	
	// �^�X�N�̋N��
	ercd = sta_tsk(s_idTsk, 0);
	
	return ercd;
}