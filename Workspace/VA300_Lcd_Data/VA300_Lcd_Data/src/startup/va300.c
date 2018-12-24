//=============================================================================
/**
 *	VA-300�v���O����
 *
 *	@file va300.c
 *	@version 1.00
 *
 *	@author OYO Electric Co.Ltd F.Saeki
 *	@date   2012/05/11
 *	@brief  ���C�����W���[��
 *
 * #define _SCI1_USE_	SCI1�ŉғ�
 * #define _ROM_DUMMY_	EEPROM�̃_�~�[�ŋN��
 * #define _DEBUG_		�f�o�b�O����SCI1�ɏo��
 * #define _OTHER_		�f�o�b�O�p�ɍ쐬�����{�[�h�p
 *	Copyright (C) 2010, OYO Electric Corporation
 */
//=============================================================================
#define VA300

#ifndef NOFILE_VER          /* 1: NORTi�T���v���t���t�@�C���V�X�e�� */
#define NOFILE_VER  1       /* 2: HTTPd for NORTi�t���t�@�C���V�X�e�� */
#endif                      /* 4: ���i�� NORTi File System Ver.4 */

#ifndef NFTP                /* FTP�T�[�o�[�ɓ����ڑ��\�ȃN���C�A���g�� */
#define NFTP        1       /* �� nonftpd.c �ł́A1�ɌŒ�̂��� */
#endif                      /* �V nofftpd.c �ł́A2�ȏ���`�\ */

#ifndef POL                 /* LAN�h���C�o�f�o�b�O�p */
#define POL         0       /* �����݂��g��(0), �|�[�����O�Ńe�X�g(1) */
                            /*
                               ����:
                                 �|�[�����O(POL=1)�̏ꍇ�A�|�[�����O���J�n����
                                 �܂ŁA�l�b�g���[�N�͓��삵�Ȃ��B
                            */
#endif

#ifndef SH4
#define SH4                 /* SH4 ���`����kernel.h��include���Ă������� */
#endif

#define USE_MON		0		// �f�o�b�O�E���j�^�[���g�p����/���Ȃ�

#define	_MAIN_
#include <stdio.h>
#include <string.h>
#include <kernel.h>
#include "sh7750.h"
#include "nonet.h"
#include "nonitod.h"
#include "nonethw.h"

#if (NOFILE_VER == 4)
#include "nofile.h"
#include "nofftp.h"
#include "nofchkd.h"
#else
#include "nonfile.h"
#include "nonftp.h"
#endif

#include "nonteln.h"
#include "net_prm.h"
#include "lan9118.h"

#include "id.h"
#include "udp.h"
#include "net_prm.h"
//#include "message.h"
#include "va300.h"
#include "err_ctrl.h"
#include "mon.h"
//#include "drv_rtc.h"

#include "drv_eep.h"
#include "drv_led.h"
#include "drv_dsw.h"
#include "drv_tim.h"
#include "drv_buz.h"
#include "drv_irled.h"
#include "drv_cmr.h"

#pragma section VER
#include "version.h"
#pragma section

/* �T�C�N���b�N�n���h�� ID */
const ID ID_CYC_TIM      = 1;   		///< �^�C�}�p

/* UDP �ʐM�[�_ ID */

const ID ID_UDP_OYOCMD   = 1;			///< UDP ���X�|���X�p
const ID ID_UDP_CLIENT   = 2;   		///< UDP �N���C�A���g�p

/* TCP �ʐM�[�_ ID */

const ID ID_TELNETD_CEP  = 1;			///< TELNET �f�[����

/* TCP ��t�� ID */

const ID ID_TELNETD_REP  = 1;			///< TELNET �f�[����

/* TCP/IP �R���t�B�O���[�V���� */

#define TCP_REPID_MAX	4				///< TCP��t���ő��
#define TCP_CEPID_MAX	4				///< TCP�ʐM�[�_�ő��
#define UDP_CEPID_MAX	2				///< UDP�ʐM�[�_�ő��
#define TCP_TSKID_TOP	TSK_MAX			///< TCP/IP�p�^�X�NID�擪
#define TCP_SEMID_TOP	SEM_MAX			///< TCP/IP�p�Z�}�t�HID�擪
#define TCP_MBXID_TOP	MBX_MAX			///< TCP/IP�p���C���{�b�N�XID�擪
#define TCP_MPFID_TOP	MPF_MAX			///< TCP/IP�p�Œ蒷�������v�[��ID�擪
#define ETH_QCNT		16
#define REP_QCNT		2
#define TCP_QCNT		4
#define UDP_QCNT		2

#include "nonetc.h"

/* System configuration */

#define TSKID_MAX   (TSK_MAX + TCP_TSKID_CNT)	///< �^�X�NID���
#define SEMID_MAX	(SEM_MAX + TCP_SEMID_CNT)	///< �Z�}�t�HID���
#define FLGID_MAX	FLG_MAX						///< �C�x���g�t���OID���
#define MBXID_MAX	(MBX_MAX + TCP_MBXID_CNT)	///< ���C���{�b�N�XID���
#define	MBFID_MAX	MBF_MAX						///< Maximum ID for MESSAGE BUFFER
#define	PORID_MAX	1							///< Maximum ID for RENDEVOUZ PORT
#define	MPLID_MAX	1							///< Maximum ID for VALIABLE LENGTH MEMORY POOL
#define MPFID_MAX	(MPF_MAX + TCP_MPFID_CNT)	///< �Œ蒷�������v�[��ID���
#define	CYCNO_MAX	CYC_MAX						///< Maximum ID for CYCRIC HANDLER
#define	ALMNO_MAX	ALH_MAX						///< Maximum ID for ALARM HANDLER

#define TPRI_MAX	9		///<���^�X�N�D��x�ő�l

#define ISTKSZ      2048	//��Stack size for Interval timer interrupt
#define TSTKSZ		512		//���^�C�}�n���h���X�^�b�N�T�C�Y


#include "nocfg4.h"
#include "nosio.h"

#ifndef NFILE
#define NFILE       8       /* �����I�[�v���\�ȃt�@�C���� */
#endif

/* ����u���b�N */

T_FTP ftp[ NFTP ];						///< FTP Server ����u���b�N
T_FILE file[ NFILE ];					///< �t�@�C������u���b�N
T_DISK disk[ 1 ];						///< �f�B�X�N�h���C�u����u���b�N

T_TELNETD telnetd;						///< TELNETD ����u���b�N

/* RAM Disk�Ŏg�p����A�h���X�ƃT�C�Y */

#define RAMDISK_ADDR     0           /* address of RAM disk area */
#define RAMDISK_SIZE     0x100000    /* size of RAM disk area */

#if (RAMDISK_ADDR == 0)              /* define as array if macro value is 0 */
UW RAMDISK[(RAMDISK_SIZE)/4];        /* UW is for long word alignment */
#undef RAMDISK_ADDR
#define RAMDISK_ADDR     (UW)RAMDISK
#endif

/* Information for creating task */
TASK MainTask(void);
TASK RcvTask(INT);
//TASK UdpRcvTask(INT);
TASK SndTask(INT);
TASK UdpSndTask(INT);
extern BOOL terminal_sendbin(T_TERMINAL *t, const B *s, INT len);
//extern static ER CmrCmdTest(void);
extern ER CmrCmdSleep(void);
extern ER CmrCmdWakeUp(void);
extern void yb_init_all();

const T_CTSK ctsk_main    = { TA_HLNG, NULL, MainTask,       5, 4096, NULL, (B *)"main" };
const T_CTSK ctsk_disp    = { TA_HLNG, NULL, DispTask,       6, 2048, NULL, (B *)"display" };	// 256 -->1024

const T_CTSK ctsk_rcv1    = { TA_HLNG, NULL, RcvTask,     8, 4096, NULL, (B *)"rcvtask" };
//const T_CTSK ctsk_rcv1    = { TA_HLNG, NULL, RcvTask,     3, 4096, NULL, (B *)"rcvtask" };
const T_CTSK ctsk_snd1    = { TA_HLNG, NULL, SndTask,     8, 2172, NULL, (B *)"sndtask" };
//const T_CTSK ctsk_snd1    = { TA_HLNG, NULL, SndTask,     3, 2172, NULL, (B *)"sndtask" };
const T_CTSK ctsk_urcv    = { TA_HLNG, NULL, UdpRcvTask,  5, 4096, NULL, (B *)"udprcvtask" };
const T_CTSK ctsk_usnd    = { TA_HLNG, NULL, UdpSndTask,  5, 2172, NULL, (B *)"udpsndtask" };
const T_CTSK ctsk_lancmd  = { TA_HLNG, NULL, LanCmdTask,     4, 40960, NULL, (B *)"lan_cmd" };
//const T_CTSK ctsk_lancmd  = { TA_HLNG, NULL, LanCmdTask,     5, 40960, NULL, (B *)"lan_cmd" };
const T_CTSK ctsk_io      = { TA_HLNG, NULL, IoTask, 2, 2048, NULL, (B *)"I/O task" };

const T_CMBX cmbx_lancmd  = { TA_TFIFO|TA_MFIFO, 2, NULL, (B *)"mbx_lancmd" };
const T_CMBX cmbx_ressnd  = { TA_TFIFO|TA_MPRI, 2, NULL, (B *)"mbx_ressnd" };
const T_CMBX cmbx_snd     = { TA_TFIFO|TA_MPRI, 2, NULL, (B *)"mbx_snd" };
const T_CMBX cmbx_disp    = { TA_TFIFO|TA_MFIFO, 2, NULL, (B *)"mbx_disp" };

const T_CMPF cmpf_com   = { TA_TFIFO, 32, sizeof (T_COMMSG), NULL, (B *)"mpf_com" };
const T_CMPF cmpf_lres  = { TA_TFIFO,  8, sizeof (T_LANRESMSG), NULL, (B *)"mpf_lres" };
const T_CMPF cmpf_disp  = { TA_TFIFO,  8, sizeof (ST_DISPMSG), NULL, (B *)"mpf_disp" };

const T_CFLG cflg_main  = { TA_TFIFO, 0, (B *)"main_flag" };
const T_CFLG cflg_io    = { TA_WMUL, 0, (B *)"io_flag" };
const T_CFLG cflg_ts  =   { TA_TFIFO, 0, (B *)"ts_flag" };
const T_CFLG cflg_camera  = { TA_TFIFO, 0, (B *)"camera_flag" };
const T_CFLG cflg_lcd  =  { TA_TFIFO, 0, (B *)"lcd_flag" };

const T_CMBF cmbf_lcd = {TA_TFIFO, 128, 512, NULL, (B *)"cmbf_lcd"};
const T_CMBF cmbf_lan = {TA_TFIFO, 1032, 0, NULL, (B *)"cmbf_lan"};

const T_CSEM csem_rtc   = { TA_TFIFO, 1, 1, (B *)"sem_rtc" };
const T_CSEM csem_err   = { TA_TFIFO, 1, 1, (B *)"sem_err" };
const T_CSEM csem_fpga  = { TA_TFIFO, 1, 1, (B *)"sem_fpga" };
const T_CSEM csem_spf   = { TA_TFIFO, 1, 1, (B *)"sem_sprintf" };
const T_CSEM csem_stkn  = { TA_TFIFO, 1, 1, (B *)"sem_strtok" };
const T_CSEM csem_stl   = { TA_TFIFO, 1, 1, (B *)"sem_strtol" };

//2013.05.08 Miya ���b�Z�[�WNG�̂��ߋ��p�������őΉ�����
struct{
	unsigned int LcdMsgSize;
	UB LcdMsgBuff[1024];
}g_LcdmsgData;

UINT rcv_mbf_Lcd(ID idnum, UB *msg);


static BOOL read_ethernet_addr(void);
static BOOL read_ip_addr(void);
static void ini_intr(void);
static char *ftp_passwd_check(T_FTP *ftp, const char *user, const char *pass);
static B *telnet_passwd_check(T_TELNETD *t, const B *name, const B *passwd);

void SystemParamInit( void );	// VA300 �V�X�e������p�����[�^��Initial
static UB GetScreenNo(void);			// ���ݕ\���X�N���[���ԍ��̎擾
static void ChgScreenNo( UB NewScreenNo );	// ��ʑJ�ڏ��(�ԍ�)���X�V����
ER NextScrn_Control( void );	// ���̉�ʃR���g���[��
ER static power_on_process( void );		// PC�Ƃ̃R�~���j�P�[�V�����J�n�A�����l�ݒ�v��
ER static send_WakeUp( void );	/** PC��WakeUp�m�F���M	**/
ER static send_camera_param_req( void );	/** �J�����E�p�����[�^�̏����l�v���̑��M	**/
ER static send_cap_param_req( void );		/** �摜�����̏����l�v���̑��M	**/	
ER static send_Led_param_req( void );		/** LED���ʐ��l�̏����l�v���̑��M	**/	
ER static send_touroku_param_req( void );	/** �o�^�f�[�^�̏����l�v���̑��M	**/	

static ER	SndCmdCngMode( UINT stat );		// PC�փ��[�h�ؑւ��ʒm�𑗐M

static ER send_nomal_mode_Wait_Ack_Retry( void ); // ���[�h�ؑ֒ʒm�̑��M�AAck�ENack�҂��ƃ��g���C�t���i�ʏ탂�[�h�ڍs���j
static ER send_nomal_mode( void );			// ���[�h�ؑ֒ʒm�̑��M�i�ʏ탂�[�h�ڍs���j
static ER send_touroku_delete_Wait_Ack_Retry( void );	// �w�i�o�^�j�f�[�^�̍폜�v���̑��M�AAck�ENack�҂��ƃ��g���C�t��
static ER send_touroku_delete_req( void );	// �w�i�o�^�j�f�[�^�̍폜�v���̑��M�i�ʏ탂�[�h���j
static ER send_donguru_chk_Wait_Ack_Retry( void );		// �h���O���̗L���m�F�𑗐M�AAck�ENack�҂��ƃ��g���C�t��
static ER send_donguru_chk_req( void );		// �h���O���̗L���m�F�̑��M�i�ʏ탂�[�h���j
static ER send_password_chk_Wait_Ack_Retry( void );		// �����e�i���X�E���[�h�ڍs���̃p�X���[�h�m�F�v���̑��M�AAck�ENack�҂��ƃ��g���C�t��
static ER send_password_chk_req( void );	// �����e�i���X�E���[�h�ڍs���̃p�X���[�h�m�F�v���̑��M�i�ʏ탂�[�h���j
static ER send_meinte_mode_Wait_Ack_Retry( void );		// ���[�h�ؑ֒ʒm�̑��M�AAck�ENack�҂��ƃ��g���C�t���i�����e�i���X�E���[�h�ڍs���j
static ER send_meinte_mode( void );			// ���[�h�ؑ֒ʒm�̑��M�i�����e�i���X�E���[�h�ڍs���j
static ER send_touroku_init_Wait_Ack_Retry( void );		// �o�^�f�[�^�������v���̑��M�AAck�ENack�҂��ƃ��g���C�t���i�����e�i���X�E���[�h���j
static ER send_touroku_init_req( void );	// �o�^�f�[�^�������v���̑��M�i�����e�i���X�E���[�h���j
static ER send_kinkyuu_touroku_Wait_Ack_Retry( void );	// PC�ցA�ً}�J���ԍ��ʒm���M�AAck�ENack�҂��ƃ��g���C�t��
static ER send_kinkyuu_touroku_req( void );	// PC�ցA�ً}�J���ԍ��ʒm���M�B
static ER send_kinkyuu_8keta_Wait_Ack_Retry( void );	// PC�ցA�ً}�W���ԍ��f�[�^�v�����M�AAck�ENack�҂��ƃ��g���C�t��
static ER send_kinkyuu_8keta_req( void );	// PC�ցA�ً}�W���ԍ��f�[�^�v�����M�B
static ER send_kinkyuu_kaijyou_Wait_Ack_Retry( void );	// PC�ցA�ً}�J���ԍ����M�AAck�ENack�҂��ƃ��g���C�t��
static ER send_kinkyuu_kaijyou_no( void );	// PC�ցA�ً}�J���ԍ����M�v�����M�B
static ER send_kinkyuu_input_Wait_Ack_Retry( void );	// PC�ցA�ً}�ԍ��̑Ó����₢���킹�m�F�v�����M�AAck�ENack�҂��ƃ��g���C�t��
static ER send_kinkyuu_input_no( void );	// PC�ցA�ً}�ԍ��̑Ó����₢���킹�m�F�v�����M�B
static void reload_CAP_Param( void );		// Gain. FixShutter,IR_LED�̃p�����[�^���A�����v���l�ɖ߂��B�iCAP��Retry���ɕς�����\��������ׁB�j

static int ercdStat = 0;				// �G���[�R�[�h�L��
static UB sys_ScreenNo;					// ���݂̃X�N���[��No
static UINT sys_Mode;			// ��ԕϐ�
static UINT sys_SubMode;		// ��ԕϐ��i�T�u�j
static 	UB	s_CapResult;		// �w�F�؂̌���
static UB s_DongleResult;		// �h���O���̗L���m�F�̌���
static UB s_PasswordResult;		// �p�X���[�h�m�F�̌���
static UB s_KinkyuuTourokuResult; // �ً}�o�^�ʒm�̌���
static FLGPTN befor_scrn_no;	// ��ʑJ�ڗp�ϐ��B�u���~���܂����H�v�u�������v�̎��̖߂���FLG_PTN�ԍ��B
static UB  befor_scrn_for_ctrl; // ��ʑJ�ڗp�ϐ��B�u���~���܂����H�v�u�������v�̎��̖߂��̉�ʔԍ��B

static UINT rcv_ack_nak;		// ack��M�t���O�@=0�F����M�A=1:ACK��M�A=-1�Fnak��M

static UB req_restart = 0;		// �p���[�E�I���E�v���Z�X�̍ċN���v���t���O�@=0:�v�������A=1:�v������

// ��M�f�[�^�̈�
#define	RCV_BUF_SIZE	1024
static char cSioRcvBuf[ RCV_BUF_SIZE ];	// �V���A����M�f�[�^�o�b�t�@
static unsigned short usSioRcvCount;	// �V���A����M�f�[�^��

static BOOL s_bMonRs485;				// ���j�^��RS485�t���O

#if defined(_DRV_TEST_)
// �h���C�o�e�X�g
extern BOOL drvTest();

#endif


/********************************************************************/

/**
 * �N���b�N�������̗�
 *
 * �n�[�h�E�F�A�ɍ��킹�ăJ�X�^�}�C�Y���Ă��������B
 */
void ini_clk(void)
{

}


/********************************************************************/
/*
 * rcv_mbf_Lcd
 * 2013.05.08 Miya ���b�Z�[�WNG�̂��ߋ��p�������őΉ�����
 * rcv_mbf �̑���
 */
/********************************************************************/
UINT rcv_mbf_Lcd(ID idnum, UB *msg)
{
	UB *buf;
	int i;
	UINT size;
	
	size = g_LcdmsgData.LcdMsgSize;
	
	buf = msg;
	
	for( i = 0 ; i < size ; i++ ){
		*buf++ = g_LcdmsgData.LcdMsgBuff[i];
	}

	return(size);
}




/********************************************************************/
/*
 * Main task
 */
/********************************************************************/
TASK MainTask(void)
{
	ER ercd;
	FLGPTN	flgptn;	

	int n;
	INT iMonCh, iLBusCh;
	
	T_YBDATA *ybdata;
	
	ErrStatusInit();					// �G���[�X�e�[�^�X������
	
	ercdStat = 0;
	dly_tsk( 3000/MSEC );				// �f�o�C�X�N���ׂ̈̏���Wait

	rtcInit();							// RTC������
	TplInit( SEM_TPL );					// �^�b�`�p�l���R���g���[��������

	ercd = CmrInit( SEM_CMR, TSK_CAMERA );		// �J����������(������M���̓J�����^�X�N�N�����Ȍ�ɍs������)
	if ( ercd != E_OK ) {
		ercdStat = 1;
		ErrCodeSet( ercd );
	}
	IrLedInit( SEM_IRLED );				// �ԊO��LED������
	IrLedCrlOff( IR_LED_2 | IR_LED_3 | IR_LED_4 | IR_LED_5 ); 	// �ԊO��LED�̑S����
	BuzInit( SEM_BUZ );					// �u�U�[������
	LcdTaskInit( TSK_LCD );				// LCD�\���^�X�N������
	
//	SoundInit( TSK_BUZ, ID_FLG_BUZ );	// �u�U�[�e�X�g�^�X�N������
	
	TsTaskInit( TSK_TS );				// ���̌��m�Z���T�^�X�N������
	ExKeyTaskInit( TSK_EXKEY );			// 10�L�[�^�X�N������
	
	SystemParamInit();					// VA300 �V�X�e������p�����[�^��Initial

	/**	UDP�|�[�g�ԍ��AIP�A�h���X��EEPROM�����ݒ�	**/
	lan_set_port( default_udp_port );
	lan_set_ip( ini_ipaddr );
	
	lan_set_mask( ini_mask );			// �T�u�l�b�g�}�X�N��EEPROM�����ݒ�

	// EEPROM�ɃA�N�Z�X���邽�߂�LAN�f�o�C�X�̏��������s���B
	// Ethernet Address ���܂��̓f�t�H���g�ɐݒ肵�Ă�����
	// �ォ��ύX����
	read_ethernet_addr();
	lan_ini(ethernet_addr);

	// IP Address �̓ǂݏo��
	read_ip_addr();

	ercd = tcp_ini();					// �v���g�R���X�^�b�N������ 
	if( ercd != E_OK) {
		ercdStat = 2;
	    ErrCodeSet( ercd );
		slp_tsk();
//		goto LAN_INI_END;
	}
	
	// UDP �ʐM������
	ercd = udp_ini( TSK_UDPRCV, ID_UDP_OYOCMD, udp_portno);	
//	ercd = udp_ini( 0, ID_UDP_OYOCMD, udp_portno);	
	if( ercd != E_OK) {
		ercdStat = 3;
	    ErrCodeSet( ercd );
		slp_tsk();
//	    goto LAN_INI_END;
	}

//	MdCngMode( MD_NORMAL );		// ���u���[�h��"�ʏ탂�[�h"�ŏ����ݒ�

LAN_INI_END:
	sta_tsk(TSK_DISP,    0);			// �\���^�X�N�N��
	sta_tsk(TSK_IO, 0);					// I/O���m�^�X�N�N��
	sta_tsk(TSK_COMMUNICATE, 0);		// UDP�d�������^�X�N
	sta_tsk(TSK_UDP_SEND, 0);			// UDP���M�^�X�N
	sta_tsk(TSK_CMD_LAN,  0);			// �R�}���h�����^�X�N�N��
	
	TmInit(CYC_TMO);					// �^�C���A�E�g�Ď��p�����N���n���h���N��

	ExKeyTaskInit( TSK_EXKEY );			// �L�[���̓^�X�N�N��

#ifndef VA300
	// �`�����l���ؑ�
	iMonCh  = 0;
	iLBusCh = 1;
	s_bMonRs485 = FALSE;
	if (DswGet() & DSW1_7) {
		iMonCh  = 1;
		iLBusCh = 0;
		s_bMonRs485 = TRUE;
	}
#endif

#if ( USE_MON == 1 )
	ercd = Mon_ini( iMonCh );			// ���j�^�R�}���h�^�X�N������

	sta_tsk(TSK_SND1, iMonCh);			// (�f�o�b�O�p)�V���A�����M�^�X�N�N��
#endif
	
	LcdcInit( SEM_LCD );				// LCD�R���g���[���̏�����

	CameraTaskInit( TSK_CAMERA );		// �J�����R���g���[���^�X�N������
	LedInit(SEM_LED);					// LED������
	
	yb_init_all();						// �w���(�������ȊO)�A�ً}�J�����̏�����
	
	LedOut(LED_POW, LED_ON);			// �d���\��LED��ON����i��j
	
PowerOn_Process:
	req_restart = 0;
	
	ercd = set_flg( ID_FLG_LCD, FPTN_LCD_INIT );		// LCD�̏�����ʕ\���̃��N�G�X�g
	if ( ercd != E_OK ) ercdStat = 5;
	
#if defined(_DRV_TEST_)
	drvTest();
#endif
			
	dly_tsk( 1000/MSEC );
	
	MdCngMode( MD_POWER_ON );			// ���u���[�h���p���[�I�����[�h��
	ercd = power_on_process();			// PC�Ƃ̃R�~���j�P�[�V�����J�n�A�@�탂�[�h�ݒ�A�����l�ݒ�
	
	// ��ʕ\���̊J�n
	if ( MdGetMode() == MD_INITIAL ){ 	// �����o�^��ʂ̎��́A��ʂP�ցB
		
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN1 );	// ID�ԍ����͉�ʂցB	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN1 );	// ��ʔԍ��@<-�@���̉��
		}
				
	} else if ( MdGetMode() == MD_NORMAL ){	// �ʏ탂�[�h�̎��́A���100�ցB
		
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// �ʏ탂�[�h���͑ҋ@��ʂցB	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN100 );	// ��ʔԍ��@<-�@���̉��
		}
					
	} else {
		nop();
	}
	
	// ��ʐؑւ��ƁA�J�����B�e����
	for (;;) {
		ercd = wai_flg( ID_FLG_MAIN, ( /*�@FPTN_START_RAWCAP 		// �J�����B�e�v���̎�M�҂� 
									  |*/ FPTN_START_CAP			/* �J�����B�e�v���̎�M�҂� */
									  | FPTN_LCD_CHG_REQ		/* LCD��ʐؑ֗v��(LCD�����C��) */
									  ), TWF_ORW, &flgptn );
		if ( ercd != E_OK ){
			ercdStat = 7;
			break;
		}
		
		switch ( flgptn ) {
			case FPTN_LCD_CHG_REQ: // LCD�^�X�N����A��ʕύX�v������
		
				ercd = clr_flg( ID_FLG_MAIN, ~FPTN_LCD_CHG_REQ );			// �t���O�̃N���A
				if ( ercd != E_OK ){
					ercdStat = 10;
					break;
				}
			
				ercd = NextScrn_Control();		// ���̉�ʕ\���v���t���O�Z�b�g			
				
				break;
				
			case FPTN_START_CAP:	// ���̌��m�Z���T�[����A�J�����B�e�v������	
			
				ercd = clr_flg( ID_FLG_MAIN, ~FPTN_START_CAP );				// �t���O�̃N���A
				if ( ercd != E_OK ){
					ercdStat = 8;
					break;
				}
			
				if ( ( MdGetMode() == MD_NORMAL ) 			// �m�[�}�����[�h�ŁA���101,102,121,141,161�\�����Ȃ�
				    && ( ( GetScreenNo() == LCD_SCREEN101 ) 
					  || ( GetScreenNo() == LCD_SCREEN102 ) 
					  || ( GetScreenNo() == LCD_SCREEN121 ) 
					  || ( GetScreenNo() == LCD_SCREEN141 )
					  || ( GetScreenNo() == LCD_SCREEN161 ) ) ){
						  
//					reload_CAP_Param();			// Gain. FixShutter,IR_LED�̃p�����[�^���A�����v���l�ɖ߂��B�iCAP��Retry���ɕς�����\��������ׁB�j

					ercd = set_flg( ID_FLG_CAMERA, FPTN_START_CAP211 );				// �J�����B�e+�F�؏����i�R�}���h211�j�ցB
					if ( ercd != E_OK ) break;
					
				} else if ( MdGetMode() == MD_NORMAL ){			// �m�[�}�����[�h�ŁA���127�A���129�\�����Ȃ�
			
					if ( ( GetScreenNo() == LCD_SCREEN127 ) 
					  || ( GetScreenNo() == LCD_SCREEN129 ) ){
						
//						if ( GetScreenNo() == LCD_SCREEN127 ){
//							reload_CAP_Param();			// Gain. FixShutter,IR_LED�̃p�����[�^���A�����v���l�ɖ߂��B�iCAP��Retry���ɕς�����\��������ׁB�j
//						} 

						ercd = set_flg( ID_FLG_CAMERA, FPTN_START_CAP204 );			// �J�����B�e+�o�^�����i�R�}���h204�j�ցB
						if ( ercd != E_OK ) break;
					}
				} else if ( MdGetMode() == MD_INITIAL ){		// �����o�^���[�h�ŁA���6�A���8�\�����Ȃ�
			
					if ( ( GetScreenNo() == LCD_SCREEN6 ) 
					  || ( GetScreenNo() == LCD_SCREEN8 ) ){
						  
//						if ( GetScreenNo() == LCD_SCREEN6 ){
//							reload_CAP_Param();			// Gain. FixShutter,IR_LED�̃p�����[�^���A�����v���l�ɖ߂��B�iCAP��Retry���ɕς�����\��������ׁB�j
//						}
												
						ercd = set_flg( ID_FLG_CAMERA, FPTN_START_CAP204 );			// �J�����B�e+�o�^�����i�R�}���h204�j�ցB
						if ( ercd != E_OK ) break;
					}
				} else if ( MdGetMode() == MD_MAINTE ){		// �����e�i���X���[�h�ŁA���6�A���203�\�����Ȃ�
			
					if ( GetScreenNo() == LCD_SCREEN203 ){
						
//						reload_CAP_Param();			// Gain. FixShutter,IR_LED�̃p�����[�^���A�����v���l�ɖ߂��B�iCAP��Retry���ɕς�����\��������ׁB�j

						ercd = set_flg( ID_FLG_CAMERA, FPTN_START_CAP141 );			// �J�����B�e+�t���摜�����i�R�}���h141�j�ցB
						if ( ercd != E_OK ) break;
					}
				}
				break;
/**			
			case FPTN_START_RAWCAP:	// �R�}���h141����A���摜�J�����B�e�v������	
			
				ercd = clr_flg( ID_FLG_MAIN, ~FPTN_START_RAWCAP );			// �t���O�̃N���A
				if ( ercd != E_OK ){
					ercdStat = 9;
					break;
				}
				
//				reload_CAP_Param();			// Gain. FixShutter,IR_LED�̃p�����[�^���A�����v���l�ɖ߂��B�iCAP��Retry���ɕς�����\��������ׁB�j
				
				ercd = set_flg( ID_FLG_CAMERA, FPTN_START_CAP141 );
				if ( ercd != E_OK ) break;	
	    		break;
**/
			default:
				break;
		}
		
		if ( req_restart == 1 ) goto PowerOn_Process;
		
	}
	PrgErrSet();
	slp_tsk();		//�@�����֗��鎞�͎����G���[
}


/*==========================================================================*/
/**
 *	���̉�ʕ\���v���t���O�Z�b�g
 */
/*==========================================================================*/
ER NextScrn_Control( void )
{
	ER ercd;
	UB S_No;
	ER_UINT i;
	ER_UINT rcv_length;
	UB msg[ 128 ];	
	
	S_No = GetScreenNo();
	//memset(msg, 0, sizeof(msg));
	
	switch ( S_No ) {
		
		
	//�@�����o�^���[�h
		case LCD_SCREEN1:		// �����o�^����ID���͉�ʁB
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( ( msg[ 0 ] == LCD_NEXT ) || ( msg[ 0 ] == LCD_USER_ID ) && ( rcv_length >= 5 ) ){
				
				yb_touroku_data.user_id[ 0 ] = msg[ 1 ];	// ���[�U�[ID
				yb_touroku_data.user_id[ 1 ] = msg[ 2 ];
				yb_touroku_data.user_id[ 2 ] = msg[ 3 ];
				yb_touroku_data.user_id[ 3 ] = msg[ 4 ];
				yb_touroku_data.user_id[ 4 ] = ',';
				
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN2 );	// �����o�^/�����e�i���X�̑I����ʂցB	
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN2 );		// ��ʔԍ��@<-�@���̉��
				}
			} else {
				nop();	
			}
			break;

		case LCD_SCREEN2:		// �����o�^ or �����e�i���X�{�^���I�����
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
						
			if ( ( msg[ 0 ] == LCD_INIT_INPUT ) && ( rcv_length >= 1 ) ){	// 	�����o�^�{�^���������ꂽ
				
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN3 );	// �ӔC�Ҕԍ��@�I����ʂցB	
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN3 );		// ��ʔԍ��@<-�@���̉��
				}
				
			} else if ( ( msg[ 0 ] == LCD_MAINTE ) && ( rcv_length >= 1 ) ){// 	�����e�i���X�E�{�^���������ꂽ
				
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN201 );	// �����e�i���X��ʁE�ӔC�Ҕԍ��@�I����ʂցB	
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN201 );		// ��ʔԍ��@<-�@���̉��
					MdCngMode( MD_MAINTE );				// ���u���[�h�������e�i���X�E���[�h��
//					ercd = SndCmdCngMode( MD_MAINTE );	// PC��	�����e�i���X�E���[�h�ؑւ��ʒm�𑗐M
					if ( ercd != E_OK ){
						nop();		// �G���[�����̋L�q	
					}
				}
				
			} else {
				nop();	
			}
			break;

		case LCD_SCREEN3:		// �ӔC�Ҏw�ԍ��@�I�����
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( ( msg[ 0 ] == LCD_YUBI_ID ) && ( rcv_length >= 4 ) ){	
			
				yb_touroku_data.yubi_seq_no[ 0 ] = msg[ 1 ];	// �ӔC�Ҏw�ԍ�
				yb_touroku_data.yubi_seq_no[ 1 ] = msg[ 2 ];
				yb_touroku_data.yubi_seq_no[ 2 ] = msg[ 3 ];
				yb_touroku_data.yubi_seq_no[ 3 ] = ',';
				
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN4 );	// �����@���͉�ʂ�
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN4 );		// ��ʔԍ��@<-�@���̉��
				}
				
			} else if ( ( msg[ 0 ] == LCD_CANCEL ) && ( rcv_length >= 1 ) ){
			
				befor_scrn_no = FPTN_LCD_SCREEN3;
				befor_scrn_for_ctrl = LCD_SCREEN3;
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN12 );	// 	�u���~���Ă�..�v��ʂցB
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN12 );	// ��ʔԍ��@<-�@���̉��
				}	
								
			} else {
				nop();	
			}
			break;

		case LCD_SCREEN4:		// �����@���͉��
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( ( msg[ 0 ] == LCD_NAME ) && ( rcv_length >= 25 ) ){	

				for(i = 0 ; i < 24 ; i++ ){					// ����
					yb_touroku_data.name[ i ] = msg[ 1 + i ];
				}
				
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN5 );	// 	�w��ʉ�ʂցB
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN5 );		// ��ʔԍ��@<-�@���̉��
				}
				
			} else if ( ( msg[ 0 ] == LCD_CANCEL ) && ( rcv_length >= 1 ) ){
			
				befor_scrn_no = FPTN_LCD_SCREEN4;
				befor_scrn_for_ctrl = LCD_SCREEN4;
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN12 );	// 	�u���~���Ă�..�v��ʂցB
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN12 );	// ��ʔԍ��@<-�@���̉��
				}	
								
			} else {
				nop();	
			}
			break;

		case LCD_SCREEN5:		// �w��ʑI�����
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( ( msg[ 0 ] == LCD_YUBI_SHUBETU ) && ( rcv_length >= 3 ) ){	
			
				yb_touroku_data.yubi_no[ 0 ] = msg[ 1 ];	// �w���
				yb_touroku_data.yubi_no[ 1 ] = msg[ 2 ];
				yb_touroku_data.yubi_no[ 2 ] = ',';
				
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN6 );	// 	�u�w���Z�b�g����..�v��ʂցB
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN6 );		// ��ʔԍ��@<-�@���̉��
				}
				
			} else if ( ( msg[ 0 ] == LCD_CANCEL ) && ( rcv_length >= 1 ) ){
			
				befor_scrn_no = FPTN_LCD_SCREEN5;
				befor_scrn_for_ctrl = LCD_SCREEN5;
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN12 );	// 	�u���~���Ă�..�v��ʂցB
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN12 );	// ��ʔԍ��@<-�@���̉��
				}	
								
			} else {
				nop();	
			}
			break;			
		
		case LCD_SCREEN6:		// �u�w���Z�b�g����..�v��ʁB
			break;				// �o�^�̐��̃Z���T�[���mON�҂��BLCD�^�X�N�́A�����ւ�change Req�͏o���Ȃ��͂��B

		case LCD_SCREEN7:		// �u�w�𔲂��ĉ�����..�v��ʁB
			break;				// �o�^�̐��̃Z���T�[���mOFF�҂��BLCD�^�X�N�́A�����ւ�change Req�͏o���Ȃ��͂��B

		case LCD_SCREEN8:		// �u������x�w���Z�b�g����..�v��ʁB
			break;				// �o�^�̐��̃Z���T�[���m�҂��BLCD�^�X�N�́A�����ւ�change Req�͏o���Ȃ��͂��B

		case LCD_SCREEN9:		// �o�^��������ʁB
			
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN11 );	// �u�o�^�𑱂��܂����v��ʂցB
			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN11 );		// ��ʔԍ��@<-�@���̉��
			}		
			break;		

		case LCD_SCREEN10:		// �o�^���s�~��ʁB
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN2 );	// �u�����o�^���j���[�v��ʂցB
			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN2 );			// ��ʔԍ��@<-�@���̉��
			}
//			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN11 );	// �u�o�^�𑱂��܂����v��ʂցB
//			if ( ercd == E_OK ){
//				ChgScreenNo( LCD_SCREEN11 );		// ��ʔԍ��@<-�@���̉��
//			}
			break;		

		case LCD_SCREEN11:		// �o�^���s�́u�͂��v�u�������v�I����ʁB
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_YES ){			// �͂�
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN3 );	// �ӔC�Ҕԍ��I����ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN3 );		// ��ʔԍ��@<-�@���̉��
					}
				} else if ( msg[ 0 ] == LCD_NO ){	// ������
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// �ʏ��ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN100 );		// ��ʔԍ��@<-�@���̉��
						MdCngMode( MD_NORMAL );				// ���u���[�h��ʏ탂�[�h��
//						ercd = SndCmdCngMode( MD_NORMAL );	// PC��	�ʏ탂�[�h�ؑւ��ʒm�𑗐M
						if ( ercd != E_OK ){
							nop();		// �G���[�����̋L�q	
						}
					}
				} else {
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN11 );	// �u�o�^�𑱂��܂����v��ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN11 );		// ��ʔԍ��@<-�@���̉��
					}		
				}					
			}
			break;		

		case LCD_SCREEN12:		// ���~�́u�͂��v�u�������v�I����ʁB
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_YES ){			// �͂�
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN2 );	// ���[�U�[ID�ԍ��I����ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN2 );		// ��ʔԍ��@<-�@���̉��
					}
				} else if ( msg[ 0 ] == LCD_NO ){	// ������
					ercd = set_flg( ID_FLG_LCD, ( FLGPTN )befor_scrn_no );	// �ʏ��ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( ( UB )befor_scrn_for_ctrl );		// ��ʔԍ��@<-�@���̉��
					}
				}					
			}			
			break;		



	// �ʏ탂�[�h				
		case LCD_SCREEN100:		// �u�����N��ʁB
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN101 );	// �ʏ탂�[�h�̑ҋ@���
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN101 );		// ��ʔԍ��@<-�@���̉��
				}
			}	
			break;
		
		case LCD_SCREEN101:		// �ʏ탂�[�h�̃��j���[���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_TOUROKU ){		// �o�^�{�^������
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN120 );	// �ʏ탂�[�h�o�^��ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN120 );		// ��ʔԍ��@<-�@���̉��
					}
				} else if ( msg[ 0 ] == LCD_SAKUJYO ){	// �폜�{�^������
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN140 );	// �ʏ탂�[�h�폜��ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN140 );		// ��ʔԍ��@<-�@���̉��
					}
				} else if ( msg[ 0 ] == LCD_KINKYUU_SETTEI ){	// "�ً}�ԍ��ݒ�"�{�^���������ꂽ
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN160 );	// �ً}�ԍ��ݒ�E������ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN160 );		// ��ʔԍ��@<-�@���̉��
					}					
				} else if ( msg[ 0 ] == LCD_KINKYUU_KAIJYOU ){	//  "�ً}����"�{�^���������ꂽ"
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN180 );	// �ً}�J���E������ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN180 );		// ��ʔԍ��@<-�@���̉��
					}													
				} else if ( msg[ 0 ] == LCD_MAINTE ){	// �����e�i���X�E�{�^������
						
					send_donguru_chk_Wait_Ack_Retry();	// PC�ցA�h���O���̗L���m�F�𑗐M�B						
						
					// �����e�i���X���[�h��ʈڍs�́A�R�}���h002��M�������ŁAOK��M�����ꍇ�Ɏ��s����B

				}
			}
			break;
		
		case LCD_SCREEN102:		// �ʏ탂�[�h�̑ҋ@��ʂ̃u�����N�\��
			break;				// LCD�^�X�N�́A�����ւ�change Req�͏o���Ȃ��͂��B
			
		
		case LCD_SCREEN103:		// �F�؊�������ʁB		
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN101 );	// �ʏ탂�[�h�̑ҋ@��ʂցB
			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN101 );		// ��ʔԍ��@<-�@���̉��
				MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h�ցi�K�v�������O�̂��߁j
			}
			break;
		
		case LCD_SCREEN104:		// �F�؊����~��ʁB
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN101 );	// �ʏ탂�[�h�̃��j���[��ʂցB
			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN101 );		// ��ʔԍ��@<-�@���̉��
				MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h�ցi�K�v�������O�̂��߁j
			}
			break;
		
	// �ʏ탂�[�h�E�o�^
		case LCD_SCREEN120:		// �ʏ탂�[�h�E�o�^�̃u�����N���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN121 );	// 
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN121 );		// ��ʔԍ��@<-�@���̉��
				}
			}
			break;
			
		case LCD_SCREEN121:		// �o�^���E�ӔC�҂̔F�؉��
			break;				// ���̃Z���T�[���m�҂��BLCD�^�X�N�́A�����ւ�change Req�͏o���Ȃ��͂��B
			
		case LCD_SCREEN122:		// �ӔC�҂̔F�ؐ��������
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN124);	// �ӔC�ҁE��ʎ҂̓o�^�ԍ��I����ʂ�
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN124 );		// ��ʔԍ��@<-�@���̉��
				}
			}
			break;
			
		case LCD_SCREEN123:		// �ӔC�҂̔F�؎��s�~���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// �ʏ탂�[�h�̃u�����N���
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN100 );		// ��ʔԍ��@<-�@���̉��
				}
			}
			break;
			
		case LCD_SCREEN124:		// �ӔC�ҁE��ʎ҂̓o�^�ԍ��I�����
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( ( msg[ 0 ] == LCD_YUBI_ID ) && ( rcv_length >= 4 ) ){	
			
				yb_touroku_data.yubi_seq_no[ 0 ] = msg[ 1 ];	// �ӔC�Ҏw�ԍ�
				yb_touroku_data.yubi_seq_no[ 1 ] = msg[ 2 ];
				yb_touroku_data.yubi_seq_no[ 2 ] = msg[ 3 ];
				yb_touroku_data.yubi_seq_no[ 3 ] = ',';
				
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN125 );	// �����@���͉�ʂ�
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN125 );		// ��ʔԍ��@<-�@���̉��
				}
				
			} else if ( ( msg[ 0 ] == LCD_CANCEL ) && ( rcv_length >= 1 ) ){
			
				befor_scrn_no = FPTN_LCD_SCREEN124;
				befor_scrn_for_ctrl = LCD_SCREEN124;
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN132 );	// 	�u���~���Ă�..�v��ʂցB
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN132 );	// ��ʔԍ��@<-�@���̉��
				}	
								
			} else {
				nop();	
			}	
			break;
			
		case LCD_SCREEN125:		// ���O���͉��
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( ( msg[ 0 ] == LCD_NAME ) && ( rcv_length >= 25 ) ){	

				for(i = 0 ; i < 24 ; i++ ){					// ����
					yb_touroku_data.name[ i ] = msg[ 1 + i ];
				}
				
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN126 );	// 	�w��ʉ�ʂցB
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN126 );		// ��ʔԍ��@<-�@���̉��
				}
				
			} else if ( ( msg[ 0 ] == LCD_CANCEL ) && ( rcv_length >= 1 ) ){
			
				befor_scrn_no = FPTN_LCD_SCREEN125;
				befor_scrn_for_ctrl = LCD_SCREEN125;
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN132 );	// 	�u���~���Ă�..�v��ʂցB
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN132 );	// ��ʔԍ��@<-�@���̉��
				}	
								
			} else {
				nop();	
			}
			break;
			
		case LCD_SCREEN126:		// �w��ʑI�����
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( ( msg[ 0 ] == LCD_YUBI_SHUBETU ) && ( rcv_length >= 3 ) ){	
			
				yb_touroku_data.yubi_no[ 0 ] = msg[ 1 ];	// �w���
				yb_touroku_data.yubi_no[ 1 ] = msg[ 2 ];
				yb_touroku_data.yubi_no[ 2 ] = ',';
				
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN127 );	// 	�u�w���Z�b�g����..�v��ʂցB
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN127 );		// ��ʔԍ��@<-�@���̉��
				}
				
			} else if ( ( msg[ 0 ] == LCD_CANCEL ) && ( rcv_length >= 1 ) ){
			
				befor_scrn_no = FPTN_LCD_SCREEN126;
				befor_scrn_for_ctrl = LCD_SCREEN126;
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN132 );	// 	�u���~���Ă�..�v��ʂցB
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN132 );	// ��ʔԍ��@<-�@���̉��
				}	
								
			} else {
				nop();	
			}
			break;
			
		case LCD_SCREEN127:		// �u�w���Z�b�g����...�v���
			break;				// ���̃Z���T�[���m�҂��BLCD�^�X�N�́A�����ւ�change Req�͏o���Ȃ��͂��B
			
		case LCD_SCREEN128:		// �u�w�𔲂��ĉ������v���
			break;				// ���̃Z���T�[���m�҂��BLCD�^�X�N�́A�����ւ�change Req�͏o���Ȃ��͂��B
			
		case LCD_SCREEN129:		// �u������x�A�w��...�v���
			break;				// ���̃Z���T�[���m�҂��BLCD�^�X�N�́A�����ւ�change Req�͏o���Ȃ��͂��B
			
		case LCD_SCREEN130:		// �o�^���������
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN124);	// �ӔC�ҁE��ʎ҂̓o�^�ԍ��I����ʂ�
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN124 );		// ��ʔԍ��@<-�@���̉��
				}
			}	
			break;
			
		case LCD_SCREEN131:		// �o�^���s�~���

			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN124);	// �ӔC�ҁE��ʎ҂̓o�^�ԍ��I����ʂ�
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN124 );		// ��ʔԍ��@<-�@���̉��
				}
			}
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			break;
			
		case LCD_SCREEN132:		// ���~�́u�͂��v�u�������v�I����ʁB
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_YES ){			// �͂�
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// ���[�U�[ID�ԍ��I����ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN100 );		// ��ʔԍ��@<-�@���̉��
					}
				} else if ( msg[ 0 ] == LCD_NO ){	// ������
					//���Ή��@20130510 Miya
//					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN124 );	// ���[�U�[ID�ԍ��I����ʂցB
//					if ( ercd == E_OK ){
//						ChgScreenNo( LCD_SCREEN124 );		// ��ʔԍ��@<-�@���̉��
//					}

					ercd = set_flg( ID_FLG_LCD, ( FLGPTN )befor_scrn_no );	// �ʏ��ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( ( UB )befor_scrn_for_ctrl );		// ��ʔԍ��@<-�@���̉��
					}

				}					
			}			
			break;
			
		
	// �ʏ탂�[�h�E�폜
		case LCD_SCREEN140:		// �ʏ탂�[�h�E�폜�̃u�����N���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			//20130620_Miya �f���p�ɍ폜��ʑJ�ڕύX
/*
			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN145 );	// �ʏ탂�[�h�폜���E�ӔC�҂̔F�؉��
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN145 );		// ��ʔԍ��@<-�@���̉��
				}
			}
*/

/*�@20130620_Miya �f���p�ɍ폜��ʑJ�ڕύX�@�� */
			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN141 );	// �ʏ탂�[�h�폜���E�ӔC�҂̔F�؉��
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN141 );		// ��ʔԍ��@<-�@���̉��
				}
			}
/**/

			break;
			
		case LCD_SCREEN141:		// �폜���E�ӔC�҂̔F�؉��
			break;				// ���̃Z���T�[���m�҂��BLCD�^�X�N�́A�����ւ�change Req�͏o���Ȃ��͂��B
			
		case LCD_SCREEN142:		// �ӔC�҂̔F�ؐ��������
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN144);	// �ӔC�ҁE��ʎ҂̍폜�ԍ��I����ʂ�
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN144 );		// ��ʔԍ��@<-�@���̉��
				}
			}	
			break;
			
		case LCD_SCREEN143:		// �ӔC�҂̔F�؎��s�~���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100);	// �ʏ탂�[�h�u�����N��ʂ�
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN100 );		// ��ʔԍ��@<-�@���̉��
				}
			}
			break;
			
		case LCD_SCREEN144:		// �ӔC�ҁE��ʎ҂̍폜�ԍ��I�����
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
		
			if ( ( msg[ 0 ] == LCD_YUBI_ID ) && ( rcv_length >= 4 ) ){	
			
				yb_touroku_data.yubi_seq_no[ 0 ] = msg[ 1 ];	// �폜���̎w�ԍ�
				yb_touroku_data.yubi_seq_no[ 1 ] = msg[ 2 ];
				yb_touroku_data.yubi_seq_no[ 2 ] = msg[ 3 ];
				yb_touroku_data.yubi_seq_no[ 3 ] = ',';
				
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN145 );	// �u�폜���Ă���낵���ł����v��ʂ�
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN145 );		// ��ʔԍ��@<-�@���̉��
				}
				
			} else if ( ( msg[ 0 ] == LCD_CANCEL ) && ( rcv_length >= 1 ) ){
			
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN146 );	// 	�u���~���Ă�..�v��ʂցB
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN146 );	// ��ʔԍ��@<-�@���̉��
				}	
								
			} else {
				nop();	
			}	
			break;
			
		case LCD_SCREEN145:		// �u�폜���Ă���낵���ł����v���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );


			if ( rcv_length >= 1 ){
/*
				//20130620_Miya �f���p�ɍ폜��ʑJ�ڕύX
				if ( msg[ 0 ] == LCD_YES ){			// �͂�
//					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// �폜�ԍ��I����ʂցB
//					if ( ercd == E_OK ){
//						ChgScreenNo( LCD_SCREEN100 );		// ��ʔԍ��@<-�@���̉��
						//send_touroku_delete_Wait_Ack_Retry();	// PC�ցA�w�o�^���P���̍폜�v�����M�B
						send_touroku_init_Wait_Ack_Retry();		// PC�ցA�o�^�f�[�^�̏������v�����M�B
					if ( ercd == E_OK ){
						req_restart = 1;			// �p���[�I���v���Z�X�̍ċN���v���t���O�̃Z�b�g�B
					}
						if ( ercd == E_OK ){
							req_restart = 1;		// �p���[�I���v���Z�X�̍ċN���v���t���O�̃Z�b�g�B
						}
//					}

				} else if ( msg[ 0 ] == LCD_NO ){	// ������
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// ���~�́u�͂��v�u�������v�I�����
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN100 );		// ��ʔԍ��@<-�@���̉��
					}
				}					
*/				
				
/*�@20130620_Miya �f���p�ɍ폜��ʑJ�ڕύX�@�� */
				if ( msg[ 0 ] == LCD_YES ){			// �͂�
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN144 );	// �폜�ԍ��I����ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN144 );		// ��ʔԍ��@<-�@���̉��
						send_touroku_delete_Wait_Ack_Retry();	// PC�ցA�w�o�^���P���̍폜�v�����M�B
					}

				} else if ( msg[ 0 ] == LCD_NO ){	// ������
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN144 );	// ���~�́u�͂��v�u�������v�I�����
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN144 );		// ��ʔԍ��@<-�@���̉��
					}
				}	
/**/				
			}
			break;
			
		case LCD_SCREEN146:		// ���~�́u�͂��v�u�������v�I�����
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_YES ){			// �͂�
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// ���[�U�[ID�ԍ��I����ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN100 );		// ��ʔԍ��@<-�@���̉��
					}
				} else if ( msg[ 0 ] == LCD_NO ){	// ������
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN144 );	// �ʏ��ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN144 );		// ��ʔԍ��@<-�@���̉��
					}
				}					
			}	
			break;

	// �ʏ탂�[�h�E�ً}�J���ԍ��ݒ�
		case LCD_SCREEN160:		// �ʏ탂�[�h�E�ً}�J���ԍ��ݒ�̃u�����N���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN161 );	// �u�ӔC�҂̎w��...�v���
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN161 );		// ��ʔԍ��@<-�@���̉��
				}
			}
			break;

		case LCD_SCREEN161:		// �ʏ탂�[�h�E�ً}�J���ԍ��ݒ�E�u�ӔC�҂̎w��...�v���
			break;				// ���̃Z���T�[���m�҂��BLCD�^�X�N�́A�����ւ�change Req�͏o���Ȃ��͂��B
			
		case LCD_SCREEN162:		// �ʏ탂�[�h�E�ً}�J���ԍ��ݒ�E�ӔC�҂̔F�ؐ��������
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN164 );	// �ً}�J���ԍ��̐ݒ���
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN164 );		// ��ʔԍ��@<-�@���̉��
				}
			}
			break;
			
		case LCD_SCREEN163:		// �ʏ탂�[�h�E�ً}�J���ԍ��ݒ�E�ӔC�҂̔F�؎��s�~���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// �ʏ탂�[�h�E�������
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN100 );		// ��ʔԍ��@<-�@���̉��
				}
			}
			break;

		case LCD_SCREEN164:		// �ʏ탂�[�h�E�ً}�J���ԍ��̐ݒ���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			if ( rcv_length >= 1 ){			
				if ( ( ( msg[ 0 ] == LCD_NEXT ) || ( msg[ 0 ] == LCD_KINKYUU_BANGOU ) ) && ( rcv_length >= 5 ) ){
				
					kinkyuu_touroku_no[ 0 ] = msg[ 1 ];		// �ً}�J���ԍ�
					kinkyuu_touroku_no[ 1 ] = msg[ 2 ];
					kinkyuu_touroku_no[ 2 ] = msg[ 3 ];
					kinkyuu_touroku_no[ 3 ] = msg[ 4 ];
					kinkyuu_touroku_no[ 4 ] = 0;
				
					send_kinkyuu_touroku_Wait_Ack_Retry();	// PC�ցA�ً}�J���ԍ��ʒm���M�B
								
					// ����ʁi�ʏ탂�[�h�E������ʁj�ւ̑J�ڂ́A�ً}�J���ԍ��ʒm���M��OK���ʂ���M��A���̎�M�R�}���h�����Ŏ��s����B	

				} else if ( msg[ 0 ] == LCD_CANCEL ){		// �u���~�v�{�^���������ꂽ
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN165 );	// �ʏ탂�[�h������ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN165 );		// ��ʔԍ��@<-�@���̉��
					}	
				}
			}
			break;
			
		case LCD_SCREEN165:		// �ʏ탂�[�h�E�ً}�J���ԍ��u���~���Ă���낵���ł����H�v���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_YES ){					// �u�͂��v�������ꂽ
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// �ʏ탂�[�h�E�������
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN100 );		// ��ʔԍ��@<-�@���̉��
					}
										
				} else if ( msg[ 0 ] == LCD_NO ){			// �u�������v�������ꂽ
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN164 );	// �ʏ탂�[�h�E�ً}�J���ԍ��̐ݒ���
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN164 );		// ��ʔԍ��@<-�@���̉��
					}					
				}
			}
			break;			

			
	// �ʏ탂�[�h�E�ً}�J��
		case LCD_SCREEN180:		// �ʏ탂�[�h�E�ً}�J���̃u�����N���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN181 );	// �u�R�[���Z���^�[�ɘA������...�v���
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN181 );		// ��ʔԍ��@<-�@���̉��
				}					
			}
			break;

		case LCD_SCREEN181:		// �ʏ탂�[�h�E�ً}�J���́u�R�[���Z���^�[�ɘA������...�v���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 0 ){
				
				if ( msg[ 0 ] == LCD_NEXT ){			// �u�y�[�W�v�{�^���������ꂽ
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN182 );	// �uID�ԍ�����͂���...�v���
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN182 );	// ��ʔԍ��@<-�@���̉��
					}
					
				} else if ( msg[ 0 ] == LCD_CANCEL ){	// �u���~�v�{�^���������ꂽ
					befor_scrn_no = FPTN_LCD_SCREEN181;
					befor_scrn_for_ctrl = LCD_SCREEN181;
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN188 );	// �ʏ탂�[�h�E�ً}�J���u���~���܂����H�v���
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN188 );	// ��ʔԍ��@<-�@���̉��
					}	
				}					
			}
			break;

		case LCD_SCREEN182:		// �ʏ탂�[�h�E�ً}�J���́uID�ԍ�����͂���...�v���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( ( ( msg[ 0 ] == LCD_NEXT ) || ( msg[ 0 ] == LCD_USER_ID ) ) && ( rcv_length >= 5 ) ){
					if ( ( yb_touroku_data.user_id[ 0 ] == msg[ 1 ] )
					  && ( yb_touroku_data.user_id[ 1 ] == msg[ 2 ] )
					  && ( yb_touroku_data.user_id[ 2 ] == msg[ 3 ] )
					  && ( yb_touroku_data.user_id[ 3 ] == msg[ 4 ] ) ){	// ID�ԍ�����v�������B
						
						send_kinkyuu_8keta_Wait_Ack_Retry();	// PC�ցA�ً}�W���ԍ��f�[�^�v�����M�AAck�ENack�҂��ƃ��g���C�t��
						
						// ����ʂւ̑J�ڂ́A�ً}�W���ԍ��f�[�^�v���̉����ŁA�ԍ��ʒm���󂯎�������Ɏ�M�������ōs���B
						
					} else {	// �o�^�ς݃��[�U�[ID�ԍ��ƁA���͂����ԍ����s��v�̎��A
						
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN187 );	// �F�؎��s�~���
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN187 );		// ��ʔԍ��@<-�@���̉��
						}						
					}
					 
				} else if ( msg[ 0 ] == LCD_CANCEL ){	// �u���~�v�{�^���������ꂽ
					befor_scrn_no = FPTN_LCD_SCREEN182;
					befor_scrn_for_ctrl = LCD_SCREEN182;
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN188 );	// �ʏ탂�[�h�E�ً}�J���u���~���܂����H�v���
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN188 );		// ��ʔԍ��@<-�@���̉��
					}	
				}
			}
			break;

		case LCD_SCREEN183:		// �ʏ탂�[�h�E�ً}�J���́u�ԍ����R�[���Z���^�[��...�v���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_NEXT ){			// �u�y�[�W�v�{�^���������ꂽ
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN184 );	// �u�W���ԍ�����͂���...�v���
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN184 );	// ��ʔԍ��@<-�@���̉��
					}			
				
				} else if ( msg[ 0 ] == LCD_CANCEL ){	// �u���~�v�{�^���������ꂽ
					befor_scrn_no = FPTN_LCD_SCREEN183;
					befor_scrn_for_ctrl = LCD_SCREEN183;
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN188 );	// �ʏ탂�[�h�E�ً}�J���u���~���܂����H�v���
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN188 );		// ��ʔԍ��@<-�@���̉��
					}	
				}
			}
			break;			


		case LCD_SCREEN184:		// �ʏ탂�[�h�E�ً}�J���́u�W���ԍ�����͂��ĉ�����...�v���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( ( ( msg[ 0 ] == LCD_NEXT ) || ( msg[ 0 ] == LCD_KINKYUU_KAIJYOU_BANGOU ) ) && ( rcv_length >= 9 ) ){								
					kinkyuu_kaijyo_no[ 0 ] = msg[ 1 ];		// �ً}�W���ԍ�
					kinkyuu_kaijyo_no[ 1 ] = msg[ 2 ];
					kinkyuu_kaijyo_no[ 2 ] = msg[ 3 ];
					kinkyuu_kaijyo_no[ 3 ] = msg[ 4 ];
					kinkyuu_kaijyo_no[ 4 ] = msg[ 5 ];
					kinkyuu_kaijyo_no[ 5 ] = msg[ 6 ];
					kinkyuu_kaijyo_no[ 6 ] = msg[ 7 ];
					kinkyuu_kaijyo_no[ 7 ] = msg[ 8 ];
					kinkyuu_kaijyo_no[ 8 ] = 0;

					send_kinkyuu_kaijyou_Wait_Ack_Retry();	// PC�ցA�ً}�J���ԍ����M�AAck�ENack�҂��ƃ��g���C�t��
								
					// ����ʁi�ً}�ԍ����͉�ʁj�ւ̑J�ڂ́A�ً}�J���ԍ��W�����M��OK���ʂ���M��A���̎�M�R�}���h�����Ŏ��s����B			
				
				} else if ( msg[ 0 ] == LCD_CANCEL ){	// �u���~�v�{�^���������ꂽ
					befor_scrn_no = FPTN_LCD_SCREEN184;
					befor_scrn_for_ctrl = LCD_SCREEN184;
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN188 );	// �ʏ탂�[�h�E�ً}�J���u���~���܂����H�v���
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN188 );		// ��ʔԍ��@<-�@���̉��
					}	
				}
			}
			break;


		case LCD_SCREEN185:		// �ʏ탂�[�h�E�u�ً}�ԍ�����͂��ĉ�����...�v���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){	
				if ( ( ( msg[ 0 ] == LCD_NEXT ) || ( msg[ 0 ] == LCD_KINKYUU_BANGOU ) ) && ( rcv_length >= 5 ) ){								
					kinkyuu_input_no[ 0 ] = msg[ 1 ];		// ���͂��ꂽ�ً}�ԍ�
					kinkyuu_input_no[ 1 ] = msg[ 2 ];
					kinkyuu_input_no[ 2 ] = msg[ 3 ];
					kinkyuu_input_no[ 3 ] = msg[ 4 ];
					kinkyuu_input_no[ 4 ] = 0;
					
					send_kinkyuu_input_Wait_Ack_Retry();	// PC�ցA�ً}�ԍ��̑Ó����₢���킹�m�F�v�����M�AAck�ENack�҂��ƃ��g���C�t��
								
					// ����ʁiOK/NG��ʁj�ւ̑J�ڂ́A�ً}�ԍ��Ó����m�F�v�����M��OK���ʂ���M��A���̎�M�R�}���h�����Ŏ��s����B			
				
				} else if ( msg[ 0 ] == LCD_CANCEL ){	// �u���~�v�{�^���������ꂽ
					befor_scrn_no = FPTN_LCD_SCREEN185;
					befor_scrn_for_ctrl = LCD_SCREEN185;
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN188 );	// �ʏ탂�[�h�E�ً}�J���u���~���܂����H�v���
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN188 );		// ��ʔԍ��@<-�@���̉��
					}	
				}
			}
			break;

		case LCD_SCREEN186:		// �ʏ탂�[�h�E�ً}�J���̔F�ؐ��������
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// �ʏ탂�[�h�E������ʂցB
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN100 );		// ��ʔԍ��@<-�@���̉��
				}
			}
			break;
			
		case LCD_SCREEN187:		// �ʏ탂�[�h�E�ً}�J���̔F�؎��s�~���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// �ʏ탂�[�h�E������ʂցB
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN100 );		// ��ʔԍ��@<-�@���̉��
				}
			}
			break;

			
		case LCD_SCREEN188:		// �ʏ탂�[�h�E�ً}�J�����u���~���܂����H�v���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_YES ){					// �u�͂��v�������ꂽ
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// �ʏ탂�[�h�E�������
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN100 );		// ��ʔԍ��@<-�@���̉��
					}
										
				} else if ( msg[ 0 ] == LCD_NO ){			// �u�������v�������ꂽ
					ercd = set_flg( ID_FLG_LCD, ( FLGPTN )befor_scrn_no );	// �O��ʂ֖߂�
					if ( ercd == E_OK ){
						ChgScreenNo( ( UB )befor_scrn_for_ctrl );	// ��ʔԍ��@<-�@���̉��
					}					
				}
			}
			break;			

		
	// �����e�i���X���
		case LCD_SCREEN200:		// �����e�i���X�E���[�h�̃u�����N���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN201 );	// �����e�i���X�E���[�h��ID�ԍ����͉��
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN201 );		// ��ʔԍ��@<-�@���̉��
				}					
			}
			break;

		case LCD_SCREEN201:		// �����e�i���X��ʂ̃p�X���[�h�ԍ����͉��
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){			
				if ( ( ( msg[ 0 ] == LCD_NEXT ) || ( msg[ 0 ] == LCD_USER_ID ) ) && ( rcv_length >= 5 ) ){
				
					mainte_password[ 0 ] = msg[ 1 ];	// �p�X���[�h�ԍ�
					mainte_password[ 1 ] = msg[ 2 ];
					mainte_password[ 2 ] = msg[ 3 ];
					mainte_password[ 3 ] = msg[ 4 ];
					mainte_password[ 4 ] = 0;
				
					send_password_chk_Wait_Ack_Retry();	// PC�ցA�p�X���[�h�̈�v�m�F�v�����M�B
								
					// �����e�i���X�E���j���[��ʂւ̑J�ڂ́A�p�X���[�h�̈�v�m�F�v�����M�̌��ʂ�OK���������A���̎�M�R�}���h�����Ŏ��s����B�B	

				} else if ( msg[ 0 ] == LCD_CANCEL ){	// �u���~�v�{�^���������ꂽ
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// �ʏ탂�[�h������ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN100 );		// ��ʔԍ��@<-�@���̉��
						MdCngMode( MD_NORMAL );				// ���u���[�h��ʏ탂�[�h��
//						ercd = SndCmdCngMode( MD_NORMAL );	// PC��	�ʏ탂�[�h�ؑւ��ʒm�𑗐M
					}	
				}
			}
			break;
			
		case LCD_SCREEN202:		// �����e�i���X�E���j���[���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_FULL_PIC_SEND_REQ ){	// �t���摜���M�{�^��
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN203 );	// �u�w���Z�b�g����...�v��ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN203 );		// ��ʔԍ��@<-�@���̉��
					}
					
				} else if ( msg[ 0 ] == LCD_MAINTE_SHOKIKA_REQ ){	// �������{�^��
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN206 );	// �������ڍs�u�폜���Ă���낵���ł����H�v��ʂ�
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN206 );		// ��ʔԍ��@<-�@���̉��
					}			
				
				} else if ( msg[ 0 ] == LCD_MAINTE_END ){	// �I���{�^��
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// �ʏ탂�[�h��ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN100 );		// ��ʔԍ��@<-�@���̉��
						MdCngMode( MD_NORMAL );				// ���u���[�h��ʏ탂�[�h��
//						ercd = SndCmdCngMode( MD_NORMAL );	// PC��	�ʏ탂�[�h�ؑւ��ʒm�𑗐M
					}
				}		
			break;
			
		case LCD_SCREEN203:		// �t���摜���M�E�u�w���Z�b�g����...�v���
			break;				// ���̃Z���T�[���m�҂��BLCD�^�X�N�́A�����ւ�change Req�͏o���Ȃ��͂��B

			
		case LCD_SCREEN204:		// �t���摜���M�E�w�摜�̎擾���������
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN202 );	// �����e�i���X�E���j���[��ʂ�
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN202 );		// ��ʔԍ��@<-�@���̉��
				}
			}
			break;
			
		case LCD_SCREEN205:		// �t���摜���M�E�w�摜�̎擾���s�~���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN202 );	// �����e�i���X�E���j���[��ʂ�
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN202 );		// ��ʔԍ��@<-�@���̉��
				}
			}
			break;
			
		case LCD_SCREEN206:		// �������ڍs�u�폜���Ă���낵���ł����H�v���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_YES ){				// �u�͂��v�������ꂽ

					send_touroku_init_Wait_Ack_Retry();		// PC�ցA�o�^�f�[�^�̏������v�����M�B
					if ( ercd == E_OK ){
						req_restart = 1;			// �p���[�I���v���Z�X�̍ċN���v���t���O�̃Z�b�g�B
					}
										
				} else if ( msg[ 0 ] == LCD_NO ){		// �u�������v�������ꂽ
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN202 );	// �����e�i���X�E���j���[�I�����
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN202 );		// ��ʔԍ��@<-�@���̉��
					}					
				}
			}
			break;
			
		}
		 
		 
		default:
			break;
	}
}


/*==========================================================================*/
/**
 *	PC�Ƃ̃R�~���j�P�[�V�����J�n�A�e�평���l�ݒ�v�����C��
 */
/*==========================================================================*/
static ER power_on_process( void )
{
	ER ercd;
	int Retry;
	
	/** PC��WakeUp�m�F���M	**/
	MdCngSubMode( SUB_MD_WAKEUP );			// �T�u���[�h���AWakeUp�₢���킹�֐ݒ�
	
	while ( 1 ){							// WakeUp�₢���킹��PC����̉�������������܂ŁA�P�b�����ŌJ��Ԃ��B
	
		ercd = send_WakeUp();				// WakeUp��₢���킹�B
		if ( ercd == E_OK ){				// Ack��������
			nop();
			break;

		} else if ( ercd == CODE_NACK ){	// Nak��������
			dly_tsk( 1000/MSEC );
			continue;
							
		} else if ( ercd == E_TMOUT ){		// �P�b�^�C���A�E�g
			nop();
			continue;
			
		} else if ( ercd == E_OBJ ){		// Ack/Nak�ȊO����M
			nop();
			continue;
			
		} else {
			PrgErrSet();
			slp_tsk();		//�@�����֗��鎞�͎����G���[
		}
	}
	
	
	/** �J�����E�p�����[�^�̏����l�v���̑��M	**/
	while ( MdGetSubMode() != SUB_MD_IDLE ){	// WakeUp�₢���킹��PC����̉���������҂B
		dly_tsk( 25/MSEC );	
	}
	
	MdCngSubMode( SUB_MD_CAMERA_PARAM_REQ );	// �T�u���[�h���A�J�����p�����[�^�̏����l�v�����֐ݒ�
	
	Retry = 0;
	while ( Retry <= 3 ){
		ercd = send_camera_param_req();		// �J�����E�p�����[�^�̏����l�v���̑��M
		if ( ercd == E_OK ){				// Ack��������
			nop();
			break;
		} else if ( ercd == CODE_NACK ){	// Nak��������
			dly_tsk( 1000/MSEC );
			continue;
							
		} else if ( ercd == E_TMOUT ){		// �P�b�^�C���A�E�g
			nop();
			
		} else if ( ercd == E_OBJ ){		// Ack/Nak�ȊO����M
			nop();
			continue;
			
		} else {
			PrgErrSet();
			slp_tsk();		//�@�����֗��鎞�͎����G���[
		}
		Retry++;
	}

	
	/** �摜�����̏����l�v���̑��M	**/	
	while ( MdGetSubMode() != SUB_MD_IDLE ){	// �J�����E�p�����[�^�̏����l�v����PC����̉���������҂B
		dly_tsk( 25/MSEC );	
	}
	
	MdCngSubMode( SUB_MD_CAP_PARAM_REQ );	// �T�u���[�h���A�摜�����̏����l�v�����֐ݒ�
	
	Retry = 0;
	while ( Retry <= 3 ){
		ercd = send_cap_param_req();		// �摜�����̏����l�v���̑��M
		if ( ercd == E_OK ){				// Ack��������
			nop();
			break;
		} else if ( ercd == CODE_NACK ){	// Nak��������
			dly_tsk( 1000/MSEC );
			continue;
							
		} else if ( ercd == E_TMOUT ){		// �P�b�^�C���A�E�g
			nop();
			
		} else if ( ercd == E_OBJ ){		// Ack/Nak�ȊO����M
			nop();
			continue;
			
		} else {
			PrgErrSet();
			slp_tsk();		//�@�����֗��鎞�͎����G���[
		}
		Retry++;
	}

	
	/** LED���ʐ��l�̏����l�v���̑��M	**/	
	while ( MdGetSubMode() != SUB_MD_IDLE ){	// �摜�����̏����l�v����PC����̉���������҂B
		dly_tsk( 25/MSEC );	
	}
	
	MdCngSubMode( SUB_MD_LED_PARAM_REQ );	// �T�u���[�h���ALED���ʐ��l�̏����l�v�����֐ݒ�
	
	Retry = 0;
	while ( Retry <= 3 ){
		ercd = send_Led_param_req();		// LED���ʐ��l�̏����l�v���̑��M
		if ( ercd == E_OK ){				// Ack��������
			nop();
			break;
		} else if ( ercd == CODE_NACK ){	// Nak��������
			dly_tsk( 1000/MSEC );
			continue;
							
		} else if ( ercd == E_TMOUT ){		// �P�b�^�C���A�E�g
			nop();
			
		} else if ( ercd == E_OBJ ){		// Ack/Nak�ȊO����M
			nop();
			continue;
			
		} else {
			PrgErrSet();
			slp_tsk();		//�@�����֗��鎞�͎����G���[
		}
		Retry++;
	}

	
	/** �o�^�f�[�^�̏����l�v���̑��M	**/	
	while ( MdGetSubMode() != SUB_MD_IDLE ){	// LED���ʐ��l�̏����l�v����PC����̉���������҂B
		dly_tsk( 25/MSEC );	
	}
	
	MdCngSubMode( SUB_MD_TOUROKU_PARAM_REQ );	// �T�u���[�h���A�o�^�f�[�^�̏����l�v�����֐ݒ�
	
	Retry = 0;
	while ( Retry <= 3 ){
		ercd = send_touroku_param_req();	// �o�^�f�[�^�̏����l�v���̑��M
		if ( ercd == E_OK ){				// Ack��������
			nop();
			break;
		} else if ( ercd == CODE_NACK ){	// Nak��������
			dly_tsk( 1000/MSEC );
			continue;
							
		} else if ( ercd == E_TMOUT ){		// �P�b�^�C���A�E�g
			nop();
			
		} else if ( ercd == E_OBJ ){		// Ack/Nak�ȊO����M
			nop();
			continue;
			
		} else {
			PrgErrSet();
			slp_tsk();		//�@�����֗��鎞�͎����G���[
		}
		Retry++;
	}

	while ( MdGetSubMode() != SUB_MD_IDLE ){	// �o�^�f�[�^�̏����l�v����PC����̉���������҂B
		dly_tsk( 25/MSEC );	
	}
	return ercd;
	
}

/*==========================================================================*/
/**
 *	���[�h�ؑ֒ʒm�̑��M�i�e�탂�[�h�ڍs���j
 */
/*==========================================================================*/
static ER SndCmdCngMode( UINT stat )	// PC�փ��[�h�ؑւ��ʒm�𑗐M
{
	ER ercd = E_OK;

	switch ( stat ){
		case MD_POWER_OFF:		///< �d��OFF
		
			break;
				
		case MD_INITIAL:		///< �����o�^���[�h	
			
			break;
			
		case MD_MAINTE:			///< �����e�i���X���[�h	

			ercd = send_meinte_mode_Wait_Ack_Retry();

	    	break;
			
		case MD_NORMAL:			///< �ʏ탂�[�h

			ercd = send_nomal_mode_Wait_Ack_Retry();

	    	break;
			
		case MD_POWER_FAIL:		///< ��d�����샂�[�h
			
	    	break;
			
		case MD_PANIC:			///< ��펞�J�����[�h
			
	    	break;

		default:
			break;
	}
	return ercd;
}

/*==========================================================================*/
//	���[�h�ؑ֒ʒm�̑��M�AAck�ENack�҂��ƃ��g���C�t���i�ʏ탂�[�h�ڍs���j
/*==========================================================================*/
static ER send_nomal_mode_Wait_Ack_Retry( void )
{
	UINT Retry = 0;
	ER ercd;
	
	while ( Retry <= 3 ){
		ercd = send_nomal_mode();			// ���[�h�ؑ֒ʒm�̑��M�i�ʏ탂�[�h�ڍs���j
		if ( ercd == E_OK ){				// Ack��������
			MdCngSubMode( SUB_MD_CHG_NORMAL );	//�@�T�u���[�h�E�X�e�[�^�X���A�h�m�[�}�����[�h�ֈڍs���h�ցB
			nop();
			break;
		} else if ( ercd == CODE_NACK ){	// Nak��������
			dly_tsk( 1000/MSEC );
			continue;
							
		} else if ( ercd == E_TMOUT ){		// �P�b�^�C���A�E�g
			nop();
			
		} else if ( ercd == E_OBJ ){		// Ack/Nak�ȊO����M
			nop();
			continue;
			
		} else {
			PrgErrSet();
			slp_tsk();		//�@�����֗��鎞�͎����G���[
		}
		Retry++;
	}
	return ercd;
}

/*==========================================================================*/
/**
 *	���[�h�ؑ֒ʒm�̑��M�i�ʏ탂�[�h�ڍs���j
 */
/*==========================================================================*/
static ER send_nomal_mode( void )
{
	ER ercd = E_TMOUT;
	int count;
	char com_data[ 15 ];
	
	/** �R�}���h�f�[�^�̏���	**/
	com_data[ 0 ] = 0x23;			//�@�w�b�_�@1Byte�@ASCII
	com_data[ 1 ] = 0;				//�@�f�[�^���@�Q�o�C�gBinary
	com_data[ 2 ] = 0x0d;
	com_data[ 3 ] = 0x31;			//�@���M����ʁ@1Byte�@ASCII
	com_data[ 4 ] = '2';			//�@�R�}���h�ԍ��@�R��ASCII
	com_data[ 5 ] = '0';
	com_data[ 6 ] = '0';
	com_data[ 7 ] = '0';			//�@�u���b�N�ԍ� �S��ASCII
	com_data[ 8 ] = '0';
	com_data[ 9 ] = '0';
	com_data[ 10 ] = '0';
	com_data[ 11 ] = ' ';			//�@�`���f�[�^�@�Q��ASCII
	com_data[ 12 ] = ' ';
	com_data[ 13 ] = CODE_CR;		//�@�I�[�R�[�h
	com_data[ 14 ] = CODE_LF;
	
	/** �R�}���h���M	**/
	SendBinaryData( &com_data, ( 13 + 2 ) );
	
	ercd = Wait_Ack_forBlock();		// �R�}���h�܂��̓u���b�N�f�[�^���M���Ack�����҂�
	count = 0;
	while( count <= 100 ){			//�@ACK/NAK�����҂��i�ő�P�b�ԁj
		
		if ( ercd == E_OK ){
			nop();			// Ack��M
			break;
		} else if ( ercd == CODE_NACK ){
			nop();			// Nak��M	
			break;
		} else if ( ercd == E_TMOUT ){
			nop();			// �^�C���A�E�g	
			break;
		} else if ( ercd == E_OBJ ){
			nop();			// Ack/Nak�ȊO����M	
			break;
		}
		dly_tsk( 10/MSEC );
		count++;
	}
	
	rxtid = rxtid_org;						// GET_UDP�ׂ̈ɖ{���̎�M�^�X�NID�itsk_rcv�j���ăZ�b�g
	
	return ercd;
}



/*==========================================================================*/
//	�w�i�o�^�j�f�[�^�̍폜�v���̑��M�AAck�ENack�҂��ƃ��g���C�t��
/*==========================================================================*/
static ER send_touroku_delete_Wait_Ack_Retry( void )
{
	UINT Retry = 0;
	ER ercd;
	
	while ( Retry <= 3 ){
		ercd = send_touroku_delete_req();	// �w�o�^�f�[�^�폜�v���̑��M
		if ( ercd == E_OK ){				// Ack��������
			nop();
			break;
		} else if ( ercd == CODE_NACK ){	// Nak��������
			dly_tsk( 1000/MSEC );
			continue;
							
		} else if ( ercd == E_TMOUT ){		// �P�b�^�C���A�E�g
			nop();
			
		} else if ( ercd == E_OBJ ){		// Ack/Nak�ȊO����M
			nop();
			continue;
			
		} else {
			PrgErrSet();
			slp_tsk();		//�@�����֗��鎞�͎����G���[
		}
		Retry++;
	}
	return ercd;
}

/*==========================================================================*/
/**
 *	�w�i�o�^�j�f�[�^�̍폜�v���̑��M�i�ʏ탂�[�h���j
 */
/*==========================================================================*/
static ER send_touroku_delete_req( void )
{
	ER ercd = E_TMOUT;
	int count;
	char com_data[ 24 ];
	
	/** �R�}���h�f�[�^�̏���	**/
	com_data[ 0 ] = 0x23;			//�@�w�b�_�@1Byte�@ASCII
	com_data[ 1 ] = 0;				//�@�f�[�^���@�Q�o�C�gBinary
	com_data[ 2 ] = 0x15;
	com_data[ 3 ] = 0x31;			//�@���M����ʁ@1Byte�@ASCII
	com_data[ 4 ] = '2';			//�@�R�}���h�ԍ��@�R��ASCII
	com_data[ 5 ] = '0';
	com_data[ 6 ] = '9';
	com_data[ 7 ] = '0';			//�@�u���b�N�ԍ� �S��ASCII
	com_data[ 8 ] = '0';
	com_data[ 9 ] = '0';
	com_data[ 10 ] = '0';
	
	//�@�`���f�[�^�@
	com_data[ 11 ] = yb_touroku_data.tou_no[ 0 ];	//�@���ԍ�
	com_data[ 12 ] = yb_touroku_data.tou_no[ 1 ];
	com_data[ 13 ] = ',';

	com_data[ 14 ] = yb_touroku_data.user_id[ 0 ];	//�@���[�U�[ID
	com_data[ 15 ] = yb_touroku_data.user_id[ 1 ];
	com_data[ 16 ] = yb_touroku_data.user_id[ 2 ];
	com_data[ 17 ] = yb_touroku_data.user_id[ 3 ];
	com_data[ 18 ] = ',';

	com_data[ 19 ] = yb_touroku_data.yubi_seq_no[ 0 ];	//�@�ӔC�ҁ^��ʎ҂̓o�^�w���i�w�o�^�ԍ��O�`�P�O�O�j
	com_data[ 20 ] = yb_touroku_data.yubi_seq_no[ 1 ];
	com_data[ 21 ] = yb_touroku_data.yubi_seq_no[ 2 ];

	com_data[ 22 ] = CODE_CR;		//�@�I�[�R�[�h
	com_data[ 23 ] = CODE_LF;
	
	/** �R�}���h���M	**/
	SendBinaryData( &com_data, ( 22 + 2 ) );
	
	ercd = Wait_Ack_forBlock();		// �R�}���h�܂��̓u���b�N�f�[�^���M���Ack�����҂�
	count = 0;
	while( count <= 100 ){			//�@ACK/NAK�����҂��i�ő�P�b�ԁj
		
		if ( ercd == E_OK ){
			nop();			// Ack��M
			break;
		} else if ( ercd == CODE_NACK ){
			nop();			// Nak��M	
			break;
		} else if ( ercd == E_TMOUT ){
			nop();			// �^�C���A�E�g	
			break;
		} else if ( ercd == E_OBJ ){
			nop();			// Ack/Nak�ȊO����M	
			break;
		}
		dly_tsk( 10/MSEC );
		count++;
	}
	
	rxtid = rxtid_org;						// GET_UDP�ׂ̈ɖ{���̎�M�^�X�NID�itsk_rcv�j���ăZ�b�g
	
	return ercd;
}

/*==========================================================================*/
//	�ً}�J���ԍ��ʒm���M�i�ʏ탂�[�h���j�AAck�ENack�҂��ƃ��g���C�t��
/*==========================================================================*/
static ER send_kinkyuu_touroku_Wait_Ack_Retry( void )	// PC�ցA�ً}�J���ԍ��ʒm���M�AAck�ENack�҂��ƃ��g���C�t��
{
	UINT Retry = 0;
	ER ercd;
	
	while ( Retry <= 3 ){
		ercd = send_kinkyuu_touroku_req();	// �ً}�J���ԍ��ʒm���M
		if ( ercd == E_OK ){				// Ack��������
			nop();
			MdCngSubMode( SUB_MD_KINKYU_TOUROKU );	//�@�T�u���[�h�E�X�e�[�^�X���A�h�ً}�ԍ��o�^�����V�[�P���X���h�ցB
			break;
			
		} else if ( ercd == CODE_NACK ){	// Nak��������
			dly_tsk( 1000/MSEC );
			continue;
							
		} else if ( ercd == E_TMOUT ){		// �P�b�^�C���A�E�g
			nop();
			
		} else if ( ercd == E_OBJ ){		// Ack/Nak�ȊO����M
			nop();
			continue;
			
		} else {
			PrgErrSet();
			slp_tsk();		//�@�����֗��鎞�͎����G���[
		}
		Retry++;
	}
	return ercd;
}


/*==========================================================================*/
/**
 *	�ً}�J���ԍ��ʒm���M�i�ʏ탂�[�h���j
 */
/*==========================================================================*/
static ER send_kinkyuu_touroku_req( void )		// PC�ցA�ً}�J���ԍ��ʒm���M
{
	ER ercd = E_TMOUT;
	int count;
	char com_data[ 17 ];
	
	/** �R�}���h�f�[�^�̏���	**/
	com_data[ 0 ] = 0x23;			//�@�w�b�_�@1Byte�@ASCII
	com_data[ 1 ] = 0;				//�@�f�[�^���@�Q�o�C�gBinary
	com_data[ 2 ] = 15;
	com_data[ 3 ] = 0x31;			//�@���M����ʁ@1Byte�@ASCII
	com_data[ 4 ] = '2';			//�@�R�}���h�ԍ��@�R��ASCII
	com_data[ 5 ] = '7';
	com_data[ 6 ] = '0';
	com_data[ 7 ] = '0';			//�@�u���b�N�ԍ� �S��ASCII
	com_data[ 8 ] = '0';
	com_data[ 9 ] = '0';
	com_data[ 10 ] = '0';
	
	//�@�`���f�[�^�@
	com_data[ 11 ] = kinkyuu_touroku_no[ 0 ];	//�@�ً}�J���ԍ��S��
	com_data[ 12 ] = kinkyuu_touroku_no[ 1 ];
	com_data[ 13 ] = kinkyuu_touroku_no[ 2 ];
	com_data[ 14 ] = kinkyuu_touroku_no[ 3 ];
	
	com_data[ 15 ] = CODE_CR;		//�@�I�[�R�[�h
	com_data[ 16 ] = CODE_LF;
	
	/** �R�}���h���M	**/
	SendBinaryData( &com_data, ( 15 + 2 ) );
	
	ercd = Wait_Ack_forBlock();		// �R�}���h�܂��̓u���b�N�f�[�^���M���Ack�����҂�
	count = 0;
	while( count <= 100 ){			//�@ACK/NAK�����҂��i�ő�P�b�ԁj
		
		if ( ercd == E_OK ){
			nop();			// Ack��M
			break;
		} else if ( ercd == CODE_NACK ){
			nop();			// Nak��M	
			break;
		} else if ( ercd == E_TMOUT ){
			nop();			// �^�C���A�E�g	
			break;
		} else if ( ercd == E_OBJ ){
			nop();			// Ack/Nak�ȊO����M	
			break;
		}
		dly_tsk( 10/MSEC );
		count++;
	}
	
	rxtid = rxtid_org;						// GET_UDP�ׂ̈ɖ{���̎�M�^�X�NID�itsk_rcv�j���ăZ�b�g
	
	return ercd;	
}

/*==========================================================================*/
//	�ً}�W���ԍ��f�[�^�v�����M�i�ʏ탂�[�h���j�AAck�ENack�҂��ƃ��g���C�t��
/*==========================================================================*/
static ER send_kinkyuu_8keta_Wait_Ack_Retry( void )	// PC�ցA�ً}�W���ԍ��f�[�^�v�����M�AAck�ENack�҂��ƃ��g���C�t��
{
	UINT Retry = 0;
	ER ercd;
	
	while ( Retry <= 3 ){
		ercd = send_kinkyuu_8keta_req();	// �ً}�W���ԍ��f�[�^�v��
		if ( ercd == E_OK ){				// Ack��������
			nop();
			MdCngSubMode( SUB_MD_KINKYU_8KETA_REQ );	//�@�T�u���[�h�E�X�e�[�^�X���A�h�ً}�W���ԍ��v���ʐM���h�ցB
			break;
			
		} else if ( ercd == CODE_NACK ){	// Nak��������
			dly_tsk( 1000/MSEC );
			continue;
							
		} else if ( ercd == E_TMOUT ){		// �P�b�^�C���A�E�g
			nop();
			
		} else if ( ercd == E_OBJ ){		// Ack/Nak�ȊO����M
			nop();
			continue;
			
		} else {
			PrgErrSet();
			slp_tsk();		//�@�����֗��鎞�͎����G���[
		}
		Retry++;
	}
	return ercd;
}


/*==========================================================================*/
/**
 *	�ً}�W���ԍ��f�[�^�v�����M�i�ʏ탂�[�h���j
 */
/*==========================================================================*/
static ER send_kinkyuu_8keta_req( void )			// PC�ցA�ً}�W���ԍ��f�[�^�v�����M�B
{
	ER ercd = E_TMOUT;
	int count;
	char com_data[ 22 ];
	
	/** �R�}���h�f�[�^�̏���	**/
	com_data[ 0 ] = 0x23;			//�@�w�b�_�@1Byte�@ASCII
	com_data[ 1 ] = 0;				//�@�f�[�^���@�Q�o�C�gBinary
	com_data[ 2 ] = 20;
	com_data[ 3 ] = 0x31;			//�@���M����ʁ@1Byte�@ASCII
	com_data[ 4 ] = '2';			//�@�R�}���h�ԍ��@�R��ASCII
	com_data[ 5 ] = '7';
	com_data[ 6 ] = '1';
	com_data[ 7 ] = '0';			//�@�u���b�N�ԍ� �S��ASCII
	com_data[ 8 ] = '0';
	com_data[ 9 ] = '0';
	com_data[ 10 ] = '0';
	
	//�@�`���f�[�^�@
	com_data[ 11 ] = kinkyuu_hyouji_no[ 0 ];	//�@�ً}�W���ԍ��\���f�[�^
	com_data[ 12 ] = kinkyuu_hyouji_no[ 1 ];
	com_data[ 13 ] = kinkyuu_hyouji_no[ 2 ];
	com_data[ 14 ] = kinkyuu_hyouji_no[ 3 ];
	com_data[ 15 ] = kinkyuu_hyouji_no[ 4 ];
	com_data[ 16 ] = kinkyuu_hyouji_no[ 5 ];
	com_data[ 17 ] = kinkyuu_hyouji_no[ 6 ];
	com_data[ 18 ] = kinkyuu_hyouji_no[ 7 ];
	com_data[ 19 ] = kinkyuu_hyouji_no[ 8 ];

	com_data[ 20 ] = CODE_CR;		//�@�I�[�R�[�h
	com_data[ 21 ] = CODE_LF;
	
	/** �R�}���h���M	**/
	SendBinaryData( &com_data, ( 20 + 2 ) );
	
	ercd = Wait_Ack_forBlock();		// �R�}���h�܂��̓u���b�N�f�[�^���M���Ack�����҂�
	count = 0;
	while( count <= 100 ){			//�@ACK/NAK�����҂��i�ő�P�b�ԁj
		
		if ( ercd == E_OK ){
			nop();			// Ack��M
			break;
		} else if ( ercd == CODE_NACK ){
			nop();			// Nak��M	
			break;
		} else if ( ercd == E_TMOUT ){
			nop();			// �^�C���A�E�g	
			break;
		} else if ( ercd == E_OBJ ){
			nop();			// Ack/Nak�ȊO����M	
			break;
		}
		dly_tsk( 10/MSEC );
		count++;
	}
	
	rxtid = rxtid_org;						// GET_UDP�ׂ̈ɖ{���̎�M�^�X�NID�itsk_rcv�j���ăZ�b�g
	
	return ercd;	
}


/*==========================================================================*/
//	�ً}�J���ԍ����M�i�ʏ탂�[�h���j�AAck�ENack�҂��ƃ��g���C�t��
/*==========================================================================*/
static ER send_kinkyuu_kaijyou_Wait_Ack_Retry( void )	// PC�ցA�ً}�J���ԍ����M�AAck�ENack�҂��ƃ��g���C�t��
{
	UINT Retry = 0;
	ER ercd;
	
	while ( Retry <= 3 ){
		ercd = send_kinkyuu_kaijyou_no();	// �ً}�J���ԍ����M�v�����M
		if ( ercd == E_OK ){				// Ack��������
			nop();
			MdCngSubMode( SUB_MD_KINKYU_KAIJYO_SEND );	//�@�T�u���[�h�E�X�e�[�^�X���A�h�ً}�J���ԍ��ʒm�ʐM���h�ցB
			break;
			
		} else if ( ercd == CODE_NACK ){	// Nak��������
			dly_tsk( 1000/MSEC );
			continue;
							
		} else if ( ercd == E_TMOUT ){		// �P�b�^�C���A�E�g
			nop();
			
		} else if ( ercd == E_OBJ ){		// Ack/Nak�ȊO����M
			nop();
			continue;
			
		} else {
			PrgErrSet();
			slp_tsk();		//�@�����֗��鎞�͎����G���[
		}
		Retry++;
	}
	return ercd;
	
}


/*==========================================================================*/
/**
 *	�ً}�J���ԍ����M�i�ʏ탂�[�h���j
 */
/*==========================================================================*/
static ER send_kinkyuu_kaijyou_no( void)			// PC�ցA�ً}�J���ԍ����M�v�����M�B
{
	ER ercd = E_TMOUT;
	int count;
	char com_data[ 22 ];
	
	/** �R�}���h�f�[�^�̏���	**/
	com_data[ 0 ] = 0x23;			//�@�w�b�_�@1Byte�@ASCII
	com_data[ 1 ] = 0;				//�@�f�[�^���@�Q�o�C�gBinary
	com_data[ 2 ] = 20;
	com_data[ 3 ] = 0x31;			//�@���M����ʁ@1Byte�@ASCII
	com_data[ 4 ] = '2';			//�@�R�}���h�ԍ��@�R��ASCII
	com_data[ 5 ] = '7';
	com_data[ 6 ] = '3';
	com_data[ 7 ] = '0';			//�@�u���b�N�ԍ� �S��ASCII
	com_data[ 8 ] = '0';
	com_data[ 9 ] = '0';
	com_data[ 10 ] = '0';
	
	//�@�`���f�[�^�@
	com_data[ 11 ] = kinkyuu_kaijyo_no[ 0 ];	//�@�ً}�J���ԍ��W��
	com_data[ 12 ] = kinkyuu_kaijyo_no[ 1 ];
	com_data[ 13 ] = kinkyuu_kaijyo_no[ 2 ];
	com_data[ 14 ] = kinkyuu_kaijyo_no[ 3 ];
	com_data[ 15 ] = kinkyuu_kaijyo_no[ 4 ];
	com_data[ 16 ] = kinkyuu_kaijyo_no[ 5 ];
	com_data[ 17 ] = kinkyuu_kaijyo_no[ 6 ];
	com_data[ 18 ] = kinkyuu_kaijyo_no[ 7 ];
	com_data[ 19 ] = kinkyuu_kaijyo_no[ 8 ];	
	
	com_data[ 20 ] = CODE_CR;		//�@�I�[�R�[�h
	com_data[ 21 ] = CODE_LF;
	
	/** �R�}���h���M	**/
	SendBinaryData( &com_data, ( 20 + 2 ) );
	
	ercd = Wait_Ack_forBlock();		// �R�}���h�܂��̓u���b�N�f�[�^���M���Ack�����҂�
	count = 0;
	while( count <= 100 ){			//�@ACK/NAK�����҂��i�ő�P�b�ԁj
		
		if ( ercd == E_OK ){
			nop();			// Ack��M
			break;
		} else if ( ercd == CODE_NACK ){
			nop();			// Nak��M	
			break;
		} else if ( ercd == E_TMOUT ){
			nop();			// �^�C���A�E�g	
			break;
		} else if ( ercd == E_OBJ ){
			nop();			// Ack/Nak�ȊO����M	
			break;
		}
		dly_tsk( 10/MSEC );
		count++;
	}
	
	rxtid = rxtid_org;						// GET_UDP�ׂ̈ɖ{���̎�M�^�X�NID�itsk_rcv�j���ăZ�b�g
	
	return ercd;		
}


/*==========================================================================*/
//	�ً}�ԍ��̑Ó����₢���킹�m�F�v�����M�i�ʏ탂�[�h���j�AAck�ENack�҂��ƃ��g���C�t��
/*==========================================================================*/
static ER send_kinkyuu_input_Wait_Ack_Retry( void )	// PC�ցA�ً}�ԍ��̑Ó����₢���킹�m�F�v�����M�AAck�ENack�҂��ƃ��g���C�t��
{
	UINT Retry = 0;
	ER ercd;
	
	while ( Retry <= 3 ){
		ercd = send_kinkyuu_input_no();	// �ً}�ԍ��̑Ó����₢���킹�m�F�v�����M
		if ( ercd == E_OK ){				// Ack��������
			nop();
			MdCngSubMode( SUB_MD_KINKYU_NO_CHK_REQ );	//�@�T�u���[�h�E�X�e�[�^�X���A�h�ً}�ԍ��Ó����m�F�v���ʐM���h�ցB
			break;
			
		} else if ( ercd == CODE_NACK ){	// Nak��������
			dly_tsk( 1000/MSEC );
			continue;
							
		} else if ( ercd == E_TMOUT ){		// �P�b�^�C���A�E�g
			nop();
			
		} else if ( ercd == E_OBJ ){		// Ack/Nak�ȊO����M
			nop();
			continue;
			
		} else {
			PrgErrSet();
			slp_tsk();		//�@�����֗��鎞�͎����G���[
		}
		Retry++;
	}
	return ercd;
		
}


/*==========================================================================*/
/**
 *	�ً}�ԍ��̑Ó����₢���킹�m�F�v�����M�i�ʏ탂�[�h���j
 */
/*==========================================================================*/
static ER send_kinkyuu_input_no( void )			// PC�ցA�ً}�ԍ��̑Ó����₢���킹�m�F�v�����M�B
{
	ER ercd = E_TMOUT;
	int count;
	char com_data[ 17 ];
	
	/** �R�}���h�f�[�^�̏���	**/
	com_data[ 0 ] = 0x23;			//�@�w�b�_�@1Byte�@ASCII
	com_data[ 1 ] = 0;				//�@�f�[�^���@�Q�o�C�gBinary
	com_data[ 2 ] = 15;
	com_data[ 3 ] = 0x31;			//�@���M����ʁ@1Byte�@ASCII
	com_data[ 4 ] = '2';			//�@�R�}���h�ԍ��@�R��ASCII
	com_data[ 5 ] = '7';
	com_data[ 6 ] = '4';
	com_data[ 7 ] = '0';			//�@�u���b�N�ԍ� �S��ASCII
	com_data[ 8 ] = '0';
	com_data[ 9 ] = '0';
	com_data[ 10 ] = '0';
	
	//�@�`���f�[�^�@
	com_data[ 11 ] = kinkyuu_input_no[ 0 ];	//�@���͂��ꂽ�ً}�J���ԍ��S��
	com_data[ 12 ] = kinkyuu_input_no[ 1 ];
	com_data[ 13 ] = kinkyuu_input_no[ 2 ];
	com_data[ 14 ] = kinkyuu_input_no[ 3 ];	
	
	com_data[ 15 ] = CODE_CR;		//�@�I�[�R�[�h
	com_data[ 16 ] = CODE_LF;
	
	/** �R�}���h���M	**/
	SendBinaryData( &com_data, ( 15 + 2 ) );
	
	ercd = Wait_Ack_forBlock();		// �R�}���h�܂��̓u���b�N�f�[�^���M���Ack�����҂�
	count = 0;
	while( count <= 100 ){			//�@ACK/NAK�����҂��i�ő�P�b�ԁj
		
		if ( ercd == E_OK ){
			nop();			// Ack��M
			break;
		} else if ( ercd == CODE_NACK ){
			nop();			// Nak��M	
			break;
		} else if ( ercd == E_TMOUT ){
			nop();			// �^�C���A�E�g	
			break;
		} else if ( ercd == E_OBJ ){
			nop();			// Ack/Nak�ȊO����M	
			break;
		}
		dly_tsk( 10/MSEC );
		count++;
	}
	
	rxtid = rxtid_org;						// GET_UDP�ׂ̈ɖ{���̎�M�^�X�NID�itsk_rcv�j���ăZ�b�g
	
	return ercd;			
}

/*==========================================================================*/
//	�h���O���̗L���m�F�𑗐M�AAck�ENack�҂��ƃ��g���C�t��
/*==========================================================================*/
static ER send_donguru_chk_Wait_Ack_Retry( void )
{
	UINT Retry = 0;
	ER ercd;
	
	while ( Retry <= 3 ){
		ercd = send_donguru_chk_req();	// �h���O���̗L���m�F�̑��M
		if ( ercd == E_OK ){				// Ack��������
			nop();
			MdCngSubMode( SUB_MD_DONGURU_CHK );	//�@�T�u���[�h�E�X�e�[�^�X���A�h�h���O���̗L���m�F���h�ցB
			break;
			
		} else if ( ercd == CODE_NACK ){	// Nak��������
			dly_tsk( 1000/MSEC );
			continue;
							
		} else if ( ercd == E_TMOUT ){		// �P�b�^�C���A�E�g
			nop();
			
		} else if ( ercd == E_OBJ ){		// Ack/Nak�ȊO����M
			nop();
			continue;
			
		} else {
			PrgErrSet();
			slp_tsk();		//�@�����֗��鎞�͎����G���[
		}
		Retry++;
	}
	return ercd;
}


/*==========================================================================*/
/**
 *	�h���O���̗L���m�F�̑��M�i�ʏ탂�[�h���j
 */
/*==========================================================================*/
static ER send_donguru_chk_req( void )
{
	ER ercd = E_TMOUT;
	int count;
	char com_data[ 15 ];
	
	/** �R�}���h�f�[�^�̏���	**/
	com_data[ 0 ] = 0x23;			//�@�w�b�_�@1Byte�@ASCII
	com_data[ 1 ] = 0;				//�@�f�[�^���@�Q�o�C�gBinary
	com_data[ 2 ] = 13;
	com_data[ 3 ] = 0x31;			//�@���M����ʁ@1Byte�@ASCII
	com_data[ 4 ] = '2';			//�@�R�}���h�ԍ��@�R��ASCII
	com_data[ 5 ] = '1';
	com_data[ 6 ] = '2';
	com_data[ 7 ] = '0';			//�@�u���b�N�ԍ� �S��ASCII
	com_data[ 8 ] = '0';
	com_data[ 9 ] = '0';
	com_data[ 10 ] = '0';
	
	//�@�`���f�[�^�@
	com_data[ 11 ] = ' ';			//�@�X�y�[�X�Q��
	com_data[ 12 ] = ' ';

	com_data[ 13 ] = CODE_CR;		//�@�I�[�R�[�h
	com_data[ 14 ] = CODE_LF;
	
	/** �R�}���h���M	**/
	SendBinaryData( &com_data, ( 13 + 2 ) );
	
	ercd = Wait_Ack_forBlock();		// �R�}���h�܂��̓u���b�N�f�[�^���M���Ack�����҂�
	count = 0;
	while( count <= 100 ){			//�@ACK/NAK�����҂��i�ő�P�b�ԁj
		
		if ( ercd == E_OK ){
			nop();			// Ack��M
			break;
		} else if ( ercd == CODE_NACK ){
			nop();			// Nak��M	
			break;
		} else if ( ercd == E_TMOUT ){
			nop();			// �^�C���A�E�g	
			break;
		} else if ( ercd == E_OBJ ){
			nop();			// Ack/Nak�ȊO����M	
			break;
		}
		dly_tsk( 10/MSEC );
		count++;
	}
	
	rxtid = rxtid_org;						// GET_UDP�ׂ̈ɖ{���̎�M�^�X�NID�itsk_rcv�j���ăZ�b�g
	
	return ercd;
}


/*==========================================================================*/
//	�����e�i���X�E���[�h�ڍs���̃p�X���[�h�m�F�v���̑��M�AAck�ENack�҂��ƃ��g���C�t��
/*==========================================================================*/
static ER send_password_chk_Wait_Ack_Retry( void )
{
	UINT Retry = 0;
	ER ercd;
	
	while ( Retry <= 3 ){
		ercd = send_password_chk_req();		// �����e�i���X�E���[�h�ڍs���̃p�X���[�h�m�F�v���̑��M
		if ( ercd == E_OK ){				// Ack��������
			nop();
			MdCngSubMode( SUB_MD_PASSWORD_CHK );	//�@�T�u���[�h�E�X�e�[�^�X���A�h�p�X���[�h�m�F���h�ցB			
			break;
			
		} else if ( ercd == CODE_NACK ){	// Nak��������
			dly_tsk( 1000/MSEC );
			continue;
							
		} else if ( ercd == E_TMOUT ){		// �P�b�^�C���A�E�g
			nop();
			
		} else if ( ercd == E_OBJ ){		// Ack/Nak�ȊO����M
			nop();
			continue;
			
		} else {
			PrgErrSet();
			slp_tsk();		//�@�����֗��鎞�͎����G���[
		}
		Retry++;
	}
	return ercd;
}


/*==========================================================================*/
/**
 *	�����e�i���X�E���[�h�ڍs���̃p�X���[�h�m�F�v���̑��M�i�ʏ탂�[�h���j
 */
/*==========================================================================*/
static ER send_password_chk_req( void )
{
	ER ercd = E_TMOUT;
	int count;
	char com_data[ 17 ];
	
	/** �R�}���h�f�[�^�̏���	**/
	com_data[ 0 ] = 0x23;			//�@�w�b�_�@1Byte�@ASCII
	com_data[ 1 ] = 0;				//�@�f�[�^���@�Q�o�C�gBinary
	com_data[ 2 ] = 15;
	com_data[ 3 ] = 0x31;			//�@���M����ʁ@1Byte�@ASCII
	com_data[ 4 ] = '2';			//�@�R�}���h�ԍ��@�R��ASCII
	com_data[ 5 ] = '1';
	com_data[ 6 ] = '3';
	com_data[ 7 ] = '0';			//�@�u���b�N�ԍ� �S��ASCII
	com_data[ 8 ] = '0';
	com_data[ 9 ] = '0';
	com_data[ 10 ] = '0';
	
	//�@�`���f�[�^�@
	com_data[ 11 ] = mainte_password[ 0 ];	//�@�p�X���[�h�S��
	com_data[ 12 ] = mainte_password[ 1 ];
	com_data[ 13 ] = mainte_password[ 2 ];
	com_data[ 14 ] = mainte_password[ 3 ];
	
	com_data[ 15 ] = CODE_CR;		//�@�I�[�R�[�h
	com_data[ 16 ] = CODE_LF;
	
	/** �R�}���h���M	**/
	SendBinaryData( &com_data, ( 15 + 2 ) );
	
	ercd = Wait_Ack_forBlock();		// �R�}���h�܂��̓u���b�N�f�[�^���M���Ack�����҂�
	count = 0;
	while( count <= 100 ){			//�@ACK/NAK�����҂��i�ő�P�b�ԁj
		
		if ( ercd == E_OK ){
			nop();			// Ack��M
			break;
		} else if ( ercd == CODE_NACK ){
			nop();			// Nak��M	
			break;
		} else if ( ercd == E_TMOUT ){
			nop();			// �^�C���A�E�g	
			break;
		} else if ( ercd == E_OBJ ){
			nop();			// Ack/Nak�ȊO����M	
			break;
		}
		dly_tsk( 10/MSEC );
		count++;
	}
	
	rxtid = rxtid_org;						// GET_UDP�ׂ̈ɖ{���̎�M�^�X�NID�itsk_rcv�j���ăZ�b�g
	
	return ercd;
}


/*==========================================================================*/
//	���[�h�ؑ֒ʒm�̑��M�AAck�ENack�҂��ƃ��g���C�t���i�����e�i���X�E���[�h�ڍs���j
/*==========================================================================*/
static ER send_meinte_mode_Wait_Ack_Retry( void )
{
	UINT Retry = 0;
	ER ercd;
	
	while ( Retry <= 3 ){
		ercd = send_meinte_mode();			// ���[�h�ؑ֒ʒm�̑��M�i�ʏ탂�[�h�ڍs���j
		if ( ercd == E_OK ){				// Ack��������
			MdCngSubMode( SUB_MD_CHG_MAINTE );	//�@�T�u���[�h�E�X�e�[�^�X���A�h�����e�i���X���[�h�ֈڍs���h�ցB
			nop();
			break;
		} else if ( ercd == CODE_NACK ){	// Nak��������
			dly_tsk( 1000/MSEC );
			continue;
							
		} else if ( ercd == E_TMOUT ){		// �P�b�^�C���A�E�g
			nop();
			
		} else if ( ercd == E_OBJ ){		// Ack/Nak�ȊO����M
			nop();
			continue;
			
		} else {
			PrgErrSet();
			slp_tsk();		//�@�����֗��鎞�͎����G���[
		}
		Retry++;
	}
	return ercd;
}


/*==========================================================================*/
/**
 *	���[�h�ؑ֒ʒm�̑��M�i�����e�i���X�E���[�h�ڍs���j
 */
/*==========================================================================*/
static ER send_meinte_mode( void )
{
	ER ercd = E_TMOUT;
	int count;
	char com_data[ 15 ];
	
	/** �R�}���h�f�[�^�̏���	**/
	com_data[ 0 ] = 0x23;			//�@�w�b�_�@1Byte�@ASCII
	com_data[ 1 ] = 0;				//�@�f�[�^���@�Q�o�C�gBinary
	com_data[ 2 ] = 0x0d;
	com_data[ 3 ] = 0x31;			//�@���M����ʁ@1Byte�@ASCII
	com_data[ 4 ] = '1';			//�@�R�}���h�ԍ��@�R��ASCII
	com_data[ 5 ] = '0';
	com_data[ 6 ] = '0';
	com_data[ 7 ] = '0';			//�@�u���b�N�ԍ� �S��ASCII
	com_data[ 8 ] = '0';
	com_data[ 9 ] = '0';
	com_data[ 10 ] = '0';
	com_data[ 11 ] = ' ';			//�@�`���f�[�^�@�Q��ASCII
	com_data[ 12 ] = ' ';
	com_data[ 13 ] = CODE_CR;		//�@�I�[�R�[�h
	com_data[ 14 ] = CODE_LF;
	
	/** �R�}���h���M	**/
	SendBinaryData( &com_data, ( 13 + 2 ) );
	
	ercd = Wait_Ack_forBlock();		// �R�}���h�܂��̓u���b�N�f�[�^���M���Ack�����҂�
	count = 0;
	while( count <= 100 ){			//�@ACK/NAK�����҂��i�ő�P�b�ԁj
		
		if ( ercd == E_OK ){
			nop();			// Ack��M
			break;
		} else if ( ercd == CODE_NACK ){
			nop();			// Nak��M	
			break;
		} else if ( ercd == E_TMOUT ){
			nop();			// �^�C���A�E�g	
			break;
		} else if ( ercd == E_OBJ ){
			nop();			// Ack/Nak�ȊO����M	
			break;
		}
		dly_tsk( 10/MSEC );
		count++;
	}
	
	rxtid = rxtid_org;						// GET_UDP�ׂ̈ɖ{���̎�M�^�X�NID�itsk_rcv�j���ăZ�b�g
	
	return ercd;
}


/*==========================================================================*/
//�@�o�^�f�[�^�������v���̑��M�AAck�ENack�҂��ƃ��g���C�t���i�����e�i���X�E���[�h���j
/*==========================================================================*/
static ER send_touroku_init_Wait_Ack_Retry( void )
{
	UINT Retry = 0;
	ER ercd;
	
	while ( Retry <= 3 ){
		ercd = send_touroku_init_req();		// �o�^�f�[�^�������v���̑��M�i�����e�i���X�E���[�h���j
		if ( ercd == E_OK ){				// Ack��������
			nop();			
			break;
			
		} else if ( ercd == CODE_NACK ){	// Nak��������
			dly_tsk( 1000/MSEC );
			continue;
							
		} else if ( ercd == E_TMOUT ){		// �P�b�^�C���A�E�g
			nop();
			
		} else if ( ercd == E_OBJ ){		// Ack/Nak�ȊO����M
			nop();
			continue;
			
		} else {
			PrgErrSet();
			slp_tsk();		//�@�����֗��鎞�͎����G���[
		}
		Retry++;
	}
	return ercd;
}



/*==========================================================================*/
/**
 *	�o�^�f�[�^�������v���̑��M�i�����e�i���X�E���[�h���j
 */
/*==========================================================================*/
static ER send_touroku_init_req( void )
{
	ER ercd = E_TMOUT;
	int count;
	char com_data[ 15 ];
	
	/** �R�}���h�f�[�^�̏���	**/
	com_data[ 0 ] = 0x23;			//�@�w�b�_�@1Byte�@ASCII
	com_data[ 1 ] = 0;				//�@�f�[�^���@�Q�o�C�gBinary
	com_data[ 2 ] = 0x0d;
	com_data[ 3 ] = 0x31;			//�@���M����ʁ@1Byte�@ASCII
	com_data[ 4 ] = '1';			//�@�R�}���h�ԍ��@�R��ASCII
	com_data[ 5 ] = '1';
	com_data[ 6 ] = '1';
	com_data[ 7 ] = '0';			//�@�u���b�N�ԍ� �S��ASCII
	com_data[ 8 ] = '0';
	com_data[ 9 ] = '0';
	com_data[ 10 ] = '0';
	com_data[ 11 ] = ' ';			//�@�`���f�[�^�@�Q��ASCII
	com_data[ 12 ] = ' ';
	com_data[ 13 ] = CODE_CR;		//�@�I�[�R�[�h
	com_data[ 14 ] = CODE_LF;
	
	/** �R�}���h���M	**/
	SendBinaryData( &com_data, ( 13 + 2 ) );
	
	ercd = Wait_Ack_forBlock();		// �R�}���h�܂��̓u���b�N�f�[�^���M���Ack�����҂�
	count = 0;
	while( count <= 100 ){			//�@ACK/NAK�����҂��i�ő�P�b�ԁj
		
		if ( ercd == E_OK ){
			nop();			// Ack��M
			break;
		} else if ( ercd == CODE_NACK ){
			nop();			// Nak��M	
			break;
		} else if ( ercd == E_TMOUT ){
			nop();			// �^�C���A�E�g	
			break;
		} else if ( ercd == E_OBJ ){
			nop();			// Ack/Nak�ȊO����M	
			break;
		}
		dly_tsk( 10/MSEC );
		count++;
	}
	
	rxtid = rxtid_org;						// GET_UDP�ׂ̈ɖ{���̎�M�^�X�NID�itsk_rcv�j���ăZ�b�g
	
	return ercd;
}

/*==========================================================================*/
/**
 *	PC��UDP��ʂ���WakeUp�̖₢���킹���s���i�d��ON���j
 */
/*==========================================================================*/
static ER send_WakeUp( void )
{
	ER ercd = E_TMOUT;
	int count;
	char com_data[ 15 ];
	
	/** �R�}���h�f�[�^�̏���	**/
	com_data[ 0 ] = 0x23;			//�@�w�b�_�@1Byte�@ASCII
	com_data[ 1 ] = 0;				//�@�f�[�^���@�Q�o�C�gBinary
	com_data[ 2 ] = 0x0d;
	com_data[ 3 ] = 0x31;			//�@���M����ʁ@1Byte�@ASCII
	com_data[ 4 ] = '0';			//�@�R�}���h�ԍ��@�R��ASCII
	com_data[ 5 ] = '2';
	com_data[ 6 ] = '1';
	com_data[ 7 ] = '0';			//�@�u���b�N�ԍ� �S��ASCII
	com_data[ 8 ] = '0';
	com_data[ 9 ] = '0';
	com_data[ 10 ] = '0';
	com_data[ 11 ] = ' ';			//�@�`���f�[�^�@�Q��ASCII
	com_data[ 12 ] = ' ';
	com_data[ 13 ] = CODE_CR;		//�@�I�[�R�[�h
	com_data[ 14 ] = CODE_LF;
	
	/** �R�}���h���M	**/
	SendBinaryData( &com_data, ( 13 + 2 ) );
	
	ercd = Wait_Ack_forBlock();		// �R�}���h�܂��̓u���b�N�f�[�^���M���Ack�����҂�
	count = 0;
	while( count <= 100 ){			//�@ACK/NAK�����҂��i�ő�P�b�ԁj
		
		if ( ercd == E_OK ){
			nop();			// Ack��M
			break;
		} else if ( ercd == CODE_NACK ){
			nop();			// Nak��M	
			break;
		} else if ( ercd == E_TMOUT ){
			nop();			// �^�C���A�E�g	
			break;
		} else if ( ercd == E_OBJ ){
			nop();			// Ack/Nak�ȊO����M	
			break;
		}
		dly_tsk( 10/MSEC );
		count++;
	}
	
	rxtid = rxtid_org;						// GET_UDP�ׂ̈ɖ{���̎�M�^�X�NID�itsk_rcv�j���ăZ�b�g
	
	return ercd;
}



/*==========================================================================*/
/**
 *	�J�����E�p�����[�^�̏����l�v���̑��M�i�d��ON���j
 */
/*==========================================================================*/
static ER send_camera_param_req( void )
{
	ER ercd = E_TMOUT;
	int count;
	char com_data[ 15 ];
	
	/** �R�}���h�f�[�^�̏���	**/
	com_data[ 0 ] = 0x23;			//�@�w�b�_�@1Byte�@ASCII
	com_data[ 1 ] = 0;				//�@�f�[�^���@�Q�o�C�gBinary
	com_data[ 2 ] = 0x0d;
	com_data[ 3 ] = 0x31;			//�@���M����ʁ@1Byte�@ASCII
	com_data[ 4 ] = '0';			//�@�R�}���h�ԍ��@�R��ASCII
	com_data[ 5 ] = '1';
	com_data[ 6 ] = '6';
	com_data[ 7 ] = '0';			//�@�u���b�N�ԍ� �S��ASCII
	com_data[ 8 ] = '0';
	com_data[ 9 ] = '0';
	com_data[ 10 ] = '0';
	com_data[ 11 ] = ' ';			//�@�`���f�[�^�@�Q��ASCII
	com_data[ 12 ] = ' ';
	com_data[ 13 ] = CODE_CR;		//�@�I�[�R�[�h
	com_data[ 14 ] = CODE_LF;
	
	/** �R�}���h���M	**/
	SendBinaryData( &com_data, ( 13 + 2 ) );
	
	ercd = Wait_Ack_forBlock();		// �R�}���h�܂��̓u���b�N�f�[�^���M���Ack�����҂�
	count = 0;
	while( count <= 100 ){			//�@ACK/NAK�����҂��i�ő�P�b�ԁj
		
		if ( ercd == E_OK ){
			nop();			// Ack��M
			break;
		} else if ( ercd == CODE_NACK ){
			nop();			// Nak��M	
			break;
		} else if ( ercd == E_TMOUT ){
			nop();			// �^�C���A�E�g	
			break;
		} else if ( ercd == E_OBJ ){
			nop();			// Ack/Nak�ȊO����M	
			break;
		}
		dly_tsk( 10/MSEC );
		count++;
	}
	
	rxtid = rxtid_org;						// GET_UDP�ׂ̈ɖ{���̎�M�^�X�NID�itsk_rcv�j���ăZ�b�g
	
	return ercd;
}


/*==========================================================================*/
/**
 *	�摜�����̏����l�v���̑��M�i�d��ON���j
 */
/*==========================================================================*/
static ER send_cap_param_req( void )
{
	ER ercd = E_TMOUT;
	int count;
	char com_data[ 15 ];
	
	/** �R�}���h�f�[�^�̏���	**/
	com_data[ 0 ] = 0x23;			//�@�w�b�_�@1Byte�@ASCII
	com_data[ 1 ] = 0;				//�@�f�[�^���@�Q�o�C�gBinary
	com_data[ 2 ] = 0x0d;
	com_data[ 3 ] = 0x31;			//�@���M����ʁ@1Byte�@ASCII
	com_data[ 4 ] = '0';			//�@�R�}���h�ԍ��@�R��ASCII
	com_data[ 5 ] = '1';
	com_data[ 6 ] = '8';
	com_data[ 7 ] = '0';			//�@�u���b�N�ԍ� �S��ASCII
	com_data[ 8 ] = '0';
	com_data[ 9 ] = '0';
	com_data[ 10 ] = '0';
	com_data[ 11 ] = ' ';			//�@�`���f�[�^�@�Q��ASCII
	com_data[ 12 ] = ' ';
	com_data[ 13 ] = CODE_CR;		//�@�I�[�R�[�h
	com_data[ 14 ] = CODE_LF;
	
	/** �R�}���h���M	**/
	SendBinaryData( &com_data, ( 13 + 2 ) );
	
	ercd = Wait_Ack_forBlock();		// �R�}���h�܂��̓u���b�N�f�[�^���M���Ack�����҂�
	count = 0;
	while( count <= 100 ){			//�@ACK/NAK�����҂��i�ő�P�b�ԁj
		
		if ( ercd == E_OK ){
			nop();			// Ack��M
			break;
		} else if ( ercd == CODE_NACK ){
			nop();			// Nak��M	
			break;
		} else if ( ercd == E_TMOUT ){
			nop();			// �^�C���A�E�g	
			break;
		} else if ( ercd == E_OBJ ){
			nop();			// Ack/Nak�ȊO����M	
			break;
		}
		dly_tsk( 10/MSEC );
		count++;
	}
	
	rxtid = rxtid_org;						// GET_UDP�ׂ̈ɖ{���̎�M�^�X�NID�itsk_rcv�j���ăZ�b�g
	
	return ercd;
}


/*==========================================================================*/
/**
 *	LED���ʐ��l�̏����l�v���̑��M�i�d��ON���j
 */
/*==========================================================================*/
static ER send_Led_param_req( void )
{
	ER ercd = E_TMOUT;
	int count;
	char com_data[ 15 ];
	
	/** �R�}���h�f�[�^�̏���	**/
	com_data[ 0 ] = 0x23;			//�@�w�b�_�@1Byte�@ASCII
	com_data[ 1 ] = 0;				//�@�f�[�^���@�Q�o�C�gBinary
	com_data[ 2 ] = 0x0d;
	com_data[ 3 ] = 0x31;			//�@���M����ʁ@1Byte�@ASCII
	com_data[ 4 ] = '0';			//�@�R�}���h�ԍ��@�R��ASCII
	com_data[ 5 ] = '1';
	com_data[ 6 ] = '7';
	com_data[ 7 ] = '0';			//�@�u���b�N�ԍ� �S��ASCII
	com_data[ 8 ] = '0';
	com_data[ 9 ] = '0';
	com_data[ 10 ] = '0';
	com_data[ 11 ] = ' ';			//�@�`���f�[�^�@�Q��ASCII
	com_data[ 12 ] = ' ';
	com_data[ 13 ] = CODE_CR;		//�@�I�[�R�[�h
	com_data[ 14 ] = CODE_LF;
	
	/** �R�}���h���M	**/
	SendBinaryData( &com_data, ( 13 + 2 ) );
	
	ercd = Wait_Ack_forBlock();		// �R�}���h�܂��̓u���b�N�f�[�^���M���Ack�����҂�
	count = 0;
	while( count <= 100 ){			//�@ACK/NAK�����҂��i�ő�P�b�ԁj
		
		if ( ercd == E_OK ){
			nop();			// Ack��M
			break;
		} else if ( ercd == CODE_NACK ){
			nop();			// Nak��M	
			break;
		} else if ( ercd == E_TMOUT ){
			nop();			// �^�C���A�E�g	
			break;
		} else if ( ercd == E_OBJ ){
			nop();			// Ack/Nak�ȊO����M	
			break;
		}
		dly_tsk( 10/MSEC );
		count++;
	}
	
	rxtid = rxtid_org;						// GET_UDP�ׂ̈ɖ{���̎�M�^�X�NID�itsk_rcv�j���ăZ�b�g
	
	return ercd;
}


/*==========================================================================*/
/**
 *	�o�^�f�[�^�̏����l�v���̑��M�i�d��ON���j
 */
/*==========================================================================*/
static ER send_touroku_param_req( void )
{
	ER ercd = E_TMOUT;
	int count;
	char com_data[ 15 ];
	
	/** �R�}���h�f�[�^�̏���	**/
	com_data[ 0 ] = 0x23;			//�@�w�b�_�@1Byte�@ASCII
	com_data[ 1 ] = 0;				//�@�f�[�^���@�Q�o�C�gBinary
	com_data[ 2 ] = 0x0d;
	com_data[ 3 ] = 0x31;			//�@���M����ʁ@1Byte�@ASCII
	com_data[ 4 ] = '0';			//�@�R�}���h�ԍ��@�R��ASCII
	com_data[ 5 ] = '1';
	com_data[ 6 ] = '9';
	com_data[ 7 ] = '0';			//�@�u���b�N�ԍ� �S��ASCII
	com_data[ 8 ] = '0';
	com_data[ 9 ] = '0';
	com_data[ 10 ] = '0';
	com_data[ 11 ] = ' ';			//�@�`���f�[�^�@�Q��ASCII
	com_data[ 12 ] = ' ';
	com_data[ 13 ] = CODE_CR;		//�@�I�[�R�[�h
	com_data[ 14 ] = CODE_LF;
	
	/** �R�}���h���M	**/
	SendBinaryData( &com_data, ( 13 + 2 ) );
	
	ercd = Wait_Ack_forBlock();		// �R�}���h�܂��̓u���b�N�f�[�^���M���Ack�����҂�
	count = 0;
	while( count <= 100 ){			//�@ACK/NAK�����҂��i�ő�P�b�ԁj
		
		if ( ercd == E_OK ){
			nop();			// Ack��M
			break;
		} else if ( ercd == CODE_NACK ){
			nop();			// Nak��M	
			break;
		} else if ( ercd == E_TMOUT ){
			nop();			// �^�C���A�E�g	
			break;
		} else if ( ercd == E_OBJ ){
			nop();			// Ack/Nak�ȊO����M	
			break;
		}
		dly_tsk( 10/MSEC );
		count++;
	}
	
	rxtid = rxtid_org;						// GET_UDP�ׂ̈ɖ{���̎�M�^�X�NID�itsk_rcv�j���ăZ�b�g
	
	return ercd;
}


/*==========================================================================*/
/**
 *	VA300 �V�X�e������p�����[�^��Initial
 */
/*==========================================================================*/
void SystemParamInit( void )
{
	ercdStat = 0;					// �G���[�R�[�h�L��
	sys_ScreenNo = FPTN_LCD_INIT;	// ���݂̃X�N���[��No
	sys_Mode = MD_POWER_ON;			// �ŐV�̑��u���[�h
	sys_SubMode = SUB_MD_IDLE;		// ��ԕϐ��i�T�u�j
	rcv_ack_nak = 0;				// UDP�ʐM�@ACK/NAK��M�t���O
	
	s_CapResult	= CAP_JUDGE_IDLE;				// �w�F�؂̌���
	s_DongleResult = DONGURU_JUDGE_IDLE;		// �h���O���̗L���m�F�̌���
	s_PasswordResult = PASSWORD_JUDGE_IDLE;		// �p�X���[�h�m�F�̌���
	
//	SUB_MD_SETTING		// �ŐV�̑��u�T�u���[�h
	
}	

/*==========================================================================*/
/**
 *	�ŐV�̉�ʑJ�ڏ��(�ԍ�)��Ԃ�
 */
/*==========================================================================*/
UB GetScreenNo(void)
{
	return sys_ScreenNo;
}

/*==========================================================================*/
/**
 *	��ʑJ�ڏ��(�ԍ�)���X�V����
 */
/*==========================================================================*/
void ChgScreenNo( UB NewScreenNo )
{
	if ( ( NewScreenNo < LCD_SCREEN_MAX ) 
	  && ( NewScreenNo >= LCD_INIT ) ){
		  
		sys_ScreenNo = NewScreenNo;
	}
}


/*==========================================================================*/
/**
 * 	Gain. FixShutter,IR_LED�̃p�����[�^���AHost����̏����v���l�ɖ߂��B
 * �iCAP��Retry���ɕς�����\��������ׁB�j
 */
/*==========================================================================*/
void reload_CAP_Param( void )
{
	UB ercd;
	
	cmrGain = ini_cmrGain;						// �J�����E�Q�C���l�����l�@
	cmrFixShutter1 = ini_cmrFixShutter1;		// Fix Shutter Control�l�����l(�P���)�@
	cmrFixShutter2 = ini_cmrFixShutter2;		// Fix Shutter Control�l�����l(�Q���)			����
	cmrFixShutter3 = ini_cmrFixShutter3;		// Fix Shutter Control�l�����l(�R��ځj

//	ercd = set_flg( ID_FLG_CAMERA, FPTN_SETREQ_GAIN );	// �J�����^�X�N�ɁA���ڂ̃Q�C���ݒ�l��ݒ���˗��iFPGA���o�R���Ȃ��j	
//	if ( ercd != E_OK ){
//		ErrCodeSet( ercd );
//	}
		
	irDuty2 = ini_irDuty2;		// IR Duty�lirDuty2�@�����l;
	irDuty3 = ini_irDuty3;
	irDuty4 = ini_irDuty4;
	irDuty5 = ini_irDuty5;
	
	ercd = set_flg( ID_FLG_CAMERA, FPTN_SETREQ_SHUT1 );	// �J�����^�X�N�ɁA���ڂ̘I�o�R�ݒ�l��ݒ���˗��iFPGA���o�R���Ȃ��j	
	if ( ercd != E_OK ){
		ErrCodeSet( ercd );
	}
	
}

/*==========================================================================*/
static BOOL read_ethernet_addr(void)
{
	static const UB no_mac_addr[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

	if( lan_get_mac( ethernet_addr ) != E_OK ||
		!memcmp( ethernet_addr, no_mac_addr, sizeof ethernet_addr)) {
		ethernet_addr[0] = ini_mac_addr[0];	/* �b��I�ɑ��l!! */
		ethernet_addr[1] = ini_mac_addr[1];
		ethernet_addr[2] = ini_mac_addr[2];
		ethernet_addr[3] = ini_mac_addr[3];
		ethernet_addr[4] = ini_mac_addr[4];
		ethernet_addr[5] = ini_mac_addr[5];
	}
	return TRUE;
}


/**
 * Initialize peripherals on evaluation board�i�� Please customize !!�j
 *
 */
void init_peripheral(void)
{
	
}

/**
 * Initialize port�i�� Please customize !!�j
 *
 */

void ini_pio(void)
{
	sfr_outw(BSC_GPIOIC,0x0000);
	sfr_outw(BSC_PDTRB, 0x0000);		// 
	sfr_outl(BSC_PCTRB, 0x00000014);	// PORT17,18(TEST Pin1,2)�o��
	
	TP_CLR(1);							// �e�X�g�s��1������
	TP_CLR(2);							// �e�X�g�s��2������
	
}

/**
 * Initialize interrupt�i�� Please customize !!�j
 *
 */

static void ini_intr(void)
{
	FpgaInit();							// FPGA�֘A������

	sfr_outw(INTC_IPRD, 0x0000);
	sfr_setw(INTC_ICR,  0x0000);
}

/*****************************************************************************
* TELNET �̃p�X���[�h�`�F�b�N
*
******************************************************************************/

static B *telnet_passwd_check(T_TELNETD *t, const B *name, const B *passwd)
{
	if( !strcmp(name, LOGIN_ID) && !strcmp(passwd, LOGIN_PASS))
	    return (B *)">";
	else 
		return (B *)NULL;
}

/*****************************************************************************
* TELNET �̃R�}���h����
*
******************************************************************************/

BOOL telnetd_callback(T_TERMINAL *t, B *s)
{
    return GetCmd( s );
}

/**
 * TELNET���M
 */
 
void telnetd_send(B *s)
{
	terminal_print((T_TERMINAL *)&telnetd, s);
}

/**
 * TELNET�o�C�i�����M
 */
 
void telnetd_send_bin(B *s, INT len)
{
	terminal_sendbin((T_TERMINAL *)&telnetd, s, len);
}


/**
 * IP Address �̓���
 *
 * @retval TRUE �Ǐo������
 * @retval FALSE �Ǐo�����s
 */

static BOOL read_ip_addr(void)
{
	BOOL flg;
	ER ercd;
	static const UB no_ip_addr[] = { 0xFF, 0xFF, 0xFF, 0xFF };
	static const UB no_subnet_mask[] = { 0xFF, 0xFF, 0xFF, 0xFF };
	static const UH no_portno = 0xFFFFFFFF;
	//	UH serial_no;
	
	flg = TRUE;
	
	default_gateway[0] = 0;
	default_gateway[1] = 0;
	default_gateway[2] = 0;
	default_gateway[3] = 0;
	telnet_portno = default_telnet_port;
	
//	if( lan_get_ip( default_ipaddr) != E_OK) flg = FALSE;
	ercd = lan_get_ip( default_ipaddr );
	if ( ( ercd != E_OK ) || ( !memcmp( default_ipaddr, no_ip_addr, sizeof default_ipaddr ) ) ){
		flg = FALSE;
	}

//	if( lan_get_mask( subnet_mask) != E_OK)  flg = FALSE;
	ercd = lan_get_mask( subnet_mask );
	if ( ( ercd != E_OK ) || ( !memcmp( subnet_mask, no_subnet_mask, sizeof subnet_mask ) ) ){
		flg = FALSE;
	}
	
//	if( lan_get_port( &udp_portno) != E_OK) flg = FALSE;
	ercd = lan_get_port( &udp_portno );
	if ( ( ercd != E_OK ) || ( udp_portno == no_portno ) ){
		flg = FALSE;
	}
	
	if (flg == FALSE) {
		default_ipaddr[0] = ini_ipaddr[0];	/* �ʃt�@�C���Œ�` */
		default_ipaddr[1] = ini_ipaddr[1];
		default_ipaddr[2] = ini_ipaddr[2];
		default_ipaddr[3] = ini_ipaddr[3];
	}
		
	if (flg == FALSE) {
#if 0
		if (EepGetSNo( &serial_no) == E_OK) {
			if (serial_no && serial_no != 0xff) {	// �V���A���ԍ��ǂݏo�����Ƃ���IP�A�h���X�̍ŉ��ʂ�ύX����B
				default_ipaddr[3] = (UB)(serial_no & 0xff);
			}
		}
#endif

		subnet_mask[0] = ini_mask[0];		/* �b��I�ɑ��l!! */
		subnet_mask[1] = ini_mask[1];
		subnet_mask[2] = ini_mask[2];
		subnet_mask[3] = ini_mask[3];
		
		udp_portno = default_udp_port;
	}
	
	return TRUE;
}

/**
 * Receive task
 *
 *	�ύX����
 *		04/08/23	OYO���j�^�v���O�����ɕύX
 *
 */
TASK RcvTask(INT ch)
{
	char	code;
	BOOL	stx_flag;

	/* initialize */


//	ini_sio(ch, (B *)"115200 B8 PN S1");		//�����j�^�v���O�����p
	ini_sio(ch, (B *)"38400 B8 PN S1");		//�����j�^�v���O�����p
	
	ctl_sio(ch, TSIO_RXE|TSIO_TXE|TSIO_RTSON);
	memset( cSioRcvBuf, 0, RCV_BUF_SIZE);		// ��M�o�b�t�@�N���A
	usSioRcvCount = 0;							// �V���A����M�Ł[�����N���A

	for (;;)
	{
		stx_flag = FALSE;						// STX��M�t���O�N���A
		
		for(;;) {
			get_sio( ch, (UB*)&code, TMO_FEVR);	// ��M�f�[�^�҂�

			if( code == 0x08) {
				if( usSioRcvCount ) cSioRcvBuf[ --usSioRcvCount ] = 0;
					continue;
			}
			if( code == STX) {
				usSioRcvCount = 0;
				memset( cSioRcvBuf, 0, RCV_BUF_SIZE);
				cSioRcvBuf[ usSioRcvCount++ ] = code;
				stx_flag = TRUE;
				continue;
			}
			if(( code == ETX) && ( stx_flag == TRUE)) {
				cSioRcvBuf[ usSioRcvCount++ ] = 0;
				break;
			}
			if(( stx_flag == TRUE) && ( usSioRcvCount < (RCV_BUF_SIZE - 1))) {
				cSioRcvBuf[ usSioRcvCount++ ] = code;
			}
		}
	}
}

/**
 * Send Task
 *
 * @param ch �`�����l���ԍ�
 */
TASK SndTask(INT ch)
{
	T_COMMSG *msg;
	UINT i;
	UB c;
	ER	ercd;

	for (;;)
	{
		/* Wait message */

//		ercd = rcv_mbx(MBX_SND+ch, &msg);
		ercd = rcv_mbx(MBX_SND, &msg);
		
		if( ercd == E_OK) {
	        
	        /* Send 1 line */
			for (i = 0;i < msg->cnt;) {
				c = msg->buf[i++];
				put_sio(ch, c, TMO_FEVR);
			}

			/* Release memory block */
			rel_mpf(MPF_COM, msg);

			/* Wait completion */
			fls_sio(ch, TMO_FEVR);
			
    	} else {							/* �R�[�f�B���O�G���[ */
	    	ErrCodeSet( ercd );
    	}
    }
}

/**
 * main
 *
 */

int main(void)
{
 	int _mbf_size;

   /* Initialize processor�i�� Please customize !!�j*/

	init_peripheral();

	
	/* Initialize system */

    sysini();

	ini_clk();

	ini_pio();

	/* Create tasks */

	cre_tsk(TSK_MAIN,      &ctsk_main);
	cre_tsk(TSK_CMD_LAN,   &ctsk_lancmd);
	cre_tsk(TSK_SND1,      &ctsk_snd1);
	cre_tsk(TSK_COMMUNICATE,  &ctsk_urcv);//�d������
	cre_tsk(TSK_UDP_SEND,    &ctsk_usnd);//UDP���M

	cre_tsk(TSK_DISP,      &ctsk_disp);
	cre_tsk(TSK_IO,        &ctsk_io);

	/* create objects */

	cre_mpf(MPF_COM,   &cmpf_com);		/* Create fixed memory pool */
	cre_mpf(MPF_DISP,  &cmpf_disp);		/* Create fixed memory pool */
	cre_mpf(MPF_LRES,  &cmpf_lres);		/* Create fixed memory pool */

	cre_mbx(MBX_CMD_LAN, &cmbx_lancmd);	/* Create mail box */
	cre_mbx(MBX_RESSND,&cmbx_ressnd);	/* Create mail box */
	cre_mbx(MBX_SND,   &cmbx_snd);		/* Create mail box */
	cre_mbx(MBX_DISP,    &cmbx_disp);	/* Create mail box */
//	cre_mbx(MBX_MODE,    &cmbx_mode);	/* Create mail box */
	
	cre_flg(ID_FLG_IO,  &cflg_io);		/* Create flag */
	cre_flg(ID_FLG_MAIN,  &cflg_main);	/* Create flag */		
	cre_flg(ID_FLG_TS,  &cflg_ts);		/* Create flag */
	cre_flg(ID_FLG_CAMERA,  &cflg_camera);		/* Create flag */
	cre_flg(ID_FLG_LCD,  &cflg_lcd);	/* Create flag */
	
	cre_mbf(MBF_LCD_DATA, &cmbf_lcd);
//	cre_mbf(MBF_LAN_CMD, &cmbf_lan);


	//2013.05.08 Miya ���b�Z�[�WNG�̂��ߋ��p�������őΉ�����
	g_LcdmsgData.LcdMsgSize = 0;
	memset(&g_LcdmsgData.LcdMsgBuff[0], 0x20, 1024);

	//FIFO�p�ɃG���A���m�ۂ��Đݒ肷��Bbut LcdCmd_buf[]�ɃX�g�A�[����Ȃ�����
/*	
	_mbf_size = TSZ_MBF(128, 4);
	if( _mbf_size < 128 * 7 ){
		cmbf_lcd.mbfatr = TA_TFIFO;
		cmbf_lcd.maxmsz = 128;
		cmbf_lcd.mbfsz = _mbf_size;
		cmbf_lcd.mbf = &LcdCmd_buf[0];
		cre_mbf(MBF_LCD_DATA, &cmbf_lcd);
	}else{
		nop();
	}
*/


//	cre_sem(SEM_LOG,    &csem_log);		/* Create semaphore */	
	cre_sem(SEM_RTC,    &csem_rtc);		/* Create semaphore */	
	cre_sem(SEM_ERR,    &csem_err);		/* Create semaphore */	
//	cre_sem(SEM_LED,    &csem_led);		/* Create semaphore */	
//	cre_sem(SEM_7SEG,   &csem_seg);		/* Create semaphore */	
	cre_sem(SEM_FPGA,   &csem_fpga);	/* Create semaphore */	
	cre_sem(SEM_SPF,    &csem_spf);		/* Create semaphore */	
	cre_sem(SEM_STKN,   &csem_stkn);	/* Create semaphore */	
	cre_sem(SEM_STL,    &csem_stl);		/* Create semaphore */	

	ini_intr();

	/* Start task */

	sta_tsk(TSK_MAIN, 0);

	/* Start multitask system */

	intsta();                   /* Start interval timer interrupt */
	syssta();                   /* enter into multi task */
}

/* end */
