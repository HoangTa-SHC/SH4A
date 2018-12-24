/**
*	VA-300�v���O����
*
*	@file drv_dsw.c
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/08/30
*	@brief  DSW���W���[��(���Č����痬�p)
*
*	Copyright (C) 2010, OYO Electric Corporation
*/
#define	_DRV_DSW_C_
#include "kernel.h"
#include "sh7750.h"
#include "drv_dsw.h"
#include "drv_fpga.h"

// ��`
#define	dsw_in()	((~sfr_inw(BSC_PDTRA)) & DSW1_ALL)	///< DSW�̃|�[�g����Ǎ���(SIL)

/*==========================================================================*/
/**
 * DSW�̎擾
 *
 * @return DSW�̏��(ON:bit��ON�A���8bit��DSW1�A����8bit��DSW2)
 */
/*==========================================================================*/
UH DswGet(void)
{
	UH uhDsw;
	
	// �|�[�g�ɐڑ�����Ă���DSW
	uhDsw = dsw_in();
	
	return uhDsw;
}
