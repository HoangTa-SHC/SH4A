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

//=== ��`
enum COM_CH {								///< �ʐM�`�����l����`
	COM_CH_MON,								///< ���j�^�`�����l��
	COM_CH_LAN,								///< LAN�`�����l��
};

enum CAP_STATUS {							///< �w�F�،���
	CAP_JUDGE_IDLE,
	CAP_JUDGE_OK,							///< �w�F��OK
	CAP_JUDGE_NG,							///< �w�F��NG
	CAP_JUDGE_RT,							///< �w�F�؃��g���C�v��
	CAP_JUDGE_E1,							///< �w�F�؉摜�ُ�i�q�X�g�O�����ُ�j�i�B�e�摜���Q�������ꂽ�ُ�摜�j
	CAP_JUDGE_E2							///< �w�F�؉摜�ُ�i�X���b�g�ُ�j�i�w���c��ő}�����ꂽ�ꍇ�j
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

enum KINKYU_TOUROKU_STATUS {				///< �p�X���[�h�m�F�̌���
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
	MD_POWER_FAIL,							///< ��d�����샂�[�h
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
	SUB_MD_DONGURU_CHK,						///< PC�փh���O���̗L���m�F�̊m�F�ʐM��
	SUB_MD_PASSWORD_CHK,					///< PC�փp�X���[�h�̊m�F�ʐM��
	SUB_MD_KINKYU_TOUROKU,					///< �ً}�ԍ��o�^�����V�[�P���X��
	SUB_MD_KINKYU_8KETA_REQ,				///< �ً}�W���ԍ��v���ʐM��
	SUB_MD_KINKYU_KAIJYO_SEND,				///< �ً}�J���ԍ��ʒm�ʐM��
	SUB_MD_KINKYU_NO_CHK_REQ,				///< �ً}�ԍ��Ó����m�F�v���ʐM��
	SUB_MD_CHG_MAINTE,						///< �����e�i���X���[�h�ֈڍs��
	SUB_MD_CHG_NORMAL,						///< �ʏ탂�[�h�ֈڍs��
	
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

	LCD_SCREEN180,		// ��ʂP�W�O�\�����@�ʏ탂�[�h�i�ً}�J���j
	LCD_SCREEN181,		// ��ʂP�W�P�\����
	LCD_SCREEN182,		// ��ʂP�W�Q�\����
	LCD_SCREEN183,		// ��ʂP�W�R�\����
	LCD_SCREEN184,		// ��ʂP�W�S�\����
	LCD_SCREEN185,		// ��ʂP�W�T�\����
	LCD_SCREEN186,		// ��ʂP�W�U�\����
	LCD_SCREEN187,		// ��ʂP�W�V�\����
	LCD_SCREEN188,		// ��ʂP�W�W�\����

	LCD_SCREEN200,		// ��ʂQ�O�O�\�����@�����e�i���X�E���[�h
	LCD_SCREEN201,		// ��ʂQ�O�P�\����
	LCD_SCREEN202,		// ��ʂQ�O�Q�\����
	LCD_SCREEN203,		// ��ʂQ�O�R�\����
	LCD_SCREEN204,		// ��ʂQ�O�S�\����
	LCD_SCREEN205,		// ��ʂQ�O�T�\����
	LCD_SCREEN206,		// ��ʂQ�O�U�\����
		
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
	
	LCD_USER_ID,				// "���[�U�[ID"�����͂��ꂽ
	LCD_YUBI_ID,				// "�ӔC��/��ʎ�"�̎wID���I�����ꂽ
	LCD_NAME,					// "����"�����͂��ꂽ
	LCD_YUBI_SHUBETU,			// "�w���"���I�����ꂽ
	LCD_FULL_PIC_SEND_REQ,		// "�t���摜���M"�{�^���������ꂽ
	LCD_MAINTE_SHOKIKA_REQ,		// "�������h�{�^���������ꂽ
	LCD_MAINTE_END,				// "�����e�i���X���[�h�I��"�{�^���������ꂽ
	LCD_KINKYUU_KAIJYOU_BANGOU,	// "�ً}�J���ԍ��@�W��"�����͂��ꂽ
	LCD_KINKYUU_BANGOU,			// "���[�U�[���ً͂̋}�ԍ��@�S�������͂��ꂽ			
	
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
#define RE_SIZE_Y		320				// �k���O��Y�T�C�Y

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
	UB	ybCapBuf[ ( ( RE_SIZE_X * RE_SIZE_Y ) * 3 ) ];	// �w�摜�f�[�^�i�k���摜�R�����j
} T_YBDATA;


// �ǉ��@20130510 Miya
typedef struct t_ybdata20{		//=== �o�^���̒�` ===
	UB	yubi_seq_no[ 4 ];		// �o�^�w�ԍ��i�ӔC�ҁ{��ʎҁj�R���{","
	UB	kubun[ 2 ];				// �ӔC��/��ʎ҂̋敪�P���{","
	UB	yubi_no[ 3 ];			// �w�̎��ʔԍ��i�E�A���A�ǂ̎w�j�Q���{","
	UB	name[ 25 ];				// �o�^�Ҏw���Q�S���{","
	UB	ybCapBuf[ ( ( RE_SIZE_X * RE_SIZE_Y ) * 3 ) ];	// �w�摜�f�[�^�i�k���摜�R�����j
} T_YBDATA20;




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

extern ID	rxtid_org;		// GET_UDP�ׂ̈̌��X�̃^�X�NID
extern ID	rxtid;			// GET_UDP�ׂ̈̃^�X�NID

//@}

/// @name �v���g�^�C�v�錾
//@{

// va300.c
EXTERN void telnetd_send(B *s);					///< TELNET���M
EXTERN void telnetd_send_bin(B *s, INT len);	///< TELNET�o�C�i�����M
EXTERN UB GetScreenNo(void);					// ���ݕ\���X�N���[���ԍ��̎擾
EXTERN void ChgScreenNo( UB NewScreenNo );		// ��ʑJ�ڏ��(�ԍ�)���X�V����
EXTERN void reload_CAP_Param( void );			// Gain. FixShutter,IR_LED�̃p�����[�^���A�����v���l�ɖ߂��B�iCAP��Retry���ɕς�����\��������ׁB�j
EXTERN UB sys_ScreenNo;
EXTERN UB s_CapResult;		// �w�F�؂̌���
EXTERN UB s_DongleResult;	// �h���O���̗L���m�F�̌���
EXTERN UB s_PasswordResult;	// �p�X���[�h�m�F�̌���
EXTERN UB s_KinkyuuTourokuResult;	// �ً}�o�^�ʒm�̌���
EXTERN UINT rcv_ack_nak;	// ack��M�t���O�@=0�F����M�A=1:ACK��M�A=-1�Fnak��M
EXTERN ER	SndCmdCngMode( UINT stat );		// PC�փ��[�h�ؑւ��ʒm�𑗐M

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
extern T_YBDATA yb_touroku_data;				// �w�o�^���i�P�w���j
extern T_YBDATA20 yb_touroku_data20[21];		// �w�o�^���i20�w���j//�ǉ��@20130510 Miya

extern UB kinkyuu_tel_no[17];					// �ً}�J���d�b�ԍ��P�U���i�z��ŏI�Ԗڂ͋�؂�L���h,�h�j
extern UB kinkyuu_touroku_no[5];				// �ً}�J���ً̋}�o�^�ԍ��S���i�z��ŏI�Ԗڂ͋�؂�L���@NUL�@�j�@
extern UB kinkyuu_hyouji_no[9];					// �ً}�J���ً̋}�ԍ��W���\���f�[�^�i�z��ŏI�Ԗڂ͋�؂�L���@NUL�@�j�@
extern UB kinkyuu_kaijyo_no[9];					// �ً}�J���̊J���ԍ��W���f�[�^�i�z��ŏI�Ԗڂ͋�؂�L���@NUL�@�j
extern UB kinkyuu_input_no[5];					// �ً}�J�����ɓ��͂��ꂽ�ԍ��S���i�z��ŏI�Ԗڂ͋�؂�L���@NUL�@�j

extern UB mainte_password[5];					// �����e�i���X�E���[�h�ڍs���̊m�F�p�p�X���[�h�S���B�i�z��ŏI�Ԗڂ͋�؂�L���@NUL�@�j�@

extern ER CmrCmdFixShutterCtl( UB FSC_val );
extern ER CmrCmdManualGainCtl( UB gain_val );
extern ER Wait_Ack_forBlock( void );			// �R�}���h�܂��̓u���b�N�f�[�^���M���Ack�����҂�

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


//@}
#endif										// end of _VA300_H_
