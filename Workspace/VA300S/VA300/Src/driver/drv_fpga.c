//=============================================================================
/**
 *
 * VA-300�v���O����
 * <<FPGA�h���C�o���W���[��>>
 *
 *	@brief FPGA�ւ̃A�N�Z�X�h���C�o�BLED�ȂǌŗL�̂��̂͏��O�B
 *	
 *	@file drv_fpga.c
 *	
 *	@date	2012/08/31
 *	@version 1.00 �V�K�쐬
 *	@author	F.Saeki
 *
 *	Copyright (C) 2012, OYO Electric Corporation
 */
//=============================================================================
#define	_DRV_FPGA_C_
#include "kernel.h"
#include "va300.h"
#include "id.h"
#include "drv_fpga.h"
#include "err_ctrl.h"
#include "sh7750.h"

// ��`


// �O���ϐ�


// �v���g�^�C�v�錾


// �ϐ��錾
static ID s_idTsk;						// �N���^�X�NID

//=============================================================================
/**
 * FPGA�֘A�̏�����
 *
 * @retval E_OK ����������
 */
//=============================================================================
ER FpgaInit(void)
{
	ER ercd;
	
	// FPGA�̃��W�X�^������
	ercd = FpgaRegInit();
	
	// FPGA�����ݐݒ�(���ꂼ��̃h���C�o�Őݒ肷�邩��)

	
	return ercd;
}

//=============================================================================
/**
 * FPGA���W�X�^�̏�����
 *
 * @retval E_OK ����
 */
//=============================================================================
ER FpgaRegInit(void)
{
	ER ercd;

	ercd = E_OK;
	
	return ercd;
}

//=============================================================================
/**
 * FPGA�̃o�[�W�����擾
 *
 * @param puhMajor ���W���[�o�[�W�����̊i�[��
 * @param puhMinor �}�C�i�[�o�[�W�����̊i�[��
 * @param puhRelease �����[�X�o�[�W�����̊i�[��
 */
//=============================================================================
void FpgaGetVer(UH* puhMajor, UH* puhMinor, UH* puhRelease)
{
	ER	ercd;
	
	ercd = twai_sem(SEM_FPGA, 1000/MSEC);
	if (ercd != E_OK) {
		return;
	}
	*puhMajor   = (fpga_inw(FPGA_VER) >> 12) & 0x000F;
	*puhMinor   = (fpga_inw(FPGA_VER) >> 4) & 0x00FF;
	*puhRelease = fpga_inw(FPGA_VER) & 0x000F;

	sig_sem(SEM_FPGA);
}

//=============================================================================
/**
 * FPGA�ɒl��ݒ肷��
 *
 * @param iReg ���W�X�^�A�h���X
 * @param uhVal �ݒ�l
 * @param uhMask �}�X�N(bit on�ŋ���)
 * @retval E_OK ������OK
 */
//=============================================================================
ER FpgaSetWord(int iReg, UH uhVal, UH uhMask)
{
	ER	ercd;
	UH	uhRdVal;
	
	// �Z�}�t�H�l��
	ercd = twai_sem(SEM_FPGA, 1000/MSEC);
	if (ercd == E_OK) {
		uhRdVal = fpga_inw(iReg);			// ���݂̒l��ǂݍ���
		fpga_outw(iReg, ((uhRdVal & ~uhMask) | (uhVal & uhMask)));
		sig_sem(SEM_FPGA);					// �Z�}�t�H�ԋp
		
		return ercd;
	}

	return ercd;
}

/*==========================================================================*/
/**
 * �摜�f�[�^��1���C���擾
 *
 *	@param iBank �L���v�`���[�̈�ԍ�
 *	@param iX X���W(8�Ŋ���؂��l���w��)
 *	@param iY Y���W
 *	@param iSize �w��r�b�g��(8�Ŋ���؂��T�C�Y)
 *	@param p �i�[��A�h���X
 */
/*==========================================================================*/
void FpgaCapLineGet(int iBank, int iX, int iY, int iSize, UB *p)
{
	UW uwOfs;
	UB *dp;
	
	uwOfs = iX + iY * CAP_X_SIZE;
	dp = (UB*)&cap_dat(iBank, uwOfs);
	
	while(iSize) {
		*p = *dp;
		iSize--;
		p++;
		dp++;
	}
}

/*==========================================================================*/
/**
 * �ԊO��LED��Duty�ݒ�
 *
 *	@param ubSel �ԊO��LED�̎w��
 *	@param ubVal �ݒ�l(0�`255)
 */
/*==========================================================================*/
void FpgaIrLedSet(UB ubSel, UB ubVal)
{
	const UB ubIrLed[] = {
		IRLED_CRL_LED1,					// �ԊO��LED1
		IRLED_CRL_LED2,					// �ԊO��LED2
		IRLED_CRL_LED3,					// �ԊO��LED3
		IRLED_CRL_LED4,					// �ԊO��LED4
		IRLED_CRL_LED5,					// �ԊO��LED5
	};
	int nIrLed = sizeof ubIrLed / sizeof ubIrLed[ 0 ];
	int i;
	
	for(i = 0;i < nIrLed;i++) {
		if (ubSel & ubIrLed[ i ]) {
			fpga_outw((IRLED_DUTY + (i << 1)), ubVal);
		}
	}
}

//=============================================================================
/**
 * FPGA�̃��W�X�^�f�[�^�𕶎���ŕԂ�
 *
 * @param iLine �s
 * @param p �f�[�^�i�[��
 * @return ������
 */
//=============================================================================
int FpgaRegDisplayLine(int iLine, char* p)
{
	static const struct {				// �\������FPGA�̃��W�X�^
		char *pcRegName;
		int  iAdr;
		int  iSize;
	} stReg[] = {
		{"FPGA_VER ", FPGA_VER, 2},
		{"SENS_MON ", SENS_MON, 2},
		{"AUX_OUT  ", AUX_OUT,  2},
		{"AUX_IN   ", AUX_IN,   2},
		{"TPL_CRL  ", TPL_CRL,  2},
		{"TPL_CBYTE ", TPL_CBYTE, 2},
		{"TPL_DBYTE ", TPL_DBYTE, 2},
		{"FROM_BANK ", FROM_BANK, 2},
		{"INT_CRL  ", INT_CRL,   2},
		{"LED_CRL  ", LED_CRL,   2},
		{"BUZ_CRL  ", BUZ_CRL,   2},
		{"BUZ1_CYC ", BUZ1_CYC,  2},
		{"SEG1_CRL ", SEG1_CRL,  2},
		{"SEG2_CRL ", SEG2_CRL,  2},
		{"SEG3_CRL ", SEG3_CRL,  2},
		{"SEG4_CRL ", SEG4_CRL,  2},
		{"SEG5_CRL ", SEG5_CRL,  2},
		{"SEG6_CRL ", SEG6_CRL,  2},
		{"SEG7_CRL ", SEG7_CRL,  2},
		{"SEG8_CRL ", SEG8_CRL,  2},
		{"KEY_IN   ", KEY_IN,    2},
		{"IRLED_CRL ", IRLED_CRL, 2},
		{"IRLED1_DUTY ", IRLED1_DUTY, 2},
		{"IRLED2_DUTY ", IRLED2_DUTY, 2},
		{"IRLED3_DUTY ", IRLED3_DUTY, 2},
		{"IRLED4_DUTY ", IRLED4_DUTY, 2},
		{"IRLED5_DUTY ", IRLED5_DUTY, 2},
		{"CNF_ROM  ", CNF_ROM, 2},
		{"CMR_CAP  ", CMR_CAP, 2},
		{"CMR_PRM_AES ", CMR_PRM_AES, 2},
		{"CMR_PRM_SHT ", CMR_PRM_SHT, 2},
		{"CMR_MOD_WAIT ", CMR_MOD_WAIT, 2},
		{"CMR_CRL  ", CMR_CRL, 2},
		{"CMR_BAU  ", CMR_BAU, 2},
		{"CMR_CSZ  ", CMR_CSZ, 2},
		{"CMR_CMON ", CMR_CMON, 2},
		{"CMR_RSZ  ", CMR_RSZ, 2},
		{"CMR_RMON ", CMR_RMON, 2},
		{"CMR_CSZ  ", CMR_CSZ, 2},

		{"CMR_CMD  ", CMR_CMD, 2},
		{"CMR_RES  ", CMR_RES, 2},
		
	};
	static const int nReg = (sizeof stReg) / (sizeof stReg[0]);
	int	 iCnt;
	
	iCnt = 0;
	
	if (iLine < nReg) {
		if (stReg[ iLine ].iSize == 4) {				// LONG�T�C�Y�̂Ƃ�
			iCnt += _sprintf(p, "%s(%08X): %08X ", stReg[ iLine ].pcRegName, 
								(FPGA_BASE + stReg[ iLine ].iAdr), fpga_inl(stReg[ iLine ].iAdr));
		} else {
			iCnt += _sprintf(p, "%s(%08X):   %04X ", stReg[ iLine ].pcRegName, 
								(FPGA_BASE + stReg[ iLine ].iAdr), fpga_inw(stReg[ iLine ].iAdr));
		}
	}
	return iCnt;
}


/*==========================================================================*/
/**
 * �摜�f�[�^��1���C���擾
 *
 *	@param iBank �L���v�`���[�̈�ԍ�
 *	@param iX X���W(8�Ŋ���؂��l���w��)
 *	@param iY Y���W
 *	@param iSize �w��r�b�g��(8�Ŋ���؂��T�C�Y)
 *	@param p �i�[��A�h���X
 */
/*==========================================================================*/
void FpgaWrtDat(int iBank, int iX, int iY, int iSize, UB *p)
{
	int i;
	
	for(i = 0 ; i < iSize ; i++){
		wrt_dat(iBank, 0x04000000 + i , *p++);
	}
	
/*
	UW uwOfs;
	
	uwOfs = iX + iY * CAP_X_SIZE;
	
	while(iSize) {
		wrt_dat(iBank, uwOfs, *p);
		iSize--;
		p++;
	}
*/
}