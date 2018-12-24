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
#include "tsk_learnData.h"

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

#include "udp.h"
#include "net_prm.h"
#include "va300.h"
#include "id.h"

#include "err_ctrl.h"
#include "mon.h"

#include "drv_eep.h"
#include "drv_led.h"
#include "drv_dsw.h"
#include "drv_tim.h"
#include "drv_buz.h"
#include "drv_irled.h"
#include "drv_cmr.h"
#include "drv_tpl.h"

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

EXTERN TASK NinshouTask( void );
EXTERN TASK LogTask( void );

extern BOOL terminal_sendbin(T_TERMINAL *t, const B *s, INT len);
//extern static ER CmrCmdTest(void);
extern ER CmrCmdSleep(void);
extern ER CmrCmdWakeUp(char sw);
extern void yb_init_all();

//const T_CTSK ctsk_learn_data = {TA_HLNG, NULL, (FP)TaskLearnData, 8, 0x2000, NULL, "Lean Data Task"};
const T_CTSK ctsk_main    = { TA_HLNG, NULL, MainTask,       5, 4096, NULL, (B *)"main" };
const T_CTSK ctsk_disp    = { TA_HLNG, NULL, DispTask,       6, 2048, NULL, (B *)"display" };	// 256 -->1024

const T_CTSK ctsk_rcv1    = { TA_HLNG, NULL, RcvTask,     3, 4096, NULL, (B *)"rcvtask" };
const T_CTSK ctsk_snd1    = { TA_HLNG, NULL, SndTask,     3, 2172, NULL, (B *)"sndtask" };

#if ( VA300S == 0 )
const T_CTSK ctsk_urcv    = { TA_HLNG, NULL, UdpRcvTask,  5, 4096, NULL, (B *)"udprcvtask" };
const T_CTSK ctsk_usnd    = { TA_HLNG, NULL, UdpSndTask,  5, 2172, NULL, (B *)"udpsndtask" };
const T_CTSK ctsk_lancmd  = { TA_HLNG, NULL, LanCmdTask,     4, 40960, NULL, (B *)"lan_cmd" };

#endif
#if ( VA300S == 1 )
//const T_CTSK ctsk_ninshou  = { TA_HLNG, NULL, NinshouTask, 2, 40960, NULL, (B *)"ninshoutask" };
const T_CTSK ctsk_log  = { TA_HLNG, NULL, LogTask,     5, 4096, NULL, (B *)"logtask" };
#endif
#if ( VA300S == 2 )
const T_CTSK ctsk_urcv    = { TA_HLNG, NULL, UdpRcvTask,  5, 4096, NULL, (B *)"udprcvtask" };
const T_CTSK ctsk_usnd    = { TA_HLNG, NULL, UdpSndTask,  5, 2172, NULL, (B *)"udpsndtask" };
const T_CTSK ctsk_lancmd  = { TA_HLNG, NULL, LanCmdTask,     4, 40960, NULL, (B *)"lan_cmd" };

const T_CTSK ctsk_ninshou  = { TA_HLNG, NULL, NinshouTask, 2, 40960, NULL, (B *)"ninshoutask" };
const T_CTSK ctsk_log  = { TA_HLNG, NULL, LogTask,     5, 4096, NULL, (B *)"logtask" };
#endif


const T_CTSK ctsk_io      = { TA_HLNG, NULL, IoTask, 2, 2048, NULL, (B *)"I/O task" };

#if ( VA300S == 0 )
const T_CMBX cmbx_lancmd  = { TA_TFIFO|TA_MFIFO, 2, NULL, (B *)"mbx_lancmd" };
const T_CMBX cmbx_ressnd  = { TA_TFIFO|TA_MPRI, 2, NULL, (B *)"mbx_ressnd" };

const T_CMPF cmpf_lres  = { TA_TFIFO,  8, sizeof (T_LANRESMSG), NULL, (B *)"mpf_lres" };

#endif
#if ( VA300S == 1 )
const T_CMBX cmbx_snd_ninshou  = { TA_TFIFO|TA_MFIFO, 2, NULL, (B *)"mbx_snd_ninshou" };
const T_CMBX cmbx_log_data  = { TA_TFIFO|TA_MPRI, 2, NULL, (B *)"mbx_log_data" };


const T_CMPF cmpf_snd_ninshou   = { TA_TFIFO, 32, sizeof (T_COMMSG), NULL, (B *)"mpf_snd_ninshou" };
const T_CMPF cmpf_log_data   = { TA_TFIFO, 8, sizeof (T_COMMSG), NULL, (B *)"mpf_log_data" };
#endif
#if ( VA300S == 2 )
const T_CMBX cmbx_lancmd  = { TA_TFIFO|TA_MFIFO, 2, NULL, (B *)"mbx_lancmd" };
const T_CMBX cmbx_ressnd  = { TA_TFIFO|TA_MPRI, 2, NULL, (B *)"mbx_ressnd" };

const T_CMPF cmpf_lres  = { TA_TFIFO,  8, sizeof (T_LANRESMSG), NULL, (B *)"mpf_lres" };

const T_CMBX cmbx_snd_ninshou  = { TA_TFIFO|TA_MFIFO, 2, NULL, (B *)"mbx_snd_ninshou" };
const T_CMBX cmbx_log_data  = { TA_TFIFO|TA_MPRI, 2, NULL, (B *)"mbx_log_data" };


const T_CMPF cmpf_snd_ninshou   = { TA_TFIFO, 32, sizeof (T_COMMSG), NULL, (B *)"mpf_snd_ninshou" };
const T_CMPF cmpf_log_data   = { TA_TFIFO, 8, sizeof (T_COMMSG), NULL, (B *)"mpf_log_data" };
#endif

const T_CMBX cmbx_snd     = { TA_TFIFO|TA_MPRI, 2, NULL, (B *)"mbx_snd" };
const T_CMBX cmbx_disp    = { TA_TFIFO|TA_MFIFO, 2, NULL, (B *)"mbx_disp" };

const T_CMPF cmpf_snd_sio   = { TA_TFIFO, 32, sizeof (T_COMMSG), NULL, (B *)"mpf_snd_sio" };
const T_CMPF cmpf_com   = { TA_TFIFO, 32, sizeof (T_COMMSG), NULL, (B *)"mpf_com" };
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

// �T�C�N���b�N�E�n���h����`
static void cycle1_hdr(void);
const T_CCYC ccyc_tim1  = { TA_HLNG|TA_STA, NULL, cycle1_hdr, (10/MSEC), (10/MSEC)};

//2013.05.08 Miya ���b�Z�[�WNG�̂��ߋ��p�������őΉ�����
struct{
	unsigned int LcdMsgSize;
	UB LcdMsgBuff[1024];
}g_LcdmsgData;

UINT rcv_mbf_Lcd(ID idnum, UB *msg);

static void cycle1_hdr(void);
static void ini_intr(void);

static BOOL read_ethernet_addr(void);
static BOOL read_ip_addr(void);
static char *ftp_passwd_check(T_FTP *ftp, const char *user, const char *pass);
static B *telnet_passwd_check(T_TELNETD *t, const B *name, const B *passwd);

static ER lan_get_sys_spec( UB *pval );		// VA300�@�d�l�ؑւ��t���O�̓��e���AEEPROM����ǂݏo���B

static void init_wdt( void );				// WDT(�E�H�b�`�h�b�O�E�^�C�}�[)�̏����ݒ�
static void reset_wdtc( void );				// WDT(�E�H�b�`�h�b�O�E�^�C�}�[)�J�E���^�̃N���A
static void stop_wdt( void );				// WDT(�E�H�b�`�h�b�O�E�^�C�}�[)�̖������ݒ�
static void reset_wdtmem( void );			// WDT�J�E���^�p�������̃��Z�b�g����
static void reset_wdt_cnt( void );			// WDT�J�E���^�p�������̃_�C���N�g�E���Z�b�g����(�t���b�V���E�������E�h���C�o������p)
											// �eTask��WDT�N���A�E�t���O�𖳎����āA�J�E���^���N���A����̂ŁA
											// �h���C�o���ł̎g�p�Ɍ���B�i���p����ƁA�^�X�N�P�ʂł�WDT�@�\�̈Ӗ����Ȃ��Ȃ�B�j
#if ( VA300S == 1 || VA300S == 2 )
static ER lan_get_Pfail_cnt( UB *pval );	// VA300S�@��d�J��Ԃ��J�E���^�̓��e���AEEPROM����ǂݏo��
#endif

static void SystemParamInit( void );		// VA300 �V�X�e������p�����[�^��Initial
static UB GetScreenNo(void);				// ���ݕ\���X�N���[���ԍ��̎擾
static void ChgScreenNo( UB NewScreenNo );	// ��ʑJ�ڏ��(�ԍ�)���X�V����

static ER self_diagnosis( char mode );	//20140930Miya
static void Init_Cmr_Param(void);	//20140930Miya
static void SetError(int err);			//20140930Miya
ER static power_on_process( void );		// PC�Ƃ̃R�~���j�P�[�V�����J�n�A�����l�ݒ�v��
static ER set_initial_param_for_Lcd( void );
static ER set_reg_param_for_Lcd( void );
static ER set_initial_param( void );		// VA-300s�̏ꍇ�́A�e��p�����[�^�̏����ݒ�B

static void LcdPosAdj(int calc);		//20161031Miya Ver2204 LCDADJ

#if ( VA300S == 0 || VA300S == 2 )
ER static send_WakeUp( void );			/** PC��WakeUp�m�F���M	**/
ER static send_camera_param_req( void );	/** �J�����E�p�����[�^�̏����l�v���̑��M	**/
ER static send_cap_param_req( void );		/** �摜�����̏����l�v���̑��M	**/	
ER static send_Led_param_req( void );		/** LED���ʐ��l�̏����l�v���̑��M	**/	
ER static send_touroku_param_req( void );	/** �o�^�f�[�^�̏����l�v���̑��M	**/	

static ER	SndCmdCngMode( UINT stat );		// PC�փ��[�h�ؑւ��ʒm�𑗐M

static ER send_nomal_mode_Wait_Ack_Retry( void ); // ���[�h�ؑ֒ʒm�̑��M�AAck�ENack�҂��ƃ��g���C�t���i�ʏ탂�[�h�ڍs���j
static ER send_nomal_mode( void );			// ���[�h�ؑ֒ʒm�̑��M�i�ʏ탂�[�h�ڍs���j
static ER send_Pfail_mode_Wait_Ack_Retry( void ); // ���[�h�ؑ֒ʒm�̑��M�AAck�ENack�҂��ƃ��g���C�t���i��d���[�h�ڍs���j
static ER send_Pfail_mode( void );			// ���[�h�ؑ֒ʒm�̑��M�i��d���[�h�ڍs���j
static ER send_shutdown_req_Wait_Ack_Retry( void ); // �V���b�g�_�E���v���̑��M�AAck�ENack�҂��ƃ��g���C�t��
static ER send_shutdown_req( void );			// �V���b�g�_�E���v���̑��M
static ER send_touroku_delete_Wait_Ack_Retry( void );	// �w�i�o�^�j�f�[�^�̍폜�v���̑��M�AAck�ENack�҂��ƃ��g���C�t��
static ER send_touroku_delete_req( void );	// �w�i�o�^�j�f�[�^�̍폜�v���̑��M�i�ʏ탂�[�h���j
static ER send_ID_No_check_req_Wait_Ack_Retry( void );	// ID�ԍ��⍇���̑��M�AAck�ENack�҂��ƃ��g���C�t��
static ER send_ID_No_check_req( void );		// ID�ԍ��⍇���̑��M�i�ʏ탂�[�h���j
static ER send_ID_Authority_check_req_Wait_Ack_Retry( void );	// ID�����⍇���̑��M�AAck�ENack�҂��ƃ��g���C�t��
static ER send_ID_Authority_check_req( void );	// ID�����⍇���̑��M�i�ʏ탂�[�h���j
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
#endif

static void reload_CAP_Param( void );		// Gain. FixShutter,IR_LED�̃p�����[�^���A�����v���l�ɖ߂��B�iCAP��Retry���ɕς�����\��������ׁB�j
static UB GetSysSpec( void );		// �}���V�����d�l/�P�΂P�d�l�@��Ԃ�Ԃ��B0:�}���V����(��L��)�d�l�A1:1�΂P�d�l�B

static UB sys_kindof_spec = 0; // �}���V�����d�l/�P�΂P�d�l�@�ؑւ��t���O�@0:�}���V����(��L��)�d�l�A1:1�΂P�i�@�l�j�d�l
						       // 			�A2�F�}���V�����i���p���j�d�l�B3:�f���@�E�}���V����(��L��)�d�l�A4:�f���@�E1�΂P�i�@�l�j�d�l 
							   // 			�A5�F�f���@�E�}���V�����i���p���j�d�l�B
static UB nyuu_shutu_kubun = 0;	// ���^�J�d�l�E���ގ��敪�@0�F�����ށA1�F�����A2�F�o��							   

static UH g_DipSwCode;	
						   
static int g_AuthCnt;	//20140423Miya �F�؃��g���C��

static int ercdStat = 0;		// �G���[�R�[�h�L��
static UINT sys_ScreenNo;		// ���݂̃X�N���[��No
static UINT sys_Mode;			// ��ԕϐ�
static UINT sys_SubMode;		// ��ԕϐ��i�T�u�j
static UINT sys_demo_flg;		// �f���d�l�t���O
static 	UB	s_CapResult;		// �w�F�؂̌���
static UB s_DongleResult;		// �h���O���̗L���m�F�̌���
static UB s_PasswordResult;		// �p�X���[�h�m�F�̌���
static UB s_ID_NO_Result;		// ID�ԍ��m�F�̌���
static UB s_ID_Authority_Result;  // ID�����m�F�̌���
static UB s_ID_Authority_Level; // ID�����⍇���R�}���h�Ŗ⍇�������[�U�[ID�̌������x���B  ASCII�R�[�h�B
static UB s_Kantoku_num[ 2 ];	// ID�����⍇�������R�}���h�œ����ē҂̑����B  ASCII�R�[�h�B
static UB s_Kanri_num[ 2 ];		// ID�����⍇�������R�}���h�œ����Ǘ��҂̑����B ASCII�R�[�h�B
static UB s_Ippan_num[ 6 ];		// ID�����⍇�������R�}���h�œ�����ʎ҂̑����B ASCII�R�[�h�B
static UB s_KinkyuuTourokuResult; // �ً}�o�^�ʒm�̌���
static FLGPTN befor_scrn_no;	// ��ʑJ�ڗp�ϐ��B�u���~���܂����H�v�u�������v�̎��̖߂���FLG_PTN�ԍ��B
static UB  befor_scrn_for_ctrl; // ��ʑJ�ڗp�ϐ��B�u���~���܂����H�v�u�������v�̎��̖߂��̉�ʔԍ��B

static UINT rcv_ack_nak;		// ack��M�t���O�@=0�F����M�A=1:ACK��M�A=-1�Fnak��M

static UB req_restart = 0;		// �p���[�E�I���E�v���Z�X�̍ċN���v���t���O�@=0:�v�������A=1:�v������
static UB Pfail_mode_count;		// ��d���[�h�̏ꍇ�̋N����(�����[�h�ŋN�������ꍇ�́Areset�����B)
static UINT Pfail_sense_flg;	// ��d���m�ʒm��M�t���O�@0:��M�����A1:��M����

static UINT g_CapTimes;			// 1:�B�e1��� 2:�ĎB�e	//20131210Miya add

static UH Flbuf[0x10000];		//�t���b�V���ޔ�p�o�b�t�@(1�Z�N�V������)

static UINT door_open_over_time;	// �h�A�ߊJ���ݒ莞��
static UINT Test_1sec_timer;		// 1�b�T�C�N���b�N�E�J�E���^�E�e�X�g�p
static UINT Pfail_start_timer;		// ��d�^�C�}�[�T�C�N���b�N�E�J�E���g�p
static unsigned long WDT_counter;	// WDT�^�C�}�[�p�T�C�N���b�N�E�J�E���^
static UINT main_TSK_wdt = FLG_ON;	// main�^�X�N�@WDT���Z�b�g�E���N�G�X�g�E�t���O
static UINT camera_TSK_wdt = FLG_ON;	// �J�����^�X�N�@WDT���Z�b�g�E���N�G�X�g�E�t���O
static UINT ts_TSK_wdt = FLG_ON;		// �^�b�`�Z���T�E�^�X�N�@WDT���Z�b�g�E���N�G�X�g�E�t���O
static UINT lcd_TSK_wdt = FLG_ON;		// LCD�^�X�N�@WDT���Z�b�g�E���N�G�X�g�E�t���O
static UINT sio_rcv_TSK_wdt = FLG_ON;	// SIO��M�^�X�N�@WDT���Z�b�g�E���N�G�X�g�E�t���O
static UINT sio_snd_TSK_wdt = FLG_ON;	// SIO���M�^�X�N�@WDT���Z�b�g�E���N�G�X�g�E�t���O

static UINT timer_10ms = 0, count_1sec = 0, count_1min = 0, count_1hour = 0;	// �����b�J�E���g

static BOOL s_bMonRs485;		// ���j�^��RS485�t���O

static unsigned short FpgaVerNum;	//20140905Miya lbp�ǉ� FPGA�o�[�W�����A�b�v
static UB f_fpga_ctrl=1;				//20140915Miya FPGA����t���O 0:V1.001�p�@1:V1.002�p
static int g_key_arry[10];	//20140925Miya password open

static int g_Diagnosis_start=0;	//20140930Miya	//�f�f�J�n�t���O

#if defined(_DRV_TEST_)
// �h���C�o�e�X�g
extern BOOL drvTest();

#endif

static int	g_SameImgGet;	//20151118Miya ���摜�ĎB�e
static int	g_SameImgCnt;	//20160115Miya ���摜�ĎB�e
static int	g_AuthType=0;		//20160120Miya 0:�w�F�� 1:�p�X���[�h�F�� 2:�ً}�J��
static int	ode_oru_sw;						//20160108Miya FinKeyS ���ł����E�������SW
static int	g_TestCap_start;
static UW	dbg_flwsize;
static UW	dbg_flwsizeIn;

static int	dbg_ts_flg;
static int	dbg_cam_flg;
static int	dbg_nin_flg;
static int	dbg_cap_flg;
static int g_MainteLvl;	//20160711Miya �f���@

static int	g_pcproc_f;	//20160930Miya PC����VA300S�𐧌䂷�� //0:�A�C�h�� 1:�F�� 2:�o�^
static int	g_capallow; //20160930Miya PC����VA300S�𐧌䂷��
static int	g_pc_authnum; //20160930Miya PC����VA300S�𐧌䂷��
static int	g_pc_authtime; //20160930Miya PC����VA300S�𐧌䂷��

//20161031Miya Ver2204 LCDADJ ->
ST_POS_REV BaseT;
ST_POS_REV InputT;
ST_TPL_REV RevT;
BkDataNoClear g_BkDataNoClear;
static int	g_LedCheck;
//20161031Miya Ver2204 LCDADJ <-

static int dbg_dbg1;


//20170315Miya 400Finger ->
//static int g_RegBlockNum;		// �o�^�u���b�N�ԍ�(1 �` 7) -1:�o�^�Ȃ�
static int g_RegAddrNum;		// �o�^�Ԓn(0 �` 239) -1:�o�^�Ȃ�
//static int g_RegTotalYubiNum;	// �o�^����Ă���w�̑���
static int g_BufAuthScore[240][20];	// �ɏ��F�؂̃X�R�A�[
//static unsigned char g_taikyo_flg = 0;		//20170320Miya 400FingerM2 �Ö��ދ��t���O
//20170315Miya 400Finger <-
static unsigned short g_FPGA_ErrFlg;	//20170706Miya FPGA�t���[�Y�΍�

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
	ER memerr1, memerr2;
	FLGPTN	flgptn;	

	int n, cnt, i;
	INT iMonCh, iLBusCh;
	
	UINT screen_timer;					// ��ʕ\���^�C�}�[
	UINT old_screenNo;					// �O��\�����̉��No
	UB ret_stat;
	UB sys_smt=0;		//20160112Miya FinKeyS
	
	int st_y;
	
	T_YBDATA *ybdata;

	int dbg_cnt1, dbg_cnt2;
	
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
	FlInit( SEM_FL );					// �t���b�V���������h���C�o������
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
	}

#if ( VA300S == 0 || VA300S == 2 )	
	// UDP �ʐM������
	ercd = udp_ini( TSK_UDPRCV, ID_UDP_OYOCMD, udp_portno);	
//	ercd = udp_ini( 0, ID_UDP_OYOCMD, udp_portno);	
	if( ercd != E_OK) {
		ercdStat = 3;
	    ErrCodeSet( ercd );
		slp_tsk();
	}
#endif


LAN_INI_END:
	sta_tsk(TSK_DISP,    0);			// �\���^�X�N�N��
	sta_tsk(TSK_IO, 0);					// I/O���m�^�X�N�N��
	
#if ( VA300S == 0 )		
	sta_tsk(TSK_COMMUNICATE, 0);		// UDP�d�������^�X�N
	
	sta_tsk(TSK_UDP_SEND, 0);			// UDP���M�^�X�N
	sta_tsk(TSK_CMD_LAN,  0);			// �R�}���h�����^�X�N�N��
#endif
#if ( VA300S == 1 )		
//	sta_tsk( TSK_NINSHOU, 0 );				// �F�؃^�X�N�N��
	sta_tsk( TSK_LOG, 0 );					// ���M���O�^�X�N�N��
#endif
#if ( VA300S == 2 )		
	sta_tsk(TSK_COMMUNICATE, 0);		// UDP�d�������^�X�N
	
	sta_tsk(TSK_UDP_SEND, 0);			// UDP���M�^�X�N
	sta_tsk(TSK_CMD_LAN,  0);			// �R�}���h�����^�X�N�N��

	sta_tsk( TSK_NINSHOU, 0 );				// �F�؃^�X�N�N��
	sta_tsk( TSK_LOG, 0 );					// ���M���O�^�X�N�N��
#endif
	
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

//#if ( USE_MON == 1 )
//	ercd = Mon_ini( iMonCh );			// ���j�^�R�}���h�^�X�N������
//
//	sta_tsk(TSK_SND1, iMonCh);			// �V���A�����M�^�X�N�N��
//#endif

	sta_tsk(TSK_SND1, 0);				// �V���A�����M�^�X�N�N��
	sta_tsk(TSK_RCV1, 0);				// �V���A����M�^�X�N�N��
	
	LcdcInit( SEM_LCD );				// LCD�R���g���[���̏�����

	CameraTaskInit( TSK_CAMERA );		// �J�����R���g���[���^�X�N������
	LedInit(SEM_LED);					// LED������
	
	yb_init_all();						// �w���(�������ȊO)�A�ً}�J�����̏�����
	
	LedOut(LED_POW, LED_ON);			// �d���\��LED��ON����i��j
	
PowerOn_Process:
	req_restart = 0;
	LedOut(LED_ERR, LED_OFF);			// �d���\��LED��OFF����i�ԁj
	LedOut(LED_OK, LED_OFF);			// �d���\��LED��OFF����i�΁j

	//20160112Miya FinKeyS
	sys_smt = 0;
	ercd = lan_get_sys_spec( &sys_kindof_spec );	// VA300�@�d�l�ؑւ��t���O�̓��e���ALAN�p EEPROM����ǂݏo���B

#if KOUJYOUCHK	//20160610Miya
	//sys_kindof_spec = 6;
#endif

	if (ercd != E_OK) {
		sys_kindof_spec = SYS_SPEC_MANTION;			// EEPROM����d�l�ؑւ��t���O���e���ǂݏo���Ȃ��ꍇ�́A�h�}���V�����i��L���j�d�l�h��Set�B
		sys_demo_flg = SYS_SPEC_NOMAL;
	}else{
		if(sys_kindof_spec > SYS_SPEC_SMT){
			ercd = lan_set_eep( EEP_SYSTEM_SPEC, SYS_SPEC_MANTION );	// �ݒ�d�l����EEPROM�ւ̏����݁B
			sys_kindof_spec = SYS_SPEC_MANTION;
			sys_demo_flg = SYS_SPEC_NOMAL;
		}else{
			if(sys_kindof_spec == SYS_SPEC_SMT){
				sys_smt = sys_kindof_spec;
				sys_kindof_spec = SYS_SPEC_MANTION;
			}				
			sys_demo_flg = SYS_SPEC_NOMAL;
		}
	}


/*	20160112Miya FinKeyS Del
	ercd = lan_get_sys_spec( &sys_kindof_spec );	// VA300�@�d�l�ؑւ��t���O�̓��e���ALAN�p EEPROM����ǂݏo���B
	if (ercd != E_OK) {
		sys_kindof_spec = SYS_SPEC_MANTION;			// EEPROM����d�l�ؑւ��t���O���e���ǂݏo���Ȃ��ꍇ�́A�h�}���V�����i��L���j�d�l�h��Set�B
		sys_demo_flg = SYS_SPEC_NOMAL;
	}	else {
		if ( ( sys_kindof_spec >= 0 ) && ( sys_kindof_spec <= 2 ) ){
			sys_demo_flg = SYS_SPEC_NOMAL;			// �f���d�l�łȂ��ꍇ
		}	else	{
			if ( sys_kindof_spec == 3 ){
				sys_kindof_spec = SYS_SPEC_MANTION;	// �}���V�����E��L���d�l
				sys_demo_flg = SYS_SPEC_DEMO;		// �f���d�l�̏ꍇ
			} else if ( sys_kindof_spec == 4 ){
				sys_kindof_spec = SYS_SPEC_OFFICE;	// �P�΂P�d�l�i�I�t�B�X�d�l�j
				sys_demo_flg = SYS_SPEC_DEMO;		// �f���d�l�̏ꍇ
			} else if ( sys_kindof_spec == 5 ){
				sys_kindof_spec = SYS_SPEC_ENTRANCE;// �}���V�����E���p���d�l
				sys_demo_flg = SYS_SPEC_DEMO;		// �f���d�l�̏ꍇ
//			} else if ( sys_kindof_spec == 7 ){
//				sys_kindof_spec = SYS_SPEC_OFFICE_NO_ID;	// �P�Α��d�l�i�I�t�B�X�EID�ԍ������d�l�j
//				sys_demo_flg = SYS_SPEC_DEMO;		// �f���d�l�̏ꍇ
			} else {
				sys_kindof_spec = SYS_SPEC_MANTION;
				sys_demo_flg = SYS_SPEC_NOMAL;
			}	
		}
	}
*/
	
	ercd = set_flg( ID_FLG_LCD, FPTN_LCD_INIT );	// LCD�̏�����ʕ\���̃��N�G�X�g
	if ( ercd != E_OK ) ercdStat = 5;
	
#if defined(_DRV_TEST_)
	drvTest();
#endif
			
	dly_tsk( 500/MSEC );

PowerOn_Process2:
	dly_tsk( 500/MSEC );
	
	MdCngMode( MD_POWER_ON );			// ���u���[�h���p���[�I�����[�h��
	ercd = power_on_process();			// ����Box�Ƃ̃R�~���j�P�[�V�����J�n�A�@�탂�[�h�ݒ�A�����l�ݒ�
	if(ercd == 1){
		goto PowerOn_Process;
	}

	//20140915Miya FPGA����t���O 0:V1.001�p�@1:V1.002�p
	FpgaVerNum = *(volatile unsigned short *)(FPGA_BASE + 0x0000) ; // fpga version no.
	if( FpgaVerNum == 0x1001 ){
		f_fpga_ctrl = 0;
	}else{
		f_fpga_ctrl = 1;
	}

	g_InPasswordOpen.sw 		= FLG_OFF;	//�p�X���[�h�J��SW 0:OFF 1:ON
	g_InPasswordOpen.kigou_inp 	= FLG_OFF;	//�L������SW 0:OFF 1:ON
	g_InPasswordOpen.hide_num 	= FLG_OFF;	//��\��SW 0:OFF 1:ON(��\�����{�A�*��\��)
	g_InPasswordOpen.random_key = FLG_OFF;	//�L�[�{�[�h�����_���\��SW 0:OFF 1:ON
	g_InPasswordOpen.keta 		= 4;		//���̓P�^��(4�`8)
	for(i = 0 ; i < 8 ; i++ ){
		g_InPasswordOpen.password[i] = 20;	//�p�X���[�h(0�`19) �����l:20
	}
			
	main_TSK_wdt = FLG_ON;					// WDT�J�E���^�p�������N���A(20�b��WDT�N�� = �p���[�I���E���Z�b�g)
	
	// ��ʕ\���̊J�n
	if ( MdGetMode() == MD_INITIAL ){ 	// �����o�^��ʂ̎��́A��ʂP�ցB
		//20160112Miya FinKeyS �X�}�[�g����Őݒ肩�珉�����ɂ��A�X�}�[�g����Őݒ�ɂ���
		if( sys_smt == SYS_SPEC_SMT ){
			g_TechMenuData.SysSpec = 2;
			g_PasswordOpen.sw = 1;
			g_PasswordOpen2.family_sw = 1;
			SaveBkAuthDataFl();
			ercd = lan_set_eep( EEP_SYSTEM_SPEC, SYS_SPEC_MANTION );	// �ݒ�d�l����EEPROM�ւ̏����݁B
			sys_kindof_spec = SYS_SPEC_MANTION;
			sys_demo_flg = SYS_SPEC_NOMAL;
		}
		
		if ( GetSysSpec() == SYS_SPEC_MANTION ){	// �}���V����(��L��)�d�l�̏ꍇ�B
		
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN1 );	// ID�ԍ����͉�ʂցB	
			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN1 );	// ��ʔԍ��@<-�@���̉��
			}
			
		} else if ( GetSysSpec() == SYS_SPEC_OFFICE ){		// �P�΂P�d�l�̏ꍇ�B
		
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN401 );	// �����o�^���j���[��ʂցB	
			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN401 );	// ��ʔԍ��@<-�@���̉��
			}						
		} else if ( GetSysSpec() == SYS_SPEC_KOUJIM || GetSysSpec() == SYS_SPEC_KOUJIO || GetSysSpec() == SYS_SPEC_KOUJIS ){//20160112Miya FinKeyS		// �H�����
			g_PasswordOpen2.family_sw = 0;
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN620 );	// �ʏ탂�[�h���͑ҋ@��ʂցB�i�P�΂P�d�l�j	
			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN620 );	// ��ʔԍ��@<-�@���̉��
			}						
		}
				
	} else if ( ( MdGetMode() == MD_NORMAL ) || ( MdGetMode() == MD_PFAIL ) ){	// �ʏ탂�[�h�A��d���[�h�̎��B

		if ( GetSysSpec() == SYS_SPEC_MANTION ){	// �}���V����(��L��)�d�l�̏ꍇ�B

			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// �ʏ탂�[�h���͑ҋ@��ʂցB	
			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN100 );	// ��ʔԍ��@<-�@���̉��
			}
			
		} else if ( GetSysSpec() == SYS_SPEC_OFFICE ){		// �P�΂P�d�l�̏ꍇ�B
		
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN500 );	// �ʏ탂�[�h���͑ҋ@��ʂցB�i�P�΂P�d�l�j	
			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN500 );	// ��ʔԍ��@<-�@���̉��
			}						
		}
							
	} else {
		nop();
	}

	MdCngSubMode(SUB_MD_MEMCHK);
	LedOut(LED_OK, LED_ON);
	LedOut(LED_ERR, LED_ON);
	dly_tsk( 1000/MSEC );
#if ( VA300S == 1 || VA300S == 2 )	
	memerr1 = MemCheck(0);
	if(memerr1 > 0){
		LedOut(LED_OK, LED_ON);
		if( memerr1 > 8 ){
			dly_tsk( 1000/MSEC );
			//FpgaSetWord(FPGA_RECONF, 0x01, 0x01);
		}
	}else{
		LedOut(LED_OK, LED_OFF);
	}
#endif
	//dly_tsk( 1000/MSEC );

	//ercd = set_flg( ID_FLG_CAMERA, FPTN_CHECK_IMAGE );			// �������`�F�b�N
/*	
	cnt = 0;
	Check_Cap_Raw_flg = 1;						// 2014.6.23 Added T.Nagai �B�e���t���O�̃Z�b�g
	while(1){
		dly_tsk( 1000/MSEC );
		if ( Check_Cap_Raw_flg == 0 ){			// 2014.6.23 Modify T.Nagai �B�e���t���O�̃Z�b�g
//		if ( MdGetSubMode() == SUB_MD_IDLE ){
			break;
		}
		++cnt;
		if( (cnt / 2) == 0 ){
			LedOut(LED_ERR, LED_OFF);
		}else{
			LedOut(LED_ERR, LED_ON);
		}

		++cnt;
		if( cnt > 10 ){
			req_restart = 1;
			break;
		}
	}
*/
			req_restart = 0;

	if( req_restart == 1 ){
		req_restart = 0;	
		LedOut(LED_ERR, LED_ON);
		dly_tsk( 1000/MSEC );
		FpgaSetWord(FPGA_RECONF, 0x01, 0x01);
	}else{
		LedOut(LED_ERR, LED_OFF);
	}
	//reload_CAP_Param();	//�J�����̏����l�ݒ�
	
	//�J��������m�F
#if(NEWCMR && FREEZTEST == 0)
	Check_Cap_Raw_flg = 1;
	Init_Cmr_Param();
	if(Check_Cap_Raw_flg == 3){
		LedOut(LED_ERR, LED_ON);
		if(g_BkDataNoClear.InitRetryCnt < 5){
			g_BkDataNoClear.InitRetryCnt++;
			//ercd = SaveBkDataNoClearFl();	//20161031Miya Ver2204 LCDADJ
			send_sio_VA300Reset();
			dly_tsk( 5000/MSEC );
		}else{
			g_BkDataNoClear.InitRetryCnt = 0;
			//ercd = SaveBkDataNoClearFl();	//20161031Miya Ver2204 LCDADJ
		}
	}else{
		g_BkDataNoClear.InitRetryCnt = 0;
		//ercd = SaveBkDataNoClearFl();	//20161031Miya Ver2204 LCDADJ
	}
#endif
/*
	ercd = self_diagnosis(0);	//20140930Miya
	if( ercd != E_OK ){
		nop();
	}
*/	
	screen_timer = 0;							// ��ʕ\���^�C�}�[�̏������B
	old_screenNo = GetScreenNo();				// ���ݕ\�����̉��No��ۑ��B
				
	main_TSK_wdt = FLG_ON;							// WDT�J�E���^�p�������N���A(20�b��WDT�N�� = �p���[�I���E���Z�b�g)
	dbg_dbg1 = 0;

#if( FREEZTEST )
	g_FPGA_ErrFlg = 0;
	for(dbg_cnt2 = 0 ; dbg_cnt2 < 20 ; ++dbg_cnt2){
		if( (dbg_cnt2 % 2) == 0 ) 
			LedOut(LED_ERR, LED_ON);
		else
			LedOut(LED_ERR, LED_OFF);

		for(dbg_cnt1 = 0 ; dbg_cnt1 < 6 ; ++dbg_cnt1){
			dbg_dbg1 = dbg_cnt1;
			DoAuthProc(0);
			dly_tsk( 1000/MSEC );
			
			if(g_FPGA_ErrFlg != 0){
				LedOut(LED_OK, LED_ON);
				while(1){
					dly_tsk( 1000/MSEC );
				}	
			}
			
		}
	}
	dly_tsk( 1000/MSEC );
	send_sio_VA300Reset();
	dly_tsk( 5000/MSEC );
#endif
		
	// ��ʐؑւ��ƁA�J�����B�e����
	for (;;) {
		ercd = twai_flg( ID_FLG_MAIN, ( FPTN_START_CAP			/* �J�����B�e�v���̎�M�҂� */
									  | FPTN_LCD_CHG_REQ		/* LCD��ʐؑ֗v��(LCD�����C��) */
									  | FPTN_SEND_REQ_MAINTE_CMD /* �����e�i���X���[�h�ؑւ��ʒm���M(��M�R�}���h��̓^�X�N�����C��Task)�@*/
									  ), TWF_ORW, &flgptn, 1000/MSEC );
		if ( ercd == E_TMOUT ){
			
			main_TSK_wdt = FLG_ON;					// WDT�J�E���^�p�������N���A(20�b��WDT�N�� = �p���[�I���E���Z�b�g)
		
			if ( GetScreenNo() == old_screenNo ){	// �����ʂ�15�b�ԑ����āA���A��d���[�h�܂��͒�d���m�ʒm��M�̏ꍇ�́A�����I�ɃV���b�g�_�E���B
				if ( screen_timer >= 15 ){
					if ( ( Pfail_sense_flg == PFAIL_SENSE_ON ) || ( MdGetMode() == MD_PFAIL ) ){
						
						Pfail_shutdown();			// �V���b�g�_�E���̎��s
						
					}	else	{
						screen_timer = 0;
						continue;
					}
				} else {
					screen_timer++;
					continue;	
				}
				
			}	else if ( Pfail_sense_flg == PFAIL_SENSE_ON ){
//			}	else if ( ( Pfail_sense_flg == PFAIL_SENSE_ON ) || ( MdGetMode() == MD_PFAIL ) ){
				
				// ��d���m�ʒm����M�Ȃ�A�F�ؑ��쒆�łȂ���΃V���b�g�_�E��
				ret_stat = Chk_shutdown_ok();				
				if ( ret_stat == SHUTDOWN_OK ){
						
						Pfail_shutdown();			// �V���b�g�_�E���̎��s
						
				}	else	{
					screen_timer = 0;
					old_screenNo = GetScreenNo();	// ���ݕ\�����̉��No��ۑ��B
					continue;
				}	

			}	else	{
				screen_timer = 0;
				old_screenNo = GetScreenNo();		// ���ݕ\�����̉��No��ۑ��B
				continue;
			}
			
		} else if ( ercd != E_OK ){
			ercdStat = 7;
			break;
		}
		
		screen_timer = 0;							// ��ʕ\���^�C�}�[�̏������B
		old_screenNo = GetScreenNo();				// ���ݕ\�����̉��No��ۑ��B
		
		main_TSK_wdt = FLG_ON;						// WDT�J�E���^�p�������N���A(20�b��WDT�N�� = �p���[�I���E���Z�b�g)
		
		switch ( flgptn ) {
			case FPTN_LCD_CHG_REQ: // LCD�^�X�N����A��ʕύX�v������
				ercd = 0;	
				ercd = clr_flg( ID_FLG_MAIN, ~FPTN_LCD_CHG_REQ );			// �t���O�̃N���A
				if ( ercd != E_OK ){
					ercdStat = 10;
					break;
				}
				
				//if ( GetSysSpec() == SYS_SPEC_MANTION ){	// �}���V����(��L��)�d�l�̏ꍇ�B
				if ( GetSysSpec() == SYS_SPEC_MANTION || GetSysSpec() == SYS_SPEC_KOUJIM || GetSysSpec() == SYS_SPEC_KOUJIO || GetSysSpec() == SYS_SPEC_KOUJIS ){	// �}���V����(��L��)�d�l�̏ꍇ�B
				
					ercd = NextScrn_Control_mantion();		// ���̉�ʕ\���v���t���O�Z�b�g�i�}���V����(��L��)�d�l�̏ꍇ�j
						
				} else if ( GetSysSpec() == SYS_SPEC_OFFICE ){		// �P�΂P�d�l�̏ꍇ�B
				
					ercd = NextScrn_Control_office();		// ���̉�ʕ\���v���t���O�Z�b�g�i�P�΂P�d�l�̏ꍇ�j
				
				} else {
					nop();	//�G���[�����̋L�q
				}
				
				ercd = NextScrn_Control();					// ���̉�ʕ\���v���t���O�Z�b�g�i���ʎd�l�̏ꍇ�j
				if(GetScreenNo() == LCD_SCREEN110 && g_Diagnosis_start == 1){
						
					ercd = self_diagnosis(2);
					g_Diagnosis_start = 0;

					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN101 );
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN101 );			// ��ʔԍ��@<-�@���̉��
					}
				}
				if(GetScreenNo() == LCD_SCREEN613 && g_Diagnosis_start == 1){	//20160108Miya FinKeyS
						
					ercd = self_diagnosis(2);
					g_Diagnosis_start = 0;

					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN601 );
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN601 );			// ��ʔԍ��@<-�@���̉��
					}
				}
				if(GetScreenNo() == LCD_SCREEN203 && g_Diagnosis_start == 1){
						
						//Check_Cap_Raw_flg = 0;
						//ercd = set_flg( ID_FLG_CAMERA, FPTN_CMR_INIT );

						g_MainteLog.chk_num = 0;
						ercd = self_diagnosis(1);
						if(ercd == 0)
							ercd = self_diagnosis(1);

						//ercd = set_flg( ID_FLG_CAMERA, FPTN_CMR_INIT );

					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN204 );	// ���Ȑf�f���ʉ�ʂցB	
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN204 );			// ��ʔԍ��@<-�@���̉��
					}
				}
				if(GetScreenNo() == LCD_SCREEN266 && g_TestCap_start > 0){
					dly_tsk( 500/MSEC );
					ercd = self_diagnosis(0);
					if(ercd == 0){
#if (NEWCMR == 0)	//20160601Miya
						ImgTriming(30);
						g_RegUserInfoData.trim_sty = 30;
#endif
					}
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN267 );	// �����e�i���X�E���j���[���(�ݒ�ύX)��
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN267 );		// ��ʔԍ��@<-�@���̉��
					}
				}
				
				break;
				
			case FPTN_START_CAP:	// ���̌��m�Z���T�[����A�J�����B�e�v������	
			
				ercd = clr_flg( ID_FLG_MAIN, ~FPTN_START_CAP );				// �t���O�̃N���A
				if ( ercd != E_OK ){
					ercdStat = 8;
					break;
				}
			
				if ( ( ( MdGetMode() == MD_NORMAL ) || ( MdGetMode() == MD_PFAIL ) )			// �m�[�}�����[�h�ŁA���101,102,121,141,161�\�����Ȃ�
				    && ( ( GetScreenNo() == LCD_SCREEN101 ) 	// �}���V����(��L��)�d�l�̏ꍇ
					  || ( GetScreenNo() == LCD_SCREEN102 ) 
					  || ( GetScreenNo() == LCD_SCREEN105 ) 	//20140423Miya �F�؃��g���C�ǉ�
					  || ( GetScreenNo() == LCD_SCREEN121 ) 
					  || ( GetScreenNo() == LCD_SCREEN141 )
					  || ( GetScreenNo() == LCD_SCREEN161 )
					  || ( GetScreenNo() == LCD_SCREEN181 )
					  || ( GetScreenNo() == LCD_SCREEN601 )		// 20160108Miya FinKeyS
					  || ( GetScreenNo() == LCD_SCREEN602 )		// 20160108Miya FinKeyS
					  || ( GetScreenNo() == LCD_SCREEN605 )		// 20160108Miya FinKeyS
					  || ( GetScreenNo() == LCD_SCREEN610 )		// 20160108Miya FinKeyS
					  || ( GetScreenNo() == LCD_SCREEN611 )		// 20160108Miya FinKeyS
					  || ( GetScreenNo() == LCD_SCREEN503 )		// �P�΂P�d�l�̏ꍇ
					  || ( GetScreenNo() == LCD_SCREEN523 )
					  || ( GetScreenNo() == LCD_SCREEN544 ) ) ){
						  
//					reload_CAP_Param();			// Gain. FixShutter,IR_LED�̃p�����[�^���A�����v���l�ɖ߂��B�iCAP��Retry���ɕς�����\��������ׁB�j

					ercd = set_flg( ID_FLG_CAMERA, FPTN_START_CAP211 );				// �J�����B�e+�F�؏����i�R�}���h211�j�ցB
					if ( ercd != E_OK ) break;
					
				} else if ( ( MdGetMode() == MD_NORMAL ) || ( MdGetMode() == MD_PFAIL ) ){			// �m�[�}�����[�h�ŁA���127�A���129�\�����Ȃ�
			
					if ( ( GetScreenNo() == LCD_SCREEN127 )  	// �}���V����(��L��)�d�l�̏ꍇ
					  || ( GetScreenNo() == LCD_SCREEN129 )
					  || ( GetScreenNo() == LCD_SCREEN530 )		// �P�΂P�d�l�̏ꍇ
					  || ( GetScreenNo() == LCD_SCREEN532 ) ){
						
//						if ( GetScreenNo() == LCD_SCREEN127 ){
//							reload_CAP_Param();			// Gain. FixShutter,IR_LED�̃p�����[�^���A�����v���l�ɖ߂��B�iCAP��Retry���ɕς�����\��������ׁB�j
//						} 

						ercd = set_flg( ID_FLG_CAMERA, FPTN_START_CAP204 );			// �J�����B�e+�o�^�����i�R�}���h204�j�ցB
						if ( ercd != E_OK ) break;
					}
				} else if ( MdGetMode() == MD_INITIAL ){		// �����o�^���[�h�ŁA���6�A���8�\�����Ȃ�
			
					if ( ( GetScreenNo() == LCD_SCREEN6 )   	// �}���V����(��L��)�d�l�̏ꍇ
					  || ( GetScreenNo() == LCD_SCREEN8 )
					  || ( GetScreenNo() == LCD_SCREEN405 )		// �P�΂P�d�l�̏ꍇ
					  || ( GetScreenNo() == LCD_SCREEN407 ) ){
						  
//						if ( GetScreenNo() == LCD_SCREEN6 ){
//							reload_CAP_Param();			// Gain. FixShutter,IR_LED�̃p�����[�^���A�����v���l�ɖ߂��B�iCAP��Retry���ɕς�����\��������ׁB�j
//						}
												
						ercd = set_flg( ID_FLG_CAMERA, FPTN_START_CAP204 );			// �J�����B�e+�o�^�����i�R�}���h204�j�ցB
						if ( ercd != E_OK ) break;
					}
				} else if ( MdGetMode() == MD_MAINTE ){		// �����e�i���X���[�h�ŁA���6�A���203�\�����Ȃ�
/*			
					if ( GetScreenNo() == LCD_SCREEN203 ){
						
//						reload_CAP_Param();			// Gain. FixShutter,IR_LED�̃p�����[�^���A�����v���l�ɖ߂��B�iCAP��Retry���ɕς�����\��������ׁB�j

						ercd = set_flg( ID_FLG_CAMERA, FPTN_START_CAP141 );			// �J�����B�e+�t���摜�����i�R�}���h141�j�ցB
						if ( ercd != E_OK ) break;
					}
*/
				}
				break;

			case FPTN_SEND_REQ_MAINTE_CMD:	// ��M�R�}���h��̓^�X�N�����C��Task�ցA�����e�i���X���[�h�ؑւ��ʒm���M���˗��B

				ercd = clr_flg( ID_FLG_MAIN, ~FPTN_SEND_REQ_MAINTE_CMD );			// �t���O�̃N���A
				if ( ercd != E_OK ){
					ercdStat = 9;
					break;
				}
				
#if ( VA300S == 0 )				
				ercd = SndCmdCngMode( (UINT)MD_MAINTE );	// PC��	�����e�i���X���[�h�ؑւ��ʒm�𑗐M
				if ( ercd != E_OK ){
					nop();		// �G���[�����̋L�q	
				}
#endif
				
			default:
				break;
		}

//		if(dbg_dbg1 == 0){
//			dbg_dbg1 = 1;
//				MakeTestImg();	//20160902Miya FPGA������ forDebug
//				WrtImgToRam(0, 0);	//[�B�e][R1]
//				WrtImgToRam(1, 0);	//[�o�^][R1]
//				WrtImgToRam(0, 1);	//[�B�e][R2]
//				WrtImgToRam(1, 1);	//[�o�^][R2]
//		}			
		
		if ( req_restart == 1 ) goto PowerOn_Process;
		
	}
	PrgErrSet();
	slp_tsk();		//�@�����֗��鎞�͎����G���[
}



/*==========================================================================*/
/**
 *	���Ȑf�f����
 *	����	char mode;		//0:�e�X�g�B�e�̂� 1:���Ȑf�f(���j���[) 2:���Ȑf�f(�^�C�}�[)
 */
/*==========================================================================*/
static ER self_diagnosis( char mode )
{
	ER ercd = 0;
	char result;
	unsigned long i, cnt, size, offset, loop;	
	double ave;
	UB memerr=0;
	unsigned short para, inpara;

	result = 0;
	if(mode == 0)	loop = 1;
	else			loop = 2;

//�B�e�e�X�g1(�J�������x��ς��āA���x�Ⴂ�̉摜����)
	size = iReSizeX * iReSizeY;
	for(i = 0 ; i < loop ; i++){
		Check_Cap_Raw_flg = 1;
		ercd = set_flg( ID_FLG_CAMERA, FPTN_CHECK_IMAGE );			// �������`�F�b�N
			 
		cnt = 0;
		while(1){
			dly_tsk( 200/MSEC );
			if ( Check_Cap_Raw_flg == 0 ){
				dly_tsk( 1000/MSEC );
				break;
			}
			if( Check_Cap_Raw_flg == 2){
				result = result | 0x01;	//BIT1
				break;
			}	
			if( CmrCapNg > 0 ){
				result = result | 0x01;	//BIT1
				break;
			}

			++cnt;
			if( cnt > 20 ){
				result = result | 0x01;	//BIT1
				break;
			}
		}
		if( result != 0 )
			break;
	}

	if( mode == 0){
		if( result != 0 ){
			ercd = 1;
		}
		return(ercd);
	}

	memcpy(&Flbuf[0], &g_ubCapBuf[0], size );
	memcpy(&Flbuf[0x8000], &g_ubCapBuf[size], size );
	cnt = 0;
	for(i = 0 ; i < size/2 ; i++){
		if( Flbuf[i] == Flbuf[0x8000+i] ){
			++cnt;
		}
	}
	if( cnt == size/2 ){
		result = result | 0x02;	//BIT2
	}

//�J�����̏�f�f
	cnt = 0;
	for(i = 0 ; i < size/2 ; i++){
		if( Flbuf[i] == 0x0000 ){
			++cnt;
		}
	}
	if( cnt == size/2 ){
		result = result | 0x04;	//BIT3
	}

//�ٕ��`�F�b�N
	ave = 0.0;
	cnt = 0;
	//offset = 2 * iReSizeX * iReSizeY;
	offset = 0;
	for(i = 0 ; i < size/4 ; i++ ){
		ave += (double)g_ubCapBuf[offset + cnt];
		cnt += 4;
	}
	ave = ave / (double)(size / 4);
	
	cnt = 0;
	for(i = 0 ; i < size ; i++ ){
		if( g_ubCapBuf[offset + i] > ave ){
			++cnt;
		}
	}
	
	if( g_MainteLog.diag_cnt2 == 0 || g_MainteLog.chk_num == 0 ){
		g_MainteLog.chk_num = (unsigned short)(0.8 * (double)cnt);
		g_MainteLog.chk_ave = (unsigned short)(0.8 * ave);
		memcpy(&g_ubCapBuf[3 * iReSizeX * iReSizeY], &g_ubCapBuf[0], iReSizeX * iReSizeY);
	}else{
		if( cnt < g_MainteLog.chk_num || (unsigned short)ave < g_MainteLog.chk_ave ){
			result = result | 0x08;	//BIT4
			memcpy(&g_ubCapBuf[2 * iReSizeX * iReSizeY], &g_ubCapBuf[3 * iReSizeX * iReSizeY], iReSizeX * iReSizeY);
		}
	}

//�摜������ R/W�e�X�g
	memerr = MemCheck(0);
	if(memerr > 0){
		result = result | 0x10;	//BIT5
	}		
	memerr = MemCheck(0x4000000);
	if(memerr > 0){
		result = result | 0x20;	//BIT6
	}		
	memerr = MemCheck(0x800000);
	if(memerr > 0){
		result = result | 0x40;	//BIT7
	}		

	cnt = g_MainteLog.diag_cnt1;
	if( result & 0x01 )		g_MainteLog.diag_buff[cnt][0] = 1;	//�f�f�G���[����(�B�e�ُ�)
	else					g_MainteLog.diag_buff[cnt][0] = 0;	//�f�f�G���[�Ȃ�

	if( result & 0x02 )		g_MainteLog.diag_buff[cnt][1] = 1;	//�f�f�G���[����(���������t���b�V���ُ�)
	else					g_MainteLog.diag_buff[cnt][1] = 0;	//�f�f�G���[�Ȃ�

	if( result & 0x04 )		g_MainteLog.diag_buff[cnt][2] = 1;	//�f�f�G���[����(�J�����ُ�)
	else					g_MainteLog.diag_buff[cnt][2] = 0;	//�f�f�G���[�Ȃ�

	if( result & 0x08 )		g_MainteLog.diag_buff[cnt][3] = 1;	//�f�f�G���[����(�ٕ��ُ�)
	else					g_MainteLog.diag_buff[cnt][3] = 0;	//�f�f�G���[�Ȃ�

	if( result & 0x10 )		g_MainteLog.diag_buff[cnt][4] = 1;	//�f�f�G���[����(������RW�ُ�)
	else					g_MainteLog.diag_buff[cnt][4] = 0;	//�f�f�G���[�Ȃ�

	if( result & 0x20 )		g_MainteLog.diag_buff[cnt][4] = 1;	//�f�f�G���[����(������RW�ُ�)
	else					g_MainteLog.diag_buff[cnt][4] = 0;	//�f�f�G���[�Ȃ�

	if( result & 0x40 )		g_MainteLog.diag_buff[cnt][4] = 1;	//�f�f�G���[����(������RW�ُ�)
	else					g_MainteLog.diag_buff[cnt][4] = 0;	//�f�f�G���[�Ȃ�

	++g_MainteLog.diag_cnt1;
	g_MainteLog.diag_cnt1 = g_MainteLog.diag_cnt1 & 0x07;
	++g_MainteLog.diag_cnt2;


	if( mode == 2 ){
		g_AuthOkCnt = 0;
		ercd = SaveBkAuthDataFl();
		ercd = SaveRegImgFlArea( 0 );
		ercd = SaveRegImgFlArea( 10 );
	}


	if( result & 0x01 ){
		SetError(40);
		//return(0xff);
	}

	//if(mode == 2 && result > 0){	//���X�B�e�֘A�̃G���[���������邪���A����̂ŃG���[�\�����Ȃ���
	if(mode == 2 && (g_MainteLog.diag_buff[cnt][2] == 1 || g_MainteLog.diag_buff[cnt][4] == 1) ){
		SetError(31);
	}

	return(ercd);
}


static void Init_Cmr_Param(void)	//20140930Miya
{
	ER ercd = 0;
	int cnt, i;
	
	cnt = 0;
	while(1){
		if( Cmr_Start > 0 ){
			break;
		}
		dly_tsk( 500/MSEC );
		++cnt;
		if(cnt >= 6){
			break;
		}
	}
	
	irDuty2 = ini_irDuty2;		// IR Duty�lirDuty2�@�����l;
	irDuty3 = ini_irDuty3;
	irDuty4 = ini_irDuty4;
	irDuty5 = ini_irDuty5;

	cmrGain = ini_cmrGain;						// �J�����E�Q�C���l�����l�@
	cmrFixShutter1 = ini_cmrFixShutter1;		// Fix Shutter Control�l�����l(�P���)�@
	cmrFixShutter2 = ini_cmrFixShutter2;		// Fix Shutter Control�l�����l(�Q���)			����
	cmrFixShutter3 = ini_cmrFixShutter3;		// Fix Shutter Control�l�����l(�R��ځj

	if( Cmr_Start > 0){
		dly_tsk( 500/MSEC );
		ercd = set_flg( ID_FLG_CAMERA, FPTN_CMR_INIT );
	}else{
		SetError(32);
	}

#if(NEWCMR == 1)
	while(1){
		if(Check_Cap_Raw_flg == 0)
			break;
	}
	dly_tsk( 500/MSEC );
	ercd = self_diagnosis(0);
	if(ercd != 0){
		nop();	
	}
	cnt = 0;
	for( i= 0 ; i< 10 ; i++){
		if(g_ubCapBuf[i] == 0xFF)	++cnt;
	}
	if(cnt > 0){
		Check_Cap_Raw_flg = 3;
	}else{
		Check_Cap_Raw_flg = 0;
	}
#endif
}



static void SetError(int err)
{
	int cnt;
	unsigned short chk;

	if( err == 39 || err == 40 ){			//�J�����V�[�P���X�G���[
		g_MainteLog.cmr_seq_err_f = g_MainteLog.cmr_seq_err_f | 0x01;
	}else if( err == 41 ){		//�J����WakeUp�G���[
		g_MainteLog.cmr_seq_err_f = g_MainteLog.cmr_seq_err_f | 0x02;
	}else{
		cnt = g_MainteLog.err_wcnt;
		g_MainteLog.err_buff[cnt][0] = err;
		g_MainteLog.err_buff[cnt][1] = 0;
		g_MainteLog.err_buff[cnt][2] = 0;
		g_MainteLog.err_buff[cnt][3] = 0;
		g_MainteLog.err_buff[cnt][4] = 1;
		++g_MainteLog.err_wcnt;
		g_MainteLog.err_wcnt = g_MainteLog.err_wcnt & 0x7F;
	}
}






/*==========================================================================*/
/**
 *	����Box�Ƃ̃R�~���j�P�[�V�����J�n�A�e�평���l�ݒ�v�����C��
 */
/*==========================================================================*/
static ER power_on_process( void )
{
	ER ercd = 0;
	int Retry;
	int i, j;
	
	/** ����Box��WakeUp�m�F���M	**/
	MdCngSubMode( SUB_MD_WAKEUP );			// �T�u���[�h���AWakeUp�₢���킹�֐ݒ�

									// �{������ցB2014.9.29
									// �ȉ��̉��������R���̗L���ݒ���A�t���b�V���E����������Ǐo���ď����ݒ肵�ĉ������B	
	ercd = ReadBkAuthData();	//20141003Miya add set_initial_param()����ړ�

	ercd = ReadBkDataNoClearFl();	//20161031Miya Ver2204 LCDADJ
	//ercd = SaveBkDataNoClearFl();	//20161031Miya Ver2204 LCDADJ
	LcdPosAdj(0);					//20161031Miya Ver2204 LCDADJ
	
	// �������iVA-300,VA-300�����p�j�o�[�W�����ԍ��̏�����
	//KeyIO_board_soft_VER[ 0 ] = '2';
	//KeyIO_board_soft_VER[ 1 ] = '.';
	//KeyIO_board_soft_VER[ 2 ] = '0';
	//KeyIO_board_soft_VER[ 3 ] = '2';
	KeyIO_board_soft_VER[ 0 ] = '1';
	KeyIO_board_soft_VER[ 1 ] = '.';
	KeyIO_board_soft_VER[ 2 ] = '0';
	KeyIO_board_soft_VER[ 3 ] = '0';

//20140125Miya nagaiBug�C���@
//while(1)���ŁAsend_sio_WakeUp()���R�[������̂ŁAACK�����O�ɃR�}���h���o����ʐM���������Ȃ��B
//#if ( VA300S == 1 || VA300S == 2 )	
//#if ( VA300S == 1 )	//20140905Miya
//#if ( VA300S == 1 && AUTHTEST == 0 )	//20160715Miya
#if ( VA300S == 1 && AUTHTEST == 0 && PCCTRL == 0 )	//20160930Miya PC����VA300S�𐧌䂷��

	sio_mode = SIO_SEND_MODE;			// �V���A���ʐM���A���M���[�h�֐ݒ�B
	for(i = 0 ; i < 3 ; i++){
	
		send_sio_WakeUp();					// VA300S����Box��WakeUp��₢���킹�B
		j = 0;	
		while(1){
			dly_tsk( 1200/MSEC );			// �ύX2014.4.21 T.Nagai ��d����̍ăX�^�[�g����Test�R�}���h�ԐM��M�̃^�C�~���O����
//			dly_tsk( 1000/MSEC );
					
			if ( MdGetSubMode() == SUB_MD_IDLE ){	// WakeUp�₢���킹�ɑ΂��āA����Box�����Test�R�}���h����������҂B
				j = 0;
				ercd = set_initial_param();		// �V�X�e���E�������[�h�A�J�����E�p�����[�^�A�摜�����p�����[�^�A
											// LED���ʁA�o�^�f�[�^�A�Ȃǂ̊e��p�����[�^�̏��������s���B
											// VA-300s�̏ꍇ�A����Box����p�����[�^��M�͖����ׁA��L������ݒ肷��K�v������B


				send_sio_Touroku_InitAll();		// VA300S����Box�փV���A���œo�^�����R�}���h(01)�𑗐M(�d��ON���̈ꊇ���M)�B

				break;
			}
			
			++j;
			if( j >= 5 ){
//			if( j >= 5 ){
				break;
			}
		}
		
		if( j == 0 ){
			break;
		}
		
		LedOut(LED_ERR, LED_ON);
	}

	if ( MdGetSubMode() != SUB_MD_IDLE ){
		LedOut(LED_OK, LED_ON);
		LedOut(LED_ERR, LED_ON);
		dly_tsk( 5000/MSEC );				// �ύX2014.4.21 T.Nagai ��d����̍ăX�^�[�g����Test�R�}���h�ԐM��M�̃^�C�~���O����
//		dly_tsk( 1000/MSEC );
		ercd = 1;
		return ercd;
	}
	
	send_sio_Kakaihou_time();				// VA300s����Box�։ߊJ�����Ԃ̐ݒ�v���R�}���h�𑗐M����B
	MdCngSubMode( SUB_MD_KAKAIHOU_TIME );	// �T�u���[�h���A�ߊJ�����Ԃ̐ݒ�v�����M���ցB
	j = 0;
	while( MdGetSubMode() != SUB_MD_IDLE ){	// ����Box����̉ߊJ�����Ԃ̐ݒ�v���R�}���h����������҂B
		dly_tsk( 100/MSEC );
		j++;
		if ( j >= 30 ){						// �R�b�o�ߌ�AAck���������ꍇ�́APaowerONProcess�ď����ցB
			LedOut(LED_OK, LED_ON);
			LedOut(LED_ERR, LED_ON);
			ercd = 1;
			return ercd;
		}
	}
	g_MainteLvl = 0;	//20160711Miya �f���@
	if(FORDEMO == 1){	//20160711Miya �f���@
		g_TechMenuData.DemoSw = FLG_ON;
		g_PasswordOpen.sw = FLG_ON;
	}
#else
	MdCngSubMode( SUB_MD_IDLE );	//�@�T�u���[�h�E�X�e�[�^�X���A�h�A�C�h���h�ցB
	ercd = set_initial_param();		// �V�X�e���E�������[�h�A�J�����E�p�����[�^�A�摜�����p�����[�^�A
	dip_sw_data[0] = 1;
#endif


#if ( VA300S == 2 )	//20140905Miya
	MdCngSubMode( SUB_MD_IDLE );	//�@�T�u���[�h�E�X�e�[�^�X���A�h�A�C�h���h�ցB
	ercd = set_initial_param();		// �V�X�e���E�������[�h�A�J�����E�p�����[�^�A�摜�����p�����[�^�A
	dip_sw_data[0] = 1;
#endif


//#else
#if ( VA300S == 0 )	//20140905Miya
	while ( 1 ){							// WakeUp�₢���킹��PC����̉�������������܂ŁA�P�b�����ŌJ��Ԃ��B
		ercd = send_WakeUp();				// PC��WakeUp��₢���킹�B
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
#endif

/*
	while ( 1 ){							// WakeUp�₢���킹��PC����̉�������������܂ŁA�P�b�����ŌJ��Ԃ��B

#if ( VA300S == 1 || VA300S == 2 )	
		sio_mode = SIO_SEND_MODE;			// �V���A���ʐM���A���M���[�h�֐ݒ�B
		send_sio_WakeUp();					// VA300S����Box��WakeUp��₢���킹�B
		
		dly_tsk( 1000/MSEC );
		
		if ( MdGetSubMode() == SUB_MD_IDLE ){	// WakeUp�₢���킹�ɑ΂��āA����Box�����Test�R�}���h����������҂B
	
			ercd = set_initial_param();		// �V�X�e���E�������[�h�A�J�����E�p�����[�^�A�摜�����p�����[�^�A
											// LED���ʁA�o�^�f�[�^�A�Ȃǂ̊e��p�����[�^�̏��������s���B
			break;							// VA-300s�̏ꍇ�A����Box����p�����[�^��M�͖����ׁA��L������ݒ肷��K�v������B
	
		}

#else
		ercd = send_WakeUp();				// PC��WakeUp��₢���킹�B
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
#endif
	}
*/

#if ( VA300S == 1 || VA300S == 2 )	

	//�@�J�����Q�C���ݒ�
	cmrGain = 7;
	ini_cmrGain = 7;
	//�@�I�o�P�ݒ�
	cmrFixShutter1 = 2;
	ini_cmrFixShutter1 = 2;
	//�@�I�o�Q�ݒ�
	cmrFixShutter2 = 4;
	ini_cmrFixShutter2 = 4;
	//�@�I�o�R�ݒ�
	cmrFixShutter3 = 6;
	ini_cmrFixShutter3 = 6;

	// IR LED�̓_�������l
	irDuty2 = 255;		
	irDuty3 = 255;
	irDuty4 = 255;
	irDuty5 = 0;
	// IR LED�̓_�������l���L��
	ini_irDuty2 = 255;
	ini_irDuty3 = 255;
	ini_irDuty4 = 255;
	ini_irDuty5 = 0;

#if (NEWCMR == 0)	//20160601Miya
	// �摜�؂�o���T�C�Y�w��
	iSizeX = 640;
	iSizeY = 560;
	// 	�g���~���O�̍��W��ݒ肷��
	iStartX = 160;	//(1280 - 640) / 2
	iStartY = 140;	//(720 - 320) / 2

	//20160120Miya
	if(g_DipSwCode == 0){
		iStartY = 140;
	}else if(g_DipSwCode == 0x10){	//DIP-SW5
		iStartY = 110;
	}else if(g_DipSwCode == 0x20){	//DIP-SW6
		iStartY = 80;
	}
#else
	// �摜�؂�o���T�C�Y�w��
	iSizeX = 400;
	iSizeY = 160;
	// 	�g���~���O�̍��W��ݒ肷��
	iStartX = 180;	
	iStartY = 160;	//(480 - 160) / 2 = 160

	if(g_BkDataNoClear.LedPosi == 1){	//20161031Miya Ver2204 �����΍� �J�n���W������
		iStartX = 180 - 64;
	}

	if(g_DipSwCode == 0x10){	//DIP-SW5
		iStartY = 160 - 10;	//-10
		g_TechMenuData.CmrCenter = -10;
	}
	if(g_DipSwCode == 0x20){	//DIP-SW6
		iStartY = 160 + 10;	//+10
		g_TechMenuData.CmrCenter = +10;
	}
#endif

	// ���T�C�Y�̏k������ݒ肷��
	iReSizeMode = RSZ_MODE_1;	//< ��1/4
	iReSizeX = iSizeX / 4;		//20131210Miya add
	iReSizeY = iSizeY / 4;		//20131210Miya add
/*
	if(g_DipSwCode != 0){
		if( g_DipSwCode == 0x10 ){
			//�@�J�����Q�C���ݒ�
			cmrGain = 8;
			ini_cmrGain = 8;
		}
		if( g_DipSwCode == 0x20 || g_DipSwCode == 0x30 ){
			//�@�J�����Q�C���ݒ�
			cmrGain = 9;
			ini_cmrGain = 9;
		}
		if( g_DipSwCode == 0x30 ){
			//�@�J�����Q�C���ݒ�
			cmrGain = 10;
			ini_cmrGain = 10;
		}
		
	}
*/
	InitImgAuthPara();	//�F�؃p�����[�^

	//reload_CAP_Param();	//�J�����̏����l�ݒ�

	ercd = 0;
	
//#if ( VA300S == 0 ) 
#else	
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

	while ( MdGetSubMode() != SUB_MD_IDLE ){	// LED���ʐ��l�̏����l�v����PC����̉���������҂B
		dly_tsk( 25/MSEC );	
	}
	

	// �}���V����(��L��)�d�l�̏ꍇ�̂݁A�o�^�f�[�^�̏����l�v�����s���B
	if ( GetSysSpec() == SYS_SPEC_MANTION ){
		
		/** �o�^�f�[�^�̏����l�v���̑��M	**/	
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
	}
#endif	
	
	return ercd;
	
}


/*==========================================================================*/
/*
 *	LCD�\���p�̃p�����[�^������
 */
/*==========================================================================*/
static ER set_initial_param_for_Lcd( void )
{
	ER ercd = E_OK;
	int i, j;
	
	// �ً}�J���d�b�ԍ�
	kinkyuu_tel_no[0] = '0';
	kinkyuu_tel_no[1] = '8';
	kinkyuu_tel_no[2] = '0';
	kinkyuu_tel_no[3] = '0';
	kinkyuu_tel_no[4] = 0x20;
	kinkyuu_tel_no[5] = 0x20;
	kinkyuu_tel_no[6] = 0x20;
	kinkyuu_tel_no[7] = 0x20;
	kinkyuu_tel_no[8] = '1';
	kinkyuu_tel_no[9] = '7';
	kinkyuu_tel_no[10] = '0';
	kinkyuu_tel_no[11] = 0x20;
	kinkyuu_tel_no[12] = '3';
	kinkyuu_tel_no[13] = '1';
	kinkyuu_tel_no[14] = '3';
	kinkyuu_tel_no[15] = '1';
	kinkyuu_tel_no[16] = ',';

	//20140925Miya add mainte
	for( i = 0 ; i < 16 ; i++ ){
		g_RegUserInfoData.MainteTelNum[i] = kinkyuu_tel_no[i];
	}

	// ���ԍ�
	yb_touroku_data.tou_no[ 0 ] = '0';
	yb_touroku_data.tou_no[ 1 ] = '0';
	yb_touroku_data.tou_no[ 2 ]	= ',';
	// ���[�U�[ID
	yb_touroku_data.user_id[ 0 ] = '0';	
	yb_touroku_data.user_id[ 1 ] = '0';
	yb_touroku_data.user_id[ 2 ] = '0';
	yb_touroku_data.user_id[ 3 ] = '0';
	yb_touroku_data.user_id[ 4 ] = ',';

	for( j = 0 ; j < 21 ; j++ ){	//�@��M�z���񂪁A�Q�O�񕪁��Q�P�񕪂ɕύX�B2013.7.15
		yb_touroku_data20[j].yubi_seq_no[ 0 ]	= '0';	// �ӔC��/��ʎ҂̓o�^�w���
		yb_touroku_data20[j].yubi_seq_no[ 1 ]	= '0';
		yb_touroku_data20[j].yubi_seq_no[ 2 ]	= '0';
		yb_touroku_data20[j].yubi_seq_no[ 3 ]	= ',';

		yb_touroku_data20[j].kubun[ 0 ]			= 0;	// �ӔC��/��ʎҋ敪�A�f�z���g�́h��ʎҁh
		yb_touroku_data20[j].kubun[ 1 ]			= ',';

		yb_touroku_data20[j].yubi_no[ 0 ]		= '0';	// �o�^�w�ԍ��i�w��ʁj
		yb_touroku_data20[j].yubi_no[ 1 ]		= '0';
		yb_touroku_data20[j].yubi_no[ 2 ]		= ',';
		
		for ( i=0; i<24; i++ ){
			yb_touroku_data20[j].name[ i ] = 0;	
		}
	}
	// �ӔC��/��ʎ҂̓o�^�w���
	yb_touroku_data.yubi_seq_no[ 0 ]	= yb_touroku_data20[ 1 ].yubi_seq_no[ 0 ];	
	yb_touroku_data.yubi_seq_no[ 1 ]	= yb_touroku_data20[ 1 ].yubi_seq_no[ 1 ];
	yb_touroku_data.yubi_seq_no[ 2 ]	= yb_touroku_data20[ 1 ].yubi_seq_no[ 2 ];
	yb_touroku_data.yubi_seq_no[ 3 ]	= ',';
	// �ӔC��/��ʎҋ敪�A�f�z���g�́h��ʎҁh
	yb_touroku_data.kubun[ 0 ]			= yb_touroku_data20[ 1 ].kubun[ 0 ];	
	yb_touroku_data.kubun[ 1 ]			= ',';
	// �o�^�w�ԍ��i�w��ʁj
	yb_touroku_data.yubi_no[ 0 ]		= yb_touroku_data20[ 1 ].yubi_no[ 0 ];	
	yb_touroku_data.yubi_no[ 1 ]		= yb_touroku_data20[ 1 ].yubi_no[ 1 ];
	yb_touroku_data.yubi_no[ 2 ]		= ',';

	for ( i=0; i<24; i++ ){
		yb_touroku_data.name[ i ] = yb_touroku_data20[ 1 ].name[ i ];	
	}
	
	return ercd;
	
}

#if ( VA300S == 1 || VA300S == 2 )	
/*==========================================================================*/
/*
 *	LCD�\���p�̃p�����[�^�ɓo�^�f�[�^���Z�b�g����
 */
/*==========================================================================*/
static ER set_reg_param_for_Lcd( void )
{
	ER ercd = E_OK;
	int i, j;
	unsigned short uw_dat0, uw_dat1000, uw_dat100, uw_dat10, uw_dat1;
	UB ubtmp;

	// �ً}�J���d�b�ԍ�
	kinkyuu_tel_no[0] = '0';
	kinkyuu_tel_no[1] = '8';
	kinkyuu_tel_no[2] = '0';
	kinkyuu_tel_no[3] = '0';
	kinkyuu_tel_no[4] = 0x20;
	kinkyuu_tel_no[5] = 0x20;
	kinkyuu_tel_no[6] = 0x20;
	kinkyuu_tel_no[7] = 0x20;
	kinkyuu_tel_no[8] = '1';
	kinkyuu_tel_no[9] = '7';
	kinkyuu_tel_no[10] = '0';
	kinkyuu_tel_no[11] = 0x20;
	kinkyuu_tel_no[12] = '3';
	kinkyuu_tel_no[13] = '1';
	kinkyuu_tel_no[14] = '3';
	kinkyuu_tel_no[15] = '1';
	kinkyuu_tel_no[16] = ',';

	//20140925Miya add mainte
	for( i = 0 ; i < 16 ; i++ ){
		kinkyuu_tel_no[i] = g_RegUserInfoData.MainteTelNum[i];
	}
	kinkyuu_tel_no[16] = ',';

	// ���ԍ�
	uw_dat0 = g_RegUserInfoData.BlockNum;
	uw_dat10 = uw_dat0 / 10;
	uw_dat1  = uw_dat0 - 10 * uw_dat10;
	yb_touroku_data.tou_no[ 0 ] = (UB)uw_dat10 + 0x30;
	yb_touroku_data.tou_no[ 1 ] = (UB)uw_dat1 + 0x30;
	yb_touroku_data.tou_no[ 2 ]	= ',';
	// ���[�U�[ID
	uw_dat0 = g_RegUserInfoData.UserId;
	uw_dat1000 = uw_dat0 / 1000;
	uw_dat100  = (uw_dat0 - 1000 * uw_dat1000) / 100;
	uw_dat10   = (uw_dat0 - 1000 * uw_dat1000 - 100 * uw_dat100) / 10;
	uw_dat1    = (uw_dat0 - 1000 * uw_dat1000 - 100 * uw_dat100 - 10 * uw_dat10);
	yb_touroku_data.user_id[ 0 ] = (UB)uw_dat1000 + 0x30;	
	yb_touroku_data.user_id[ 1 ] = (UB)uw_dat100 + 0x30;
	yb_touroku_data.user_id[ 2 ] = (UB)uw_dat10 + 0x30;
	yb_touroku_data.user_id[ 3 ] = (UB)uw_dat1 + 0x30;
	yb_touroku_data.user_id[ 4 ] = ',';

	//for( j = 0 ; j < 21 ; j++ ){	//�@��M�z���񂪁A�Q�O�񕪁��Q�P�񕪂ɕύX�B2013.7.15
	for( j = 0 ; j < 20 ; j++ ){	//�@VA300S�ł́A��M�z���񂪁A�Q�O�� 2014.01.31
		// �ӔC��/��ʎ҂̓o�^�w���
		if( g_RegBloodVesselTagData[j].RegInfoFlg == 1){
			uw_dat0 = g_RegBloodVesselTagData[j].RegNum + 1;
		}else{
			uw_dat0 = 0;
		}
		uw_dat100 = uw_dat0 / 100;
		uw_dat10   = (uw_dat0 - 100 * uw_dat100) / 10;
		uw_dat1    = (uw_dat0 - 100 * uw_dat100 - 10 * uw_dat10);
		yb_touroku_data20[j+1].yubi_seq_no[ 0 ]	= (UB)uw_dat100 + 0x30;	
		yb_touroku_data20[j+1].yubi_seq_no[ 1 ]	= (UB)uw_dat10 + 0x30;
		yb_touroku_data20[j+1].yubi_seq_no[ 2 ]	= (UB)uw_dat1 + 0x30;
		yb_touroku_data20[j+1].yubi_seq_no[ 3 ]	= ',';

		// �ӔC��/��ʎҋ敪�A�f�z���g�́h��ʎҁh
		uw_dat0 = g_RegBloodVesselTagData[j].Level;
		uw_dat1 = uw_dat0;
		yb_touroku_data20[j+1].kubun[ 0 ]			= (UB)uw_dat1 + 0x30;
		yb_touroku_data20[j+1].kubun[ 1 ]			= ',';

		// �o�^�w�ԍ��i�w��ʁj
		uw_dat0 = g_RegBloodVesselTagData[j].RegFinger;
		uw_dat10 = uw_dat0 / 10;
		uw_dat1  = uw_dat0 - 10 * uw_dat10;
		yb_touroku_data20[j+1].yubi_no[ 0 ]		= (UB)uw_dat10 + 0x30;	
		yb_touroku_data20[j+1].yubi_no[ 1 ]		= (UB)uw_dat1 + 0x30;
		yb_touroku_data20[j+1].yubi_no[ 2 ]		= ',';
		
		for ( i=0; i<24; i++ ){
			yb_touroku_data20[j+1].name[ i ] = g_RegBloodVesselTagData[j].Name[i];	
		}
	}
	// �ӔC��/��ʎ҂̓o�^�w���
	yb_touroku_data.yubi_seq_no[ 0 ]	= yb_touroku_data20[ 1 ].yubi_seq_no[ 0 ];	
	yb_touroku_data.yubi_seq_no[ 1 ]	= yb_touroku_data20[ 1 ].yubi_seq_no[ 1 ];
	yb_touroku_data.yubi_seq_no[ 2 ]	= yb_touroku_data20[ 1 ].yubi_seq_no[ 2 ];
	yb_touroku_data.yubi_seq_no[ 3 ]	= ',';
	// �ӔC��/��ʎҋ敪�A�f�z���g�́h��ʎҁh
	yb_touroku_data.kubun[ 0 ]			= yb_touroku_data20[ 1 ].kubun[ 0 ];	
	yb_touroku_data.kubun[ 1 ]			= ',';
	// �o�^�w�ԍ��i�w��ʁj
	yb_touroku_data.yubi_no[ 0 ]		= yb_touroku_data20[ 1 ].yubi_no[ 0 ];	
	yb_touroku_data.yubi_no[ 1 ]		= yb_touroku_data20[ 1 ].yubi_no[ 1 ];
	yb_touroku_data.yubi_no[ 2 ]		= ',';

	for ( i=0; i<24; i++ ){
		yb_touroku_data.name[ i ] = yb_touroku_data20[ 1 ].name[ i ];	
	}
	
	return ercd;
	
}





/*==========================================================================*/
/**
 *	�V�X�e���E�������[�h�A�J�����E�p�����[�^�A�摜�����p�����[�^�A
 *   LED���ʁA�o�^�f�[�^�A�Ȃǂ̊e��p�����[�^�̏��������s���B�j
 */
/*==========================================================================*/
static ER set_initial_param( void )
{
	ER ercd = E_OK;

	g_DipSwCode = DswGet();

	//ercd = InitFlBkAuthArea();//
	//ercd = ReadBkAuthData();	//20141003Miya del power_on_process()�̐擪�ֈړ�

	if( GetSysSpec() == SYS_SPEC_KOUJIM || GetSysSpec() == SYS_SPEC_KOUJIO || GetSysSpec() == SYS_SPEC_KOUJIS ){
		MdCngMode( MD_INITIAL );
		set_initial_param_for_Lcd();
		Pfail_mode_count = 0;
	}else{

#if(KOUJYOUCHK)
		g_RegUserInfoData.RegSts = 1;	//20160601Miya forDebug
		dip_sw_data[0] = 1;
#endif

//#if(AUTHTEST >= 2)	//20160613Miya
//#if(AUTHTEST >= 1)	//20160613Miya
#if(AUTHTEST >= 1 || PCCTRL == 1)	//20160930Miya PC����VA300S�𐧌䂷��
		if( g_RegUserInfoData.RegSts == 0 ){	//�o�^�f�[�^�Ȃ�
			ercd = InitRegImgArea();
			g_RegUserInfoData.RegSts = 1;	//20160601Miya forDebug
			SaveBkAuthDataFl();
		}
#endif

		if( g_RegUserInfoData.RegSts == 0 ){	//�o�^�f�[�^�Ȃ�
			ercd = InitRegImgArea();
			MdCngMode( MD_INITIAL );
			set_initial_param_for_Lcd();
			Pfail_mode_count = 0;
		}else{									//�o�^�f�[�^����
			ercd = ReadRegImgArea(0);
			ercd = ReadRegImgArea(10);
			ercd = AddRegImgFromRegImg(0, 0);		//20160312Miya �ɏ����xUP
			MdCngMode( MD_NORMAL );	
			set_reg_param_for_Lcd();
			Pfail_mode_count = 0;

#if(AUTHTEST >= 1)	//20160613Miya
			ReadTestRegImgCnt();
			g_sv_okcnt = 0;
			g_sv_okcnt0 = g_sv_okcnt1 + g_sv_okcnt2 + g_sv_okcnt3 + g_sv_okcnt4;
			g_imgsv_f = 0;
#endif	

		}
	}
	
	//MdCngMode( MD_INITIAL );		// �V�X�e���E�������[�h���AINITIAL�ցB
	
	g_AuthOkCnt = 0;
	CmrReloadFlg = 0;

	g_AuthType = 0;	//20160120Miya
	ode_oru_sw = 0;	//20160120Miya
	
	dbg_ts_flg = 0;
	dbg_cam_flg = 0;
	dbg_nin_flg = 0;
	dbg_cap_flg = 0;

	DBG_send_cnt = 0;
	
	ercd = lan_get_Pfail_cnt( &Pfail_mode_count );		// VA300S�@��d���N���J��Ԃ��J�E���^�̓��e���ALAN�p EEPROM����ǂݏo���B
	if ( Pfail_sense_flg == PFAIL_SENSE_ON ){		// Test�R�}���h�ŁA��d������M�H
		Pfail_mode_count++;
		ercd = lan_set_eep( EEP_PFAIL_CNT, Pfail_mode_count );	// EEPROM�փC���N�������g�����J�E���^�̓��e�������݁B
		
		if ( MdGetMode() != MD_INITIAL ){			// �������[�h�ȊO�̏ꍇ�́A��d���[�h�ֈڍs�B
			MdCngMode( MD_PFAIL );
			Pfail_sense_flg = PFAIL_SENSE_NO;		// ��d���[�h�ڍs��́A����Box����̒�d�ʒm�t���O��OFF����B
													// ��d�ʒm�t���O��ON�����܂܂��ƁA��������SHUTDown�����ɓ����Ă��܂��ׁB
		}
		
	} else {										// Test�R�}���h�ŁA����d������������M
		Pfail_mode_count = 0;						// ��d���[�h�ł̋N���񐔂��������B
		ercd = lan_set_eep( EEP_PFAIL_CNT, Pfail_mode_count );	// EEPROM�փC���N�������g�����J�E���^�̓��e�������݁B
		dly_tsk( 100/MSEC );
	}
	
	door_open_over_time = 30;		// 	�h�A�ߊJ�����ԃf�z���g�i30�b�j

	g_pcproc_f = 0;	//20160930Miya PC����VA300S�𐧌䂷��
	g_capallow = 0; //20160930Miya PC����VA300S�𐧌䂷��

	g_LedCheck = 0;	//20161031Miya Ver2204

	return ercd;
	
}
#endif


//20161031Miya Ver2204 LCDADJ
static void LcdPosAdj(int calc)
{
	if(calc == 0){
		BaseT.tPos[0].iX = 40;
		BaseT.tPos[0].iY = 40;
		BaseT.tPos[1].iX = 432;
		BaseT.tPos[1].iY = 232;
		BaseT.tPos[2].iX = 432;
		BaseT.tPos[2].iY = 40;

		if(g_BkDataNoClear.LcdAdjFlg != 0x1234){
			InputT.tPos[0].iX = 40;
			InputT.tPos[0].iY = 40;
			InputT.tPos[1].iX = 432;
			InputT.tPos[1].iY = 232;
			InputT.tPos[2].iX = 432;
			InputT.tPos[2].iY = 40;
		}else{
			InputT.tPos[0].iX = g_BkDataNoClear.LcdAdjInputX[0];
			InputT.tPos[0].iY = g_BkDataNoClear.LcdAdjInputY[0];
			InputT.tPos[1].iX = g_BkDataNoClear.LcdAdjInputX[1];
			InputT.tPos[1].iY = g_BkDataNoClear.LcdAdjInputY[1];
			InputT.tPos[2].iX = g_BkDataNoClear.LcdAdjInputX[2];
			InputT.tPos[2].iY = g_BkDataNoClear.LcdAdjInputY[2];
		}
	}else{
		BaseT.tPos[0].iX = 40;
		BaseT.tPos[0].iY = 40;
		BaseT.tPos[1].iX = 432;
		BaseT.tPos[1].iY = 232;
		BaseT.tPos[2].iX = 432;
		BaseT.tPos[2].iY = 40;

		InputT.tPos[0].iX = g_lcdpos[0][0];
		InputT.tPos[0].iY = g_lcdpos[0][1];
		InputT.tPos[1].iX = g_lcdpos[1][0];
		InputT.tPos[1].iY = g_lcdpos[1][1];
		InputT.tPos[2].iX = g_lcdpos[2][0];
		InputT.tPos[2].iY = g_lcdpos[2][1];
		
		g_BkDataNoClear.LcdAdjFlg = 0x1234;
		g_BkDataNoClear.LcdAdjBaseX[0] = BaseT.tPos[0].iX;
		g_BkDataNoClear.LcdAdjBaseX[1] = BaseT.tPos[1].iX;
		g_BkDataNoClear.LcdAdjBaseX[2] = BaseT.tPos[2].iX;
		g_BkDataNoClear.LcdAdjBaseY[0] = BaseT.tPos[0].iY;
		g_BkDataNoClear.LcdAdjBaseY[1] = BaseT.tPos[1].iY;
		g_BkDataNoClear.LcdAdjBaseY[2] = BaseT.tPos[2].iY;
		g_BkDataNoClear.LcdAdjInputX[0] = InputT.tPos[0].iX;
		g_BkDataNoClear.LcdAdjInputX[1] = InputT.tPos[1].iX;
		g_BkDataNoClear.LcdAdjInputX[2] = InputT.tPos[2].iX;
		g_BkDataNoClear.LcdAdjInputY[0] = InputT.tPos[0].iY;
		g_BkDataNoClear.LcdAdjInputY[1] = InputT.tPos[1].iY;
		g_BkDataNoClear.LcdAdjInputY[2] = InputT.tPos[2].iY;
	}

	TplRevCalc(&BaseT, &InputT, &RevT);
	TplRevSet(RevT.fXx, RevT.fXy, RevT.fXofs, RevT.fYx, RevT.fYy, RevT.fYofs);
}


#if ( VA300S == 0 || VA300S == 2 ) 
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
		
			Pfail_mode_count = 0;	// ��d���[�h�̏ꍇ�̋N����
			
			break;
			
		case MD_MAINTE:			///< �����e�i���X���[�h	

			ercd = send_meinte_mode_Wait_Ack_Retry();
			
			Pfail_mode_count = 0;	// ��d���[�h�̏ꍇ�̋N����

	    	break;
			
		case MD_NORMAL:			///< �ʏ탂�[�h

			ercd = send_nomal_mode_Wait_Ack_Retry();
			
			Pfail_mode_count = 0;	// ��d���[�h�̏ꍇ�̋N����

	    	break;
			
		case MD_PFAIL:			///< ��d�����샂�[�h
		
			ercd = send_Pfail_mode_Wait_Ack_Retry();
			
	    	break;
			
		case MD_PANIC:			///< ��펞�J�����[�h
		
			Pfail_mode_count = 0;	// ��d���[�h�̏ꍇ�̋N����
			
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
//	���[�h�ؑ֒ʒm�̑��M�AAck�ENack�҂��ƃ��g���C�t���i��d���[�h�ڍs���j
/*==========================================================================*/
static ER send_Pfail_mode_Wait_Ack_Retry( void )
{
	UINT Retry = 0;
	ER ercd;
	
	while ( Retry <= 3 ){
		ercd = send_Pfail_mode();			// ���[�h�ؑ֒ʒm�̑��M�i��d���[�h�ڍs���j
		if ( ercd == E_OK ){				// Ack��������
			MdCngSubMode( SUB_MD_CHG_PFAIL );	//�@�T�u���[�h�E�X�e�[�^�X���A�h��d���[�h�ֈڍs���h�ցB
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
 *	���[�h�ؑ֒ʒm�̑��M�i��d���[�h�ڍs���j
 */
/*==========================================================================*/
static ER send_Pfail_mode( void )
{
	ER ercd = E_TMOUT;
	int count;
	char com_data[ 15 ];
	
	/** �R�}���h�f�[�^�̏���	**/
	com_data[ 0 ] = 0x23;			//�@�w�b�_�@1Byte�@ASCII
	com_data[ 1 ] = 0;				//�@�f�[�^���@�Q�o�C�gBinary
	com_data[ 2 ] = 0x0d;
	com_data[ 3 ] = 0x31;			//�@���M����ʁ@1Byte�@ASCII
	com_data[ 4 ] = '4';			//�@�R�}���h�ԍ��@�R��ASCII
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
//	�V���b�g�_�E���v���̑��M�AAck�ENack�҂��ƃ��g���C�t��
/*==========================================================================*/
static ER send_shutdown_req_Wait_Ack_Retry( void )
{
	UINT Retry = 0;
	ER ercd;
	
	while ( Retry <= 3 ){
		ercd = send_shutdown_req();			// �V���b�g�_�E���v���̑��M
		if ( ercd == E_OK ){				// Ack��������
			MdCngSubMode( SUB_MD_REQ_SHUTDOWN );	//�@�T�u���[�h�E�X�e�[�^�X���A�h�V���b�g�_�E���v�����h�ցB
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
 *	�V���b�g�_�E���v���̑��M
 */
/*==========================================================================*/
static ER send_shutdown_req( void )
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
	com_data[ 5 ] = '5';
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

	if ( GetSysSpec() == SYS_SPEC_OFFICE ){ //  �P�΂P�F�؎d�l�̏ꍇ
		com_data[ 19 ] = ' ';				//�@�ӔC�ҁ^��ʎ҂̓o�^�w���i�w�o�^�ԍ��O�`�P�O�O�j
		com_data[ 20 ] = ' ';
		com_data[ 21 ] = ' ';
		
	} else { 								//  �}���V����(��L��)�d�l�̏ꍇ
		com_data[ 19 ] = yb_touroku_data.yubi_seq_no[ 0 ];	//�@�ӔC�ҁ^��ʎ҂̓o�^�w���i�w�o�^�ԍ��O�`�P�O�O�j
		com_data[ 20 ] = yb_touroku_data.yubi_seq_no[ 1 ];
		com_data[ 21 ] = yb_touroku_data.yubi_seq_no[ 2 ];		
	}

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
//	ID�ԍ��⍇���̑��M�AAck�ENack�҂��ƃ��g���C�t��
/*==========================================================================*/
static ER send_ID_No_check_req_Wait_Ack_Retry( void )
{
	UINT Retry = 0;
	ER ercd;
	
	while ( Retry <= 3 ){
		ercd = send_ID_No_check_req();			// ID�ԍ��⍇���̑��M
		if ( ercd == E_OK ){				// Ack��������
			nop();
			MdCngSubMode( SUB_MD_ID_NO_CHECK_REQ );	//�@�T�u���[�h�E�X�e�[�^�X���A�hID�ԍ��⍇�������V�[�P���X���h�ցB
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
 *	ID�ԍ��⍇���̑��M�i�ʏ탂�[�h���j
 */
/*==========================================================================*/
static ER send_ID_No_check_req( void )
{
	ER ercd = E_TMOUT;
	int count;
	char com_data[ 24 ];
	
	/** �R�}���h�f�[�^�̏���	**/
	com_data[ 0 ] = 0x23;			//�@�w�b�_�@1Byte�@ASCII
	com_data[ 1 ] = 0;				//�@�f�[�^���@�Q�o�C�gBinary
	com_data[ 2 ] = 0x13;
	com_data[ 3 ] = 0x31;			//�@���M����ʁ@1Byte�@ASCII
	com_data[ 4 ] = '2';			//�@�R�}���h�ԍ��@�R��ASCII
	com_data[ 5 ] = '1';
	com_data[ 6 ] = '4';
	com_data[ 7 ] = '0';			//�@�u���b�N�ԍ� �S��ASCII
	com_data[ 8 ] = '0';
	com_data[ 9 ] = '0';
	com_data[ 10 ] = '0';
	
	//�@�`���f�[�^�@
	com_data[ 11 ] = yb_touroku_data.tou_no[ 0 ];	//�@�����ԍ�
	com_data[ 12 ] = yb_touroku_data.tou_no[ 1 ];
	com_data[ 13 ] = ',';

	com_data[ 14 ] = yb_touroku_data.user_id[ 0 ];	//�@���[�U�[ID
	com_data[ 15 ] = yb_touroku_data.user_id[ 1 ];
	com_data[ 16 ] = yb_touroku_data.user_id[ 2 ];
	com_data[ 17 ] = yb_touroku_data.user_id[ 3 ];

	com_data[ 18 ] = CODE_CR;		//�@�I�[�R�[�h
	com_data[ 19 ] = CODE_LF;
	
	/** �R�}���h���M	**/
	SendBinaryData( &com_data, ( 18 + 2 ) );
	
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
//	ID�����⍇���̑��M�AAck�ENack�҂��ƃ��g���C�t��
/*==========================================================================*/
static ER send_ID_Authority_check_req_Wait_Ack_Retry( void )
{
	UINT Retry = 0;
	ER ercd;
	
	while ( Retry <= 3 ){
		ercd = send_ID_Authority_check_req();			// ID�ԍ��⍇���̑��M
		if ( ercd == E_OK ){				// Ack��������
			nop();
			MdCngSubMode( SUB_MD_ID_AUTHORITY_CHECK_REQ );	//�@�T�u���[�h�E�X�e�[�^�X���A�hID�����⍇�������V�[�P���X���h�ցB
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
 *	ID�����⍇���̑��M�i�ʏ탂�[�h���j
 */
/*==========================================================================*/
static ER send_ID_Authority_check_req( void )
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
	com_data[ 5 ] = '1';
	com_data[ 6 ] = '5';
	com_data[ 7 ] = '0';			//�@�u���b�N�ԍ� �S��ASCII
	com_data[ 8 ] = '0';
	com_data[ 9 ] = '0';
	com_data[ 10 ] = '0';
	
	//�@�`���f�[�^�@
	com_data[ 11 ] = yb_touroku_data.tou_no[ 0 ];	//�@�����ԍ�
	com_data[ 12 ] = yb_touroku_data.tou_no[ 1 ];
	com_data[ 13 ] = ',';

	com_data[ 14 ] = yb_touroku_data.user_id[ 0 ];	//�@���[�U�[ID
	com_data[ 15 ] = yb_touroku_data.user_id[ 1 ];
	com_data[ 16 ] = yb_touroku_data.user_id[ 2 ];
	com_data[ 17 ] = yb_touroku_data.user_id[ 3 ];
	com_data[ 18 ] = ',';
	
	if ( GetScreenNo() == LCD_SCREEN522 ){
		com_data[ 19 ] = ' ';		// �Ǘ��ҋ敪<--�@�s��B	
	} else if ( GetScreenNo() == LCD_SCREEN543 ){
		com_data[ 19 ] = ' ';		// �Ǘ��ҋ敪<--�@�s��B
	} else if ( GetScreenNo() == LCD_SCREEN547 ){
		com_data[ 19 ] = s_ID_Authority_Level;  		// �Ǘ��ҋ敪<--�@�R�}���h216�@�œ����Ǘ��ҋ敪��Set�B
	} else {						// �����ɗ��鎞�́A�v���O�����~�X�B
		com_data[ 19 ] = yb_touroku_data.kubun[ 0 ];	//�@�Ǘ��ҋ敪		
	}

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
									//  �d�l��ʂ�ݒ�
	if ( GetSysSpec() == SYS_SPEC_MANTION ){				// �}���V�����E��L���d�l
		if ( sys_demo_flg != SYS_SPEC_DEMO ){
			com_data[ 12 ] = '0';
		}	else {
			com_data[ 12 ] = '3';	//	�f���d�l�̏ꍇ
		}
	} else if ( GetSysSpec() == SYS_SPEC_OFFICE ){			// �P�΂P�d�l�i�I�t�B�X�d�l�j
		if ( sys_demo_flg != SYS_SPEC_DEMO ){
			com_data[ 12 ] = '1';
		}	else {
			com_data[ 12 ] = '4';	//	�f���d�l�̏ꍇ
		}	
	} else if ( GetSysSpec() == SYS_SPEC_ENTRANCE ){		// �}���V�����E���p���d�l
		if ( sys_demo_flg != SYS_SPEC_DEMO ){
			com_data[ 12 ] = '2';
		}	else {
			com_data[ 12 ] = '5';	//	�f���d�l�̏ꍇ
		}	
	} else {
		com_data[ 12 ] = '0';		// �d�l�ݒ肪�s��̏ꍇ�́A�}���V�����E��L���d�l
	}
	
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

#endif


/*==========================================================================*/
/**
 *	VA300�@�d�l�ؑւ��t���O�̓��e���AEEPROM����ǂݏo���B
 */
/*==========================================================================*/
ER lan_get_sys_spec( UB *pval )	
{
	ER ercd;
	UB dat, dat2;
	
	ercd = lan_get_eep( EEP_SYSTEM_SPEC, &dat );
	if ( ercd != E_OK ) {
		return ercd;
	}

	ercd = lan_get_eep( EEP_SYSTEM_SPEC, &dat2 );
	if ( ercd != E_OK ) {
		return ercd;
	}
	
	if ( dat == dat2 ){
		*pval = dat;
	}

	return ercd;
}

#if ( VA300S == 1 || VA300S == 2 )
/*==========================================================================*/
/**
 *	VA300S�@��d�J��Ԃ��J�E���^�̓��e���AEEPROM����ǂݏo���B
 */
/*==========================================================================*/
ER lan_get_Pfail_cnt( UB *pval )	
{
	ER ercd;
	UB dat, dat2;
	
	ercd = lan_get_eep( EEP_PFAIL_CNT, &dat );
	if ( ercd != E_OK ) {
		return ercd;
	}

	ercd = lan_get_eep( EEP_PFAIL_CNT, &dat2 );
	if ( ercd != E_OK ) {
		return ercd;
	}
	
	if ( dat == dat2 ){
		*pval = dat;
	}

	return ercd;
}
#endif


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
	
	Pfail_mode_count = 0;			// ��d���[�h�̏ꍇ�̋N����(�����[�h�ŋN�������ꍇ�́Areset�����B)
	Pfail_sense_flg = PFAIL_SENSE_NO;			// ��d���m�t���O�@OFF

//	SUB_MD_SETTING		// �ŐV�̑��u�T�u���[�h

	g_CapTimes = 0;	//20131210Miya add
	
	WDT_counter = 0;				// WDT�^�C�}�[�p�T�C�N���b�N�E�J�E���^
	main_TSK_wdt = FLG_ON;			// main�^�X�N�@WDT�J�E���^���Z�b�g�E���N�G�X�g�E�t���O
	camera_TSK_wdt = FLG_ON;		// �J�����^�X�N�@WDT�J�E���^���Z�b�g�E���N�G�X�g�E�t���O
	ts_TSK_wdt = FLG_ON;			// �^�b�`�Z���T�E�^�X�N�@WDT�J�E���^���Z�b�g�E���N�G�X�g�E�t���O
	lcd_TSK_wdt = FLG_ON;			// LCD�^�X�N�@WDT�J�E���^���Z�b�g�E���N�G�X�g�E�t���O
	sio_rcv_TSK_wdt = FLG_ON;		// SIO��M�^�X�N�@WDT�J�E���^���Z�b�g�E���N�G�X�g�E�t���O
	sio_snd_TSK_wdt = FLG_ON;		// SIO���M�^�X�N�@WDT�J�E���^���Z�b�g�E���N�G�X�g�E�t���O
	
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
 *	�}���V�����d�l/�P�΂P�d�l�@��Ԃ�Ԃ��B�@
 *	@return �@�@0:�}���V����(��L��)�d�l�A1:1�΂P�d�l�B
 */
/*==========================================================================*/
static UB GetSysSpec( void )
{
	return sys_kindof_spec;
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

#if ( NEWCMR == 1 )		//20160610Miya
	return;
#endif


//20150930Miya
	CmrReloadFlg = 1;

	cmrGain = ini_cmrGain;						// �J�����E�Q�C���l�����l�@
	cmrFixShutter1 = ini_cmrFixShutter1;		// Fix Shutter Control�l�����l(�P���)�@
	cmrFixShutter2 = ini_cmrFixShutter2;		// Fix Shutter Control�l�����l(�Q���)			����
	cmrFixShutter3 = ini_cmrFixShutter3;		// Fix Shutter Control�l�����l(�R��ځj

	irDuty2 = ini_irDuty2;		// IR Duty�lirDuty2�@�����l;
	irDuty3 = ini_irDuty3;
	irDuty4 = ini_irDuty4;
	irDuty5 = ini_irDuty5;

	ercd = set_flg( ID_FLG_CAMERA, FPTN_REROAD_PARA );
	if ( ercd != E_OK ){
		ErrCodeSet( ercd );
	}
	
	return;

//	ercd = set_flg( ID_FLG_CAMERA, FPTN_CMR_WKAEUP );	// �J����WeakUP //20140930Miya Bio FPGA	
//	if ( ercd != E_OK ){
//		ErrCodeSet( ercd );
//	}
	
	cmrGain = ini_cmrGain;						// �J�����E�Q�C���l�����l�@
	cmrFixShutter1 = ini_cmrFixShutter1;		// Fix Shutter Control�l�����l(�P���)�@
	cmrFixShutter2 = ini_cmrFixShutter2;		// Fix Shutter Control�l�����l(�Q���)			����
	cmrFixShutter3 = ini_cmrFixShutter3;		// Fix Shutter Control�l�����l(�R��ځj

	//dly_tsk( 500/MSEC );
	ercd = set_flg( ID_FLG_CAMERA, FPTN_SETREQ_GAIN );	// �J�����^�X�N�ɁA���ڂ̃Q�C���ݒ�l��ݒ���˗��iFPGA���o�R���Ȃ��j	
	if ( ercd != E_OK ){
		ErrCodeSet( ercd );
	}
		
	irDuty2 = ini_irDuty2;		// IR Duty�lirDuty2�@�����l;
	irDuty3 = ini_irDuty3;
	irDuty4 = ini_irDuty4;
	irDuty5 = ini_irDuty5;

	dly_tsk( 500/MSEC );
	ercd = set_flg( ID_FLG_CAMERA, FPTN_SETREQ_SHUT1 );	// �J�����^�X�N�ɁA���ڂ̘I�o�R�ݒ�l��ݒ���˗��iFPGA���o�R���Ȃ��j	
	if ( ercd != E_OK ){
		ErrCodeSet( ercd );
	}

	dly_tsk( 500/MSEC );
	ercd = CmrCmdSleep();	//20150930Miya	
	
//	dly_tsk( 500/MSEC );
//	ercd = set_flg( ID_FLG_CAMERA, FPTN_CMR_SLEEP );	// �J����Sleep //20140930Miya Bio FPGA	
//	if ( ercd != E_OK ){
//		ErrCodeSet( ercd );
//	}
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


/*********************************
 * Initialize peripherals on evaluation board�i�� Please customize !!�j
 *
 ********************************/
void init_peripheral(void)
{
	
}

/*********************************
 * Initialize port�i�� Please customize !!�j
 *
 ********************************/

void ini_pio(void)
{
	sfr_outw(BSC_GPIOIC,0x0000);
	sfr_outw(BSC_PDTRB, 0x0000);		// 
	sfr_outl(BSC_PCTRB, 0x00000014);	// PORT17,18(TEST Pin1,2)�o��
	
	TP_CLR(1);							// �e�X�g�s��1������
	TP_CLR(2);							// �e�X�g�s��2������
	
}

/*********************************
 * Initialize interrupt�i�� Please customize !!�j
 *
 ********************************/

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


/*********************************
 * IP Address �̓���
 *
 * @retval TRUE �Ǐo������
 * @retval FALSE �Ǐo�����s
 ********************************/

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


/*******************************************************************
* WDT(�E�H�b�`�h�b�O�E�^�C�}�[)�̏����ݒ�
*
********************************************************************/
static void init_wdt( void )
{
	int tmp;
	
	// �E�H�b�`�h�b�O�^�C�}�J�E���^�E���W�X�^�@������
	reset_wdtc();
	
	// WDT�R���g���[���E�X�e�[�^�X�E���W�X�^�@������
	tmp = 0xa5cf;					// 0xa5 : �������݃R�}���h�E�f�[�^
									// bit7 : TME =1�A�J�E���g�A�b�v�J�n
									// bit6 : WT/IT =1�AWDT���[�h
									// bit5 : RSTS = 0�A�p���[�I���E���Z�b�g�w�� 
									// bit4 : WOVF = 1�AWDT���[�h���I�[�o�[�t���[
									// bit3 : IOVF = 1�A�C���^�[�o���^�C�}�[���[�h���I�[�o�[�t���[
									// bit2-0 : �N���b�N�Z���N�g�@111�̎��@������1/4096�A�I�[�o�[�t���[����5.25ms						 
	sfr_outw( CPG_WTCSR, tmp );
 
}

/*******************************************************************
* WDT(�E�H�b�`�h�b�O�E�^�C�}�[)�̖������ݒ�
*
********************************************************************/
static void stop_wdt( void )
{
	int tmp;
	
	// �E�H�b�`�h�b�O�^�C�}�J�E���^�E���W�X�^�@������
	reset_wdtc();
	
	// WDT�R���g���[���E�X�e�[�^�X�E���W�X�^�@������
	tmp = 0xa54f;					// 0xa5 : �������݃R�}���h�E�f�[�^
									// bit7 : TME =0�A�J�E���g�A�b�v��~
									// bit6 : WT/IT =1�AWDT���[�h
									// bit5 : RSTS = 0�A�p���[�I���E���Z�b�g�w�� 
									// bit4 : WOVF = 1�AWDT���[�h���I�[�o�[�t���[
									// bit3 : IOVF = 1�A�C���^�[�o���^�C�}�[���[�h���I�[�o�[�t���[
									// bit2-0 : �N���b�N�Z���N�g�@111�̎��@������1/4096�A�I�[�o�[�t���[����5.25ms						 
	sfr_outw( CPG_WTCSR, tmp );
 
}

/*******************************************************************
* WDT(�E�H�b�`�h�b�O�E�^�C�}�[)�J�E���^�̃N���A
*
********************************************************************/
static void reset_wdtc( void )
{
	int tmp;
	
	// �E�H�b�`�h�b�O�^�C�}�J�E���^�E���W�X�^�@�N���A
	tmp = 0x5a00;
	sfr_outw( CPG_WTCNT, tmp );
	
}

/*******************************************************************
* WDT�J�E���^�p�������̃��Z�b�g����
*	�e�^�X�N�����WDT���Z�b�g�E���N�G�X�g���S��AND�Ő��������ꍇ�ɁAWDT�J�E���^���N���A����B
********************************************************************/
static void reset_wdtmem( void )
{
	if ( ( main_TSK_wdt == FLG_ON ) &&			// main�^�X�N�@WDT���Z�b�g�E���N�G�X�g�E�t���O
		 ( camera_TSK_wdt == FLG_ON ) &&		// �J�����^�X�N�@WDT���Z�b�g�E���N�G�X�g�E�t���O
		 ( ts_TSK_wdt == FLG_ON ) &&			// �^�b�`�Z���T�E�^�X�N�@WDT���Z�b�g�E���N�G�X�g�E�t���O
		 ( lcd_TSK_wdt == FLG_ON ) &&			// LCD�^�X�N�@WDT���Z�b�g�E���N�G�X�g�E�t���O
		 ( sio_rcv_TSK_wdt == FLG_ON ) &&		// SIO��M�^�X�N�@WDT���Z�b�g�E���N�G�X�g�E�t���O
		 ( sio_snd_TSK_wdt == FLG_ON ) ){		// SIO���M�^�X�N�@WDT���Z�b�g�E���N�G�X�g�E�t���O
		 
	     WDT_counter = 0;						// WDT�J�E���^�p������(20�b��WDT�N��)���N���A�B
		 
#if( FREEZTEST )
		 main_TSK_wdt = FLG_ON;				// �e�^�X�N��WDT���Z�b�g�E���N�G�X�g�E�t���O��OFF�B
		 camera_TSK_wdt = FLG_ON;
		 ts_TSK_wdt = FLG_ON;
		 lcd_TSK_wdt = FLG_ON;
		 sio_rcv_TSK_wdt = FLG_ON;
		 sio_snd_TSK_wdt = FLG_ON;
#else
		 main_TSK_wdt = FLG_OFF;				// �e�^�X�N��WDT���Z�b�g�E���N�G�X�g�E�t���O��OFF�B
		 camera_TSK_wdt = FLG_OFF;
		 ts_TSK_wdt = FLG_OFF;
		 lcd_TSK_wdt = FLG_OFF;
		 sio_rcv_TSK_wdt = FLG_ON;
		 sio_snd_TSK_wdt = FLG_ON;
#endif
	}
}

/*******************************************************************
* WDT�J�E���^�p�������̃_�C���N�g�E���Z�b�g����(�t���b�V���E�������E�h���C�o������p)
*	�t���b�V���E��������Read/Write�h���C�o�֐����ŁAWDT�J�E���^���N���A����B
*	�eTask��WDT�N���A�E�t���O�𖳎����āA�J�E���^���N���A����̂ŁA
*	�h���C�o���ł̎g�p�Ɍ���B�i���p����ƁA�^�X�N�P�ʂł�WDT�@�\�̈Ӗ����Ȃ��Ȃ�B�j
********************************************************************/
static void reset_wdt_cnt( void )
{
	WDT_counter = 0;						// WDT�J�E���^�p������(20�b��WDT�N��)���N���A�B
}

/*******************************************************************
* Cyclic Timer Routine
*
********************************************************************/
static void cycle1_hdr(void)
{
	if ( ACK_Wait_timer > 0 ) ACK_Wait_timer--;
	if ( EOT_Wait_timer > 0 ) EOT_Wait_timer--;
	if ( Rcv_chr_timer > 0 ) Rcv_chr_timer--;
//	if ( Test_1sec_timer > 0 ) Test_1sec_timer--;		// ���g�p
	if ( Pfail_start_timer > 0 ) Pfail_start_timer--;	// ���g�p
	if ( Ninshou_wait_timer > 0 ) Ninshou_wait_timer--;	// �w�F�؎��s���̉�ʑ҂����ԁi30�b�j
	
	// �����b�̌v���B�@�������Ԃ́A�[��LCD�̉��246�ݒ肩��󂯎��B
	timer_10ms++;					// 10msec ����
	if ( timer_10ms >= 100 ){	// 1�b�̌v��
		timer_10ms = 0;
		count_1sec++;

		if ( count_1sec >= 60 ){			// 1���̌v��
			count_1sec = 0;
			count_1min++;

			if ( count_1min >= 60 ){		// 1���Ԃ̌v��
				count_1min = 0;
				count_1hour++;
				
				if ( count_1hour >= 24 ){	// 24���Ԃ̌v��
					count_1hour = 0;
				}
				g_MainteLog.now_min = count_1hour;	// "��"��LCD��ʕ\���p�������ɔ��f
			}
			g_MainteLog.now_hour = count_1min;		// "��"���@�@��
		}
		g_MainteLog.now_sec = count_1sec;			// "�b"���@�@��		
	}	
	
	if ( WDT_counter++ > 3000 ){			// WDT�J�E���^�p������(30�b��WDT�N��)
		init_wdt();							// WDT�^�C�}�[�J�n�i��5.25ms��Ƀp���[ON���Z�b�g���|����j 	
	}
	reset_wdtmem();							// WDT�J�E���^�p�������̃��Z�b�g����
}


/*********************************
 * main
 *
 ********************************/

int main(void)
{
 	int _mbf_size;

   /* Initialize processor�i�� Please customize !!�j*/

	init_peripheral();
	
	/* Initialize system */

    sysini();

	ini_clk();

	ini_pio();

	/* Create Cyclic Timer */
    cre_cyc(ID_CYC_TIM, &ccyc_tim1);   /* Create Cyclic Handler (1 sec interval) */

	/* Create tasks */
    //cre_tsk(TSK_LEARN_DATA, &ctsk_learn_data);
	//cre_tsk(TSK_MAIN,      &ctsk_main);
#if ( VA300S == 0 )
	cre_tsk(TSK_CMD_LAN,   &ctsk_lancmd);
	cre_tsk(TSK_COMMUNICATE,  &ctsk_urcv);	//�d������
	cre_tsk(TSK_UDP_SEND,    &ctsk_usnd);	//UDP���M
#endif
#if ( VA300S == 1 )
//	cre_tsk(TSK_NINSHOU,  &ctsk_ninshou);	//�F�؏���
	cre_tsk(TSK_LOG,    &ctsk_log);			//Logging����
#endif
#if ( VA300S == 2 )
	cre_tsk(TSK_CMD_LAN,   &ctsk_lancmd);
	cre_tsk(TSK_COMMUNICATE,  &ctsk_urcv);	//�d������
	cre_tsk(TSK_UDP_SEND,    &ctsk_usnd);	//UDP���M

	cre_tsk(TSK_NINSHOU,  &ctsk_ninshou);	//�F�؏���
	cre_tsk(TSK_LOG,    &ctsk_log);			//Logging����
#endif
	cre_tsk(TSK_SND1,      &ctsk_snd1);
	cre_tsk(TSK_RCV1,      &ctsk_rcv1);

	cre_tsk(TSK_DISP,      &ctsk_disp);
	cre_tsk(TSK_IO,        &ctsk_io);

	/* create objects */

	cre_mpf(MPF_COM,   &cmpf_com);		/* Create fixed memory pool */
	cre_mpf(MPF_DISP,  &cmpf_disp);		/* Create fixed memory pool */
	cre_mpf(MPF_SND_SIO,   &cmpf_snd_sio);		/* Create fixed memory pool */
	
	cre_mbx(MBX_SND_SIO,   &cmbx_snd);		/* Create mail box */
	
#if ( VA300S == 0 )
	cre_mpf(MPF_LRES,  &cmpf_lres);		/* Create fixed memory pool */

	cre_mbx(MBX_CMD_LAN, &cmbx_lancmd);	/* Create mail box */
	cre_mbx(MBX_RESSND,&cmbx_ressnd);	/* Create mail box */
#endif
#if ( VA300S == 1 )
	cre_mpf(MPF_SND_NINSHOU,   &cmpf_snd_ninshou);		/* Create fixed memory pool */
	cre_mpf(MPF_LOG_DATA,   &cmpf_log_data);		/* Create fixed memory pool */
	
	cre_mbx(MBX_SND_NINSHOU, &cmbx_snd_ninshou ); /* Create mail box */
	cre_mbx(MBX_LOG_DATA, &cmbx_log_data ); 	/* Create mail box */
#endif
#if ( VA300S == 2 )
	cre_mpf(MPF_LRES,  &cmpf_lres);		/* Create fixed memory pool */

	cre_mbx(MBX_CMD_LAN, &cmbx_lancmd);	/* Create mail box */
	cre_mbx(MBX_RESSND,&cmbx_ressnd);	/* Create mail box */

	cre_mpf(MPF_SND_NINSHOU,   &cmpf_snd_ninshou);		/* Create fixed memory pool */
	cre_mpf(MPF_LOG_DATA,   &cmpf_log_data);		/* Create fixed memory pool */
	
	cre_mbx(MBX_SND_NINSHOU, &cmbx_snd_ninshou ); /* Create mail box */
	cre_mbx(MBX_LOG_DATA, &cmbx_log_data ); 	/* Create mail box */
#endif
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
	//sta_tsk(TSK_MAIN, 0);
#if TSK_LEARN_DATA_TEST_ENABLE
    TaskLearnData();
#endif

	/* Start multitask system */
	intsta();                   /* Start interval timer interrupt */
	syssta();                   /* enter into multi task */
}

/* end */
