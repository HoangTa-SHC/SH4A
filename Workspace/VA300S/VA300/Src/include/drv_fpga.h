/**
*	VA-300�v���O����
*
*	@file drv_fpga.h
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/8/31
*	@brief  FPGA�֘A��`���(�쐬��)
*
*	Copyright (C) 2011, OYO Electric Corporation
*/
#ifndef	_DRV_FPGA_H_
#define	_DRV_FPGA_H_

/// @name �^��`
//@{
typedef unsigned short FPGA_U16;				// 
typedef unsigned long  FPGA_U32;				// 

enum BIT_CRL {									///< �r�b�g����
	BIT_CRL_OFF,								///< �r�b�gOFF
	BIT_CRL_ON									///< �r�b�gON
};

enum IO_ON_OFF {								///< I/O ��ON/OFF��`
	IO_OFF,										///< OFF
	IO_ON										///< ON
};

enum CMR_BAU {									///< �J�����ʐMI/F�{�[���[�g
	CMR_BAU4800  = 0x00,						///< 4800bps
	CMR_BAU9600,								///< 9600bps
	CMR_BAU19200,								///< 19200bps
	CMR_BAU38400,								///< 38400bps
	CMR_BAU57600,								///< 57600bps
	CMR_BAU115200,								///< 115200bps
	CMR_BAU2400  = 0x0F,						///< 2400bps
};

//@}
/// @name ��`
//@{
#define	CAP_X_SIZE		1280					///< X�T�C�Y
#define	CAP_Y_SIZE		720						///< Y�T�C�Y
//@}
/// @name ���W�X�^��`
//@{
#define	FPGA_BASE		0xB0000000				///< FPGA���W�X�^�̃x�[�X�A�h���X
#define	CAP_BASE		0xB4000000				///< �摜�f�[�^�̃x�[�X�A�h���X

// ���W�X�^���ڃA�N�Z�X
#define	fpga_inw(n)		(*(volatile FPGA_U16 *)(FPGA_BASE + (n)))			///< FPGA���W�X�^��16bit�Ǐo��
#define	fpga_outw(n,c)	(*(volatile FPGA_U16 *)(FPGA_BASE + (n)) = (c))		///< FPGA���W�X�^��16bit������
#define	fpga_inl(n)		(*(volatile FPGA_U32 *)(FPGA_BASE + (n)))			///< FPGA���W�X�^��32bit�Ǐo��
#define	fpga_outl(n,c)	(*(volatile FPGA_U32 *)(FPGA_BASE + (n)) = (c))		///< FPGA���W�X�^��32bit������
#define	fpga_setw(n,c)	(*(volatile FPGA_U16 *)(FPGA_BASE + (n)) |= (c))	///< FPGA���W�X�^��bit�Z�b�g(16bit)
#define	fpga_clrw(n,c)	(*(volatile FPGA_U16 *)(FPGA_BASE + (n)) &= ~(c))	///< FPGA���W�X�^��bit�N���A(16bit)
#define	fpga_setl(n,c)	(*(volatile FPGA_U32 *)(FPGA_BASE + (n)) |= (c))	///< FPGA���W�X�^��bit�Z�b�g(32bit)
#define	fpga_clrl(n,c)	(*(volatile FPGA_U32 *)(FPGA_BASE + (n)) &= ~(c))	///< FPGA���W�X�^��bit�N���A(32bit)
#define	cap_dat(b,n)	(*(volatile UB *)(CAP_BASE + (b) * 0x00100000 + (n)))	///< �摜�f�[�^�Ǐo��
#define	trim_dat(b,n)	(cap_dat((b+4),n))		///< �g���~���O�摜�f�[�^�Ǐo��
#define	rsz_dat(b,n)	(cap_dat((b+8),n))		///< ���k�摜�f�[�^�Ǐo��
//#define	wrt_dat(b,n,c)	(*(volatile UB *)(CAP_BASE + (b) * 0x00100000 + (n)) = (c))	///< �摜�f�[�^�Ǐo��
#define	wrt_dat(b,n,c)	(*(volatile UB *)(FPGA_BASE + (n)) = (c))	///< �摜�f�[�^�Ǐo��
// ���
#define	FPGA_VER		0x0000000				///< FPGA�o�[�W�������W�X�^
#define	SENS_MON		0x0000002				///< ���Ԍ��m�Z���T�̏�Ԃ����j�^�[����
#define	AUX_OUT			0x0000004				///< AUX OUT�̐���
#define	AUX_IN			0x0000006				///< AUX IN�̏��
#define	TPL_CRL			0x0000008				///< �^�b�`�p�l���R���g���[��
#define	TPL_CBYTE		0x000000A				///< �^�b�`�p�l���R���g���[���ւ�CONTROL BYTE���W�X�^
#define	TPL_DBYTE		0x000000C				///< �^�b�`�p�l���R���g���[���ւ�DATA BYTE���W�X�^

#define	FROM_BANK		0x0000010				///< �t���b�V���������̃o���N�I��
#define	INT_CRL			0x0000012				///< �����ݐ���
#define	LED_CRL			0x0000014				///< LED����
#define	BUZ_CRL			0x0000016				///< �u�U�[����
#define	BUZ1_CYC		0x0000018				///< �u�U�[�����ݒ�

#define	SEG_CRL			0x0000020				///< 7SEG�o��
#define	SEG1_CRL		0x0000020				///< 7SEG1�o��(�ŏ��)
#define	SEG2_CRL		0x0000022				///< 7SEG2�o��
#define	SEG3_CRL		0x0000024				///< 7SEG3�o��
#define	SEG4_CRL		0x0000026				///< 7SEG4�o��
#define	SEG5_CRL		0x0000028				///< 7SEG5�o��
#define	SEG6_CRL		0x000002A				///< 7SEG6�o��
#define	SEG7_CRL		0x000002C				///< 7SEG7�o��
#define	SEG8_CRL		0x000002E				///< 7SEG8�o��(�ŉ���)

#define	KEY_IN			0x0000030				///< �L�[�{�[�h
#define	IRLED_CRL		0x0000040				///< �ԊO��LED
#define	IRLED_DUTY		0x0000042				///< �ԊO��LED��Duty
#define	IRLED1_DUTY		0x0000042				///< �ԊO��LED1��Duty
#define	IRLED2_DUTY		0x0000044				///< �ԊO��LED2��Duty
#define	IRLED3_DUTY		0x0000046				///< �ԊO��LED3��Duty
#define	IRLED4_DUTY		0x0000048				///< �ԊO��LED4��Duty
#define	IRLED5_DUTY		0x000004A				///< �ԊO��LED5��Duty

#define	CNF_ROM			0x0000050				///< FPGA�R���t�B�O���[�V����������
#define FPGA_RECONF		0x000005E				///< FPGA���R���t�B�O���[�V����������

// GCT�ЁE�J�����֘A 
#define	CMR_CAP			0x0000100				///< �J�����L���v�`���[����
#define	CMR_PRM_AES		0x0000102				///< �J������AES����̒l
#define	CMR_PRM_SHT		0x0000104				///< �J������Fix Shutter Control�̒l
#define	CMR_MOD_WAIT	0x0000106				///< �J�����̉摜��荞�ݎ�Mode4��Wait����
#define	CMR_WAKEUP_WAIT	0x0000108				///< �J������WakeUp���Wait����	.... �ǉ��@�i��@2013.4.12
#define	CMR_CRL			0x0000200				///< �J������Serial I/F����
#define	CMR_BAU			0x0000202				///< �J�����ʐM�̃{�[���[�g
#define	CMR_CSZ			0x0000204				///< �J�����̃R�}���h�p�P�b�g�T�C�Y
#define	CMR_CMON		0x0000206				///< �J�����̃R�}���h�ʐM��ԊĎ�
#define	CMR_RSZ			0x0000208				///< �J�����̉����p�P�b�g�T�C�Y
#define	CMR_RMON		0x000020A				///< �J�����̉����ʐM��ԊĎ�

#define	CMR_CMD			0x0000400				///< �J�����̃R�}���h�f�[�^
#define	CMR_RES			0x0000600				///< �J�����̉����f�[�^
// GCT�ЁE�J�����֘A end.

#define	TRIM_CRL		0x0000210				///< �摜�g���~���O����
#define	TRIM_BNK		0x0000212				///< �摜�g���~���O�̉摜�I���A�i�[�ꏊ�w��
#define	TRIM_ST_X		0x0000214				///< �摜�g���~���O�J�nX���W
#define	TRIM_ST_Y		0x0000216				///< �摜�g���~���O�J�nY���W
#define	TRIM_SZ_X		0x0000218				///< �摜�g���~���OX�T�C�Y
#define	TRIM_SZ_Y		0x000021A				///< �摜�g���~���OY�T�C�Y

#define	RSZ_CRL			0x0000220				///< �摜���k����
#define	RSZ_BNK			0x0000222				///< �摜���k�̑I���y�ъi�[�ꏊ�w��
#define	RSZ_SZ_X		0x0000224				///< �摜���k��X�T�C�Y
#define	RSZ_SZ_Y		0x0000226				///< �摜���k��Y�T�C�Y

// NC�ЁENCM03-V�J�����֘A	/* Added T.N 2016.3.8 */
#define NCMR_SERAL_CTRL	0x0000300				///< NC�ЃJ���� I2C�R���g���[��	�pfpga���W�X�^�E�A�h���X
#define NCMR_SERAL_ADDR	0x0000302				///< NC�ЃJ���� I2C�A�h���X�pfpga���W�X�^�E�A�h���X
#define NCMR_SERAL_WDATA	0x0000304			///< NC�ЃJ���� I2C���C�g�E�f�[�^�pfpga���W�X�^�E�A�h���X
#define NCMR_SERAL_RDATA	0x0000306			///< NC�ЃJ���� I2C���[�h�E�f�[�^�pfpga���W�X�^�E�A�h���X

#define NCMR_CRL_SND 	0x0001					///< �J����I2C���M�J�n�v��
#define NCMR_CRL_RCV 	0x0002					///< �J����I2C��M�J�n�v��

#define NCMR_CAP_CTR	0x0000100				///< �J�����E�L���v�`���E�R���g���[���pfpga���W�X�^�E�A�h���X
#define NCMR_CAP_STA	0x0001					///< �L���v�`���J�n�v���@=0:�I���A=1:�J�n
#define NCMR_CAP_SEL	0x0002					///< �摜�i�[�G���A��ʁ@=0:�g���~���O�G���A�A=1:�L���v�`���G���A(�S���640x480)
#define NCMR_CAP_BNK	0x0300					///< �摜�i�[�o���N�I��  =(00):CapturBank0 ���� TrimArea0
												///< 					 =(01):CapturBank1 ���� TrimArea1
												///< 					 =(10):CapturBank2 ���� TrimArea2
												///< 					 =(11):CapturBank3 ���� TrimArea3
#define NCMR_CAP_WAIT	0x7000					///< �摜�捞�ݎ��̐擪�p���t���[����:0�`7
// NC�ЁENCM03-V�J�����֘A end.	/* Added END */


#define	CMR_HGM			0x0001000				///< �J�����摜�̃q�X�g�O����
//@}

/// @name ���W�X�^�r�b�g��`
//@{
// �r�b�g��`
#define	SENS_MON_ON		0x0001					///< �w�̑}�����

#define	AUX_OUT_1		0x0001					///< AUX OUT1
#define	AUX_OUT_2		0x0002					///< AUX OUT2

#define	AUX_IN_A0		0x0001					///< AUX IN A0
#define	AUX_IN_A1		0x0002					///< AUX IN A1
#define	AUX_IN_B0		0x0004					///< AUX IN B0
#define	AUX_IN_B1		0x0008					///< AUX IN B1

#define	TPL_CRL_WRC		0x0001					///< �^�b�`�p�l����CONTROLL BYTE�ݒ�
#define	TPL_CRL_WRD		0x0002					///< �^�b�`�p�l����DATA BYTE�ݒ�
#define	TPL_CRL_RD		0x0004					///< �^�b�`�p�l���ɓǏo���ݒ�

#define	INT_CRL_LAN		0x0001					///< LAN�̊����݋���
#define	INT_CRL_SENS	0x0002					///< �^�b�`�Z���T�̊����݋���
#define	INT_CRL_TPL		0x0004					///< �^�b�`�p�l���̊����݋���
#define	INT_CRL_LCD		0x0008					///< LCD�̊����݋���
#define	INT_CRL_CAP		0x0010					///< �J�����̃L���v�`�������݋���
#define	INT_CRL_KEY		0x0020					///< 10�L�[�̊����݋���
#define	INT_CRL_AUXIN	0x0040					///< AUXIN�̊����݋���
#define	INT_CRL_TPL_CMD	0x0080					///< �^�b�`�p�l���̃R�}���h�����݋���
#define	INT_CRL_CMR_CMD	0x0100					///< �J�����̃R�}���h�����݋���
#define	INT_CRL_CMR_RES	0x0200					///< �J�����̎�M�����݋���

#define	LED_CRL_PWR		0x0001					///< �ΐFLED
#define	LED_CRL_OK		0x0002					///< ��FLED
#define	LED_CRL_NG		0x0004					///< �ԐFLED

#define	BUZ_CRL_BZ1		0x0001					///< �u�U�[1����
#define	BUZ_CRL_BZ2		0x0002					///< �u�U�[2����

#define	KEY_IN_0		0x0001					///< '0'�̃L�[
#define	KEY_IN_1		0x0002					///< '1'�̃L�[
#define	KEY_IN_2		0x0004					///< '2'�̃L�[
#define	KEY_IN_3		0x0008					///< '3'�̃L�[
#define	KEY_IN_4		0x0010					///< '4'�̃L�[
#define	KEY_IN_5		0x0020					///< '5'�̃L�[
#define	KEY_IN_6		0x0040					///< '6'�̃L�[
#define	KEY_IN_7		0x0080					///< '7'�̃L�[
#define	KEY_IN_8		0x0100					///< '8'�̃L�[
#define	KEY_IN_9		0x0200					///< '9'�̃L�[
#define	KEY_IN_ENTER	0x0400					///< �ďo�L�[
#define	KEY_IN_ASTERISK	0x0800					///< '*'�L�[
#define	KEY_IN_CLR		0x1000					///< ����L�[
#define	KEY_IN_NONE		0x0000					///< ����������Ă��Ȃ�

#define	IRLED_CRL_LED1	0x0001					///< IR LED1
#define	IRLED_CRL_LED2	0x0002					///< IR LED2
#define	IRLED_CRL_LED3	0x0004					///< IR LED3
#define	IRLED_CRL_LED4	0x0008					///< IR LED4
#define	IRLED_CRL_LED5	0x0010					///< IR LED5

#define	CMR_CAP_CRL		0x0001					///< �J�����L���v�`������
#define	CMR_CAP_MOD		0x0010					///< �J�����L���v�`�����[�h
#define	CMR_CAP_BANK	0x0100					///< �J�����L���v�`���o���N
#define	CMR_CAP_ERR		0x8000					///< �J�����G���[�t���O

#define	CMR_MOD_0		0x0000					///< �J�������[�h0
#define	CMR_MOD_1		0x0001					///< �J�������[�h1
#define	CMR_MOD_2		0x0002					///< �J�������[�h2
#define	CMR_MOD_3		0x0003					///< �J�������[�h3
#define	CMR_MOD_4		0x0004					///< �J�������[�h4

#define	CMR_BANK_0		0x0000					///< �J�����o���N0
#define	CMR_BANK_1		0x0001					///< �J�����o���N1
#define	CMR_BANK_2		0x0002					///< �J�����o���N2
#define	CMR_BANK_3		0x0003					///< �J�����o���N3

#define	TRIM_CRL_ST		0x0001					///< �摜�g���~���O�J�n
#define	TRIM_BNK_ORG	0x0001					///< �g���~���O�̉摜�I��
#define	TRIM_BNK_DST	0x0004					///< �g���~���O�f�[�^�̊i�[�ꏊ

#define	RSZ_CRL_ST		0x0001					///< ���T�C�Y�J�n
#define	RSZ_CRL_MOD		0x0002					///< ���T�C�Y�̃��[�h
#define	RSZ_BNK_ORG		0x0001					///< ���T�C�Y�̉摜�I��
#define	RSZ_BNK_DST		0x0004					///< ���T�C�Y�f�[�^�̊i�[�ꏊ

#define	CMR_CRL_ACT		0x0001					///< �J�����ʐM����

//@}

/// ���̑���`

// EXTERN�̒�`
#if defined(EXTERN)
#undef EXTERN
#endif

#if defined(_DRV_FPGA_C_)
#define	EXTERN
#else
#define	EXTERN	extern
#endif
/// @name �v���g�^�C�v�錾
//@{
EXTERN ER FpgaInit(void);								///< FPGA������
EXTERN ER FpgaSetWord(int iReg, UH uhVal, UH uhMask);	///< FPGA�ɒl�ݒ�
EXTERN void FpgaCapLineGet(int iBank, int iX, int iY, int iSize, UB *p);	///< �摜�f�[�^��1���C���擾
EXTERN void FpgaIrLedSet(UB ubSel, UB ubVal);			///< �ԊO��LED��Duty��ݒ�
EXTERN int FpgaRegDisplayLine(int iLine, char* p);		///< FPGA���W�X�^���
EXTERN void FpgaWrtDat(int iBank, int iX, int iY, int iSize, UB *p);	//Miya
//@}
#endif									/* end of _DRV_FPGA_H_				*/
