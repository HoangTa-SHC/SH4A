/**
*	VA-300 �Z���T�[���v���O����
*
*	@file drv_cmr.h
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/09/24
*	@brief  �J������`���
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
#ifndef	_DRV_CMR_H_
#define	_DRV_CMR_H_

// ��`
enum CMR_CMD_DEF {						///< �J�����R�}���h
	CMR_CMD_REQUEST_RESPONSE = 0x10,	///< Request Response
	CMR_CMD_SET_ID           = 0x19,	///< Set ID
	CMR_CMD_GET_PARAMETERS   = 0x20,	///< Get Parameters
	CMR_CMD_LOAD_PARAMETERS  = 0x27,	///< Load Parameters
};

enum CAP_MODE {							///< �L���v�`���[���[�h
	CAP_MODE_0 = CMR_MOD_0,				///< ���[�h0
	CAP_MODE_1 = CMR_MOD_1,				///< ���[�h1
	CAP_MODE_2 = CMR_MOD_2,				///< ���[�h2
	CAP_MODE_3 = CMR_MOD_3,				///< ���[�h3
	CAP_MODE_4 = CMR_MOD_4,				///< ���[�h4
};

enum CAP_BANK {							///< �L���v�`���[�̈�̃o���N�w��
	CAP_BANK_0 = CMR_BANK_0,			///< BANK0
	CAP_BANK_1 = CMR_BANK_1,			///< BANK1
	CAP_BANK_2 = CMR_BANK_2,			///< BANK2
	CAP_BANK_3 = CMR_BANK_3,			///< BANK3
};

enum TRIM_BANK {						///< �g���~���O�摜�̊i�[��
	TRIM_BANK_0 = CMR_BANK_0,			///< BANK0
	TRIM_BANK_1 = CMR_BANK_1,			///< BANK1
	TRIM_BANK_2 = CMR_BANK_2,			///< BANK2
	TRIM_BANK_3 = CMR_BANK_3,			///< BANK3
};

enum RSZ_BANK {							///< ���k�摜�̊i�[��
	RSZ_BANK_0 = CMR_BANK_0,			///< BANK0
	RSZ_BANK_1 = CMR_BANK_1,			///< BANK1
	RSZ_BANK_2 = CMR_BANK_2,			///< BANK2
	RSZ_BANK_3 = CMR_BANK_3,			///< BANK3
};

enum RSZ_MODE {							///< ���k�摜�̏k����
	RSZ_MODE_0 = CMR_BANK_0,			///< ��1/2
	RSZ_MODE_1 = CMR_BANK_1,			///< ��1/4
	RSZ_MODE_2 = CMR_BANK_2,			///< ��1/8
};

// �擾�摜�T�C�Y�̒�`
#define	GET_IMG_SIZE_X	640				///< �擾�摜X�T�C�Y
#define	GET_IMG_SIZE_Y	360				///< �擾�摜Y�T�C�Y

// �}�N����`
// NC�ЁENCM03-V�J�����֘A	/* Added T.N 2016.3.8 */
#define	NCmrAddesWrite( n )		fpga_outw( NCMR_SERAL_ADDR, n )		///< I2C���M�J�����E�A�h���X�̏�����
#define	NCmrSendDataWrite( n )	fpga_outw( NCMR_SERAL_WDATA, n )	///< I2C���M�J�����E�f�[�^�̏�����
#define	IsNCmrSendEnd()  		( fpga_inw( NCMR_SERAL_CTRL ) & NCMR_CRL_SND )	///< NCameraI2C���M���̊m�F�@=0:�I���A=1:���M��
#define	NCmrSendStart()			fpga_setw( NCMR_SERAL_CTRL, NCMR_CRL_SND )		///< NCameraI2C���M�J�n�v��
#define	NCmrRcvDataRead()		fpga_inw( NCMR_SERAL_RDATA & 0xffff )			///< I2C��M�J�����E�f�[�^�̓Ǎ���
#define	IsNCmrReadEnd()  		( fpga_inw( NCMR_SERAL_CTRL ) & NCMR_CRL_RCV )	///< NCameraI2C��M���̊m�F�@=0:�I���A=1:���M��
#define	NCmrReadStart()			fpga_setw( NCMR_SERAL_CTRL, NCMR_CRL_RCV )		///< NCameraI2C��M�J�n�v��

//#define NCmrCapStart()			fpga_outw( NCMR_CAP_CTR, ( fpga_inw( NCMR_CAP_CTR ) | NCMR_CAP_STA ) )	///< �L���v�`���J�n�v��
//#define NCmrCapStart()			fpga_setw( NCMR_CAP_CTR, NCMR_CAP_STA ) 	///< �L���v�`���J�n�v��
#define IsNCmrCap()				( fpga_inw( NCMR_CAP_CTR ) & NCMR_CAP_STA )		///< �L���v�`���摜������ԁ@=0:�I���A=1:������
#define NCmrCapAreaSel( n )		fpga_outw( NCMR_CAP_CTR, ( fpga_inw( NCMR_CAP_CTR ) & ~( NCMR_CAP_SEL ) ) | ( ( n << 1 ) & NCMR_CAP_SEL ) )	///< �摜�i�[�G���A��ʁ@=0:�g���~���O�G���A�A=1:�L���v�`���G���A(�S���640x480)
#define NCmrCapBnkSel( n )		fpga_outw( NCMR_CAP_CTR, ( fpga_inw( NCMR_CAP_CTR ) & ~( NCMR_CAP_BNK ) ) | ( ( n << 8 ) & NCMR_CAP_BNK ) )
																	///< �摜�i�[�o���N�I��  =0:CapturBank0 ���� TrimArea0
																	///< 					 =1:CapturBank1 ���� TrimArea1
																	///< 					 =2:CapturBank2 ���� TrimArea2
																	///< 					 =3:CapturBank3 ���� TrimArea3
#define NCmrCapWaitSet( n )		fpga_outw( NCMR_CAP_CTR, ( fpga_inw( NCMR_CAP_CTR ) & ~( NCMR_CAP_WAIT ) ) | ( ( n << 12 ) & NCMR_CAP_WAIT ) )
																	///< �摜�捞�ݎ��̐擪�p���t���[����:0�`7

// NCM03-V�J�����֘A end.	/* Added END */

// GCT�ЁE�J�����֘A
#define	CmrPrmAesSet(n)		fpga_outw(CMR_PRM_AES, n)		///< �p�����[�^AES�ݒ�
#define	CmrPrmShutterSet(n)	fpga_outw(CMR_PRM_SHT, n)		///< �p�����[�^Fix Shutter Control�ݒ�
#define	CmrWaitCrl(n)		fpga_outw(CMR_MOD_WAIT, n)		///< �摜��荞�ݎ�Mode4��Wait�ݒ�
#define	CmrWakeUpWaitCrl(n)		fpga_outw(CMR_WAKEUP_WAIT, n)	///< �J����WakeUp����Wait�ݒ�.... �ǉ��@�i��@2013.4.12
#define	CmrBaudrateSet(n)	fpga_outw(CMR_BAU, n)			///< �{�[���[�g�̐ݒ�
#define	CmrHistogramGet(b,n)	fpga_inl((CMR_HGM + b * 0x400 + n * 4))		///< �q�X�g�O�����f�[�^
#define	CmrCmdPktSend()		fpga_setw(CMR_CRL, CMR_CRL_ACT)	///< �R�}���h���M
#define	CmrCmdPktSize(n)	fpga_outw(CMR_CSZ, n)			///< ���M�R�}���h�T�C�Y�̐ݒ�
#define	CmrRecvPktSize(n)	fpga_outw(CMR_RSZ, n)			///< ��M�p�P�b�g�T�C�Y�̐ݒ�
#define	CmrSendDataWrite(n,v)	fpga_outw((CMR_CMD + (n << 1)), v)	///< ���M�f�[�^�̏�����
#define	CmrRecvDataRead(n)	fpga_inw((CMR_RES + (n << 1)))	///< ��M�f�[�^�̓Ǎ���
#define	CmrCapMode(n)		fpga_outw(CMR_CAP, ((fpga_inw(CMR_CAP) & ~(CMR_CAP_MOD  * 0x07)) | (n * CMR_CAP_MOD)))	///< �L���v�`���[���[�h�ݒ�
#define	CmrCapBank(n)		fpga_outw(CMR_CAP, ((fpga_inw(CMR_CAP) & ~(CMR_CAP_BANK * 0x03)) | (n * CMR_CAP_BANK)))	///< �L���v�`���[�o���N�ݒ�
#define	IsCmrCapErr()		((fpga_inw(CMR_CAP) & CMR_CAP_ERR) == CMR_CAP_ERR)	///< �L���v�`���[�G���[�������H
#define	CmrCapErrClr()		fpga_clrw(CMR_CAP, CMR_CAP_ERR)	///< �L���v�`���[�G���[�N���A
#define	IsCmrCapStart()		((fpga_inw(CMR_CAP) & CMR_CAP_CRL) == CMR_CAP_CRL)	///< �L���v�`���[���s���H
#define	CmrCapStart()		fpga_setw(CMR_CAP, CMR_CAP_CRL)	///< �L���v�`���[�J�n
// GCT�ЁE�J�����֘A end.

#define	CmrTrimOrg(n)		fpga_outw(TRIM_BNK, ((fpga_inw(TRIM_BNK) & ~(TRIM_BNK_ORG * 0x03)) | (n * TRIM_BNK_ORG)))	///< �g���~���O�̉摜�I��
#define	CmrTrimDist(n)		fpga_outw(TRIM_BNK, ((fpga_inw(TRIM_BNK) & ~(TRIM_BNK_DST * 0x03)) | (n * TRIM_BNK_DST)))	///< �g���~���O�摜�i�[��I��
#define	CmrTrimStartX(n)	fpga_outw(TRIM_ST_X, n)			///< �摜�g���~���O�J�nX���W
#define	CmrTrimStartY(n)	fpga_outw(TRIM_ST_Y, n)			///< �摜�g���~���O�J�nY���W
#define	CmrTrimSizeX(n)		fpga_outw(TRIM_SZ_X, n)			///< �摜�g���~���OX�T�C�Y
#define	CmrTrimSizeY(n)		fpga_outw(TRIM_SZ_Y, n)			///< �摜�g���~���OY�T�C�Y
#define	IsCmrTrimStart()	(fpga_inw(TRIM_CRL) & TRIM_CRL_ST)	///< �摜�g���~���O���s��
#define	CmrTrimStart()		fpga_setw(TRIM_CRL, TRIM_CRL_ST)	///< �摜�g���~���O�J�n
#define	CmrTrimSizeXGet()	fpga_inw(TRIM_SZ_X)				///< �摜�g���~���OX�T�C�Y�擾
#define	CmrTrimSizeYGet()	fpga_inw(TRIM_SZ_Y)				///< �摜�g���~���OY�T�C�Y�擾

#define	CmrResizeMode(n)	fpga_outw(RSZ_CRL, ((fpga_inw(RSZ_CRL) & ~(RSZ_CRL_MOD * 0x03)) | (n * RSZ_CRL_MOD)))	///< ���T�C�Y�̉摜�I��
#define	CmrResizeOrg(n)		fpga_outw(RSZ_BNK, ((fpga_inw(RSZ_BNK) & ~(RSZ_BNK_ORG * 0x03)) | (n * RSZ_BNK_ORG)))	///< ���T�C�Y�̉摜�I��
#define	CmrResizeDist(n)	fpga_outw(RSZ_BNK, ((fpga_inw(RSZ_BNK) & ~(RSZ_BNK_DST * 0x03)) | (n * RSZ_BNK_DST)))	///< ���T�C�Y�̉摜�I��
#define	CmrResizeSizeX(n)	fpga_outw(RSZ_SZ_X, n)			///< ���T�C�Y����摜��X�T�C�Y
#define	CmrResizeSizeY(n)	fpga_outw(RSZ_SZ_Y, n)			///< ���T�C�Y����摜��Y�T�C�Y
#define	IsCmrResizeStart()	(fpga_inw(RSZ_CRL) & RSZ_CRL_ST)	///< �摜���T�C�Y���s��
#define	CmrResizeStart()	fpga_setw(RSZ_CRL, RSZ_CRL_ST)	///< �摜���T�C�Y�J�n

// EXTERN�錾�̒�`
#ifdef EXTERN
#undef EXTERN
#endif

#if defined(_DRV_CMR_C_)
#define	EXTERN
#else
#define	EXTERN extern
#endif

// �摜�擾�`�F�b�N�p�o�b�t�@
EXTERN UB g_ubCapTest[ CAP_X_SIZE * CAP_Y_SIZE ];	///< �摜�擾�`�F�b�N�p�o�b�t�@

// �v���g�^�C�v�錾
EXTERN ER CmrInit(ID idSem, ID idRcvTsk);		///< �J�����֘A������
EXTERN ER CmrPktSend(UB *pData, int iSendSize, int iRcvSize);	///< �J�����ʐMI/F ���M
EXTERN ER CmrPktRecv(UB *pData, int iRcvSize, TMO tmout);	///< �J�����ʐMI/F ��M
EXTERN ER CmrCapture(enum CAP_MODE eMode, enum CAP_BANK eBank, TMO tmout);	///< �J�����摜�̃L���v�`���[
EXTERN ER CmrCapture2(enum CAP_MODE eMode, enum CAP_BANK eBank, int iStartX, int iStartY, int iSizeX, int iSizeY, TMO tmout);
EXTERN ER CmrCapGet(enum CAP_BANK eBank, int iX, int iY, int iXsize, int iYsize, UB *p);	///< �J�����摜�̎擾
EXTERN ER CmrTrim(enum CAP_BANK eOrg, enum TRIM_BANK eDist, int iStartX, int iStartY, int iSizeX, int iSizeY, TMO tmout); ///< �摜�̃g���~���O
EXTERN ER CmrTrimGet(enum TRIM_BANK eBank, long lStart, long lSize, UB *p);	///< �g���~���O�摜�̎擾
EXTERN ER CmrResize(enum TRIM_BANK eOrg, enum RSZ_BANK eDist, enum RSZ_MODE eMode, int iSizeX, int iSizeY, TMO tmout);		///< �摜���k
EXTERN ER CmrResizeGet(enum RSZ_BANK eBank, long lStart, long lSize, UB *p);
EXTERN ER CmrCmdTest( void );
EXTERN ER CmrMemWrt(enum CAP_BANK eBank, int iX, int iY, int iXsize, int iYsize, UB *p);
#endif										/* end of _DRV_CMR_H_				*/
