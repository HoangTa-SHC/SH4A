/**
*	VA-300�v���O����
*
*	@file drv_tim.c
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/07/31
*	@brief  �^�C�}���W���[��(���Č����痬�p)
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
#include "kernel.h"
#include "sh7750.h"
#include "drv_tim.h"

// �v���g�^�C�v�錾
static void TimerCyc(VP_INT exinf);					// �^�C�}�[����

// �ϐ��錾
static const T_CCYC ccyc_tmo    = { TA_HLNG, NULL, TimerCyc, 10,    0};
static long s_lTmupTimer[ TIM_MAX ];				///< �^�C�}�J�E���^

//=============================================================================
/**
 * �����n���h��������
 * @param idCyc ID
 * @return �G���[�R�[�h
 */
//=============================================================================
ER TmInit(ID idCyc)
{
	ER  ercd;
	UB  i;
	
	ercd = E_PAR;
	
	// �J�E���^�N���A
	for(i = 0;i < TIM_MAX;i++) {
		s_lTmupTimer[ i ] = 0L;
	}
	
	// �T�C�N���b�N�^�C�}�[�쐬�A�N��
	if (idCyc) {
		ercd = cre_cyc(idCyc, &ccyc_tmo);		/* Create cyclic handler */
		if (ercd == E_OK) {
			ercd = sta_cyc( idCyc );
		}
	} else {
		ercd = acre_cyc(&ccyc_tmo);
		if (ercd > 0) {
			ercd = sta_cyc( ercd );
		}
	}
	
	return ercd;
}

//=============================================================================
/**
 * �^�C�}�[�ݒ�(�^�C�}�[�J�E���^��ݒ肷��)
 * @param eName �g�p����^�C�}�[
 * @param lTime ����
 */
//=============================================================================
void TmSet(enum TIM_NAME eName, const long lTime)
{
	if( eName < TIM_MAX) {
		s_lTmupTimer[ eName ] = lTime;
	}
}

//=============================================================================
/**
 * �^�C�}�[�|�[�����O
 * @param eName �g�p����^�C�}�[
 * @return ����(ID�����������Ƃ���-1��Ԃ�)
 */
//=============================================================================
long TmPol(enum TIM_NAME eName)
{
	if( eName < TIM_MAX) {
		return s_lTmupTimer[ eName ];
	} else {
		return (-1);
	}
}

//=============================================================================
/**
 * �����n���h������(�^�C�}�J�E���^�����炷)
 * @param exinf ���g�p
 */
//=============================================================================
static void TimerCyc(VP_INT exinf)
{
	enum TIM_NAME eName;
	
	// �^�C�}�[�J�E���^���Z
	for (eName = 0;eName < TIM_MAX;eName++) {
		if(s_lTmupTimer[ eName ]) s_lTmupTimer[ eName ]--;		// �J�E���^���Z
	}
}
