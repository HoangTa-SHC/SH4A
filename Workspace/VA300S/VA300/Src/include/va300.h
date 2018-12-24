//=============================================================================
/**
*	VA-300�v���O����
*
*	@file va300.h
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2011/11/04
*	@brief  ���ʒ�`���(���Č����痬�p)
*
*	Copyright (C) 2010, OYO Electric Corporation
*/
//=============================================================================
#include <ctype.h>
#include <machine.h>
#include <string.h>
#include <stdarg.h>
#include "kernel.h"
//#include "drv_rtc.h"
#include "drv_fpga.h"
#include "drv_7seg.h"
#include "id.h"
#include "command.h"

#ifndef	_VA300_H_
#define	_VA300_H_

#define NEWCMR 1
#define AUTHTEST 1			//0:OFF 1:�o�^���� 2:�o�^�Ȃ� //20160613Miya �F�؃e�X�g(Flash�ɉ摜��ۑ�����)
#define KOUJYOUCHK	0		//20160606Miya �H��`�F�b�N�p
#define	FORDEMO	0			//20160711Miya �f���@

//=== ��`�@�i�{��`�́A�t�@�C��"va300.h"�A"id.h"�̂Q�����œ�����`���s�����ƁB�j
#define VA300S	1		// =0 : VA300,   =1 : VA300S,  =2 : VA300S�̏ꍇ�Ńf�o�b�O�p�ɉ摜�f�[�^��LAN��PC�֑���B
#define VA300ST 1		// =0 : VA300,VA300S   =1 : VA300ST(���^�J�d�l)

#define NEWALGO		1	//NewAlgo 
						//2015.05.31 1:�ʒu�ɂ�錸�_�̊ɘa
						//             �ǉ����_������

#define FPGA_HI		1	//20160902Miya FPGA������ 0:�]�����x 1:����
#define BPWRCTRL	1	//20160905Miya B-PWR����

#define PCCTRL		0	//20160930Miya PC����VA300S�𐧌䂷��
#define	CMRSENS		1	//20170424Miya 	�J�������x�Ή�(FAR/FRR�Ή�)

#define FREEZTEST	0	//20170706Miya

enum COM_CH {								///< �ʐM�`�����l����`
	COM_CH_MON,								///< ���j�^�`�����l��
	COM_CH_LAN,								///< LAN�`�����l��
};

enum SYS_SPEC {								///< �V�X�e���ݒ��Ԗ₢���킹
	SYS_SPEC_MANTION = 0,					// �}���V�����E��L���d�l
	SYS_SPEC_OFFICE,						// �P�΂P�d�l�i�I�t�B�X�d�l�j
	SYS_SPEC_ENTRANCE,						// �}���V�����E���p���d�l
	SYS_SPEC_OFFICE_NO_ID,					// �P�Α��d�l�i�I�t�B�X�EID�ԍ������d�l�j
	SYS_SPEC_KOUJIM,						//20160112Miya FinKeyS //�H�����[�h(�}���V�����E��L��)
	SYS_SPEC_KOUJIO,						//20160112Miya FinKeyS //�H�����[�h(�I�t�B�X)
	SYS_SPEC_KOUJIS,						//20160112Miya FinKeyS //�H�����[�h(�}�[�g�����)
	SYS_SPEC_SMT,							//20160112Miya FinKeyS //�X�}�[�g����ՑΉ�
};

enum YUTAKA_NYUTAI_KUBUN {					///< ���^�J�d�l�@���ގ��敪
	NYUTAI_NON = 0,							//  ������
	NYUSHITU,								//  ����
	TAISHITU								//�@�ގ�
};

enum SYS_SPEC_DEMO {						///< �V�X�e���ݒ��Ԗ₢���킹(DEMO�d�l���ۂ�)
	SYS_SPEC_NOMAL = 0,						// DEMO�łȂ��ꍇ
	SYS_SPEC_DEMO							// DEMO�d�l
};

enum PFAIL_SENSE_STATUS {					///< ��d���m��M�̗L��
	PFAIL_SENSE_NO = 0,
	PFAIL_SENSE_ON							///< ON
};

enum SHUTDOWN_OK_STATUS {					///< �V���b�g�_�E��OK��M�̗L��
	SHUTDOWN_NO = 0,						///< NO
	SHUTDOWN_OK								///< OK
};

enum CAP_STATUS {							///< �w�F�،���
	CAP_JUDGE_IDLE,
	CAP_JUDGE_OK,							///< �w�F��OK
	CAP_JUDGE_NG,							///< �w�F��NG
	CAP_JUDGE_RT,							///< �w�F�؃��g���C�v��
	CAP_JUDGE_E1,							///< �w�F�؉摜�ُ�i�q�X�g�O�����ُ�j�i�B�e�摜���Q�������ꂽ�ُ�摜�j
	CAP_JUDGE_E2,							///< �w�F�؉摜�ُ�i�X���b�g�ُ�j�i�w���c��ő}�����ꂽ�ꍇ�j
	CAP_JUDGE_RT2							///< �w�F�؃��g���C�v��
};

enum DONBURU_STATUS {						///< �h���O���̗L���m�F�̌���
	DONGURU_JUDGE_IDLE = 0,
	DONGURU_JUDGE_OK,						///< OK
	DONGURU_JUDGE_NG						///< NG
};

enum PASSWORD_STATUS {						///< �p�X���[�h�m�F�̌���
	PASSWORD_JUDGE_IDLE = 0,
	PASSWORD_JUDGE_OK,						///< OK
	PASSWORD_JUDGE_NG						///< NG
};

enum ID_NO_CHECK_STATUS {					///< ID�ԍ��m�F�̌��ʁi�P�΂P�d�l�j
	ID_NO_JUDGE_IDLE = 0,
	ID_NO_JUDGE_OK,							///< OK
	ID_NO_JUDGE_NG							///< NG
};

enum ID_AUTHORITY_CHECK_STATUS {			///< ID�����m�F�̌��ʁi�P�΂P�d�l�j
	ID_AUTHORITY_JUDGE_IDLE = 0,
	ID_AUTHORITY_JUDGE_OK,					///< OK
	ID_AUTHORITY_JUDGE_NG,					///< NG
	ID_AUTHORITY_JUDGE_CN					///< CANSEL
};

enum KINKYU_TOUROKU_STATUS {				///< �ً}�ԍ��o�^�m�F�̌���
	KINKYU_TOUROKU_JUDGE_IDLE = 0,
	KINKYU_TOUROKU_JUDGE_OK,				///< OK
	KINKYU_TOUROKU_JUDGE_NG					///< NG
};

enum MODE_STATUS {							///<=== ��Ԓ�` ===
	MD_POWER_ON = 0,						///< �d��ON�i�����l)
	MD_POWER_OFF,							///< �d��OFF
	MD_INITIAL,								///< �����o�^���[�h
	MD_MAINTE,								///< �����e�i���X���[�h
	MD_NORMAL,								///< �ʏ탂�[�h
	MD_CAP,									///< �F�؏�����
	MD_PFAIL,								///< ��d�����샂�[�h
	MD_PANIC,								///< ��펞�J�����[�h
	MD_SELFTEST,							///< �Z���t�e�X�g���
	MD_ERROR,								///< �G���[�������
	MD_MAX
};

enum SUBMODE_STATUS {						///<=== �T�u��Ԓ�` ===
	SUB_MD_IDLE = 0,						///< �A�C�h����ԁi�ȉ��̂ǂ�ɂ����ěƂ܂�Ȃ����j�j
	SUB_MD_WAKEUP,							///< WakeUp�m�F��
	SUB_MD_CAMERA_PARAM_REQ,				///< �J�����p�����[�^�̏����l�v����
	SUB_MD_CAP_PARAM_REQ,					///< �摜�����̏����l�v����
	SUB_MD_LED_PARAM_REQ,					///< LED���ʐ��l�̏����l�v����
	SUB_MD_TOUROKU_PARAM_REQ,				///< �o�^�f�[�^�̏����l�v����
	SUB_MD_DELETE,							///< �폜���[�h
	SUB_MD_ID_NO_CHECK_REQ,					///< ID�ԍ��⍇���̑��M��
	SUB_MD_ID_AUTHORITY_CHECK_REQ,			///< ID�����⍇���̑��M��
	SUB_MD_DONGURU_CHK,						///< PC�փh���O���̗L���m�F�̊m�F�ʐM��
	SUB_MD_PASSWORD_CHK,					///< PC�փp�X���[�h�̊m�F�ʐM��
	SUB_MD_KINKYU_TOUROKU,					///< �ً}�ԍ��o�^�����V�[�P���X��
	SUB_MD_KINKYU_8KETA_REQ,				///< �ً}�W���ԍ��v���ʐM��
	SUB_MD_KINKYU_KAIJYO_SEND,				///< �ً}�J���ԍ��ʒm�ʐM��
	SUB_MD_KINKYU_NO_CHK_REQ,				///< �ً}�ԍ��Ó����m�F�v���ʐM��
	SUB_MD_CHG_MAINTE,						///< �����e�i���X���[�h�ֈڍs��
	SUB_MD_CHG_NORMAL,						///< �ʏ탂�[�h�ֈڍs��
	SUB_MD_CHG_PFAIL,						///< ��d���[�h�ֈڍs��
	SUB_MD_REQ_SHUTDOWN,					///< �V���b�g�_�E���v����
	SUB_MD_UNLOCK,							///< �����v����
	SUB_MD_NINSHOU,							///< �F�؊������M���iVA300���j
	SUB_MD_TOUROKU,							///< �o�^�������M���iVA300���j
	SUB_MD_KAKAIHOU_TIME,					///< �ߊJ�����Ԑݒ�R�}���h���M���iVA300���j
	SUB_MD_INIT_TIME,						///< �����̏����ݒ�R�}���h���M���iVA300���j
	SUB_MD_MEMCHK,							///< �������`�F�b�N��
	SUB_MD_ALLDEL,							///< �ꊇ�폜���M���iVA300���j
		
	SUB_MD_MAX
};

enum SCREEN_NO {
	LCD_INIT,			/// LCD�������
	LCD_SCREEN1,		// ��ʂP�\����		�����o�^���
	LCD_SCREEN2,		// ��ʂQ�\����
	LCD_SCREEN3,		// ��ʂR�\����
	LCD_SCREEN4,		// ��ʂS�\����
	LCD_SCREEN5,		// ��ʂT�\����
	LCD_SCREEN6,		// ��ʂU�\����
	LCD_SCREEN7,		// ��ʂV�\����
	LCD_SCREEN8,		// ��ʂW�\����
	LCD_SCREEN9,		// ��ʂX�\����
	LCD_SCREEN10,		// ��ʂP�O�\����
	LCD_SCREEN11,		// ��ʂP�P�\����
	LCD_SCREEN12, 		// ��ʂP�Q�\����

	LCD_SCREEN100,		// ��ʂP�O�O�\����	 �ʏ탂�[�h�i�F�؁j
	LCD_SCREEN101,		// ��ʂP�O�P�\����
	LCD_SCREEN102,		// ��ʂP�O�Q�\����
	LCD_SCREEN103,		// ��ʂP�O�R�\����
	LCD_SCREEN104,		// ��ʂP�O�S�\����
	LCD_SCREEN105,		// ��ʂP�O�T�\���� //20140423Miya �F�؃��g���C�ǉ�
	LCD_SCREEN106,		// ��ʂP�O�U�\���� //20140423Miya �F�؃��g���C�ǉ�
	LCD_SCREEN107,		// ��ʂP�O�V�\���� //20140925Miya password_open	//���101�ƕ��p�ɂ����g�p
	LCD_SCREEN108,		// ��ʂP�O�W�\���� //20140925Miya password_open	//���108����2�ʐؑւ���
	LCD_SCREEN109,		// ��ʂP�O�X�\���� //20140925Miya password_open	//�G���[�\��
	LCD_SCREEN110,		// ��ʂP�P�O�\���� //20140925Miya password_open	//���΂炭���҂�������

	LCD_SCREEN120,		// ��ʂP�Q�O�\�����@�ʏ탂�[�h�i�o�^�j
	LCD_SCREEN121,		// ��ʂP�Q�P�\����
	LCD_SCREEN122,		// ��ʂP�Q�Q�\����
	LCD_SCREEN123,		// ��ʂP�Q�R�\����
	LCD_SCREEN124,		// ��ʂP�Q�S�\����
	LCD_SCREEN125,		// ��ʂP�Q�T�\����
	LCD_SCREEN126,		// ��ʂP�Q�U�\����
	LCD_SCREEN127,		// ��ʂP�Q�V�\����
	LCD_SCREEN128,		// ��ʂP�Q�W�\����
	LCD_SCREEN129,		// ��ʂP�Q�X�\����
	LCD_SCREEN130,		// ��ʂP�R�O�\����
	LCD_SCREEN131,		// ��ʂP�R�P�\����
	LCD_SCREEN132,		// ��ʂP�R�Q�\����
	LCD_SCREEN133,		// ���133�\����

	LCD_SCREEN140,		// ��ʂP�S�O�\�����@�ʏ탂�[�h�i�폜�j
	LCD_SCREEN141,		// ��ʂP�S�P�\����
	LCD_SCREEN142,		// ��ʂP�S�Q�\����
	LCD_SCREEN143,		// ��ʂP�S�R�\����
	LCD_SCREEN144,		// ��ʂP�S�S�\����
	LCD_SCREEN145,		// ��ʂP�S�T�\����
	LCD_SCREEN146,		// ��ʂP�S�U�\����

	LCD_SCREEN160,		// ��ʂP�U�O�\�����@�ʏ탂�[�h�i�ً}�J���ԍ��ݒ�j
	LCD_SCREEN161,		// ��ʂP�U�P�\����
	LCD_SCREEN162,		// ��ʂP�U�Q�\����
	LCD_SCREEN163,		// ��ʂP�U�R�\����
	LCD_SCREEN164,		// ��ʂP�U�S�\����
	LCD_SCREEN165,		// ��ʂP�U�T�\����

	LCD_SCREEN170,		// ��ʂP�W�O�\�����@�ʏ탂�[�h�i�ً}�J���j
	LCD_SCREEN171,		// ��ʂP�W�P�\����
	LCD_SCREEN172,		// ��ʂP�W�Q�\����
	LCD_SCREEN173,		// ��ʂP�W�R�\����
	LCD_SCREEN174,		// ��ʂP�W�S�\����
	LCD_SCREEN175,		// ��ʂP�W�T�\����
	LCD_SCREEN176,		// ��ʂP�W�U�\����
	LCD_SCREEN177,		// ��ʂP�W�V�\����
	LCD_SCREEN178,		// ��ʂP�W�W�\����

	LCD_SCREEN180,		// ��ʂP�X�O�\�����@�ʏ탂�[�h�i�p�X���[�h�ύX�j
	LCD_SCREEN181,		// ��ʂP�X�P�\����
	LCD_SCREEN182,		// ��ʂP�X�Q�\����
	LCD_SCREEN183,		// ��ʂP�X�R�\����
	LCD_SCREEN184,		// ��ʂP�X�S�\����
	LCD_SCREEN185,		// ��ʂP�X�T�\����
	LCD_SCREEN186,		// ��ʂP�X�U�\����
	LCD_SCREEN187,		// ��ʂP�X�V�\����
	LCD_SCREEN188,		// ���198�\����	//20160108Miya FinKeyS

	LCD_SCREEN200,		// ��ʂQ�O�O�\�����@�����e�i���X�E���[�h
	LCD_SCREEN201,		// ��ʂQ�O�P�\����
	LCD_SCREEN202,		// ��ʂQ�O�Q�\����
	LCD_SCREEN203,		// ��ʂQ�O�R�\����
	LCD_SCREEN204,		// ��ʂQ�O�S�\����
	LCD_SCREEN205,		// ��ʂQ�O�T�\����
	LCD_SCREEN206,		// ��ʂQ�O�U�\����
	LCD_SCREEN207,		// ��ʂQ�O�V�\����
	LCD_SCREEN208,		// ��ʂQ�O�W�\����

	LCD_SCREEN220,		// ��ʂQ�Q�O�\�����@�����e�i���X�E���[�h(���)
	LCD_SCREEN221,		// ��ʂQ�Q�P�\����
	LCD_SCREEN222,		// ��ʂQ�Q�Q�\����
	LCD_SCREEN223,		// ��ʂQ�Q�R�\����
	LCD_SCREEN224,		// ��ʂQ�Q�S�\����
	LCD_SCREEN225,		// ��ʂQ�Q�T�\����

	LCD_SCREEN240,		// ��ʂQ�S�O�\�����@�����e�i���X�E���[�h(�ݒ�ύX)
	LCD_SCREEN241,		// ��ʂQ�S�P�\����
	LCD_SCREEN242,		// ��ʂQ�S�Q�\����
	LCD_SCREEN243,		// ��ʂQ�S�R�\����
	LCD_SCREEN244,		// ��ʂQ�S�S�\����
	LCD_SCREEN245,		// ��ʂQ�S�T�\����
	LCD_SCREEN246,		// ��ʂQ�S�U�\����
	LCD_SCREEN247,		// ��ʂQ�S�V�\����
	LCD_SCREEN248,		// ��ʂQ�S�W�\����
	LCD_SCREEN249,		// ��ʂQ�S�X�\����

	LCD_SCREEN260,		// ��ʂQ�U�O�\�����@�����e�i���X�E���[�h(�Z�p)
	LCD_SCREEN261,		// ��ʂQ�U�P�\����
	LCD_SCREEN262,		// ��ʂQ�U�Q�\����
	LCD_SCREEN263,		// ��ʂQ�U�R�\����
	LCD_SCREEN264,		// ��ʂQ�U�S�\����
	LCD_SCREEN265,		// ��ʂQ�U�T�\����
	LCD_SCREEN266,		// ��ʂQ�U�U�\����
	LCD_SCREEN267,		// ��ʂQ�U�V�\����
	
	LCD_SCREEN400,		// ��ʂS�O�O�\�����@�P�΂P�d�l�F�����o�^���[�h
	LCD_SCREEN401,		// ��ʂS�O�P�\����
	LCD_SCREEN402,		// ��ʂS�O�Q�\����
	LCD_SCREEN403,		// ��ʂS�O�R�\����
	LCD_SCREEN404,		// ��ʂS�O�S�\����
	LCD_SCREEN405,		// ��ʂS�O�T�\����
	LCD_SCREEN406,		// ��ʂS�O�U�\����
	LCD_SCREEN407,		// ��ʂS�O�V�\����
	LCD_SCREEN408,		// ��ʂS�O�W�\����
	LCD_SCREEN409,		// ��ʂS�O�X�\����
	LCD_SCREEN410,		// ��ʂS�P�O�\����
	LCD_SCREEN411,		// ��ʂS�P�P�\����
	
	LCD_SCREEN500,		// ��ʂT�O�O�\�����@�P�΂P�d�l�F�ʏ탂�[�h
	LCD_SCREEN501,		// ��ʂT�O�P�\����	
	LCD_SCREEN502,		// ��ʂT�O�Q�\����	
	LCD_SCREEN503,		// ��ʂT�O�R�\����	
	LCD_SCREEN504,		// ��ʂT�O�S�\����	
	LCD_SCREEN505,		// ��ʂT�O�T�\����	
	LCD_SCREEN506,		// ��ʂT�O�U�\����	
	
	LCD_SCREEN520,		// ��ʂT�Q�O�\�����@�P�΂P�d�l�F�o�^���[�h
	LCD_SCREEN521,		// ��ʂT�Q�P�\����
	LCD_SCREEN522,		// ��ʂT�Q�Q�\����
	LCD_SCREEN523,		// ��ʂT�Q�R�\����
	LCD_SCREEN524,		// ��ʂT�Q�S�\����
	LCD_SCREEN525,		// ��ʂT�Q�T�\����
	LCD_SCREEN526,		// ��ʂT�Q�U�\����
	LCD_SCREEN527,		// ��ʂT�Q�V�\����
	LCD_SCREEN528,		// ��ʂT�Q�W�\����
	LCD_SCREEN529,		// ��ʂT�Q�X�\����
	LCD_SCREEN530,		// ��ʂT�R�O�\����
	LCD_SCREEN531,		// ��ʂT�R�P�\����
	LCD_SCREEN532,		// ��ʂT�R�Q�\����
	LCD_SCREEN533,		// ��ʂT�R�R�\����
	LCD_SCREEN534,		// ��ʂT�R�S�\����
	LCD_SCREEN535,		// ��ʂT�R�T�\����
	LCD_SCREEN536,		// ��ʂT�R�U�\����
	LCD_SCREEN537,		// ��ʂT�R�V�\����
	LCD_SCREEN538,		// ��ʂT�R�W�\����
	
	LCD_SCREEN542,		// ��ʂT�S�Q�\�����@�P�΂P�d�l�F�폜���[�h
	LCD_SCREEN543,		// ��ʂT�S�R�\����
	LCD_SCREEN544,		// ��ʂT�S�S�\����
	LCD_SCREEN545,		// ��ʂT�S�T�\����
	LCD_SCREEN546,		// ��ʂT�S�U�\����
	LCD_SCREEN547,		// ��ʂT�S�V�\����
	LCD_SCREEN548,		// ��ʂT�S�W�\����
	LCD_SCREEN549,		// ��ʂT�S�X�\����
	LCD_SCREEN550,		// ��ʂT�T�O�\����
	LCD_SCREEN551,		// ��ʂT�T�P�\����

//20160108Miya FinKeyS ->
	LCD_SCREEN600,		// ���600�\����	 �ʏ탂�[�h2�iFinKeyS�j
	LCD_SCREEN601,		// ���601�\����
	LCD_SCREEN602,		// ���602�\����
	LCD_SCREEN603,		// ���603�\����
	LCD_SCREEN604,		// ���604�\����
	LCD_SCREEN605,		// ���605�\����
	LCD_SCREEN606,		// ���606�\����
	LCD_SCREEN607,		// ���607�\����
	LCD_SCREEN608,		// ���608�\����
	LCD_SCREEN609,		// ���609�\����
	LCD_SCREEN610,		// ���610�\����
	LCD_SCREEN611,		// ���611�\����
	LCD_SCREEN612,		// ���612�\����
	LCD_SCREEN613,		// ���613�\����

	LCD_SCREEN620,		// ���620�\����	 �H�����[�h
	LCD_SCREEN621,		// ���621�\����
	LCD_SCREEN622,		// ���622�\����
	LCD_SCREEN623,		// ���623�\����
	LCD_SCREEN624,		// ���624�\����
	LCD_SCREEN625,		// ���625�\����
//20160108Miya FinKeyS <-
	
	LCD_SCREEN999,		// ��ʂX�X�X�\�����@�u�V���b�g�_�E�����܂��v
		
	LCD_SCREEN_MAX
};

enum LCD_SCREEN_REQ {		// ���b�Z�[�W��o�b�t�@"MBF_LCD_DATA" �̐擪�o�C�g�Ŏg�p����B
	LCD_NEXT,					// ���̒ʏ��ʂւ̑J�ڂ�v��
	LCD_YES,					// "�͂�"�������ꂽ
	LCD_NO,						// "������"�������ꂽ
	LCD_BACK,					// "�߂�"�������ꂽ
	LCD_CANCEL,					// "���~"�������ꂽ
	LCD_INIT_INPUT,				// "�����o�^"�{�^���������ꂽ
	LCD_TOUROKU,				// "�o�^"�{�^���������ꂽ
	LCD_SAKUJYO,				// "�폜"�{�^���������ꂽ
	LCD_MAINTE,					// "�����e�i���X"�{�^���������ꂽ
	LCD_KINKYUU_SETTEI,			// "�ً}�ԍ��ݒ�"�{�^���������ꂽ
	LCD_KINKYUU_KAIJYOU,		// "�ً}����"�{�^���������ꂽ"
	LCD_PASSWORD_OPEN,			// �p�X���[�h�J���{�^���������ꂽ	//20140925Miya password_open
	LCD_KEYIN_PASSWORD,			// �J���p�X���[�h�����͂��ꂽ		//20140925Miya password_open
	LCD_ODEKAKE,				// ���ł����{�^���������ꂽ	//20160108Miya FinKeyS
	LCD_ORUSUBAN,				// ������ԃ{�^���������ꂽ	//20160108Miya FinKeyS
	LCD_SETTEI,					// �ݒ�{�^���������ꂽ	//20160108Miya FinKeyS
	
	LCD_USER_ID,				// "���[�U�[ID"�����͂��ꂽ
	LCD_YUBI_ID,				// "�ӔC��/��ʎ�"�̎wID���I�����ꂽ
	LCD_NAME,					// "����"�����͂��ꂽ
	LCD_YUBI_SHUBETU,			// "�w���"���I�����ꂽ
	LCD_MAINTE_END,				// "�I��"�{�^���������ꂽ
	LCD_KINKYUU_KAIJYOU_BANGOU,	// "�ً}�J���ԍ��@�W��"�����͂��ꂽ
	LCD_KINKYUU_BANGOU,			// "���[�U�[���ً͂̋}�ԍ��@�S�������͂��ꂽ

	LCD_PASS_HENKOU_REQ,		// �h�p�X���[�h�ύX�h�{�^���������ꂽ
	LCD_PASS_SETTEI_HENKOU_REQ,	// �h�p�X���[�h�J���ݒ�ύX�h�{�^���������ꂽ

	LCD_JYOUHOU_REQ,			// �h���h�{�^���������ꂽ
	LCD_SETTEI_HENKOU_REQ,		// �h�ݒ�ύX�h�{�^���������ꂽ
	LCD_SINDAN_REQ,				// �h�f�f�h�{�^���������ꂽ
	LCD_SYOKI_SETTEI_REQ,		// �h�����ݒ�h�{�^���������ꂽ
	LCD_VERSION_REQ,			// �h�o�[�W�����h�{�^���������ꂽ
	LCD_ERROR_REREKI_REQ,		// �h�G���[�����h�{�^���������ꂽ
	LCD_NINSYOU_JYOUKYOU_REQ,	// �h�F�؏󋵁h�{�^���������ꂽ
	LCD_JIKOKU_HYOUJI_REQ,		// �h�����m�F�h�{�^���������ꂽ
	LCD_PASS_KAIJYOU_REQ,		// �h�p�X���[�h�J���h�{�^���������ꂽ
	LCD_CALLCEN_TEL_REQ,		// �h�R�[���Z���^�[TEL�h�{�^���������ꂽ
	LCD_JIKOKU_AWASE_REQ,		// �h�������킹�h�{�^���������ꂽ
	LCD_ITI_TYOUSEI_REQ,		// �hLCD�ʒu�����h�{�^���������ꂽ
	LCD_MAINTE_SHOKIKA_REQ,		// "�������h�{�^���������ꂽ
	LCD_SPEC_CHG_REQ,			// �h�d�l�ؑցh�{�^���������ꂽ
	LCD_IMAGE_KAKUNIN_REQ,		// �h�摜�m�F�h�{�^���������ꂽ
	LCD_FULL_PIC_SEND_REQ,		// "�t���摜���M"�{�^���������ꂽ
	
	LCD_ENTER,					// "�m��"�{�^���������ꂽ
	LCD_OK,						// "�m�F"�{�^���������ꂽ
	LCD_MENU,					// "MENU"�{�^���������ꂽ
	LCD_KANTOKU,				// "�ē�"�{�^���������ꂽ
	LCD_KANRI,					// "�Ǘ���"�{�^���������ꂽ
	LCD_IPPAN,					// "��ʎ�"�{�^���������ꂽ	

	LCD_NOUSE,					// "���g�p"�{�^���������ꂽ	

	LCD_ERR_REQ,				// �ʏ탂�[�h�ҋ@���ɃG���[�����o����	//20140925Miya add err

	LCD_REQ_MAX	
};

//�ʐM�̏��
enum EmfStatus {
	ST_COM_STANDBY = 0,		// �ҋ@(0)
	ST_COM_RECEIVE,		// ��M��
		//�ȉ��͎�M���̒��̕���
	ST_COM_WAIT_LEN,	// �f�[�^���҂�
	ST_COM_WAIT_DATA,	// �f�[�^�҂�
	ST_COM_WAIT_CR,		// CR�҂�
	ST_COM_WAIT_LF,		// LF�҂�
	ST_COM_WAIT_EOT,	// EOT�҂�
	ST_COM_WAIT_ACK_NAK,	// ACK/NAK�҂�
	
	ST_COM_SLEEP,		// ��M��~
	
	ST_COM_MAX			// ��Ԑ�
};

//
enum LBUS_FMT_CHK {							///< �R�}���h�̃t�H�[�}�b�g�m�F
	FMT_PKT_ERR = -3,						///< �p�P�b�g�G���[
	FMT_SUM_ERR = -2,						///< �`�F�b�N�T���G���[
	FMT_CMD_ERR = -1,						///< �R�}���h�G���[
	FMT_CMD_NONE = 0,						///< �R�}���h�Ȃ�
	FMT_CMD_P2P,							///< ���A�h���X���p�P�b�g
	FMT_CMD_BC,								///< �u���[�h�L���X�g�p�P�b�g
};

enum SIO_MODE {
	SIO_RCV_IDLE_MODE,		// RS-232 �R�}���h��M/���M�A�C�h��
	SIO_RCV_ENQ_MODE,		// RS-232 ENQ��M�����
	SIO_RCV_WAIT_MODE,		// RS-232 ����Box����̃R�}���h��M�҂��B
	SIO_RCV_CMD_MODE,		// RS-232 ����Box����̃R�}���h��M����ԁB
	SIO_SEND_MODE,			// �R�}���h���M���B
	SIO_SEND_ACK_WAIT		// �R�}���h���M�ς݂ŁAACK/NAK�̎�M�҂��A����ю�t���B
};

enum RCV_ERR_CODE {							///< ��M�G���[�R�[�h
	RCV_ERR_NONE = 0,						///< ��M�G���[�Ȃ�
	RCV_ERR_STR_LIM,						///< ��M����������I�[�o�[
	RCV_ERR_PRTY,							///< �p���e�B�`�F�b�N�G���[
	RCV_ERR_CHKSUM,							///< �`�F�b�N�T���G���[
	RCV_ERR_OP,								///< �I�y�����h�G���[
	RCV_ERR_ACK,							///< �R�}���hAck��M�G���[
	RCV_ERR_RETRY,							///< �đ��M�񐔃I�[�o�[
	RCV_ERR_NAK,							///< �R�}���hNak��M�G���[
	RCV_ERR_EXE,							///< �R�}���hExe��M�G���[
	RCV_ERR_COMM							///< �R�}���h��M�G���[
};

enum RCV_COM_STAT {
	ERR_FIRST_CODE = -6,	// "@"�Ŗ������B
	ERR_CKSUM = -3,			// Ckeck Sum�̕s��v
	ERR_END_CODE = -2,		// �I�[�R�[�h�̕s��v
	ERR_DATA_LENG = -1,		// �f�[�^���̕s��v
	
	COMM_DATA_IDLE = 0,		// �f�[�^��MIDLE
	RCV_TEST_COM,			// �e�X�g�R�}���h�̎�M	
	RCV_NINSHOU_DATA,		// �F�؃f�[�^�̎�M
	RCV_TOUROKU_REQ,		// ����Box����o�^�v������
	RCV_NINSHOU_OK,			// ����Box����F�؋�����M�i���O�ɁA�[������F�؊m�F�R�}���h�𑗐M�ς݂ŁA����ɑ΂��鉞���R�}���h�j
	RCV_TEIDEN_STAT,		// ����Box�����d��Ԃ̉�������M�i���O�ɁA�[�������d��ԗv���R�}���h�𑗐M�ς݂ŁA����ɑ΂��鉞���R�}���h�j
	RCV_SHUTDOWN_OK,		// ����Box����V���b�g�_�E���v����������M�i���O�ɁA�[������@���v���R�}���h�𑗐M�ς݂ŁA����ɑ΂��鉞���R�}���h�j
	RCV_TEIDEN_EVENT,		// ��d��ԃC�x���g�񍐂̎�M
	RCV_ALARM_STAT,			// �x�񔭐��E�����C�x���g�񍐂̎�M
	RCV_ADJUST_TIME,		// �������킹�R�}���h�̎�M

	//20160930Miya PC����VA300S�𐧌䂷��
	RCV_TANMATU_INFO,		// A0 �[�����擾
	RCV_REGDATA_DNLD_STA,	// A1 �o�^�f�[�^�_�E�����[�h�J�n
	RCV_REGDATA_DNLD_GET,	// A2 �o�^�f�[�^�_�E�����[�h��
	RCV_REGDATA_DNLD_END,	// A3 �o�^�f�[�^�_�E�����[�h�I��
	RCV_REGDATA_UPLD_STA,	// A4 �o�^�f�[�^�A�b�v���[�h�J�n
	RCV_REGDATA_UPLD_GET,	// A5 �o�^�f�[�^�A�b�v���[�h��
	RCV_REGDATA_UPLD_END,	// A6 �o�^�f�[�^�A�b�v���[�h�I��
	RCV_REGPROC,			// A7 �o�^����
	RCV_AUTHPROC,			// A8 �F�ؑ���
	

	
	KEY_UNNOWN_REQ			// �s���Ȗ��߂̎�M
};

// �菇
enum PR_TYPE {
	PR_TYPE_A,								///< �菇A
	PR_TYPE_B,								///< �菇B
	PR_TYPE_C,								///< �菇C
	PR_TYPE_D,								///< �菇D
	PR_TYPE_E,								///< �菇E
};

enum TERM_NUMBER {							///< �[���ԍ��̒�`
	TERM_NO_1 = 1,							///< �[��1
	TERM_NO_2,								///< �[��2
};

enum SOUND_CTRL {							///< �T�E���h����
	SOUND_CTRL_EMG_ON,						///< �x��ON
	SOUND_CTRL_EMG_OFF,						///< �x��OFF
	SOUND_CTRL_TPL_OK,						///< �^�b�`�p�l���p�u�U�[OK��
	SOUND_CTRL_TPL_NG,						///< �^�b�`�p�l���p�u�U�[NG��
};

// �J�����B�e�֘A
enum CAP_COMMAND {						// �L���v�`���[���s���߃R�}���h�̎��
	COMMAND_NONE,
	COMMAND204,
	COMMAND210,
	COMMAND141
};	

#define TRIM_START_X	200				// �g���~���O�̊J�nX���W�i��������_�Ƃ��āj
#define TRIM_START_Y	180				// �g���~���O�̊J�nY���W
#define TRIM_SIZE_X		640				// �g���~���O��X�T�C�Y�i���T�C�Y�́A1280�~640�j
#define TRIM_SIZE_Y		320				// �g���~���O��Y�T�C�Y
#define RE_SIZE_X		640				// �k���O��X�T�C�Y
#define RE_SIZE_Y		560				//20131210Miya cng // �k���O��Y�T�C�Y
//#define RE_SIZE_Y		320				// �k���O��Y�T�C�Y


#if(AUTHTEST >= 1)	//20160613Miya
#define ADDR_OKIMG1			0x040000		//�F��OK�摜�ۑ��J�n�A�h���X1(Flash)
#define ADDR_OKIMG2			0x060000		//�F��OK�摜�ۑ��J�n�A�h���X2(Flash)
#define ADDR_OKIMG3			0x160000		//�F��NG�摜�ۑ��J�n�A�h���X1(Flash)
#define ADDR_OKIMG4			0x180000		//�F��NG�摜�ۑ��J�n�A�h���X2(Flash)
#endif

#define ADDR_REGAUTHDATA	0x1A0000		//�F�؃p�����[�^�ۑ��J�n�A�h���X1(Flash)
#define ADDR_REGIMG1		0x120000		//�o�^�摜�ۑ��J�n�A�h���X1(Flash)
#define ADDR_REGIMG2		0x140000		//�o�^�摜�ۑ��J�n�A�h���X2(Flash)

#define ADDR_REGAUTHDATA2	0x1C0000		//�F�؃p�����[�^�ۑ��J�n�A�h���X2(Flash) //20161031Miya Ver2204
#define ADDR_ADDLCDGAMEN	0x1E0000		//LCD�ǉ����(Flash) //20161031Miya Ver2204

//20140925Miya password_open
#define FLG_ON	1
#define FLG_OFF 0

/**
 *	���[�h�^�X�N�ւ̃R�}���h
 *	 000�` 999:�ʐM�C���^�[�t�F�[�X�̃R�}���h
 *	1000�`    :���[�h�^�X�N�ւ̃R�}���h
 */
enum MD_CMD {								///< ���[�h�^�X�N�ւ̃R�}���h)
	MD_CMD_ACK_RCV  = 1000,					///< ACK��M
	MD_CMD_EOT_RCV,							///< EOT��M
	MD_CMD_SND_FAIL,						///< ��M���s�ʒm
	
	MD_CMD_FIN_SET,							///< �w�}���ʒm
	MD_CMD_KEY_ON,							///< �^�b�`�p�l���A�O���L�[����
	
	MD_CMD_POL_REQ = 2000,					///< �|�[�����O�v��
	MD_CMD_RCV,								///< �R�}���h��M�ʒm
};

/// @name �}�N����`
//@{
//=== ����t���O ===
#define	FLAG_VERIFY		0x01

#ifndef	STX
#define	STX				0x02				///< �ʐMSTX��`
#endif
#ifndef	ETX
#define	ETX				0x03				///< �ʐMETX��`
#endif
#define	CRLF			"\x0D\x0A"			///< �ʐM�I�[��`

#ifndef	TRUE
	#define	TRUE			1				///< TRUE��`
#endif
#ifndef	FALSE
	#define	FALSE			0				///< FALSE��`
#endif

/// ���Z�b�g
#define	reset()				(*(void (*)(void))0xA0000000)()

/// 
#define	LAN_BUF_SIZE		128				///< LAN�̑���M�o�b�t�@�T�C�Y
#define	LBUS_BUF_SIZE		256				///< ���[�J���o�X�̑���M�o�b�t�@�T�C�Y
//#define	MAX_SND_LAN			1450			///< �ő�LAN���M�T�C�Y(UDP�͍ő�1472�o�C�g)
//#define	MAX_SND_LAN			65535			///< �ő�LAN���M�T�C�Y(Mispo��UDP�]���ő�T�C�Y)
#define	MAX_SND_LAN			8192			///< �ő�LAN���M�T�C�Y(Winsock�ő��M�T�C�Y)
#define	MSG_SIZE	1024+1	///< �^�X�N�ԃ��b�Z�[�W�o�b�t�@�T�C�Y

#define	LBUS_RTRY_MAX		3				///< ���[�J���o�X�̍đ��ő吔

// �o�b�N�A�b�v
#define	HEAD_MARK			0x4F796F00		///< �L���R�[�h

// �e�X�g�s����`
#define	TP_REV(n)			sfr_outw(BSC_PDTRB, (sfr_inw(BSC_PDTRB) ^ (n << 1)))
#define	TP_CLR(n)			sfr_outw(BSC_PDTRB, (sfr_inw(BSC_PDTRB) & ~(n << 1)))

//=== �f�o�b�O�p ===
#if defined( _DEBUG_)
	#define DPRINT	DebugOut
#else
	#define DPRINT
#endif


#ifndef NO_DEBUG 
#define Pfail_start_time   2400				// ��d���m��̂Q���ԊĎ��^�C�}�[�l
#define EOT_WAIT_TIME		100				// EOT�҂��^�C���A�E�g���ԁi�P�b�j
#define ACK_WAIT_TIME		100				// Ack�҂��^�C���A�E�g���ԁi�P�b�j

#else
#define Pfail_start_time    1000				// ��d���m��̂Q���ԊĎ��^�C�}�[�l
#define EOT_WAIT_TIME		1000				// EOT�҂��^�C���A�E�g���ԁi�P�b�j
#define ACK_WAIT_TIME		1000				// Ack�҂��^�C���A�E�g���ԁi�P�b�j

#endif

#define NINSHOU_WAIT_TIME	1000 * 3		// �w�F�؎��s���̉�ʑ҂����ԁi30�b�j

#define ENQ_SENDING_MAX 9999

//@}

/// @name �^��`
//@{
typedef struct st_mcn_status {			///< ���u�X�e�[�^�X 
	ER	erLan;							///< LAN��ԃX�e�[�^�X
	
} ST_MCN_STS;

typedef struct st_param{				///< �p�����[�^���\����
	UW   uwMark;						///< �Œ�l(�l�L���`�F�b�N)
	
	
	UH   uhChkSum;						///< �`�F�b�N�T��
} ST_PARAM;

typedef struct st_mcn_param{			///< ���u�p�����[�^���\����
	UW	uwMark;							///< �Œ�l(�l�L���`�F�b�N)
	TMO	tmRcvTmout;						///< ��M�^�C���A�E�g(ms)
	int	iSendRetry;						///< ���M���g���C��
	TMO	tmPoling;						///< �|�[�����O�Ԋu(ms)
	TMO	tmFirstTmout;					///< �����o�^���Ȃ��Ƃ��̃^�C���A�E�g����(ms)
	TMO	tmLCDdelay;						///< ������ʐؑ֎���(ms)
	int	iStartX;						///< �؂�o���J�nX���W
	int	iStartY;						///< �؂�o���J�nY���W
	TMO	tmPowOff;						///< ��d���d��OFF����(ms)
	enum HOST_TYPE	eTermNo;			///< �[���ԍ�
	
	UH	uhChkSum;						///< �`�F�b�N�T��
} ST_MCN_PARAM;

typedef struct st_ctrl {				///< ����\����
	UW   uwMark;						///< �Œ�l(�l�L���`�F�b�N)
	
//	ST_RTC stTime;						///< ����
	UH   uhFlag;						///< ���萧��t���O
	UH   uhChkSum;						///< �`�F�b�N�T��
} ST_CTRL;

typedef struct t_commsg{				//=== �^�X�N�ԃ��b�Z�[�W�̒�` ===
	struct t_commsg *next;				///< �擪4�޲Ă� OS �Ŏg�p(4)
	INT	msgpri;							// ���b�Z�[�W�D��x
	UH	cnt;							///< �f�[�^��
	UB	buf[ MSG_SIZE ];				///< �f�[�^
}T_COMMSG;

typedef struct t_lancmdmsg{
	struct t_lancmdmsg *next;			///< �擪4�޲Ă� OS �Ŏg�p(4)
	INT		ch;							///< �`�����l���ԍ�
	BOOL	rflg;						///< ���X�|���X�v��
	UH		cnt;						///< �f�[�^��
	B		buf[ LAN_BUF_SIZE ];
} T_LANCMDMSG;

typedef struct t_lanresmsg{
	struct t_lanresmsg *next;			///< �擪4�޲Ă� OS �Ŏg�p(4)
	INT		msgpri;						///< ���b�Z�[�W�D��x
	UH		cnt;						///< �f�[�^��
	B		buf[ MAX_SND_LAN ];
} T_LANRESMSG;

typedef struct t_mdcmdmsg{
	struct t_mdcmdmsg *next;			///< �擪4�޲Ă� OS �Ŏg�p(4)
	INT		msgpri;						///< ���b�Z�[�W�D��x
	UH		uhCmd;						///< �R�}���h
	UH		uhBlkNo;					///< �u���b�N�ԍ�
	ID		idOrg;						///< ID
	UH		cnt;						///< �f�[�^��
	B		cData[ MAX_SND_LAN ];
} T_MDCMDMSG;


typedef struct t_ybdata{		//=== �o�^���̒�` ===
	UB	tou_no[ 3 ];			// ���ԍ��Q���{","
	UB	user_id[ 5 ];			// ���[�U�[ID�S���{","
	UB	yubi_seq_no[ 4 ];		// �o�^�w�ԍ��i�ӔC�ҁ{��ʎҁj�R���{","
	UB	kubun[ 2 ];				// �ӔC��/��ʎ҂̋敪�P���{","
	UB	yubi_no[ 3 ];			// �w�̎��ʔԍ��i�E�A���A�ǂ̎w�j�Q���{","
	UB	name[ 25 ];				// �o�^�Ҏw���Q�S���{","
	//UB	ybCapBuf[ ( ( RE_SIZE_X * RE_SIZE_Y ) * 3 ) ];	// �w�摜�f�[�^�i�k���摜�R�����j
	UB	ybCapBuf[ 640 * 480 ];	// �w�摜�f�[�^�i�k���摜�R�����j//20160601Miya forDebug
} T_YBDATA;


// �ǉ��@20130510 Miya
typedef struct t_ybdata20{		//=== �o�^���̒�` ===
	UB	yubi_seq_no[ 4 ];		// �o�^�w�ԍ��i�ӔC�ҁ{��ʎҁj�R���{","
	UB	kubun[ 2 ];				// �ӔC��/��ʎ҂̋敪�P���{","
	UB	yubi_no[ 3 ];			// �w�̎��ʔԍ��i�E�A���A�ǂ̎w�j�Q���{","
	UB	name[ 25 ];				// �o�^�Ҏw���Q�S���{","
	//UB	ybCapBuf[ ( ( RE_SIZE_X * RE_SIZE_Y ) * 3 ) ];	// �w�摜�f�[�^�i�k���摜�R�����j
} T_YBDATA20;


/*���[�U���*/
typedef	struct STRUCT_REGUSERINFO{
	unsigned short	RegSts;				//�o�^�󋵊m�F�@0�F���o�^�@1�F�o�^�ς�
	unsigned short	BlockNum;			//���ԍ��@"00"?"99"
	unsigned short	UserId;				//���[�UID�i�����ԍ��j�@"0000"?"9999"
	unsigned short	RegNum;				//�����o�^�ԍ��@"001"?"100"
	unsigned short	TotalNum;			//���[�U�w�o�^�{��
	unsigned char	MainteTelNum[16];	//�ً}�ԍ�(4�P�^)
	unsigned char	KinkyuNum[4];		//�ً}�ԍ�(4�P�^)
	unsigned short	RegImageSens;		//�o�^���̎B�e���x 0:���xup(���߰��ް) 1:���xup(��ް) 2:�W�� 3:���xdown(�����E�q��)
	//unsigned short	RegDensityMax[4];	//�o�^��4�����̔Z�x�lMAX
	//unsigned short	RegDensityMin[4];	//�o�^��4�����̔Z�x�lMIN
	//unsigned short	RegSlitInfo;		//�o�^���X���b�g��� 0:�X���b�g���� 1:��X���b�g 2:���X���b�g 3:���X���b�g
	unsigned short	RegHiDensityDev[4];	//�o�^��4�����̔Z�x�lAVE
	unsigned short	RegDensityAve[3];	//H/M/L�̔Z�x�l�@0:H 1:M 2:L
	unsigned short	RegFingerSize;		//�o�^���w����(���Ή�) 0:���߰��ް 1:��ް 2:�W�� 3:�ׂ� 4:�ɍ�
	unsigned short	CapNum;				//1���2��ڋ��
	unsigned short	kinkyu_times;
	unsigned short	r1;					//�F�؃X�R�A�[(R1)	//20140925Miya add log
	unsigned short	r2;					//�F�؃X�R�A�[(R1)	//20140925Miya add log
	unsigned short	lbp_lvl;			//20140905Miya lbp�ǉ��@�F�؎���LBP���x��
	unsigned short	lbp_pls;			//20140905Miya lbp�ǉ��@�F�؎���LBP�ɂ�錸�_�ɘa���{�������ǂ���
	unsigned short	xsft;				//20140910Miya XSFT
	unsigned short	trim_sty;				//FLASH��4byte�A���C�����g�ɂ�dummy�ǉ� //20140925 add log
}RegUserInfoData;

/*�o�^�����摜�^�O���*/
typedef	struct STRUCT_REGBVATAG{
	unsigned short	CapNum;				//1���2��ڋ��
	unsigned short	RegInfoFlg;			//�o�^�󋵊m�F�@0�F���o�^�@1�F�o�^�ς݁@2?�F�w�K�摜����@0xFF�F�폜
	unsigned short	BlockNum;			//���ԍ��@"00"?"99"
	unsigned short	UserId;				//���[�UID�i�����ԍ��j�@ASCII4�����@"0000"?"9999"
	unsigned short	RegNum;				//�����o�^�ԍ��@"001"?"100"
	unsigned short	Level;				//�o�^�҃��x���@"0"�F�ēҁ@"1"�F�Ǘ��ҁ@"2"�F��ʎ�
	unsigned short	RegFinger;			//�o�^�w�@"01"�F�E�e�w�@"02"�F�E�l���w�@"03"�F�E���w�@"04"�F�E��w�@"05"�F�E���w
										//        "11"�F���e�w�@"12"�F���l���w�@"13"�F�����w�@"14"�F����w�@"15"�F�����w
	unsigned char	Name[24];			//�o�^�Җ��O
	unsigned short	RegImageSens;		//�o�^���̎B�e���x 0:���xup(���߰��ް) 1:���xup(��ް) 2:�W�� 3:���xdown(�����E�q��)
	//unsigned short	RegDensityMax[4];	//�o�^��4�����̔Z�x�lMAX
	//unsigned short	RegDensityMin[4];	//�o�^��4�����̔Z�x�lMIN
	//unsigned short	RegSlitInfo;		//�o�^���X���b�g��� 0:�X���b�g���� 1:��X���b�g 2:���X���b�g 3:���X���b�g
	unsigned short	RegHiDensityDev[4];	//�o�^��4�����̔Z�x�lAVE
	unsigned short	RegDensityAve[3];	//H/M/L�̔Z�x�l�@0:H 1:M 2:L
	unsigned short	RegFingerSize;		//�o�^���w����(���Ή�) 0:���߰��ް 1:��ް 2:�W�� 3:�ׂ� 4:�ɍ�
	unsigned char	MinAuthImg[2][200];	//�ɏ��摜
	unsigned short	end_code;
}RegBloodVesselTagData;

/*�摜�L���v�`���[�����p�����[�^*/
typedef	struct STRUCT_CAPIMGPARA{
	short		PictureSizeX;		//�J�����B�e�T�C�Y�iX)
	short		PictureSizeY;		//�J�����B�e�T�C�Y�iY)
	short		TrimStPosX;			//�g���~���O�J�n���W�iX)
	short		TrimStPosY;			//�g���~���O�J�n���W�iY)
	short		TrimSizeX;			//�g���~���O�摜�T�C�Y�iX)
	short		TrimSizeY;			//�g���~���O�摜�T�C�Y�iY)
	short		ResizeMode;			//�摜���k���@0�F��1/2�@1�F��1/4�@2�F��1/8�@3�F��1/1�i���{�j
	short		CapSizeX;			//�L���v�`���[�T�C�Y�iX)
	short		CapSizeY;			//�L���v�`���[�T�C�Y�iY)
	unsigned char	DataLoadFlg;	//�p�����[�^�̃��[�h�� 0�F�������@1�F�����ς݁@2�FINI����
}CapImgData;

/*�摜���F�؏����p�����[�^*/
typedef	struct STRUCT_IMGPROCAUTHPROCPARA{
	unsigned char	Proc;				//�o�^�󋵊m�F�p�t���O�@0�F���o�^�@1�F�o�^�ς݁@2?�F�w�K�摜����@0xFF�F�폜
	short		InpSizeX;				//���̓T�C�Y�iX)
	short		InpSizeY;				//���̓T�C�Y�iY)
	short		IrLedPos[8];			//�摜�����̈��ݒ� ���̓T�C�Y�iX)�
										//[0]�FLED1-LED2�ԁ@[1]�FLED2-LED3�ԁ@[2]�FLED3-LED4��
	unsigned short AuthLvlPara;			//�F�؃A���S��臒l
	short	LearningPara;				//�w�K�@�\�@�O�FOFF�@�P�F�ȈՊw�K�@�Q�F�ʏ�w�K
	int		LvlCameraErrHi;				//�q�X�g�O�����P�x���ϒl�̔���(�J�����G���[���)
	int		LvlCameraErrLo;				//�q�X�g�O�����P�x���ϒl�̔���(�J�����G���[����)
	int		LvlHistAveHi;				//�q�X�g�O�����P�x���ϒl�̔���(�����x�摜�������l)
	int		LvlHistAveLo;				//�q�X�g�O�����P�x���ϒl�̔���(�ኴ�x�摜�������l)
	int		LvlMeiryouLoAll;			//�א������ēx�̉����i�S�́j
	int		LvlMeiryouLoPart;			//�א������ēx�̉����i�����j
	int		LvlMeiryouHiAll;			//�א������ēx�̏���i�S�́j
	int		LvlMeiryouHiPart;			//�א������ēx�̏���i�����j
	int		ThLowPart1;					//�F�؃X�R�A�[���_臒l1(250�_�ȉ�)
	int		ThLowPart2;					//�F�؃X�R�A�[���_臒l2(200�_�ȉ�)
	int		WeightLowPart1;				//�F�؃X�R�A�[���_�W��1(1/100�l���g�p����)
	int		WeightLowPart2;				//�F�؃X�R�A�[���_�W��1(1/100�l���g�p����)
	unsigned short LvlBrightLo;			//���邳����i�Ã��x���j臒l
	unsigned short LvlBrightLo2;		//���邳����i�Ã��x���j��f��
	unsigned short LvlBrightHi;			//���邳����i�����x���j臒l
	unsigned short LvlBrightHiNum;		//���邳����i�����x���j��f��
	unsigned short LvlFingerTop;		//�w��R���g���X�g����臒l(20)
	unsigned short LvlSlitCheck;		//�X���b�g����[�����x��(20)
	unsigned short NumSlitCheck;		//�X���b�g����[����f��(20)
	unsigned short LvlSlitSensCheck;	//�X���b�g���萶�̃Z���T�[���m���x��(60)
	unsigned short StSlitSensCheck;		//�X���b�g���萶�̃Z���T�[���m�J�n��f(30)
	unsigned short EdSlitSensCheck;		//�X���b�g���萶�̃Z���T�[���m�I����f(60)
	unsigned short NgImageChkFlg;		//�摜�ُ픻��(0:�Ȃ� 1:���� 2:�������ُ�̂� 3:�w�����̂�)
	unsigned short WeightXzure;			//X�������ꌸ�_�d�݌W��(20->0.2)
	unsigned short WeightYzure;			//Y�������ꌸ�_�d�݌W��(30->0.3)
	unsigned short WeightYmuki;			//Y�������ꌸ�_�����W��(20->0.2)
	unsigned short WeightYhei;			//Y�������ꕽ�s����W��(20->0.2)
	unsigned short WeightScore400;		//�F�؃X�R�A�[400�ȉ����_�d�݌W��(90->0.9)
	unsigned short WeightScore300;		//�F�؃X�R�A�[300�ȉ����_�d�݌W��(90->0.9)
	unsigned short WeightScore200;		//�F�؃X�R�A�[200�ȉ����_�d�݌W��(75->0.75)
	unsigned short WeightScore200a;		//�F�؃X�R�A�[200�ȉ����_�W��(25->0.0025)
	unsigned short WeightScore200b;		//�F�؃X�R�A�[200�ȉ����_�ؕ�(25->0.25)
	unsigned short WeightXzureSHvy;		//�X�[�p�[�w�r�[��X�������ꌸ�_�d�݌W��(8->0.08)
	unsigned short WeightYzureSHvy;		//�X�[�p�[�w�r�[��Y�������ꌸ�_�d�݌W��(8->0.08)
	unsigned short WeightYmukiSHvy;		//�X�[�p�[�w�r�[��Y�������ꌸ�_�����W��(100->1)
	unsigned short WeightYheiSHvy;		//�X�[�p�[�w�r�[��Y�������ꕽ�s����W��(20->0.2)
	unsigned short WeightScore400SHvy;	//�X�[�p�[�w�r�[���F�؃X�R�A�[400�ȉ����_�d�݌W��(90->0.9)
	unsigned short WeightScore300SHvy;	//�X�[�p�[�w�r�[���F�؃X�R�A�[300�ȉ����_�d�݌W��(90->0.9)
	unsigned short WeightScore200SHvy;	//�X�[�p�[�w�r�[���F�؃X�R�A�[200�ȉ����_�d�݌W��(75->0.75)
	unsigned short WeightScore200aSHvy;	//�X�[�p�[�w�r�[���F�؃X�R�A�[200�ȉ����_�W��(25->0.0025)
	unsigned short WeightScore200bSHvy;	//�X�[�p�[�w�r�[���F�؃X�R�A�[200�ȉ����_�ؕ�(25->0.25)
}ImgAndAuthProcSetData;

//20160312Miya �ɏ����xUP
typedef	struct STRUCT_REGIMGDATAADD{
	unsigned short	RegImageSens;			//�o�^���̎B�e���x 0:���xup(���߰��ް) 1:���xup(��ް) 2:�W�� 3:���xdown(�����E�q��)
	unsigned short	RegFingerSize;			//�o�^���w����(���Ή�) 0:���߰��ް 1:��ް 2:�W�� 3:�ׂ� 4:�ɍ�
	unsigned short	Sel1stRegNum;			//1�ʂɑI�΂ꂽ�̓o�^�ԍ�
	unsigned short	RegContrast[2][2][16];		//16�����R���g���X�g�� [�o�^/�w�K][R1/R2][�f�[�^]
	UB				RegLbpImg[2][80*40];	//[�o�^/�w�K][��f��]
	UB				RegR1Img[2][80*40];		//[�o�^/�w�K][��f��]
	UB				RegMiniR1Img[2][20*10];	//[�o�^/�w�K][��f��]
}RegImgDataAdd;

/*�摜���F�؏����X�e�[�^�X���*/
typedef	struct STRUCT_STSIMGPROCAUTHPROC{
	unsigned char	AuthResult;		//�摜�������F�؏����̌��� 0�F����@1�FRetry�@2?�F�ُ�
	short		RetruCnt;			//�摜�������F�؏����̃��g���C��
	short		CmrGain;			//�J�����Q�C���̐ݒ� 0�F�Ȃ��@1�F+�␳�@-1�F-�␳
	short		L_ImgSSvalue;		//�V���b�^�[���x�̐ݒ� 0�F�Ȃ��@1�F+�␳�@-1�F-�␳
	short		N_ImgSSvalue;		//�V���b�^�[���x�̐ݒ� 0�F�Ȃ��@1�F+�␳�@-1�F-�␳
	short		H_ImgSSvalue;		//�V���b�^�[���x�̐ݒ� 0�F�Ȃ��@1�F+�␳�@-1�F-�␳
	short		IrLedSw[8];			//LED�_���ݒ� 0�F�ύX�����@1�F�_���@-1�F����
									//[0]�FLED1�@[1]�FLED2�@[2]�FLED3�@[3]�FLED4"
	unsigned short	StsErr;			//�G���[�󋵔c�� 0�F���� 1?�F�ُ�
}StsImgAndAuthProc;

/*���[�U�[�g�p�ԍ�*/
typedef struct STRUCT_USEPROCNUM{
	int		MainteUsb;				//�����eUSB�L���`�F�b�N�@0:�Ȃ� 1:����
	char	SvMaintePassWord[4];	//�����e�i���X�p�X���[�h(�o�^�ԍ�)
	char	InMaintePassWord[4];	//�����e�i���X�p�X���[�h(���͔ԍ�)
	char	OpenKeyNum[8];			//�ً}�����L�[8�P�^�ԍ�
	char	CalOpenCode[8];			//�ً}�����R�[�h
	char	InOpenCode[8];			//�ً}�����R�[�h
	char	InKinkyuNum[4];			//�ً}�ԍ�(���͔ԍ�)
	char	InTouNum[2];			//���ԍ�(�����ԍ�)
	char	InUserIdNum[4];			//���[�UID
	char	InChkRegLvl[1];			//�ӔC�ҋ敪 0:�ē� 1:�Ǘ��� 2:��ʎ�
}UseProcNum;

//�p�X���[�h�J��	//20140925Miya password_open
typedef struct STRUCT_PASSWORDOPEN{
	char	sw;				//�p�X���[�h�J��SW 0:OFF 1:ON
	char	hide_num;		//��\��SW 0:OFF 1:ON(��\�����{�A�*��\��)
	char	kigou_inp;		//�L������SW 0:OFF 1:ON
	char	random_key;		//�L�[�{�[�h�����_���\��SW 0:OFF 1:ON
	short	keta;			//���̓P�^��(4�`8)
	char	password[10];	//�p�X���[�h
}PasswordOpen;

//�p�X���[�h�J��2	//20160108Miya FinKeyS
typedef struct STRUCT_PASSWORDOPEN2{
	short	family_sw;			//�Ƒ����p�� 0:���Ȃ� 1:����
	short	num;
	short	keta[20];			//���̓P�^��(4�`8)
	char	password[20][10];	//�p�X���[�h
	char	dummy[4];
}PasswordOpen2;

//�Z�p�����e�@//20140925Miya add mainte
typedef struct STRUCT_TECHMENUDATA{
	short	SysSpec;		//�d�l 0:��L 1:���p 2:�@�l
	short	DemoSw;			//�f�����[�h 0:OFF 1:ON
	//short	YslitSw;			//�f�����[�h 0:OFF 1:ON
	short	HijyouRemocon;	//��탊���R���g�p 0:OFF 1:ON
	short	DebugHyouji;	//�f�o�b�O�p�\�� 0:OFF 1:ON
	short	CamKando;		//�J�������x 0:�� 1:�� 2:��
	short	CmrCenter;		//20160610Miya �J�����Z���^�[(NEWCMR) //LED���邳 0:OFF 1:�� 2:��
}TechMenuData;

//�F�؃��O	//20140925Miya add log
typedef struct STRUCT_AUTHLOG{
	unsigned long	ok_cnt;
	unsigned long	ng_cnt;
	unsigned long	ok_cnt1st;
	unsigned long	ok_cnt2nd;
	unsigned long	ok_cnt3rd;
	unsigned long	ok_finger_cnt[20];
	short			wcnt;
	short			now_result[8][4];	//[��][���e]�@���e 0:�o�^�ԍ� 1:R1 2:R2 3:LBP
	unsigned short	now_seq_num;
}AuthLog;

//�����e���O //20140925Miya add log
typedef struct STRUCT_MAINTELOG{
	unsigned short	err_wcnt;
	unsigned short	err_rcnt;
	unsigned short	err_buff[128][5];	//[��][���e] ���e 0:�G���[�ԍ� 1:�� 2:�� 3:�b 4:���(0:����1:������)
	unsigned short	now_hour;
	unsigned short	now_min;
	unsigned short	now_sec;
	unsigned short	st_hour;
	unsigned short	st_min;
	unsigned short	st_sec;
	unsigned short	diag_cnt1;
	unsigned short	diag_cnt2;	//���C�t�J�E���^�[
	unsigned short	chk_num;	//�ٕ��`�F�b�N���x��
	unsigned short	chk_ave;	//�ٕ��`�F�b�N���x��
	unsigned char	diag_buff[8][16];	//[��][�f�f���e]  0xff:NG 1:OK
	//unsigned short	dmy;				//FLASH��4byte�A���C�����g�ɂ�dummy�ǉ� //20140925 add log
	unsigned short	cmr_seq_err_f;		//�J�����V�[�P���X�G���[�����t���O(�G���[�ԍ�39)
	unsigned short	end_code;
}MainteLog;


//20150531Miya
typedef struct POSGENTEN{
	int 	zure_sum_x;		//�����SUM�l
	double	ttl_dat_x;		//�݌vSUM�l
	int 	zure_sum_y;		//�����SUM�l
	double 	ttl_dat_y;		//�݌vSUM�l
	int		pm_zure_y;		//���ꂪ+�����A-�����ɂ΂���Ă��邩 0:�΂���Ȃ� 1:�΂������
	int		parallel_y;		//���s���ꂪMAX���ǂ��� 0:MAX�łȂ� 1:MAX
	int		auth_num_in;
	int		auth_learn_in;
	int		low_f;
	int		x_scr1;
	int		scr1;
	int		scr2;
	double	gen1;
	double	gen2;
}PosGenten;

//20161031Miya Ver2204 LCDADJ
typedef struct BKDATANOCLEAR{
	int		LedPosiSetFlg;		//LED�ʒu�ݒ���{�t���O�@0x1234:���{
	int		LedPosi;			//0:�ʏ� 1:5mm����
	int		LcdAdjFlg;			//LCD�������{�t���O�@0x1234:���{
	int		LcdAdjBaseX[3];		//�ڕW���WX
	int		LcdAdjBaseY[3];		//�ڕW���WY
	int		LcdAdjInputX[3];	//���͍��WX
	int		LcdAdjInputY[3];	//���͍��WY
	int		LiveCnt;			//�N���J�E���g
	int		InitRetryCnt;		//�����N�����g���C�J�E���g
}BkDataNoClear;

//@}

/// @name �L��ϐ��錾
//@{
#if defined(EXTERN)
#undef EXTERN
#endif

#ifdef	_MAIN_
	#define	EXTERN
#else
	#define	EXTERN	extern
#endif
//-----------------------------------------------------------------------------
//	�ۑ����Ȃ��f�[�^
//-----------------------------------------------------------------------------
EXTERN ST_MCN_STS stMcnSts;
// network
EXTERN UB ethernet_addr[6];
EXTERN UB default_ipaddr[4];
EXTERN UB default_gateway[4];
EXTERN UB subnet_mask[4];
EXTERN UH udp_portno;
EXTERN UH telnet_portno;						///< TELNET�̃|�[�g�ԍ�
//-----------------------------------------------------------------------------
//	�ۑ�����f�[�^
//-----------------------------------------------------------------------------
#pragma section PARAM
EXTERN ST_PARAM stParam;
EXTERN ST_MCN_PARAM stMcnParam;
EXTERN ST_CTRL stCtrl;
#pragma section

extern UH cmrGain;			// �J�����E�Q�C���l�@PC����UDP�R�}���h�o�R�ŗ^����ꂽ�l���A�ʐM�̎w���ɏ]���Ă��̓s�x�J�����ɐݒ肷��B
extern UH cmrFixShutter1;	// Fix Shutter Control�l(�P���)�@���ۂ�FPGA�o�R�ŃJ�����R���g���[���ׂ̈ɎQ�Ƃ���l
extern UH cmrFixShutter2;	// Fix Shutter Control�l(�Q���)			����
extern UH cmrFixShutter3;	// Fix Shutter Control�l(�R��ځ��V���b�^�[�X�s�[�h�A�����l���󂯂��^�C�~���O�ŁA�J�����ɂ����ɐݒ肷��)�@����

extern UH ini_cmrGain;					// �J�����E�Q�C���l�����l�@
extern UH ini_cmrFixShutter1;			// Fix Shutter Control�l�����l(�P���)�@
extern UH ini_cmrFixShutter2;			// Fix Shutter Control�l�����l(�Q���)			����
extern UH ini_cmrFixShutter3;			// Fix Shutter Control�l�����l(�R��ځj

extern int iStartX, iStartY, iSizeX, iSizeY;// �L���v�`���摜�̃g���~���O�J�n���W�ƃT�C�Y
extern int iReSizeX, iReSizeY;				// �k���摜��X�T�C�Y�AY�T�C�Y
extern int iReSizeMode;						// �k���摜�̏k�����@0:��1/2�A1:��1/4�A2:��1/8

extern UB irDuty2;			// IR LED Duty�lirDuty2;
extern UB irDuty3;
extern UB irDuty4;
extern UB irDuty5;

extern UB ini_irDuty2;		// IR Duty�lirDuty2�@�����l;
extern UB ini_irDuty3;
extern UB ini_irDuty4;
extern UB ini_irDuty5;

extern int g_CmrParaSet;				//�J�����p�����[�^�ݒ�t���O
extern UB g_IrLed_SW;					//20160312Miya �ɏ����xUP

extern ID	rxtid_org;		// GET_UDP�ׂ̈̌��X�̃^�X�NID
extern ID	rxtid;			// GET_UDP�ׂ̈̃^�X�NID

extern UB RegImgBuf[20][2][2][80*40];	//[�o�^��][�w�K][�\�[�x��][�F�؃T�C�Y]

extern UINT timer_10ms, count_1sec, count_1min, count_1hour;	// �����b�J�E���g


extern UB CapImgBuf[2][2][80*40];		//[�o�^��][�\�[�x��][�F�؃T�C�Y] //20160906Miya



//@}

/// @name �v���g�^�C�v�錾
//@{

// va300.c
EXTERN UB sys_kindof_spec;	 // �}���V�����d�l/�P�΂P�d�l�@�ؑւ��t���O�@0:�}���V����(��L��)�d�l�A1:1�΂P�i�@�l�j�d�l
						       // 			�A2�F�}���V�����i���p���j�d�l�B3:�f���@�E�}���V����(��L��)�d�l�A4:�f���@�E1�΂P�i�@�l�j�d�l 
							   // 			�A5�F�f���@�E�}���V�����i���p���j�d�l�B
EXTERN UB nyuu_shutu_kubun;	//  ���^�J�d�l�E���ގ��敪�@0�F�����ށA1�F�����A2�F�o���B

EXTERN UH g_DipSwCode;
							   
EXTERN int g_AuthCnt;	//20140423Miya �F�؃��g���C��

EXTERN void telnetd_send(B *s);					///< TELNET���M
EXTERN void telnetd_send_bin(B *s, INT len);	///< TELNET�o�C�i�����M
EXTERN UB GetScreenNo(void);					// ���ݕ\���X�N���[���ԍ��̎擾
EXTERN void ChgScreenNo( UB NewScreenNo );		// ��ʑJ�ڏ��(�ԍ�)���X�V����
EXTERN void reload_CAP_Param( void );			// Gain. FixShutter,IR_LED�̃p�����[�^���A�����v���l�ɖ߂��B�iCAP��Retry���ɕς�����\��������ׁB�j
EXTERN ER send_shutdown_req_Wait_Ack_Retry( void ); // �V���b�g�_�E���v���̑��M�AAck�ENack�҂��ƃ��g���C�t��
EXTERN ER send_touroku_delete_Wait_Ack_Retry( void );	// �w�i�o�^�j�f�[�^�̍폜�v���̑��M�AAck�ENack�҂��ƃ��g���C�t��
EXTERN ER send_ID_No_check_req_Wait_Ack_Retry( void );	// ID�ԍ��⍇���̑��M�AAck�ENack�҂��ƃ��g���C�t��
EXTERN ER send_ID_Authority_check_req_Wait_Ack_Retry( void );	// ID�����⍇���̑��M�AAck�ENack�҂��ƃ��g���C�t��
EXTERN ER send_donguru_chk_Wait_Ack_Retry( void );		// �h���O���̗L���m�F�𑗐M�AAck�ENack�҂��ƃ��g���C�t��
EXTERN ER send_password_chk_Wait_Ack_Retry( void );		// �����e�i���X�E���[�h�ڍs���̃p�X���[�h�m�F�v���̑��M�AAck�ENack�҂��ƃ��g���C�t��
EXTERN ER send_meinte_mode_Wait_Ack_Retry( void );		// ���[�h�ؑ֒ʒm�̑��M�AAck�ENack�҂��ƃ��g���C�t���i�����e�i���X�E���[�h�ڍs���j
EXTERN ER send_touroku_init_Wait_Ack_Retry( void );		// �o�^�f�[�^�������v���̑��M�AAck�ENack�҂��ƃ��g���C�t���i�����e�i���X�E���[�h���j
EXTERN ER send_kinkyuu_touroku_Wait_Ack_Retry( void );	// PC�ցA�ً}�J���ԍ��ʒm���M�AAck�ENack�҂��ƃ��g���C�t��
EXTERN ER send_kinkyuu_8keta_Wait_Ack_Retry( void );	// PC�ցA�ً}�W���ԍ��f�[�^�v�����M�AAck�ENack�҂��ƃ��g���C�t��
EXTERN ER send_kinkyuu_kaijyou_Wait_Ack_Retry( void );	// PC�ցA�ً}�J���ԍ����M�AAck�ENack�҂��ƃ��g���C�t��
EXTERN ER send_kinkyuu_input_Wait_Ack_Retry( void );	// PC�ցA�ً}�ԍ��̑Ó����₢���킹�m�F�v�����M�AAck�ENack�҂��ƃ��g���C�t��
EXTERN UB GetSysSpec( void );		// �}���V�����d�l/�P�΂P�d�l�@�ؑւ��t���O�@0:�}���V����(��L��)�d�l�A1:1�΂P�i�@�l�j�d�l�A2�F�}���V�����i���p���j�d�l�B 
EXTERN ER set_reg_param_for_Lcd( void );
EXTERN void SetError(int err);	//20140930Miya

EXTERN void init_wdt( void );					// WDT(�E�H�b�`�h�b�O�E�^�C�}�[)�̏����ݒ�
EXTERN void reset_wdtc( void );					// WDT(�E�H�b�`�h�b�O�E�^�C�}�[)�J�E���^�̃N���A
EXTERN void stop_wdt( void );					// WDT(�E�H�b�`�h�b�O�E�^�C�}�[)�̖������ݒ�
EXTERN void reset_wdt_cnt( void );			// WDT�J�E���^�p�������̃_�C���N�g�E���Z�b�g����(�t���b�V���E�������E�h���C�o������p)
											// �eTask��WDT�N���A�E�t���O�𖳎����āA�J�E���^���N���A����̂ŁA
											// �h���C�o���ł̎g�p�Ɍ���B�i���p����ƁA�^�X�N�P�ʂł�WDT�@�\�̈Ӗ����Ȃ��Ȃ�B�j

EXTERN UINT sys_ScreenNo;		// ���݂̃X�N���[��No
EXTERN UB s_CapResult;			// �w�F�؂̌���
EXTERN UB s_DongleResult;		// �h���O���̗L���m�F�̌���
EXTERN UB s_PasswordResult;		// �p�X���[�h�m�F�̌���
EXTERN UB s_ID_NO_Result;		// ID�ԍ��m�F�̌���
EXTERN UB s_ID_Authority_Result;	// ID�����m�F�̌���
EXTERN UB s_ID_Authority_Level; // ID�����⍇���R�}���h�Ŗ⍇�������[�U�[ID�̌������x�� �B  ASCII�R�[�h�B 
EXTERN UB s_Kantoku_num[ 2 ];	// ID�����⍇�������R�}���h�œ����ē҂̑����B ASCII�R�[�h�B
EXTERN UB s_Kanri_num[ 2 ];		// ID�����⍇�������R�}���h�œ����Ǘ��҂̑����B ASCII�R�[�h�B
EXTERN UB s_Ippan_num[ 6 ];		// ID�����⍇�������R�}���h�œ�����ʎ҂̑����B ASCII�R�[�h�B
EXTERN UB s_KinkyuuTourokuResult;	// �ً}�o�^�ʒm�̌���

EXTERN UINT rcv_ack_nak;		// ack��M�t���O�@=0�F����M�A=1:ACK��M�A=-1�Fnak��M
EXTERN void LcdPosAdj(int calc);	//20161031Miya Ver2204 LCDADJ
EXTERN ER	SndCmdCngMode( UINT stat );		// PC�փ��[�h�ؑւ��ʒm�𑗐M
EXTERN FLGPTN befor_scrn_no;	// ��ʑJ�ڗp�ϐ��B�u���~���܂����H�v�u�������v�̎��̖߂���FLG_PTN�ԍ��B
EXTERN UB  befor_scrn_for_ctrl; // ��ʑJ�ڗp�ϐ��B�u���~���܂����H�v�u�������v�̎��̖߂��̉�ʔԍ��B
EXTERN UB req_restart;			// �p���[�E�I���E�v���Z�X�̍ċN���v���t���O�@=0:�v�������A=1:�v������
EXTERN UB Pfail_mode_count;	// ��d���[�h�̏ꍇ�̋N����(�����[�h�ŋN�������ꍇ�́Areset�����B)
EXTERN UINT Pfail_sense_flg;	// ��d���m�ʒm��M�t���O�@0:��M�����A1:��M����
EXTERN UINT sys_demo_flg;		// �f���d�l�t���O
EXTERN UINT g_CapTimes;			// 1:�B�e1��� 2:�ĎB�e	//20131210Miya add

EXTERN UH Flbuf[0x10000];		//�t���b�V���ޔ�p�o�b�t�@(1�Z�N�V������)

EXTERN UINT door_open_over_time;	// �h�A�ߊJ���ݒ莞��
EXTERN UINT Test_1sec_timer;	// 1�b�T�C�N���b�N�E�J�E���^�E�e�X�g�p
EXTERN UINT Pfail_start_timer;	// ��d�^�C�}�[�T�C�N���b�N�E�J�E���g�p
EXTERN unsigned long WDT_counter;		// WDT�^�C�}�[�p�T�C�N���b�N�E�J�E���^
EXTERN unsigned long Ninshou_wait_timer;	// �w�F�؎��s���̉�ʑ҂����ԁi30�b�j

EXTERN unsigned short FpgaVerNum; //20140905Miya lbp�ǉ� FPGA�o�[�W�����A�b�v
EXTERN UB f_fpga_ctrl;				//20140915Miya FPGA����t���O 0:V1.001�p�@1:V1.002�p
EXTERN int g_key_arry[10];	//20140925Miya password open
EXTERN int g_Diagnosis_start;	//20140930Miya	//�f�f�J�n�t���O

EXTERN UINT main_TSK_wdt;		// main�^�X�N�@WDT���Z�b�g�E���N�G�X�g�E�t���O
EXTERN UINT camera_TSK_wdt;		// �J�����^�X�N�@WDT���Z�b�g�E���N�G�X�g�E�t���O
EXTERN UINT ts_TSK_wdt;			// �^�b�`�Z���T�E�^�X�N�@WDT���Z�b�g�E���N�G�X�g�E�t���O
EXTERN UINT lcd_TSK_wdt;		// LCD�^�X�N�@WDT���Z�b�g�E���N�G�X�g�E�t���O
EXTERN UINT sio_rcv_TSK_wdt;	// SIO��M�^�X�N�@WDT���Z�b�g�E���N�G�X�g�E�t���O
EXTERN UINT sio_snd_TSK_wdt;	// SIO���M�^�X�N�@WDT���Z�b�g�E���N�G�X�g�E�t���O

EXTERN int	g_pcproc_f;	//20160930Miya PC����VA300S�𐧌䂷��
EXTERN int	g_capallow; //20160930Miya PC����VA300S�𐧌䂷��
EXTERN int	g_pc_authnum; //20160930Miya PC����VA300S�𐧌䂷��
EXTERN int	g_pc_authtime; //20160930Miya PC����VA300S�𐧌䂷��

EXTERN BkDataNoClear g_BkDataNoClear;	//20161031Miya Ver2204 LCDADJ
EXTERN int	g_LedCheck;					//20161031Miya Ver2204 

EXTERN int dbg_dbg1;

//20170315Miya 400Finger ->
//EXTERN int g_RegBlockNum;		// �o�^�u���b�N�ԍ�(1 �` 7) -1:�o�^�Ȃ�
EXTERN int g_RegAddrNum;		// �o�^�Ԓn(0 �` 239) -1:�o�^�Ȃ�
//EXTERN int g_RegTotalYubiNum;	// �o�^����Ă���w�̑���
EXTERN int g_BufAuthScore[240][20];	// �ɏ��F�؂̃X�R�A�[
//EXTERN unsigned char g_taikyo_flg;		//20170320Miya 400FingerM2 �Ö��ދ��t���O
//20170315Miya 400Finger <-
EXTERN unsigned short g_FPGA_ErrFlg;	//20170706Miya FPGA�t���[�Y�΍�


// Next_screen_ctrl.c
EXTERN ER NextScrn_Control_mantion( void );		// ���̉�ʃR���g���[���i�}���V����(��L��)�d�l�̏ꍇ�j
EXTERN ER NextScrn_Control_office( void );		// ���̉�ʃR���g���[���i�P�΂P�d�l�̏ꍇ�j
EXTERN ER NextScrn_Control( void );				// ���̉�ʃR���g���[��
EXTERN UB Chk_shutdown_ok( void );				// �F�ؑ��쒆���ǂ������`�F�b�N����
EXTERN ER Pfail_shutdown( void );		// ��d���[�h�A��d���m�ʒm��M�̏ꍇ�́A�V���b�g�_�E�����s

// tsk_mode.c
EXTERN TASK ModeTask();							///< ���[�h�^�X�N
EXTERN UB MdGetMode(void);						///< ���[�h�Q��
EXTERN void MdCngMode( UINT eNextMode );		///< ���[�h�ύX
EXTERN UB MdGetSubMode(void);					///< �T�u���[�h�Q��
EXTERN void MdCngSubMode( UINT eNextMode );		///< �T�u���[�h�ύX
EXTERN ER MdSendMsg(ID idOrg, UH uhCmd, UH uhBlkno, B *p, int iCount);	///< ���[�h�^�X�N�ւ̃R�}���h���b�Z�[�W���M

// tsk_disp.c
EXTERN void DispLed(int, int);					///< LED�\��
//EXTERN void Disp7Seg(enum LED_7SEG_SEL eSelect7Seg, int, enum CTRL7SEG);	///< �VSEG�\��
EXTERN TASK DispTask(void);						///< �\���^�X�N

// tsk_lcmd.c
EXTERN TASK LanCmdTask(INT ch);					///< LAN�R�}���h�����^�X�N
EXTERN void LanCmdSend(enum PR_TYPE eProt, const UH uhCmd, UH uhBlkno, B *pData);	///< �R�}���h���M
EXTERN void LanReqSend(void);							///< �₢���킹�R�}���h���M
EXTERN void LanResSend(enum PR_TYPE eProt, B *pData);	///< �����R�}���h�̑��M
EXTERN void LanFingerDataSend(B *pId);					///< �w�f�[�^�̑��M
EXTERN void LanParmErrLog(char* pCmd, char* pComment, long lData);	///< �p�����[�^�G���[�����O�Ɏc��
EXTERN UB CAPGetResult( void );					///< �ŐV�̎w�F�،��ʂ�Ԃ��i�w�F�،��ʎQ�Ɓj
EXTERN UINT sys_Mode;				// ��ԕϐ�
EXTERN UINT sys_SubMode;			// ��ԕϐ�(�T�u�j


// tsk_lrcv.c
EXTERN TASK UdpRcvTask(INT ch);					///< UDP��M�^�X�N
EXTERN void CmmSendAck(void);					///< Ack���M
EXTERN void CmmSendEot(void);					///< EOT���M
EXTERN void CmmSendNak(void);					///< NAK���M
EXTERN void CmmChangeStatus( int newStatus );	///< UDP��M��ԕύX
EXTERN BOOL CmmSendData(enum PR_TYPE eProt, const UH uhCmd, UH uhBlkno, char *cData, int iCnt);	///< �R�}���h���M
EXTERN enum HEADER_CODE HeaderCodeGet(enum PR_TYPE eProt);	///< �w�b�_�R�[�h�ɕϊ�


// tsk_io.c
EXTERN TASK IoTask(void);						///< I/O���m�^�X�N
EXTERN INTHDR PcsStartInt(void);				///< PCS����J�n���荞��
EXTERN INTHDR PcsStopInt(void);					///< PCS����I��������
EXTERN INTHDR IrStartInt(void);					///< IR����J�n���荞��
EXTERN INTHDR IrStopInt(void);					///< IR����I��������
EXTERN INTHDR IrTrendInt(void);					///< ����f�[�^�]������������
EXTERN INTHDR FpgaErrInt(void);					///< FPGA�G���[������
EXTERN INTHDR DmaStopInt(void);					///< DMA�]������������

// tsk_exkey.c
EXTERN ER ExKeyInit(ID tskid, ID flgid);		///< �O���L�[�^�X�N������
EXTERN TASK ExKeyTask(void);					///< �O���L�[����^�X�N

// tsk_lcd.c
EXTERN TASK LcdTask(void);						///< LCD�\���^�X�N
EXTERN ER LcdTaskInit(ID idTsk);				///< LCD�\���^�X�N�̏�����
extern int dbg_Auth_hcnt;	//20141014Miya
extern int g_lcdpos[3][2];	//20161031Miya Ver2204 LCDADJ

// tsk_buz.c
EXTERN ER SoundInit(ID tskid, ID flgid);		///< �T�E���h�^�X�N������
EXTERN TASK SoundTask(void);					///< �T�E���h�^�X�N
EXTERN void SoundCtrl(enum SOUND_CTRL eCtrl);	///< �T�E���h����

// tsk_ts.c
EXTERN ER TsTaskInit(ID tskid);					///< ���Ԍ��m�^�X�N������
EXTERN TASK TsTask(void);						///< ���Ԍ��m�^�X�N

// tsk_ctlmain.c
//EXTERN ER CtlMainTaskInit(ID idTsk);			///< �[���R���g���[���E���C���E�^�X�N������
//EXTERN TASK CtlMain( void );					///< �[���R���g���[���E���C���E�^�X�N

// tsk_camera.c
EXTERN ER CameraTaskInit(ID idTsk);				///< �J�����B�e�R���g���[���E�^�X�N������
EXTERN TASK CameraTask( void );					///< �J�����B�e�R���g���[���E�^�X�N
extern void yb_init_all( void );				// �w�o�^���̃f�[�^������(all)
extern int IrLedOnOffSet(int sw, UH duty2, UH duty3, UH duty4, UH duty5);	//20160312Miya �ɏ����xUP
extern T_YBDATA yb_touroku_data;				// �w�o�^���i�P�w���j
extern T_YBDATA20 yb_touroku_data20[21];		// �w�o�^���i20�w���j//�ǉ��@20130510 Miya

extern UB kinkyuu_tel_no[17];					// �ً}�J���d�b�ԍ��P�U���i�z��ŏI�Ԗڂ͋�؂�L���h,�h�j
extern UB kinkyuu_touroku_no[5];				// �ً}�J���ً̋}�o�^�ԍ��S���i�z��ŏI�Ԗڂ͋�؂�L���@NUL�@�j�@
extern UB kinkyuu_hyouji_no[9];					// �ً}�J���ً̋}�ԍ��W���\���f�[�^�i�z��ŏI�Ԗڂ͋�؂�L���@NUL�@�j�@
extern UB kinkyuu_kaijyo_no[9];					// �ً}�J���̊J���ԍ��W���f�[�^�i�z��ŏI�Ԗڂ͋�؂�L���@NUL�@�j
extern UB kinkyuu_input_no[5];					// �ً}�J�����ɓ��͂��ꂽ�ԍ��S���i�z��ŏI�Ԗڂ͋�؂�L���@NUL�@�j

extern UB mainte_password[5];					// �����e�i���X�E���[�h�ڍs���̊m�F�p�p�X���[�h�S���B�i�z��ŏI�Ԗڂ͋�؂�L���@NUL�@�j�@

//extern UB g_ubCapBuf[ ( ( 160 * 140 ) * 3 ) ];//20131210Miya cng	///< �k���摜�擾�p�o�b�t�@
extern UB g_ubCapBuf[ 640 * 480 ];	//20160601Miya

#if (NEWCMR == 1)
EXTERN ER NCmrI2C_Send( UB *pAddr, UB *pData, int iSendSize );		// �J�����E�R�}���h��I2C���M
EXTERN ER NCmrI2C_Rcv( UB *pAddr, UB *pData, int iRcvSize );		// �J�����E�R�}���h��I2C��M
#endif

extern UB g_ubHdrBuf[ 160 * 80 ];
//extern UB g_ubResizeBuf[ 80 * 40 ];
//extern UB g_ubResizeSvBuf[ 80 * 40 ];	//20140423miya �F�،���
extern UB g_ubResizeBuf[ 100 * 40 ];	//20140910Miya XSFT
extern UB g_ubResizeSvBuf[ 100 * 40 ];	//20140910Miya XSFT //20140905Miya LBP�ǉ�
extern UB g_ubSobelR1Buf[ 80 * 40 ];
extern UB g_ubSobelR2Buf[ 80 * 40 ];
extern UB g_ubSobelR3Buf[ 20 * 10 ];
extern UB g_ubSobelR1SvBuf[ 80 * 40 ];
extern UB g_ubSobelR2SvBuf[ 80 * 40 ];
extern UB g_ubSobelR3SvBuf[ 20 * 10 ];
extern unsigned short g_hdr_blend[ 160 * 80 ];
extern UB g_ubLbpBuf[ 80 * 40 ];	//20140905Miya LBP�ǉ�
extern UB g_ubResizeSvBuf2[ 80 * 40 ];	//20140905Miya LBP�ǉ�
extern UB g_ubSobelR3DbgBuf[ 20 * 10 ];	//20160312Miya �ɏ����xUP

extern unsigned short CmrCmdManualGetParameter( UB para );
extern ER CmrCmdFixShutterCtl( UB FSC_val );
extern ER CmrCmdManualGainCtl( UB gain_val );
extern ER Wait_Ack_forBlock( void );			// �R�}���h�܂��̓u���b�N�f�[�^���M���Ack�����҂�

extern int Check_Cap_Raw_flg;					// ���摜�B�e�`�F�b�N���s���t���O�@Added T.Nagai 
extern int Cmr_Start;		//20140930Miya
extern int CmrDebugCnt;
extern char CmrCapNg;						//20140930Miya
extern int	CmrWakFlg;				//20150930Miya
extern int	CmrReloadFlg;			//20150930Miya

extern UB g_XsftMiniBuf[5][ 20 * 10 ];	//20140910Miya XSFT


#if ( VA300S == 2 ) 
extern void DebugSendCmd_210( void );
#endif


// tsk_rcv_serial.c
extern UINT sio_mode;
extern UINT comm_data_stat;
extern UINT EOT_Wait_timer;
extern UINT ACK_Wait_timer;
extern UINT Rcv_chr_timer;
extern unsigned short usSioRcvCount;	// �V���A����M�f�[�^��
#define	RCV_BUF_SIZE	1024 + 4
extern UB cSioRcvBuf[ RCV_BUF_SIZE ];	// �V���A����M�f�[�^�o�b�t�@

extern UB send_Ack_buff[ 4 ];  // ACK���M�R�[�h�f�[�^�E�o�b�t�@
extern UB send_Nack_buff[ 4 ]; // ACK���M�R�[�h�f�[�^�E�o�b�t�@
extern UB send_Enq_buff[ 4 ];  // ACK���M�R�[�h�f�[�^�E�o�b�t�@
extern UINT ENQ_sending_cnt;	 // ENQ�đ��J�E���^
extern UINT sio_rcv_comm_len;	 // ��M���R�}���h�̎w�背���O�X
extern UINT sio_rcv_block_num;	 // ��M���R�}���h�̃u���b�N�ԍ�

#define SND_SIO_BUF_SIZE 1024 + 4	 // �V���A�����M�R�}���h�f�[�^�E�o�b�t�@
extern UB send_sio_buff[ SND_SIO_BUF_SIZE ];	 // �V���A�����M�R�}���h�f�[�^�E�o�b�t�@
extern UINT sio_snd_comm_len;	 // ���M���R�}���h�̎w�背���O�X
extern UINT sio_snd_block_num;	 // ���M���R�}���h�̃u���b�N�ԍ�

extern unsigned short usSioBunkatuNum;	//20160930Miya PC����VA300S�𐧌䂷��

EXTERN int chk_rcv_cmddata( UB *Buf, int cmd_len  );	// ��M�R�}���h�Q�O�o�C�g�̉�͏����B

EXTERN  int send_Comm_Ack( INT ch );
EXTERN  int send_Comm_Nack( INT ch );
EXTERN  int send_Comm_EOT( INT ch );
EXTERN  void util_i_to_char( unsigned int i, char * buff );
EXTERN  void util_i_to_char( unsigned int i, char * buff );

EXTERN UB KeyIO_board_soft_VER[ 4 ];		// ����BOX�@������SH2�@�\�t�g�E�F�A�E�o�[�W�����ԍ�
											// VA-300, VA-300s���p�B
EXTERN  UB dip_sw_data[ 4 ];	// VA-300s�̐���Box��Dip SW��� Bit0,�@0�FOFF�A1�FON�A�@��������Bit0,1,2,3

EXTERN UINT DBG_send_cnt;		// �\�t�g�E�F�A�EDebug�p

// tsk_snd_serial.c
extern void SendSerialBinaryData( UB *data, UINT cnt );	// Serial�f�[�^�̑��M�i�ėp�j
extern void send_sio_WakeUp( void );			// VA300S����Box�փV���A����WakeUp�̖₢���킹���s���iTest	�R�}���h�A�d��ON���j
extern void send_sio_ShutDown( void );			// VA300S����Box�փV���A���ŃV���b�g�E�_�E���v���R�}���h�𑗐M����B
extern void send_sio_Touroku( void );			// VA300S����Box�փV���A���œo�^�����R�}���h(01)�𑗐M�B
extern void send_sio_Touroku_Init( int j );		// VA300S����Box�փV���A���œo�^�����R�}���h(01)�𑗐M(�d��ON���̈ꊇ���M)�B
extern void send_sio_Touroku_InitAll( void );	// VA300S����Box�փV���A���œo�^�����R�}���h(01)�𑗐M���C��(�d��ON���̈ꊇ���M)�B
//extern void send_sio_Ninshou( UB result );		// VA300S����Box�փV���A���ŔF�؊����R�}���h(03)�𑗐M�B
extern void send_sio_Ninshou( UB result, UB auth_type, UB info_type );		//20160108Miya FinKeyS // VA300S����Box�փV���A���ŔF�؊����R�}���h(03)�𑗐M�B
extern void send_sio_Touroku_Del( void );		// VA300S����Box�փV���A���œo�^�f�[�^�폜�R�}���h(04)�𑗐M�B
extern void send_sio_Touroku_AllDel( void );	// VA300S����Box�փV���A���œo�^�f�[�^�������i�ꊇ�폜�j�R�}���h(05)�𑗐M�B
extern void send_sio_Kakaihou_time( void );		// VA300s����Box�փV���A���ŉߊJ�����Ԃ̐ݒ�v���R�}���h�𑗐M����B
extern void send_sio_init_time( void );			// VA300s����Box�֏��������̐ݒ�v���R�}���h(10)�𑗐M����B
extern void send_sio_force_lock_close( void );	// VA300s����Box�֋��������R�}���h(11)�𑗐M����B
extern void send_sio_BPWR( int sw );		//20160905Miya B-PWR����
extern void send_sio_VA300Reset( void );	//20161031Miya Ver2204 �[�����Z�b�g
extern void send_sio_STANDARD( void );			//20160905Miya PC����VA300S�𐧌䂷��
extern void send_sio_TANMATU_INFO( void );		//20160905Miya PC����VA300S�𐧌䂷��
extern void send_sio_REGDATA_UPLD_STA( void );	//20160905Miya PC����VA300S�𐧌䂷��
extern void send_sio_REGDATA_UPLD_GET( void );	//20160905Miya PC����VA300S�𐧌䂷��
extern void send_sio_REGPROC( int rslt );		//20160905Miya PC����VA300S�𐧌䂷��
extern void send_sio_AUTHPROC( int rslt );		//20160905Miya PC����VA300S�𐧌䂷��

extern UB IfImageBuf[2*80*40 + 20*10];

// tsk_ninshou.c

#if( FREEZTEST )
EXTERN UB DoAuthProc( UB num );
#endif

EXTERN TASK NinshouTask( void );
extern void SendNinshouData( char *data, int cnt );	// �f�[�^�̃^�X�N�ԑ��M�i�ėp�E�F�؏�����p�j
EXTERN void InitImgAuthPara( void );
EXTERN UB InitFlBkAuthArea( void );
EXTERN UB InitBkAuthData( void );
EXTERN UB SaveBkAuthDataTmpArea( void );
EXTERN UB SaveBkAuthDataFl( void );	//�摜����&�F�؏����o�b�N�A�b�v�f�[�^�ۑ�
EXTERN UB ReadBkAuthData( void );	//�摜����&�F�؏����o�b�N�A�b�v�f�[�^�Ǎ�

EXTERN UB SaveBkDataNoClearFl( void );	//20161031Miya Ver2204 LCDADJ
EXTERN UB ReadBkDataNoClearFl( void );	//20161031Miya Ver2204 LCDADJ

EXTERN UB SetImgAuthPara( UB *data );
EXTERN UB DelImgAuthPara( void );
EXTERN UB SaveRegImgTmpArea( UB proc );
EXTERN UB SaveRegImgFlArea( int num );
EXTERN UB ReadRegImgArea( int num );
EXTERN UB AddRegImgFromRegImg( int sw, int num );	//20160312Miya �ɏ����xUP
EXTERN UB InitRegImgArea( void );
EXTERN void MakeOpenKeyNum(void);
EXTERN UB ChekKinkyuKaijyouKey( void );
EXTERN void SetReCap( void );
EXTERN UB MemCheck( unsigned long offset );
EXTERN UB ChekPasswordOpenKey( void );	//20140925Miya password_open
EXTERN UB CngNameCharaCode( unsigned char code, int *num );	//20160108Miya FinKeyS
//EXTERN UB SlitImgHantei( int reg_f, int *st );
EXTERN void ImgTriming( int st_y );
EXTERN UB HiImgHantei( UB hantei );
EXTERN UB MidImgHantei( UB hantei );
EXTERN void ImgResize4(int num);

#if(AUTHTEST >= 1)	//20160613Miya
EXTERN UB InitTestRegImgFlArea( void );
EXTERN UB SaveTestRegImgFlArea( unsigned short ok_ng_f );
EXTERN UB ReadTestRegImgArea( unsigned short ok_ng_f, short cpy_f, short num, short num10 );
EXTERN UB ReadTestRegImgCnt( void );
#endif

//20160902Miya FPGA������
EXTERN int WrtImgToRam(int ts, int sbl);
EXTERN int Cal4Ave(int ts, int sbl, int *rt1, int *rt2, int *rt3, int *rt4);
EXTERN void MakeTestImg(void);
EXTERN void AutoMachingFPGA(UB num, int proc, double *scr1, double *scr2);


extern UB g_RegFlg;
extern int g_AuthOkCnt;	//�F��OK�񐔁@50��Ŋw�K�摜�ۑ�
extern RegUserInfoData			g_RegUserInfoData;				//���[�U���
extern RegBloodVesselTagData	g_RegBloodVesselTagData[20];	//�o�^�����摜�^�O���
extern UseProcNum 				g_UseProcNum;					//���[�U�[�g�p�ԍ�
extern PasswordOpen				g_PasswordOpen;					//�p�X���[�h�J��
extern PasswordOpen				g_InPasswordOpen;					//�p�X���[�h�J��
extern TechMenuData				g_TechMenuData;					//�Z�p�����e
extern AuthLog					g_AuthLog;						//�F�؃��O		//20140925Miya add log
extern MainteLog				g_MainteLog;					//�����e���O	//20140925Miya add log
extern RegImgDataAdd			g_RegImgDataAdd[20];		//20160312Miya �ɏ����xUP �o�^�摜(R1,LBP,R1�ɏ�)
extern PasswordOpen2			g_PasswordOpen2;				//20160108Miya FinKeyS //�p�X���[�h�J��

#if(AUTHTEST >= 1)	//20160613Miya
extern unsigned short g_sv_okcnt;
extern unsigned short g_sv_okcnt0;
extern unsigned short g_sv_okcnt1;
extern unsigned short g_sv_okcnt2;
extern unsigned short g_sv_okcnt3;
extern unsigned short g_sv_okcnt4;
extern UB TstAuthImgBuf[8][100*40];	//[OK/NG][8��][�V�t�g�O�T�C�Y]
extern unsigned short g_cmr_err;
extern unsigned short g_imgsv_f;
#endif

// tsk_log.c
EXTERN TASK LogTask( void );
extern void SendLogData( char *data, int cnt );	// �f�[�^�̃^�X�N�ԑ��M�i�ėp�E���M���O������p�j

// selftest.c
EXTERN BOOL SelfTest(void);						///< �Z���t�e�X�g

// prm_func.c
EXTERN void McnParamInit(void);					///< ���u�p�����[�^������������
EXTERN TMO McnParamRcvTmoutGet(void);			///< ��M�^�C���A�E�g���Ԃ��擾����
EXTERN int McnParamSendRetryGet(void);			///< ���M���g���C�񐔂��擾����
EXTERN TMO McnParamPolingGet(void);				///< �|�[�����O���Ԃ��擾����
EXTERN TMO McnParamFirstTmoutGet(void);			///< �����o�^���Ȃ��Ƃ��̃^�C���A�E�g���Ԃ��擾����
EXTERN TMO McnParamLcdDelayGet(void);			///< ������ʐؑ֎��Ԃ��擾����
EXTERN int McnParamStartXGet(void);				///< �؂�o���J�nX���W���擾����
EXTERN int McnParamStartYGet(void);				///< �؂�o���J�nY���W���擾����
EXTERN TMO McnParamPowerOffGet(void);			///< ��d���d��OFF���Ԃ��擾����
EXTERN enum HOST_TYPE McnParamTermNumberGet(void);	///< �[���ԍ����擾����
EXTERN BOOL IsMcnParamEnable(void);				///< �p�����[�^�̗L�������m�F����

// sub_func.c
EXTERN int _sprintf(char *s, const char *control, ...);	///< sprintf�̃Z�}�t�H��
EXTERN BOOL MpfUseDisplayLine(int iLine, char *pBuf);	///< �������v�[���g�p�ʕ\���f�[�^�擾

extern int	g_SameImgGet;	//20151118Miya ���摜�ĎB�e
extern int	g_SameImgCnt;	//20160115Miya ���摜�ĎB�e
extern int	g_AuthType;		//20160120Miya 0:�w�F�� 1:�p�X���[�h�F�� 2:�ً}�J��
extern int	ode_oru_sw;						//20160108Miya FinKeyS ���ł����E�������SW
extern int g_TestCap_start;
extern UW	dbg_flwsize;
extern UW	dbg_flwsizeIn;

extern int	dbg_ts_flg;
extern int	dbg_cam_flg;
extern int	dbg_nin_flg;
extern int	dbg_cap_flg;
extern int g_MainteLvl;	//20160711Miya �f���@

extern PosGenten g_PosGenten[2];

#if ( NEWCMR == 1 )
extern int g_cmr_dbgcnt;
#endif


//@}
#endif										// end of _VA300_H_
