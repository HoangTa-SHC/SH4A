/**
*	VA-300�v���O����
*
*	@file id.h
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/08/01
*	@brief  �^�X�NID��`���(���Č����痬�p)
*
*	Copyright (C) 2010, OYO Electric Corporation
*/
#ifndef	_ID_H_
#define	_ID_H_

//========================= iTron�̂��߂̒萔�錾 ===============================
//xxx_NULL�`xxx_MAX == xxxID_MAX+1
//===============================================================================

/********************************************************************
	Task ID �̐錾
********************************************************************/
enum tskid{
	TSK_NULL,
	TSK_MAIN,							///< VA300 �[�����䃁�C���^�X�N
	TSK_CMD_LAN,						///< LAN�R�}���h�����^�X�N
	TSK_DISP,							///< �\���^�X�N
	TSK_UDPRCV,							///< UDP ��M�^�X�N
	TSK_COMMUNICATE,					///< UDP����̓d������
	TSK_UDP_SEND,						///< UDP���M
	TSK_WDT,							///< �E�H�b�`�h�b�O�^�X�N
	TSK_IO,								///< I/O���m�^�X�N
	TSK_EXKEY,							///< �O���L�[���̓^�X�N
	TSK_LCD,							///< LCD�\���^�X�N
	TSK_BUZ,							///< �u�U�[�e�X�g�^�X�N
	TSK_TS,								///< ���Ԍ��m�Z���T�^�X�N
	TSK_CAMERA,							///< �J�����B�e����^�X�N
//	TSK_CMD_MON,						///< ���j�^�^�X�N
//	TSK_RCV_MON,						///< ���j�^�^�X�N
//	TSK_RCV1,							///< �V���A��CH0��M�^�X�N
	TSK_SND1,							///< �V���A��CH0���M�^�X�N
//	TSK_MODE,							///< ���[�h�Ǘ��^�X�N
//	TSK_TELNETD,						///< TELNET�^�X�N
//	TSK_SHELL1,							///< �V�F���^�X�N

	TSK_MAX
};

/********************************************************************
	�Z�}�t�H ID �̐錾
********************************************************************/
enum semid{
	SEM_NULL,
	SEM_LED,							///< LED
	SEM_7SEG,							///< 7SEG
	SEM_RTC,							///< RTC
	SEM_ERR,							///< �G���[�X�e�[�^�X
	SEM_SROM,							///< Serial EEPROM
	SEM_FPGA,							///< FPGA
	SEM_TIM,							///< �^�C�}
	SEM_BUZ,							///< �u�U�[
	SEM_CMR,							///< �J����
	SEM_IRLED,							///< �ԊO��LED
	SEM_TPL,							///< �^�b�`�p�l��
	SEM_LCD,							///< LCD
	SEM_SPF,							///< sprintf
	SEM_STKN,							///< strtok
	SEM_STL,							///< strtol
	
	SEM_MAX
};

/********************************************************************
	�����n���h�� ID�̐錾
********************************************************************/
enum cycid{
	CYC_NULL,
	CYC_TMO,							///< �^�C�}�[�p�����n���h��
	CYC_LAN,							///< LAN�p�����n���h��(�\��)

	CYC_MAX
};

/********************************************************************
	���b�Z�[�W�o�b�t�@ ID �̐錾�A�Œ蒷
********************************************************************/
enum mbfid{
	MBF_NULL,
	MBF_TELNETD,						// for telnet deamon
	MBF_SHELL1,							// for command shell
	MBF_LCD_DATA,						// for LCD input data
	MBF_LAN_CMD,						// UDP comammnd data
	
	MBF_MAX
};

/********************************************************************
	�������[�v�[�� ID �̐錾�A�Œ蒷
********************************************************************/
enum mpfid{
	MPF_NULL,
	MPF_COM,							///< for some task memory pool
	MPF_MON,							///< for monitor task memory pool
	MPF_DISP,							///< for Display task memory pool
	MPF_LRES,							///< for LAN response memory pool
	
	MPF_MAX
};

/********************************************************************
	mailBox ID �̐錾
********************************************************************/
enum mbxid{
	MBX_NULL,
	MBX_MODE,							///< ���[�h�^�X�N�̃��[���{�b�N�X
	MBX_DISP,							///< �\���^�X�N�̃��[���{�b�N�X
	MBX_RESSND,							///< ���X�|���X���M�p���[���{�b�N�X
	MBX_SND,							///< �V���A�����M�p���[���{�b�N�X
	MBX_CMD_LAN,						///< LAN�R�}���h�C���^�[�v���^ 10/02/19
	MBX_CMD_LBUS,						///< ���[�J���o�X�R�}���h�C���^�[�v���^ 10/02/19
	MBX_MON,							///< ���j�^����

	MBX_MAX
};

/********************************************************************
	alh ID �̐錾
********************************************************************/
enum alhid{
	ALH_NULL,
	ALH_TIM,							///< �^�C�}�[�p
	
	ALH_MAX
};

/********************************************************************
	flg ID �̐錾�iwai_flg�̈���"flg_id"�@�C�x���g�t���OID�j
********************************************************************/
enum flgid{
	ID_FLG_NULL,
	ID_FLG_MAIN,					///< ���C���R���g���[��
	ID_FLG_CAMERA,					///< �J�����R���g���[�� 
	ID_FLG_TS,						///< ���̌��m�p
	ID_FLG_LCD,						///< LCD�R���g���[��
	ID_FLG_LRCV,					///< UDP�R�}���h��M�R���g���[��
	ID_FLG_IO,						///< I/O���m�p
	ID_FLG_EXKEY,					///< �O���L�[���m�p
	ID_FLG_BUZ,						///< �u�U�[�p

	FLG_MAX
};

/********************************************************************
	flg �̐錾�iwai_flg�̈���"flgptn"�@�t���O�p�^�[���j
********************************************************************/
// To ���C���^�X�N�iID_FLG_MAIN�j
#define FPTN_START_CAP			0x10000001L // �J�����L���v�`���J�n�v���i�g���~���O+�k���摜�j
#define FPTN_START_RAWCAP		0x10000002L // �J�����L���v�`���J�n�v���i���摜�j
#define FPTN_LCD_CHG_REQ		0x10000004L // LCD��ʐؑ֗v��(LCD�����C��)

// To �J�����^�X�N�iID_FLG_CAMERA�j
#define FPTN_START_CAP204		0x20000001L // �J�����L���v�`���J�n�v���i�o�^�摜�A�R�}���h204�j
#define FPTN_START_CAP211		0x20000002L // �J�����L���v�`���J�n�v���i�F�ؗp�摜�A�R�}���h211�j
#define FPTN_START_CAP141		0x20000004L // �J�����L���v�`���J�n�v���i���摜�A�R�}���h141�j
#define FPTN_SETREQ_GAIN		0x20000008L // �J�����E�Q�C���̏����l�ݒ�i�R�}���h022�̎��s�j
#define FPTN_SETREQ_SHUT1		0x20000010L // �J�����E�V���b�^�[�X�s�[�h�P�̏����l�ݒ�i�R�}���h022�̎��s�j

// To ���̌��m�^�X�N�iID_FLG_TS�j
#define FPTN_CMR_TS				0x30000001L // �J�����B�e�I���ALED�̃u�����N�J�n�ցB
#define FPTN_END_TS				0x30000002L // ���̌��m�����̏I���A�Č��m�҂��ցB
#define FPTN_WAIT_CHG_SCRN		0x30000004L // ��ʂ��u�w�𔲂��ĉ������v�܂��́A���s��ʂɂȂ�̂�҂B

// To LCD�^�X�N�iID_FLG_LCD�j
#define FPTN_LCD_INIT			0x40000001L // LCD������ʕ\���v��(���C����LCD�A�m�[�}�����[�h�ڍs�̎�)

#define FPTN_LCD_SCREEN1		0x40000002L // ��ʂP�\���v��(���C����LCD) �����o�^���[�h
#define FPTN_LCD_SCREEN2		0x40000003L // ��ʂQ�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN3		0x40000004L // ��ʂR�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN4		0x40000005L // ��ʂS�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN5		0x40000006L // ��ʂT�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN6		0x40000007L // ��ʂU�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN7		0x40000008L // ��ʂV�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN8		0x40000009L // ��ʂW�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN9		0x4000000aL // ��ʂX�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN10		0x4000000bL // ��ʂP�O�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN11		0x4000001cL // ��ʂP�P�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN12		0x4000001dL // ��ʂP�Q�\���v��(���C����LCD)

#define FPTN_LCD_SCREEN100		0x4000001eL // ��ʂP�O�O�\���v��(���C����LCD)�@�ʏ탂�[�h
#define FPTN_LCD_SCREEN101		0x4000001fL // ��ʂP�O�P�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN102		0x40000020L // ��ʂP�O�Q�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN103		0x40000021L // ��ʂP�O�R�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN104		0x40000022L // ��ʂP�O�S�\���v��(���C����LCD)

#define FPTN_LCD_SCREEN120		0x40000023L // ��ʂP�Q�O�\���v��(���C����LCD)�@�ʏ탂�[�h�i�o�^�j
#define FPTN_LCD_SCREEN121		0x40000024L // ��ʂP�Q�P�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN122		0x40000025L // ��ʂP�Q�Q�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN123		0x40000026L // ��ʂP�Q�R�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN124		0x40000027L // ��ʂP�Q�S�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN125		0x40000028L // ��ʂP�Q�T�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN126		0x40000029L // ��ʂP�Q�U�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN127		0x4000002aL // ��ʂP�Q�V�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN128		0x4000002bL // ��ʂP�Q�W�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN129		0x4000002cL // ��ʂP�Q�X�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN130		0x4000002dL // ��ʂP�R�O�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN131		0x4000002eL // ��ʂP�R�P�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN132		0x4000002fL // ��ʂP�R�Q�\���v��(���C����LCD)

#define FPTN_LCD_SCREEN140		0x40000030L // ��ʂP�S�O�\���v��(���C����LCD)�@�ʏ탂�[�h�i�폜�j
#define FPTN_LCD_SCREEN141		0x40000031L // ��ʂP�S�P�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN142		0x40000032L // ��ʂP�S�Q�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN143		0x40000033L // ��ʂP�S�R�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN144		0x40000034L // ��ʂP�S�S�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN145		0x40000035L // ��ʂP�S�T�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN146		0x40000036L // ��ʂP�S�U�\���v��(���C����LCD)

#define FPTN_LCD_SCREEN160		0x40000037L // ��ʂP�U�O�\���v��(���C����LCD)�@�ʏ탂�[�h�i�ً}�J���ԍ��ݒ�j
#define FPTN_LCD_SCREEN161		0x40000038L // ��ʂP�U�P�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN162		0x40000039L // ��ʂP�U�Q�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN163		0x4000003aL // ��ʂP�U�R�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN164		0x4000003bL // ��ʂP�U�S�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN165		0x4000003cL // ��ʂP�U�T�\���v��(���C����LCD)

#define FPTN_LCD_SCREEN180		0x4000003dL // ��ʂP�W�O�\���v��(���C����LCD)�@�ʏ탂�[�h�i�ً}�J���j
#define FPTN_LCD_SCREEN181		0x4000003eL // ��ʂP�W�P�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN182		0x4000003fL // ��ʂP�W�Q�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN183		0x40000040L // ��ʂP�W�R�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN184		0x40000041L // ��ʂP�W�S�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN185		0x40000042L // ��ʂP�W�T�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN186		0x40000043L // ��ʂP�W�U�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN187		0x40000044L // ��ʂP�W�V�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN188		0x40000045L // ��ʂP�W�W�\���v��(���C����LCD)

#define FPTN_LCD_SCREEN200		0x40000046L // ��ʂQ�O�O�\���v��(���C����LCD)�@�����e�i���X�E���[�h
#define FPTN_LCD_SCREEN201		0x40000047L // ��ʂQ�O�P�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN202		0x40000048L // ��ʂQ�O�Q�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN203		0x40000049L // ��ʂQ�O�R�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN204		0x4000004aL // ��ʂQ�O�S�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN205		0x4000004bL // ��ʂQ�O�T�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN206		0x4000004cL // ��ʂQ�O�U�\���v��(���C����LCD)

// To I/O
#define	FPTN_INIT				0x50000001L	// ������
#define	FPTN_ERR				0x50010002L	// �G���[����
#define	FPTN_CAP_TR_END			0x51000004L	// DMA�]������
#define	FPTN_DEBUG				0x50000008L	// �f�o�b�O

// To �u�U�[�^�X�N�is_idFlg�j
#define	FPTN_EMG_ON				0x60000001L	// �x��p�u�U�[ON
#define	FPTN_EMG_OFF			0x60000002L	// �x��p�u�U�[OFF
#define	FPTN_TPL_OK				0x60000010L	// �^�b�`�p�l���p�u�U�[(�s�b)
#define	FPTN_TPL_NG				0x60000020L	// �^�b�`�p�l���p�u�U�[(�s�s�b)
#define	FPTN_TPL_OFF			0x60000040L	// �^�b�`�p�l���p�u�U�[OFF

// To �O���L�[
#define	FPTN_KEY				0x70000001L	// �L�[����

#endif								// end of _ID_H_