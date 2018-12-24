//=============================================================================
/**
 *
 * VA-300�v���O����
 * <<�^�b�`�p�l������֘A���W���[��>>
 *
 *	@brief �^�b�`�p�l������@�\�B�f�o�C�X��TSC2004()
 *	
 *	@file drv_tpl.c
 *	
 *	@date	2012/09/12
 *	@version 1.00 �V�K�쐬
 *	@author	F.Saeki
 *
 *	Copyright (C) 2012, OYO Electric Corporation
 */
//=============================================================================
/*
	<<�����݂ɂ���>>
	�{�h���C�o�ł̓^�b�`�p�l���������ꂽ�Ƃ��̊����ݎ��A
	�������֐��Őݒ肵���t���OID�ƃr�b�g�p�^�[�����g�p���ăt���O��
	�Z�b�g����܂��B
	�^�X�N����wai_flg()�ő҂��ď������Ă��������B
	
	<<�␳�l�ɂ���>>
	�������֐�����1�{�ɏ���������܂��B
	�V�X�e���N������TplRevSet()�֐��Œl��ݒ肵�Ă��������B
	�␳�l�ɂ��Ă͕␳�l�����߂邽�߂̒������[�h���쐬
	���Ă��������B
 */
#define	_DRV_TPL_C_
#include "kernel.h"
#include "sh7750.h"
#include "drv_tpl.h"

// �}�N����`
#define	TPL_IP			12				///< �^�b�`�p�l�������݃��x��(12)
#define	TPL_INT			INT_IRL12		///< �^�b�`�p�l�������ݔԍ�
#define	TPL_COM_IP		7				///< �^�b�`�p�l���ʐM�����݃��x��(7)
#define	TPL_COM_INT		INT_IRL7		///< �^�b�`�p�l���ʐM�����ݔԍ�

// �v���g�^�C�v�錾
static INTHDR tplInt(void);				///< �^�b�`�p�l��������
static void tpl_int(void);				///< �^�b�`�p�l��������(�{��)
static INTHDR tplComInt(void);			///< �^�b�`�p�l���ʐM������
static ER tplPosGet(int *piPosX, int *piPosY);	///< ���W�Ǎ���

// �ϐ���`
static ID s_idTsk;						///< �҂��^�X�N
static ID s_idSem;						///< �Z�}�t�HID
const T_DINH dinh_tpl     = { TA_HLNG, tplInt,   TPL_IP};		// �����ݒ�`
const T_DINH dinh_tpl_com = { TA_HLNG, tplComInt,TPL_COM_IP};	// �����ݒ�`
const T_CSEM csem_tpl     = { TA_TFIFO, 1, 1, (B *)"sem_tpl" };	// �^�b�`�p�l���̃Z�}�t�H
static ST_TPL_REV s_stTplParam;			///< �^�b�`�p�l���p�␳�l
static const ST_TPL_REV s_stTplParamDef = {	///< �^�b�`�p�l���p�␳�l�̃f�t�H���g�l
	1.0, 0.0, 0.0, 0.0, 1.0, 0.0
};

//=============================================================================
/**
 * �^�b�`�p�l���R���g���[��������
 *
 * @param idSem �Z�}�t�HID
 * @retval E_OK ����
 */
//=============================================================================
ER TplInit(ID idSem)
{
	ER ercd;
	UW psw;
	
	//
	// �ϐ�������
	//
	s_idTsk = 0;
	TplRevInit();								// �␳�l��������

	// �Z�}�t�HID��ݒ�
	if (idSem) {
		ercd = cre_sem(idSem, &csem_tpl);		// �Z�}�t�H����
		if (ercd == E_OK) {
			s_idSem = idSem;
		} else {
			return ercd;
		}
	} else {
		return E_PAR;
	}

	//
	// �|�[�g�̐ݒ�(FPGA�Ȃ̂ŕs�v�Ǝv����)
	//
	
	
	//
	// �����ݐݒ�
	//
	psw = vdis_psw();									// �����݋֎~
	
	if (ercd == E_OK) {
		ercd = def_inh(TPL_INT, &dinh_tpl);				// �^�b�`�p�l��I/F�����ݐݒ�
	}
	if (ercd == E_OK) {
		ercd = def_inh(TPL_COM_INT, &dinh_tpl_com);		// �^�b�`�p�l���ʐMI/F�����ݐݒ�
	}
	// �����݋���
	if (ercd == E_OK) {
		enable_tpl_int();
		enable_tplcom_int();
	}
	vset_psw(psw);										// �����݋���

	//
	// �^�b�`�p�l���̃f�o�C�X�����Z�b�g
	//
	if (ercd == E_OK) {
		ercd = tpl_reset();
	}
	return ercd;
}

#pragma interrupt(tplInt)
//=============================================================================
/**
 * �^�b�`�p�l��������
 */
//=============================================================================
static INTHDR tplInt(void)
{
	ent_int();
	tpl_int();						// �^�b�`�p�l�������ݏ���(�{��)
	ret_int();
}

//=============================================================================
/**
 * �^�b�`�p�l�������ݏ���(�{��)
 */
//=============================================================================
static void tpl_int(void)
{
	disable_tpl_int();				// �^�b�`�p�l�������݃t���O������
	if (s_idTsk) {
		iwup_tsk(s_idTsk);
	}
	enable_tpl_int();				// �^�b�`�p�l�������݋���
}

#pragma interrupt(tplComInt)
//=============================================================================
/**
 * �^�b�`�p�l���ʐM������
 */
//=============================================================================
static INTHDR tplComInt(void)
{
	ent_int();
	tpl_com_int();					// �^�b�`�p�l���ʐM�����ݏ���(�{��)
	ret_int();
}

//=============================================================================
/**
 * ���W�Ǐo��
 *
 * @param piPosX X���W
 * @param piPosY Y���W
 * @param tmout �^�C���A�E�g����
 */
//=============================================================================
ER TplPosGet(int *piPosX, int *piPosY, TMO tmout)
{
	ER ercd;
	
	ercd = twai_sem(s_idSem, (100/MSEC));		// �Z�}�t�H�擾
	
	if (ercd == E_OK) {
		s_idTsk = vget_tid();					// ���^�X�NID�̎擾
		vcan_wup();								// �N���v���̃N���A
		ercd = tslp_tsk( tmout );
		s_idTsk = 0;
	} else {
		return ercd;
	}
	
	// �|�[�����O���łȂ��Ƃ��͐���ȂƂ��A�|�[�����O���̂݃^�C���A�E�g����
	if ((ercd == E_OK) || ((tmout == TMO_POL) && ercd == E_TMOUT)) {
		dly_tsk( 50/MSEC );					// 
		ercd = tplPosGet(piPosX, piPosY);		// ���W�擾
	}
	
	sig_sem(s_idSem);							// �Z�}�t�H�ԋp
	
	return ercd;
}


//=============================================================================
/**
 * ���W�Ǐo��(PDIC)
 *
 * @param piPosX X���W
 * @param piPosY Y���W
 */
//=============================================================================
static ER tplPosGet(int *piPosX, int *piPosY)
{
	ER ercd;
	int iPosX, iPosY;
	ST_TPL_REV *p;
	
	p = &s_stTplParam;
	
	*piPosX = 0;
	*piPosY = 0;
	
	ercd = tpl_pos_get(&iPosX, &iPosY);
	if (ercd == E_OK) {
		// ��ʃT�C�Y�ɍ��킹�ĕ␳
		iPosX = (int)((float)iPosX * 489.0 / 1023.0);
		iPosY = (int)((float)iPosY * 271.0 / 1023.0);
		// �ǂݍ��܂ꂽ���W��␳
		*piPosX = (int)(iPosX * p->fXx + iPosY * p->fXy + p->fXofs);
		*piPosY = (int)(iPosX * p->fYx + iPosY * p->fYy + p->fYofs);
	}
	return ercd;
}

//=============================================================================
/**
 * �␳�l�̐ݒ�
 *
 *	�␳��X = X * fXx + Y * fXy + fXofs
 *	�␳��Y = Y * fYx + Y * fYy + fYofs
 * �Ōv�Z�����
 *
 * @param fXx   X���W�p X���W�l�ɂ�����l
 * @param fXy   X���W�p Y���W�l�ɂ�����l
 * @param fXofs X���W�p �I�t�Z�b�g
 * @param fYx   Y���W�p X���W�l�ɂ�����l
 * @param fYy   Y���W�p Y���W�l�ɂ�����l
 * @param fYofs Y���W�p �I�t�Z�b�g
 */
//=============================================================================
void TplRevSet(float fXx, float fXy, float fXofs, float fYx, float fYy, float fYofs)
{
	ST_TPL_REV *p;
	
	p = &s_stTplParam;
	p->fXx   = fXx;
	p->fXy   = fXy;
	p->fXofs = fXofs;
	p->fYx   = fYx;
	p->fYy   = fYy;
	p->fYofs = fYofs;
}

//=============================================================================
/**
 * �ݒ肳��Ă���␳�l�̎擾
 *
 * @param pfXx   X���W�p X���W�l�ɂ�����l
 * @param pfXy   X���W�p Y���W�l�ɂ�����l
 * @param pfXofs X���W�p �I�t�Z�b�g
 * @param pfYx   Y���W�p X���W�l�ɂ�����l
 * @param pfYy   Y���W�p Y���W�l�ɂ�����l
 * @param pfYofs Y���W�p �I�t�Z�b�g
 */
//=============================================================================
void TplRevGet(float *pfXx, float *pfXy, float *pfXofs, float *pfYx, float *pfYy, float *pfYofs)
{
	ST_TPL_REV *p;
	
	p = &s_stTplParam;
	*pfXx   = p->fXx;
	*pfXy   = p->fXy;
	*pfXofs = p->fXofs;
	*pfYx   = p->fYx;
	*pfYy   = p->fYy;
	*pfYofs = p->fYofs;
}

//=============================================================================
/**
 * �␳�l�̏�����
 */
//=============================================================================
void TplRevInit(void)
{
	s_stTplParam = s_stTplParamDef;
}

//=============================================================================
/**
 * �␳�l�̌v�Z
 *
 * @param pBase  �����p�̍��W�ʒu
 * @param pInput �^�b�`�p�l����̃|�W�V����
 * @param pRev   �i�[��␳�l�\���̂ւ̃|�C���^
 */
//=============================================================================
void TplRevCalc(ST_POS_REV *pBase, ST_POS_REV *pInput, ST_TPL_REV *pRev)
{
//	float lcd_x1 = 10.0;	// �^�b�`�p�l�������p�̍��W�ʒuX1
//	float lcd_y1 = 10.0;	// �^�b�`�p�l�������p�̍��W�ʒuY1
//	float lcd_x2 = 300.0;	// �^�b�`�p�l�������p�̍��W�ʒuX2
//	float lcd_y2 = 50.0;	// �^�b�`�p�l�������p�̍��W�ʒuY2
//	float lcd_x3 = 450.0;	// �^�b�`�p�l�������p�̍��W�ʒuX3
//	float lcd_y3 = 260.0;	// �^�b�`�p�l�������p�̍��W�ʒuY3
	float lcd_x1;						// �^�b�`�p�l�������p�̍��W�ʒuX1
	float lcd_y1;						// �^�b�`�p�l�������p�̍��W�ʒuY1
	float lcd_x2;						// �^�b�`�p�l�������p�̍��W�ʒuX2
	float lcd_y2;						// �^�b�`�p�l�������p�̍��W�ʒuY2
	float lcd_x3;						// �^�b�`�p�l�������p�̍��W�ʒuX3
	float lcd_y3;						// �^�b�`�p�l�������p�̍��W�ʒuY3
	float aa;
	int tp_x1, tp_x2, tp_x3;
	int tp_y1, tp_y2, tp_y3;

	tp_x1 = pInput->tPos[ 0 ].iX;
	tp_x2 = pInput->tPos[ 1 ].iX;
	tp_x3 = pInput->tPos[ 2 ].iX;
	tp_y1 = pInput->tPos[ 0 ].iY;
	tp_y2 = pInput->tPos[ 1 ].iY;
	tp_y3 = pInput->tPos[ 2 ].iY;
	lcd_x1 = (float)pBase->tPos[ 0 ].iX;
	lcd_x2 = (float)pBase->tPos[ 1 ].iX;
	lcd_x3 = (float)pBase->tPos[ 2 ].iX;
	lcd_y1 = (float)pBase->tPos[ 0 ].iY;
	lcd_y2 = (float)pBase->tPos[ 1 ].iY;
	lcd_y3 = (float)pBase->tPos[ 2 ].iY;
	
	aa = (float)tp_x1*(tp_y2 - tp_y3) - (float)tp_x2*(tp_y1 - tp_y3) + (float)tp_x3*(tp_y1 - tp_y2);
	
	pRev->fXx   = ( (tp_y2 - tp_y3)*lcd_x1 + -(tp_y1 - tp_y3)*lcd_x2 + (tp_y1 - tp_y2)*lcd_x3 ) / aa;
	pRev->fXy   = ( -(tp_x2 - tp_x3)*lcd_x1 + (tp_x1 - tp_x3)*lcd_x2 + -(tp_x1 - tp_x2)*lcd_x3 )/ aa;
	pRev->fXofs = ( (tp_x2*tp_y3 - tp_y2*tp_x3)*lcd_x1 + -(tp_x1*tp_y3 - tp_y1*tp_x3)*lcd_x2 + (tp_x1*tp_y2 - tp_y1*tp_x2)*lcd_x3 )/ aa;
	
	pRev->fYx   = ( (tp_y2 - tp_y3)*lcd_y1 + -(tp_y1 - tp_y3)*lcd_y2 + (tp_y1 - tp_y2)*lcd_y3 )/ aa;
	pRev->fYy   = ( -(tp_x2 - tp_x3)*lcd_y1 + (tp_x1 - tp_x3)*lcd_y2 + -(tp_x1 - tp_x2)*lcd_y3 )/ aa;
	pRev->fYofs = ( (tp_x2*tp_y3 - tp_y2*tp_x3)*lcd_y1 + -(tp_x1*tp_y3- tp_y1*tp_x3)*lcd_y2 + (tp_x1*tp_y2 - tp_y1*tp_x2)*lcd_y3 )/ aa;
}

