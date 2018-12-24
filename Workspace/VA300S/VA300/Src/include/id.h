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

//=== ��`�@�i�{��`�́A�t�@�C��"va300.h"�A"id.h"�̂Q�����œ�����`���s�����ƁB�j
#define VA300S	1		// =0 : VA300,   =1 : VA300S  =2 : VA300S�̏ꍇ�Ńf�o�b�O�p�ɉ摜�f�[�^��LAN��PC�֑���B
#define VA300ST 1		// =0 : VA300,VA300S   =1 : VA300ST(���^�J�d�l)

//========================= iTron�̂��߂̒萔�錾 ===============================
//xxx_NULL�`xxx_MAX == xxxID_MAX+1
//===============================================================================

/********************************************************************
	Task ID �̐錾
********************************************************************/
enum tskid{
	TSK_NULL,
	TSK_MAIN,							///< VA300 �[�����䃁�C���^�X�N
	TSK_DISP,							///< �\���^�X�N
#if ( VA300S == 0 )
	TSK_CMD_LAN,						///< LAN�R�}���h�����^�X�N
	TSK_UDPRCV,							///< UDP ��M�^�X�N
	TSK_COMMUNICATE,					///< UDP����̓d������
	TSK_UDP_SEND,						///< UDP���M
#endif
#if ( VA300S == 1 )
	TSK_NINSHOU,						///< �F�؃^�X�N
	TSK_LOG,							///< ���M���O�^�X�N
#endif
#if ( VA300S == 2 )
	TSK_CMD_LAN,						///< LAN�R�}���h�����^�X�N
	TSK_UDPRCV,							///< UDP ��M�^�X�N
	TSK_COMMUNICATE,					///< UDP����̓d������
	TSK_UDP_SEND,						///< UDP���M
	TSK_NINSHOU,						///< �F�؃^�X�N
	TSK_LOG,							///< ���M���O�^�X�N
#endif

	TSK_WDT,							///< �E�H�b�`�h�b�O�^�X�N
	TSK_IO,								///< I/O���m�^�X�N
	TSK_EXKEY,							///< �O���L�[���̓^�X�N
	TSK_LCD,							///< LCD�\���^�X�N
	TSK_BUZ,							///< �u�U�[�e�X�g�^�X�N
	TSK_TS,								///< ���Ԍ��m�Z���T�^�X�N
	TSK_CAMERA,							///< �J�����B�e����^�X�N
//	TSK_CMD_MON,						///< ���j�^�^�X�N
//	TSK_RCV_MON,						///< ���j�^�^�X�N
	TSK_RCV1,							///< �V���A��CH0��M�^�X�N
	TSK_SND1,							///< �V���A��CH0���M�^�X�N
//	TSK_MODE,							///< ���[�h�Ǘ��^�X�N
//	TSK_TELNETD,						///< TELNET�^�X�N
//	TSK_SHELL1,							///< �V�F���^�X�N

    TSK_LEARN_DATA,

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
	SEM_FL,								///< �t���b�V��������
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
	MPF_COM,
	MPF_SND_SIO,						///< for some task memory pool
#if ( VA300S == 0 )
	MPF_LRES,							///< for LAN response memory pool
#endif
#if ( VA300S == 1 )
	MPF_SND_NINSHOU,					///< �F�؏����f�[�^�p�������E�v�[��
	MPF_LOG_DATA,						///< �F�؏����f�[�^�p�������E�v�[��
#endif
#if ( VA300S == 2 )
	MPF_LRES,							///< for LAN response memory pool
	MPF_SND_NINSHOU,					///< �F�؏����f�[�^�p�������E�v�[��
	MPF_LOG_DATA,						///< �F�؏����f�[�^�p�������E�v�[��
#endif
	MPF_MON,							///< for monitor task memory pool
	MPF_DISP,							///< for Display task memory pool
	
	MPF_MAX
};

/********************************************************************
	mailBox ID �̐錾
********************************************************************/
enum mbxid{
	MBX_NULL,
	MBX_MODE,							///< ���[�h�^�X�N�̃��[���{�b�N�X
	MBX_DISP,							///< �\���^�X�N�̃��[���{�b�N�X
	MBX_SND_SIO,						///< �V���A�����M�p���[���{�b�N�X
#if ( VA300S == 0 )
	MBX_RESSND,							///< ���X�|���X���M�p���[���{�b�N�X
	MBX_CMD_LAN,						///< LAN�R�}���h�C���^�[�v���^ 10/02/19
#endif
#if ( VA300S == 1 )
	MBX_SND_NINSHOU,					///< �F�؏����f�[�^�̃^�X�N�ԑ��M�p���[���E�{�b�N�X
	MBX_LOG_DATA	,					///< ���M���O�����f�[�^�̃^�X�N�ԑ��M�p���[���E�{�b�N�X
#endif
#if ( VA300S == 2 )
	MBX_RESSND,							///< ���X�|���X���M�p���[���{�b�N�X
	MBX_CMD_LAN,						///< LAN�R�}���h�C���^�[�v���^ 10/02/19
	MBX_SND_NINSHOU,					///< �F�؏����f�[�^�̃^�X�N�ԑ��M�p���[���E�{�b�N�X
	MBX_LOG_DATA	,					///< ���M���O�����f�[�^�̃^�X�N�ԑ��M�p���[���E�{�b�N�X
#endif
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
	ID_FLG_IO,						///< I/O���m�p
	ID_FLG_EXKEY,					///< �O���L�[���m�p
	ID_FLG_BUZ,						///< �u�U�[�p
#if ( VA300S == 0 )
	ID_FLG_LRCV,					///< UDP�R�}���h��M�R���g���[��
#endif
#if ( VA300S == 1 )
	ID_FLG_NINSHOU,					///< �F�؏����p�i�g�p���Ȃ������H�j
	ID_FLG_LOG,						///< ���M���O�����p�i�g�p���Ȃ������H�j
#endif
#if ( VA300S == 2 )
	ID_FLG_LRCV,					///< UDP�R�}���h��M�R���g���[��
	ID_FLG_NINSHOU,					///< �F�؏����p�i�g�p���Ȃ������H�j
	ID_FLG_LOG,						///< ���M���O�����p�i�g�p���Ȃ������H�j
#endif
	FLG_MAX
};

/********************************************************************
	flg �̐錾�iwai_flg�̈���"flgptn"�@�t���O�p�^�[���j
********************************************************************/
// To ���C���^�X�N�iID_FLG_MAIN�j
#define FPTN_START_CAP			0x10000001L // �J�����L���v�`���J�n�v���i�g���~���O+�k���摜�j
#define FPTN_START_RAWCAP		0x10000002L // �J�����L���v�`���J�n�v���i���摜�j
#define FPTN_LCD_CHG_REQ		0x10000004L // LCD��ʐؑ֗v��(LCD�����C��)
#define FPTN_SEND_REQ_MAINTE_CMD 0x10000008L 	// ��M�R�}���h��̓^�X�N�����C��Task�ցA�����e�i���X���[�h�ؑւ��ʒm���M���˗��B

// To �J�����^�X�N�iID_FLG_CAMERA�j
#define FPTN_START_CAP204		0x20000001L // �J�����L���v�`���J�n�v���i�o�^�摜�A�R�}���h204�j
#define FPTN_START_CAP211		0x20000002L // �J�����L���v�`���J�n�v���i�F�ؗp�摜�A�R�}���h211�j
#define FPTN_START_CAP141		0x20000004L // �J�����L���v�`���J�n�v���i���摜�A�R�}���h141�j
#define FPTN_CHECK_IMAGE		0x20000006L	// �J�����L���v�`���������`�F�b�N
#define FPTN_SETREQ_GAIN		0x20000008L // �J�����E�Q�C���̏����l�ݒ�i�R�}���h022�̎��s�j
#define FPTN_SETREQ_SHUT1		0x20000009L // �J�����E�V���b�^�[�X�s�[�h�P�̏����l�ݒ�i�R�}���h022�̎��s�j
#define FPTN_CMR_INIT			0x2000000aL // �J����WeakUP //20140930Miya Bio FPGA
#define FPTN_REROAD_PARA		0x2000000bL // �J�����p�����[�^�����[�h//20150930Miya


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

#define FPTN_LCD_SCREEN170		0x4000003dL // ��ʕ\���v��(���C����LCD)�@�ʏ탂�[�h�i�ً}�J���j
#define FPTN_LCD_SCREEN171		0x4000003eL // ��ʕ\���v��(���C����LCD)
#define FPTN_LCD_SCREEN172		0x4000003fL // ��ʕ\���v��(���C����LCD)
#define FPTN_LCD_SCREEN173		0x40000040L // ��ʕ\���v��(���C����LCD)
#define FPTN_LCD_SCREEN174		0x40000041L // ��ʕ\���v��(���C����LCD)
#define FPTN_LCD_SCREEN175		0x40000042L // ��ʕ\���v��(���C����LCD)
#define FPTN_LCD_SCREEN176		0x40000043L // ��ʕ\���v��(���C����LCD)
#define FPTN_LCD_SCREEN177		0x40000044L // ��ʕ\���v��(���C����LCD)
#define FPTN_LCD_SCREEN178		0x40000045L // ��ʕ\���v��(���C����LCD)

#define FPTN_LCD_SCREEN200		0x40000046L // ��ʂQ�O�O�\���v��(���C����LCD)�@�����e�i���X�E���[�h
#define FPTN_LCD_SCREEN201		0x40000047L // ��ʂQ�O�P�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN202		0x40000048L // ��ʂQ�O�Q�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN203		0x40000049L // ��ʂQ�O�R�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN204		0x4000004aL // ��ʂQ�O�S�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN205		0x4000004bL // ��ʂQ�O�T�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN206		0x4000004cL // ��ʂQ�O�U�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN207		0x4000004dL // ��ʂQ�O�V�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN208		0x4000004eL // ��ʂQ�O�W�\���v��(���C����LCD)

#define FPTN_LCD_SCREEN400		0x4000004fL // ��ʂS�O�O�\���v��(���C����LCD)�@�P�΂P�d�l�F�����o�^���[�h
#define FPTN_LCD_SCREEN401		0x40000050L // ��ʂS�O�P�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN402		0x40000051L // ��ʂS�O�Q�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN403		0x40000052L // ��ʂS�O�R�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN404		0x40000053L // ��ʂS�O�S�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN405		0x40000054L // ��ʂS�O�T�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN406		0x40000055L // ��ʂS�O�U�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN407		0x40000056L // ��ʂS�O�V�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN408		0x40000057L // ��ʂS�O�W�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN409		0x40000058L // ��ʂS�O�X�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN410		0x40000059L // ��ʂS�P�O�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN411		0x4000005aL // ��ʂS�P�P�\���v��(���C����LCD)
	
#define FPTN_LCD_SCREEN500		0x4000005bL // ��ʂT�O�O�\���v��(���C����LCD)�@�P�΂P�d�l�F�ʏ탂�[�h
#define FPTN_LCD_SCREEN501		0x4000005cL // ��ʂT�O�P�\���v��(���C����LCD)	
#define FPTN_LCD_SCREEN502		0x4000005dL // ��ʂT�O�Q�\���v��(���C����LCD)	
#define FPTN_LCD_SCREEN503		0x4000005eL // ��ʂT�O�R�\���v��(���C����LCD)	
#define FPTN_LCD_SCREEN504		0x4000005fL // ��ʂT�O�S�\���v��(���C����LCD)	
#define FPTN_LCD_SCREEN505		0x40000060L // ��ʂT�O�T�\���v��(���C����LCD)	
#define FPTN_LCD_SCREEN506		0x40000061L // ��ʂT�O�U�\���v��(���C����LCD)	
	
#define FPTN_LCD_SCREEN520		0x40000062L // ��ʂT�Q�O�\���v��(���C����LCD)�@�P�΂P�d�l�F�o�^���[�h
#define FPTN_LCD_SCREEN521		0x40000063L // ��ʂT�Q�P�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN522		0x40000064L // ��ʂT�Q�Q�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN523		0x40000065L // ��ʂT�Q�R�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN524		0x40000066L // ��ʂT�Q�S�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN525		0x40000067L // ��ʂT�Q�T�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN526		0x40000068L // ��ʂT�Q�U�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN527		0x40000069L // ��ʂT�Q�V�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN528		0x4000006aL // ��ʂT�Q�W�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN529		0x4000006bL // ��ʂT�Q�X�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN530		0x4000006cL // ��ʂT�R�O�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN531		0x4000006dL // ��ʂT�R�P�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN532		0x4000006eL // ��ʂT�R�Q�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN533		0x4000006fL // ��ʂT�R�R�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN534		0x40000070L // ��ʂT�R�S�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN535		0x40000071L // ��ʂT�R�T�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN536		0x40000072L // ��ʂT�R�U�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN537		0x40000073L // ��ʂT�R�V�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN538		0x40000074L // ��ʂT�R�W�\���v��(���C����LCD)
	
#define FPTN_LCD_SCREEN542		0x40000075L // ��ʂT�S�Q�\���v��(���C����LCD)�@�P�΂P�d�l�F�폜���[�h
#define FPTN_LCD_SCREEN543		0x40000076L // ��ʂT�S�R�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN544		0x40000077L // ��ʂT�S�S�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN545		0x40000078L // ��ʂT�S�T�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN546		0x40000079L // ��ʂT�S�U�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN547		0x4000007aL // ��ʂT�S�V�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN548		0x4000007bL // ��ʂT�S�W�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN549		0x4000007cL // ��ʂT�S�X�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN550		0x4000007dL // ��ʂT�T�O�\���v��(���C����LCD)
#define FPTN_LCD_SCREEN551		0x4000007eL // ��ʂT�T�P�\���v��(���C����LCD)

#define FPTN_LCD_SCREEN105		0x4000007fL // ��ʂP�O�T�\���v��(���C����LCD) �ʏ탂�[�h //20140423Miya �F�؃��g���C�ǉ�
#define FPTN_LCD_SCREEN106		0x40000080L // ��ʂP�O�U�\���v��(���C����LCD) �ʏ탂�[�h //20140423Miya �F�؃��g���C�ǉ�

#define FPTN_LCD_SCREEN108		0x4000008aL // ��ʂP�O�W�\���v��(���C����LCD) �ʏ탂�[�h //20140925Miya password_open
#define FPTN_LCD_SCREEN109		0x4000008bL // ��ʂP�O�X�\���v��(���C����LCD) �ʏ탂�[�h //20140925Miya password_open
#define FPTN_LCD_SCREEN110		0x4000008cL // ��ʂP�O�X�\���v��(���C����LCD) �ʏ탂�[�h //20140925Miya password_open

#define FPTN_LCD_SCREEN180		0x4000008dL // ��ʂP�X�O�\���v��(���C����LCD) �ʏ탂�[�h //20140925Miya password_open
#define FPTN_LCD_SCREEN181		0x4000008eL // ��ʂP�X�P�\���v��(���C����LCD) �ʏ탂�[�h //20140925Miya password_open
#define FPTN_LCD_SCREEN182		0x4000008fL // ��ʂP�X�Q�\���v��(���C����LCD) �ʏ탂�[�h //20140925Miya password_open
#define FPTN_LCD_SCREEN183		0x40000090L // ��ʂP�X�R�\���v��(���C����LCD) �ʏ탂�[�h //20140925Miya password_open
#define FPTN_LCD_SCREEN184		0x40000091L // ��ʂP�X�S�\���v��(���C����LCD) �ʏ탂�[�h //20140925Miya password_open
#define FPTN_LCD_SCREEN185		0x40000092L // ��ʂP�X�T�\���v��(���C����LCD) �ʏ탂�[�h //20140925Miya password_open
#define FPTN_LCD_SCREEN186		0x40000093L // ��ʂP�X�U�\���v��(���C����LCD) �ʏ탂�[�h //20140925Miya password_open
#define FPTN_LCD_SCREEN187		0x40000094L // ��ʂP�X�V�\���v��(���C����LCD) �ʏ탂�[�h //20140925Miya password_open

#define FPTN_LCD_SCREEN220		0x40000095L // ��ʂQ�Q�O�\���v��(���C����LCD) �ʏ탂�[�h //20140925Miya password_open
#define FPTN_LCD_SCREEN221		0x40000096L // ��ʂQ�Q�P�\���v��(���C����LCD) �ʏ탂�[�h //20140925Miya password_open
#define FPTN_LCD_SCREEN222		0x40000097L // ��ʂQ�Q�Q�\���v��(���C����LCD) �ʏ탂�[�h //20140925Miya password_open
#define FPTN_LCD_SCREEN223		0x40000098L // ��ʂQ�Q�R�\���v��(���C����LCD) �ʏ탂�[�h //20140925Miya password_open
#define FPTN_LCD_SCREEN224		0x40000099L // ��ʂQ�Q�S�\���v��(���C����LCD) �ʏ탂�[�h //20140925Miya password_open
#define FPTN_LCD_SCREEN225		0x4000009aL // ��ʂQ�Q�T�\���v��(���C����LCD) �ʏ탂�[�h //20140925Miya password_open

#define FPTN_LCD_SCREEN240		0x4000009bL // ��ʂQ�S�O�\���v��(���C����LCD) �ʏ탂�[�h //20140925Miya password_open
#define FPTN_LCD_SCREEN241		0x4000009cL // ��ʂQ�S�P�\���v��(���C����LCD) �ʏ탂�[�h //20140925Miya password_open
#define FPTN_LCD_SCREEN242		0x4000009dL // ��ʂQ�S�Q�\���v��(���C����LCD) �ʏ탂�[�h //20140925Miya password_open
#define FPTN_LCD_SCREEN243		0x4000009eL // ��ʂQ�S�R�\���v��(���C����LCD) �ʏ탂�[�h //20140925Miya password_open
#define FPTN_LCD_SCREEN244		0x4000009fL // ��ʂQ�S�S�\���v��(���C����LCD) �ʏ탂�[�h //20140925Miya password_open
#define FPTN_LCD_SCREEN245		0x400000a0L // ��ʂQ�S�T�\���v��(���C����LCD) �ʏ탂�[�h //20140925Miya password_open
#define FPTN_LCD_SCREEN246		0x400000a1L // ��ʂQ�S�U�\���v��(���C����LCD) �ʏ탂�[�h //20140925Miya password_open
#define FPTN_LCD_SCREEN247		0x400000a2L // ��ʂQ�S�V�\���v��(���C����LCD) �ʏ탂�[�h //20140925Miya password_open
#define FPTN_LCD_SCREEN248		0x400000a3L // ��ʂQ�S�W�\���v��(���C����LCD) �ʏ탂�[�h //20140925Miya password_open
#define FPTN_LCD_SCREEN249		0x400000a4L // ��ʂQ�S�X�\���v��(���C����LCD) �ʏ탂�[�h //20140925Miya password_open

#define FPTN_LCD_SCREEN260		0x400000a5L // ��ʂQ�U�O�\���v��(���C����LCD) �ʏ탂�[�h //20140925Miya password_open
#define FPTN_LCD_SCREEN261		0x400000a6L // ��ʂQ�U�P�\���v��(���C����LCD) �ʏ탂�[�h //20140925Miya password_open
#define FPTN_LCD_SCREEN262		0x400000a7L // ��ʂQ�U�Q�\���v��(���C����LCD) �ʏ탂�[�h //20140925Miya password_open
#define FPTN_LCD_SCREEN263		0x400000a8L // ��ʂQ�U�R�\���v��(���C����LCD) �ʏ탂�[�h //20140925Miya password_open
#define FPTN_LCD_SCREEN264		0x400000a9L // ��ʂQ�U�S�\���v��(���C����LCD) �ʏ탂�[�h //20140925Miya password_open
#define FPTN_LCD_SCREEN265		0x400000aaL // ��ʂQ�U�S�\���v��(���C����LCD) �ʏ탂�[�h //20140925Miya password_open
#define FPTN_LCD_SCREEN266		0x400000abL // ��ʂQ�U�S�\���v��(���C����LCD) �ʏ탂�[�h //20140925Miya password_open
#define FPTN_LCD_SCREEN267		0x400000acL // ��ʂQ�U�S�\���v��(���C����LCD) �ʏ탂�[�h //20140925Miya password_open

//20160108Miya FinKeyS ->
#define FPTN_LCD_SCREEN600		0x400000adL // �ʏ탂�[�h2 (FinKeyS)
#define FPTN_LCD_SCREEN601		0x400000aeL // 
#define FPTN_LCD_SCREEN602		0x400000afL // 
#define FPTN_LCD_SCREEN603		0x400000b0L // 
#define FPTN_LCD_SCREEN604		0x400000b1L // 
#define FPTN_LCD_SCREEN605		0x400000b2L // 
#define FPTN_LCD_SCREEN606		0x400000b3L // 
#define FPTN_LCD_SCREEN607		0x400000b4L // 
#define FPTN_LCD_SCREEN608		0x400000b5L // 
#define FPTN_LCD_SCREEN609		0x400000b6L // 
#define FPTN_LCD_SCREEN610		0x400000b7L // 
#define FPTN_LCD_SCREEN611		0x400000b8L // 
#define FPTN_LCD_SCREEN612		0x400000b9L // 
#define FPTN_LCD_SCREEN613		0x400000baL // 

#define FPTN_LCD_SCREEN620		0x400000bbL // �H�����[�h
#define FPTN_LCD_SCREEN621		0x400000bcL // 
#define FPTN_LCD_SCREEN622		0x400000bdL // 
#define FPTN_LCD_SCREEN623		0x400000beL // 
#define FPTN_LCD_SCREEN624		0x400000bfL // 
#define FPTN_LCD_SCREEN625		0x400000c0L // 

#define FPTN_LCD_SCREEN188		0x400000c1L // �p�X���[�h�ύX�@�ǉ�(�Ƒ���)
#define FPTN_LCD_SCREEN133		0x400000c2L // ��ʂP�R�Q�\���v��(���C����LCD)
//20160108Miya FinKeyS <-

#define FPTN_LCD_SCREEN999		0x400000999 // ��ʂX�X�X�\���v��(���C����LCD)�@�u�V���b�g�_�E�����܂��v

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

#if ( VA300s == 1 )
// To �F�؃^�X�N
#define FPTN_START_NINSHOU		0x80000001L	// �F�؊J�n�v���i�g�p���Ȃ������H�j

// To ���M���O�^�X�N
#define FPTN_START_LOG			0x90000001L	// �P���R�[�h���̃��M���O�v���i�g�p���Ȃ������H�j

#endif


#endif								// end of _ID_H_