/**
*	VA-300�v���O����
*
*	@file tsk_buz.c
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/07/10
*	@brief  �u�U�[����^�X�N
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
#include <stdio.h>
#include <string.h>

#include "kernel.h"
#include "sh7750.h"
#include "sh7750R.h"
#include "id.h"
#include "command.h"
#include "drv_fpga.h"
#include "err_ctrl.h"

#include "va300.h"
#include "drv_buz.h"

// �}�N����`
#ifndef	CLK
#define	CLK		50000000
#endif

#ifndef	CH
#define	CH		2
#endif

#if (CH==1)
#define	TMU_TCOR	TMU_TCOR1
#define	TMU_TCNT	TMU_TCNT1
#define	TMU_TCR		TMU_TCR1
#define	INT_TUNI	INT_TUNI1
#else
#define	TMU_TCOR	TMU_TCOR2
#define	TMU_TCNT	TMU_TCNT2
#define	TMU_TCR		TMU_TCR2
#define	INT_TUNI	INT_TUNI2
#endif

#define	INT_LVL		10					///< �����݃��x��
#define	TCR_UNF		0x0100

// �ϐ���`
static ID s_idFlg;						///< �t���OID
static ID s_idTsk;						///< �^�X�NID
static UW s_uwCount;					///< �J�E���g�p

// �v���g�^�C�v�錾
ER SoundInit(ID tskid, ID flgid);		// ������
static ER tmInit(void);					///< �^�C�}�[������
static INTHDR tmInt(void);				///< �^�C�}�[������
static void tm_int(void);				///< �^�C�}�[�����ݖ{��

// �����ݒ�p
const T_CTSK ctsk_buz = { TA_HLNG, NULL, SoundTask, 5, 2048, NULL, (B *)"Sound task" };//
const T_CFLG cflg_buz = { TA_WMUL, 0, (B *)"buz_flag" };
const T_DINH dinh_tim = { TA_HLNG, tmInt, INT_LVL};
const T_CSEM csem_tim   = { TA_TFIFO, 1, 1, (B *)"sem_tim" };

/*==========================================================================*/
/**
 * �T�E���h�^�X�N������
 * 
 * @param tskid �^�X�NID
 * @param flgid �t���OID
 */
/*==========================================================================*/
ER SoundInit(ID tskid, ID flgid)
{
	ER ercd;
	
	// �^�X�N�̐���
	if (tskid > 0) {
		ercd = cre_tsk(tskid, &ctsk_buz);
		if (ercd == E_OK) {
			s_idTsk = tskid;
		}
	} else {
		ercd = acre_tsk(&ctsk_buz);
		if (ercd > 0) {
			s_idTsk = ercd;
		}
	}
	if (ercd < 0) {
		return ercd;
	}
	
	// �t���O�̐���
	if (flgid > 0) {
		ercd = cre_flg(flgid, &cflg_buz);
		if (ercd == E_OK) {
			s_idFlg = flgid;
		}
	} else {
		ercd = acre_flg(&cflg_buz);
		if (ercd > 0) {
			s_idFlg = ercd;
		}
	}
	if (ercd < 0) {
		return ercd;
	}
	
	// �^�C�}�ݒ�
	ercd = tmInit();
	
	if (ercd == E_OK) {
		ercd = sta_tsk(s_idTsk, 0);
	}
	return ercd;
}

/*==========================================================================*/
/**
 * �^�C�}������
 */
/*==========================================================================*/
static ER tmInit(void)
{
	ER ercd;
	unsigned long  tc;
	unsigned short tpsc;
	UW psw;
	const UH uhUsec = 62500;					// 62.5ms�Œ�
	
	psw = vdis_psw();
	 
	// �x�N�^�o�^
	ercd = def_inh(INT_TUNI, &dinh_tim);		// �^�C�}�����ݐݒ�
	if (ercd == E_OK) {
		
		// ���萔���v�Z
		if (((CLK) / (256 * 1000 * 1000)) * uhUsec >= 0x0fffffff) {
			tc = ((CLK) / (256 * 1000 * 1000)) * uhUsec;
			tpsc = 3;
		} else if (((CLK) / (64 * 1000 * 1000)) * uhUsec >= 0x0fffffff) {
			tc = ((CLK) / (64 * 1000 * 1000)) * uhUsec;
			tpsc = 2;
		} else if (((CLK) / (16 * 1000 * 1000)) * uhUsec >= 0x0fffffff) {
			tc = ((CLK) / (16 * 1000 * 1000)) * uhUsec;
			tpsc = 1;
		} else {
			tc = ((CLK) / (4 * 1000 * 1000)) * uhUsec;
			tpsc = 0;
		}

		// �^�C�}���j�b�g������
		sfrr_clr(TMU_TSTR, 0x01 << CH);		// �^�C�}��~
		sfrr_outl(TMU_TCOR, (tc-1));		// �^�C�}�R���X�^���g�ݒ�
		sfrr_outl(TMU_TCNT, (tc-1));		// �^�C�}�J�E���^�����l
		sfrr_outw(TMU_TCR, (tpsc | 0x20));	// �^�C�}�v���X�P�[���I���A�A���_�[�t���[���荞�݂�����

#if (CH == 1)
		sfr_outw(INTC_IPRA, sfr_inw(INTC_IPRA) | (INT_LVL <<  8));
#elif (CH == 2)
		sfr_outw(INTC_IPRA, sfr_inw(INTC_IPRA) | (INT_LVL <<  4));
#elif (CH == 3)
    	sfrr_outw(INTC_INTPRI00, sfr_inw(INTC_INTPRI00) | (INT_LVL <<  8));
#elif (CH == 4)
    	sfrr_outw(INTC_INTPRI00, sfr_inw(INTC_INTPRI00) | (INT_LVL <<  12));
#else

#endif
	
		ercd = cre_sem(SEM_TIM, &csem_tim);	// �Z�}�t�H�쐬
	}
	// �ϐ���������
	s_uwCount = 0;

	vset_psw(psw);
	
	return ercd;
}

/*==========================================================================*/
/**
 * �T�E���h����^�X�N
 */
/*==========================================================================*/
TASK SoundTask(void)
{
	ER		ercd;
	FLGPTN	flgptn;

	BuzTplSet(0x17, BUZ_OFF);

	// �����J�n
	for(;;) {
		ercd = wai_flg(s_idFlg, (FPTN_EMG_ON | FPTN_EMG_OFF | FPTN_TPL_OK | FPTN_TPL_NG), TWF_ORW, &flgptn);
		if (ercd == E_OK) {
			//--- �J�n ---
			if (flgptn & FPTN_EMG_ON) {
				BuzEmgOn();
				// �t���O�̃N���A
				clr_flg(s_idFlg, ~FPTN_EMG_ON);
			}
			//--- �I�� ---
			if (flgptn & FPTN_EMG_OFF) {
				BuzEmgOff();
				// �t���O�̃N���A
				clr_flg(s_idFlg, ~FPTN_EMG_OFF);
			}
			//--- �^�b�`�p�l���pOK�u�U�[ ---
			if (flgptn & FPTN_TPL_OK) {
				BuzTplOn();
				dly_tsk(50 / MSEC);
				BuzTplOff();
				// �t���O�̃N���A
				clr_flg(s_idFlg, ~FPTN_TPL_OK);
			}
			//--- �^�b�`�p�l���pNG�u�U�[ ---
			if (flgptn & FPTN_TPL_NG) {
				BuzTplOn();
				dly_tsk(50 / MSEC);
				BuzTplOff();
				dly_tsk(50 / MSEC);
				BuzTplOn();
				dly_tsk(50 / MSEC);
				BuzTplOff();
				// �t���O�̃N���A
				clr_flg(s_idFlg, ~FPTN_TPL_NG);
			}
			//--- �^�b�`�p�l���p�u�U�[OFF ---
			if (flgptn & FPTN_TPL_OFF) {
				BuzTplOff();
				// �t���O�̃N���A
				clr_flg(s_idFlg, ~FPTN_TPL_NG);
			}			
		} else {
			// �����ɂ���͎̂����G���[
			PrgErrSet();
			slp_tsk();							// �G���[�̂Ƃ��̓^�X�N�I��
		}
	}
}

#pragma interrupt(tmInt)
/*==========================================================================*/
/**
 * �^�C�}�����݃n���h��
 *
 */
/*==========================================================================*/
static INTHDR tmInt(void)
{
	ent_int();
	tm_int();
	ret_int();
}

/*==========================================================================*/
/**
 * �^�C�}�����݃n���h��(�{��)
 *
 */
/*==========================================================================*/
static void tm_int(void)
{
	// �����݃N���A
	sfrr_clrw(TMU_TCR, TCR_UNF);		// �A���_�[�t���[�t���O�N���A
	
	if (s_uwCount) {
		s_uwCount--;
	} else {
		// �^�C�}��~
#if (CH < 3)
		sfr_clr(TMU_TSTR, 0x01 << CH);	// �J�E���g�����~
#else
		sfrr_clr(TMU_TSTR, 0x01 << (CH - 3));	// �J�E���g�����~
#endif
		if (s_idFlg) {
			iset_flg(s_idFlg, FPTN_TPL_OFF);	// �^�b�`�p�l���p�u�U�[OFF
		}
	}
}

/*==========================================================================*/
/**
 * �T�E���h����
 *
 * @param eCtrl ������e
 */
/*==========================================================================*/
void SoundCtrl(enum SOUND_CTRL eCtrl)
{
	switch (eCtrl) {
	case SOUND_CTRL_EMG_ON:
		set_flg(s_idFlg, FPTN_EMG_ON);
		break;
	case SOUND_CTRL_EMG_OFF:
		set_flg(s_idFlg, FPTN_EMG_OFF);
		break;
	case SOUND_CTRL_TPL_OK:
		set_flg(s_idFlg, FPTN_TPL_OK);
		break;
	case SOUND_CTRL_TPL_NG:
		set_flg(s_idFlg, FPTN_TPL_NG);
		break;
	}
}
