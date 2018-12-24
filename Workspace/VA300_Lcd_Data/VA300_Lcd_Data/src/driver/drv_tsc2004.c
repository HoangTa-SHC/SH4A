//=============================================================================
/**
 *
 * VA-300�v���O����
 * <<TSC2004�֘A���W���[��>>
 *
 *	@brief TSC2004�p�h���C�o()
 *	
 *	@file drv_tsc2004.c
 *	
 *	@date	2012/09/12
 *	@version 1.00 �V�K�쐬
 *	@author	F.Saeki
 *
 *	Copyright (C) 2012, OYO Electric Corporation
 */
//=============================================================================
#define	_DRV_TSC2004_C_
#include "kernel.h"
#include "drv_tsc2004.h"

// ��`
#define	REG_POS_X	0						///< X���W���W�X�^�A�h���X
#define	REG_POS_Y	1						///< Y���W���W�X�^�A�h���X
#define	COM_TMOUT	1000					///< �ʐM�^�C���A�E�g����

// �R�}���h��`
static const UB s_cCmdResetOn[]  = { 0x83 };	///< ���Z�b�gON	
												//	Control Byte ID=1
												//	SWRST=1
												//	STS  =1
											
static const UB s_cCmdResetOff[] = { 0x80 };	///< ���Z�b�gOFF
												//	Control Byte ID=1
												//	SWRST=0
												//	STS  =0
											
static const UB s_cCmdCFR0Set[]  = { 0x62,		// CFR0���W�X�^�w��, PND0=1
									0x8C,		// PSM=1,STS=0,RM=0,CL1=0, CL0=1,PV2=1,PV1=0,PV0=0,
									0x50 };		// PR2=0,PR1=1,PR0=0,SNS2=1, SNS1=0,SNS0=0,DTW=0,LSM=0
									
static const UB s_cCmdCFR1Set[]  = { 0x6A,		// CFR1���W�X�^�w��, PND0=1
									0x03,		// TBM3=0,TBM2=0,TBM1=1,TBM0=1
									0x07 };		// BTD2=1,BTD1=1,BTD0=1(10SSPS)
									
static const UB s_cCmdCFR2Set[]  = { 0x72,		// CFR2���W�X�^�w��, PND0=1
									0x24,		// PINTS1=0,PINTS0=0(PINTDAV#��PENIRQ#&&DAV�ɐݒ�),M1=1,M0=0,W1=0,W0=1,TZ1=0,TZ0=0,
									0x18 };		// AZ1=0,AZ0=0,MX=1,MY=1,MZ=0,MA=0,MT=0

static const UB s_cCmdPosXGet[]  = { 0x03 };	// ���W�X�^0h(X)�̃f�[�^�Ǐo���w��
static const UB s_cCmdPosYGet[]  = { 0x0B };	// ���W�X�^1h(Y)�̃f�[�^�Ǐo���w��

// �ϐ���`
static ID s_idTsk;								///< �����ݎ��ɋN������^�X�NID

// �v���g�^�C�v�錾
static ER tsc2004Send(UB *p, int iSize);		///< �^�b�`�p�l���R���g���[���֑��M
static ER tsc2004Read(UB ubAddr, short *pVal);	///< �^�b�`�p�l���R���g���[���̃��W�X�^�Ǐo��

//=============================================================================
/**
 * �^�b�`�p�l���R���g���[��������
 *
 * @retval E_OK ����
 */
//=============================================================================
ER Tsc2004Init(void)
{
	// ���Z�b�g�𑗐M
	tscSendResetOn();
	tscSendResetOff();
	
	// CFR���W�X�^�ɐݒ�
	tscSetCFR0();
	tscSetCFR1();
	tscSetCFR2();

	s_idTsk = 0;

	return E_OK;
}

//=============================================================================
/**
 * �^�b�`�p�l���̍��W�擾
 * 
 * @param piPosX X���W
 * @param piPosY Y���W
 * @retval E_OK ����
 */
//=============================================================================
ER Tsc2004PosGet(int *piPosX, int *piPosY)
{
	ER	ercd;
	short	sVal;

	tscSendPosXGet();						// X���W�X�^�Ǐo���R�}���h���M
	ercd = tsc2004Read(REG_POS_X, &sVal);	// X���W�l�Ǐo��
	if (ercd == E_OK) {
		*piPosX = sVal;
	} else {
		return ercd;
	}
	
	tscSendPosYGet();						// Y���W�X�^�Ǐo���R�}���h���M
	ercd = tsc2004Read(REG_POS_Y, &sVal);	// Y���W�l�Ǐo��
	if (ercd == E_OK) {
		*piPosY = sVal;
	}
	
	return ercd;
}

//=============================================================================
/**
 * �^�b�`�p�l���R���g���[���֑��M
 *
 * @param p ���M�f�[�^
 * @param iSize ���M�f�[�^��
 * @param OS�̃G���[�R�[�h
 */
//=============================================================================
static ER tsc2004Send(UB *p, int iSize)
{
	ER ercd;
	UH uhVal;
	
	s_idTsk = vget_tid();				// �^�X�NID�̎擾
	vcan_wup();							// �N���v���̃N���A

	// CONTROL BYTE�ݒ�
	tscSetCtrlByte( *p );
	
	if (iSize == 1) {					// Control Byte�݂̂̑��M
		tscSendCtrlByte();
	} else {							// Data Byte�����M
		p++;
		uhVal = (UH)*p << 8;
		p++;
		uhVal |= *p;
		tscSetDataByte( uhVal );
		tscSendDataByte();
	}
	
	ercd = tslp_tsk((COM_TMOUT / MSEC));
	
	s_idTsk = 0;
	
	return ercd;	
}

//=============================================================================
/**
 * �^�b�`�p�l���R���g���[������Ǐo��
 *
 * @param p ���M�f�[�^
 * @param iSize ���M�f�[�^��
 */
//=============================================================================
static ER tsc2004Read(UB ubAddr, short *pVal)
{
	ER ercd;
	UB ubVal;
	
	ubVal = ((ubAddr & 0x0F) << 3) | 0x03;
	
	// CONTROL BYTE�ݒ�
	tscSetCtrlByte( ubVal );
	
	s_idTsk = vget_tid();				// �^�X�NID�̎擾
	vcan_wup();							// �N���v���̃N���A
	
	tscReadReg();						// �Ǐo���V�[�P���X�N��

	ercd = tslp_tsk((COM_TMOUT / MSEC));
	if (ercd == E_OK) {
		*pVal = tscGetDataByte();
	}
	s_idTsk = 0;
	
	return ercd;
}

//=============================================================================
/**
 * �^�b�`�p�l���R���g���[���ʐM�����ݏ���
 */
//=============================================================================
void Tsc2004ComInt(void)
{
	disable_tplcom_int();				// �^�b�`�p�l���ʐM�����݃t���O������
	if (s_idTsk) {
		iwup_tsk( s_idTsk );
	}
	enable_tplcom_int();				// �^�b�`�p�l���ʐM�����݋���
}
