//=============================================================================
/**
 *	VA-300プログラム
 *
 *	@file va300.c
 *	@version 1.00
 *
 *	@author OYO Electric Co.Ltd F.Saeki
 *	@date   2012/05/11
 *	@brief  メインモジュール
 *
 * #define _SCI1_USE_	SCI1で稼動
 * #define _ROM_DUMMY_	EEPROMのダミーで起動
 * #define _DEBUG_		デバッグ情報をSCI1に出力
 * #define _OTHER_		デバッグ用に作成したボード用
 *	Copyright (C) 2010, OYO Electric Corporation
 */
//=============================================================================
#define VA300

#ifndef NOFILE_VER          /* 1: NORTiサンプル付属ファイルシステム */
#define NOFILE_VER  1       /* 2: HTTPd for NORTi付属ファイルシステム */
#endif                      /* 4: 製品版 NORTi File System Ver.4 */

#ifndef NFTP                /* FTPサーバーに同時接続可能なクライアント数 */
#define NFTP        1       /* 旧 nonftpd.c では、1に固定のこと */
#endif                      /* 新 nofftpd.c では、2以上を定義可能 */

#ifndef POL                 /* LANドライバデバッグ用 */
#define POL         0       /* 割込みを使う(0), ポーリングでテスト(1) */
                            /*
                               注意:
                                 ポーリング(POL=1)の場合、ポーリングを開始する
                                 まで、ネットワークは動作しない。
                            */
#endif

#ifndef SH4
#define SH4                 /* SH4 を定義してkernel.hをincludeしてください */
#endif

#define USE_MON		0		// デバッグ・モニターを使用する/しない

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

/* サイクリックハンドラ ID */
const ID ID_CYC_TIM      = 1;   		///< タイマ用

/* UDP 通信端点 ID */

const ID ID_UDP_OYOCMD   = 1;			///< UDP レスポンス用
const ID ID_UDP_CLIENT   = 2;   		///< UDP クライアント用

/* TCP 通信端点 ID */

const ID ID_TELNETD_CEP  = 1;			///< TELNET デーモン

/* TCP 受付口 ID */

const ID ID_TELNETD_REP  = 1;			///< TELNET デーモン

/* TCP/IP コンフィグレーション */

#define TCP_REPID_MAX	4				///< TCP受付口最大個数
#define TCP_CEPID_MAX	4				///< TCP通信端点最大個数
#define UDP_CEPID_MAX	2				///< UDP通信端点最大個数
#define TCP_TSKID_TOP	TSK_MAX			///< TCP/IP用タスクID先頭
#define TCP_SEMID_TOP	SEM_MAX			///< TCP/IP用セマフォID先頭
#define TCP_MBXID_TOP	MBX_MAX			///< TCP/IP用メイルボックスID先頭
#define TCP_MPFID_TOP	MPF_MAX			///< TCP/IP用固定長メモリプールID先頭
#define ETH_QCNT		16
#define REP_QCNT		2
#define TCP_QCNT		4
#define UDP_QCNT		2

#include "nonetc.h"

/* System configuration */

#define TSKID_MAX   (TSK_MAX + TCP_TSKID_CNT)	///< タスクID上限
#define SEMID_MAX	(SEM_MAX + TCP_SEMID_CNT)	///< セマフォID上限
#define FLGID_MAX	FLG_MAX						///< イベントフラグID上限
#define MBXID_MAX	(MBX_MAX + TCP_MBXID_CNT)	///< メイルボックスID上限
#define	MBFID_MAX	MBF_MAX						///< Maximum ID for MESSAGE BUFFER
#define	PORID_MAX	1							///< Maximum ID for RENDEVOUZ PORT
#define	MPLID_MAX	1							///< Maximum ID for VALIABLE LENGTH MEMORY POOL
#define MPFID_MAX	(MPF_MAX + TCP_MPFID_CNT)	///< 固定長メモリプールID上限
#define	CYCNO_MAX	CYC_MAX						///< Maximum ID for CYCRIC HANDLER
#define	ALMNO_MAX	ALH_MAX						///< Maximum ID for ALARM HANDLER

#define TPRI_MAX	9		///<☆タスク優先度最大値

#define ISTKSZ      2048	//☆Stack size for Interval timer interrupt
#define TSTKSZ		512		//☆タイマハンドラスタックサイズ


#include "nocfg4.h"
#include "nosio.h"

#ifndef NFILE
#define NFILE       8       /* 同時オープン可能なファイル数 */
#endif

/* 制御ブロック */

T_FTP ftp[ NFTP ];						///< FTP Server 制御ブロック
T_FILE file[ NFILE ];					///< ファイル制御ブロック
T_DISK disk[ 1 ];						///< ディスクドライブ制御ブロック

T_TELNETD telnetd;						///< TELNETD 制御ブロック

/* RAM Diskで使用するアドレスとサイズ */

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

//2013.05.08 Miya メッセージNGのため共用メモリで対応する
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

void SystemParamInit( void );	// VA300 システム制御パラメータのInitial
static UB GetScreenNo(void);			// 現在表示スクリーン番号の取得
static void ChgScreenNo( UB NewScreenNo );	// 画面遷移状態(番号)を更新する
ER NextScrn_Control( void );	// 次の画面コントロール
ER static power_on_process( void );		// PCとのコミュニケーション開始、初期値設定要求
ER static send_WakeUp( void );	/** PCへWakeUp確認送信	**/
ER static send_camera_param_req( void );	/** カメラ・パラメータの初期値要求の送信	**/
ER static send_cap_param_req( void );		/** 画像処理の初期値要求の送信	**/	
ER static send_Led_param_req( void );		/** LED光量数値の初期値要求の送信	**/	
ER static send_touroku_param_req( void );	/** 登録データの初期値要求の送信	**/	

static ER	SndCmdCngMode( UINT stat );		// PCへモード切替え通知を送信

static ER send_nomal_mode_Wait_Ack_Retry( void ); // モード切替通知の送信、Ack・Nack待ちとリトライ付き（通常モード移行時）
static ER send_nomal_mode( void );			// モード切替通知の送信（通常モード移行時）
static ER send_touroku_delete_Wait_Ack_Retry( void );	// 指（登録）データの削除要求の送信、Ack・Nack待ちとリトライ付き
static ER send_touroku_delete_req( void );	// 指（登録）データの削除要求の送信（通常モード時）
static ER send_donguru_chk_Wait_Ack_Retry( void );		// ドングルの有無確認を送信、Ack・Nack待ちとリトライ付き
static ER send_donguru_chk_req( void );		// ドングルの有無確認の送信（通常モード時）
static ER send_password_chk_Wait_Ack_Retry( void );		// メンテナンス・モード移行時のパスワード確認要求の送信、Ack・Nack待ちとリトライ付き
static ER send_password_chk_req( void );	// メンテナンス・モード移行時のパスワード確認要求の送信（通常モード時）
static ER send_meinte_mode_Wait_Ack_Retry( void );		// モード切替通知の送信、Ack・Nack待ちとリトライ付き（メンテナンス・モード移行時）
static ER send_meinte_mode( void );			// モード切替通知の送信（メンテナンス・モード移行時）
static ER send_touroku_init_Wait_Ack_Retry( void );		// 登録データ初期化要求の送信、Ack・Nack待ちとリトライ付き（メンテナンス・モード時）
static ER send_touroku_init_req( void );	// 登録データ初期化要求の送信（メンテナンス・モード時）
static ER send_kinkyuu_touroku_Wait_Ack_Retry( void );	// PCへ、緊急開錠番号通知送信、Ack・Nack待ちとリトライ付き
static ER send_kinkyuu_touroku_req( void );	// PCへ、緊急開錠番号通知送信。
static ER send_kinkyuu_8keta_Wait_Ack_Retry( void );	// PCへ、緊急８桁番号データ要求送信、Ack・Nack待ちとリトライ付き
static ER send_kinkyuu_8keta_req( void );	// PCへ、緊急８桁番号データ要求送信。
static ER send_kinkyuu_kaijyou_Wait_Ack_Retry( void );	// PCへ、緊急開錠番号送信、Ack・Nack待ちとリトライ付き
static ER send_kinkyuu_kaijyou_no( void );	// PCへ、緊急開錠番号送信要求送信。
static ER send_kinkyuu_input_Wait_Ack_Retry( void );	// PCへ、緊急番号の妥当性問い合わせ確認要求送信、Ack・Nack待ちとリトライ付き
static ER send_kinkyuu_input_no( void );	// PCへ、緊急番号の妥当性問い合わせ確認要求送信。
static void reload_CAP_Param( void );		// Gain. FixShutter,IR_LEDのパラメータを、初期要求値に戻す。（CAPのRetry時に変わった可能性がある為。）

static int ercdStat = 0;				// エラーコード記憶
static UB sys_ScreenNo;					// 現在のスクリーンNo
static UINT sys_Mode;			// 状態変数
static UINT sys_SubMode;		// 状態変数（サブ）
static 	UB	s_CapResult;		// 指認証の結果
static UB s_DongleResult;		// ドングルの有無確認の結果
static UB s_PasswordResult;		// パスワード確認の結果
static UB s_KinkyuuTourokuResult; // 緊急登録通知の結果
static FLGPTN befor_scrn_no;	// 画面遷移用変数。「中止しますか？」「いいえ」の時の戻り先のFLG_PTN番号。
static UB  befor_scrn_for_ctrl; // 画面遷移用変数。「中止しますか？」「いいえ」の時の戻り先の画面番号。

static UINT rcv_ack_nak;		// ack受信フラグ　=0：未受信、=1:ACK受信、=-1：nak受信

static UB req_restart = 0;		// パワー・オン・プロセスの再起動要求フラグ　=0:要求無し、=1:要求あり

// 受信データ領域
#define	RCV_BUF_SIZE	1024
static char cSioRcvBuf[ RCV_BUF_SIZE ];	// シリアル受信データバッファ
static unsigned short usSioRcvCount;	// シリアル受信データ数

static BOOL s_bMonRs485;				// モニタはRS485フラグ

#if defined(_DRV_TEST_)
// ドライバテスト
extern BOOL drvTest();

#endif


/********************************************************************/

/**
 * クロック初期化の例
 *
 * ハードウェアに合わせてカスタマイズしてください。
 */
void ini_clk(void)
{

}


/********************************************************************/
/*
 * rcv_mbf_Lcd
 * 2013.05.08 Miya メッセージNGのため共用メモリで対応する
 * rcv_mbf の代わり
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
	
	ErrStatusInit();					// エラーステータス初期化
	
	ercdStat = 0;
	dly_tsk( 3000/MSEC );				// デバイス起動の為の初期Wait

	rtcInit();							// RTC初期化
	TplInit( SEM_TPL );					// タッチパネルコントローラ初期化

	ercd = CmrInit( SEM_CMR, TSK_CAMERA );		// カメラ初期化(応答受信時はカメラタスク起床を以後に行うこと)
	if ( ercd != E_OK ) {
		ercdStat = 1;
		ErrCodeSet( ercd );
	}
	IrLedInit( SEM_IRLED );				// 赤外線LED初期化
	IrLedCrlOff( IR_LED_2 | IR_LED_3 | IR_LED_4 | IR_LED_5 ); 	// 赤外線LEDの全消灯
	BuzInit( SEM_BUZ );					// ブザー初期化
	LcdTaskInit( TSK_LCD );				// LCD表示タスク初期化
	
//	SoundInit( TSK_BUZ, ID_FLG_BUZ );	// ブザーテストタスク初期化
	
	TsTaskInit( TSK_TS );				// 生体検知センサタスク初期化
	ExKeyTaskInit( TSK_EXKEY );			// 10キータスク初期化
	
	SystemParamInit();					// VA300 システム制御パラメータのInitial

	/**	UDPポート番号、IPアドレスのEEPROM初期設定	**/
	lan_set_port( default_udp_port );
	lan_set_ip( ini_ipaddr );
	
	lan_set_mask( ini_mask );			// サブネットマスクのEEPROM初期設定

	// EEPROMにアクセスするためにLANデバイスの初期化を行う。
	// Ethernet Address をまずはデフォルトに設定しておいて
	// 後から変更する
	read_ethernet_addr();
	lan_ini(ethernet_addr);

	// IP Address の読み出し
	read_ip_addr();

	ercd = tcp_ini();					// プロトコルスタック初期化 
	if( ercd != E_OK) {
		ercdStat = 2;
	    ErrCodeSet( ercd );
		slp_tsk();
//		goto LAN_INI_END;
	}
	
	// UDP 通信初期化
	ercd = udp_ini( TSK_UDPRCV, ID_UDP_OYOCMD, udp_portno);	
//	ercd = udp_ini( 0, ID_UDP_OYOCMD, udp_portno);	
	if( ercd != E_OK) {
		ercdStat = 3;
	    ErrCodeSet( ercd );
		slp_tsk();
//	    goto LAN_INI_END;
	}

//	MdCngMode( MD_NORMAL );		// 装置モードを"通常モード"で初期設定

LAN_INI_END:
	sta_tsk(TSK_DISP,    0);			// 表示タスク起動
	sta_tsk(TSK_IO, 0);					// I/O検知タスク起動
	sta_tsk(TSK_COMMUNICATE, 0);		// UDP電文処理タスク
	sta_tsk(TSK_UDP_SEND, 0);			// UDP送信タスク
	sta_tsk(TSK_CMD_LAN,  0);			// コマンド処理タスク起動
	
	TmInit(CYC_TMO);					// タイムアウト監視用周期起動ハンドラ起動

	ExKeyTaskInit( TSK_EXKEY );			// キー入力タスク起動

#ifndef VA300
	// チャンネル切替
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
	ercd = Mon_ini( iMonCh );			// モニタコマンドタスク初期化

	sta_tsk(TSK_SND1, iMonCh);			// (デバッグ用)シリアル送信タスク起動
#endif
	
	LcdcInit( SEM_LCD );				// LCDコントローラの初期化

	CameraTaskInit( TSK_CAMERA );		// カメラコントロールタスク初期化
	LedInit(SEM_LED);					// LED初期化
	
	yb_init_all();						// 指情報(属性情報以外)、緊急開錠情報の初期化
	
	LedOut(LED_POW, LED_ON);			// 電源表示LEDをONする（橙）
	
PowerOn_Process:
	req_restart = 0;
	
	ercd = set_flg( ID_FLG_LCD, FPTN_LCD_INIT );		// LCDの初期画面表示のリクエスト
	if ( ercd != E_OK ) ercdStat = 5;
	
#if defined(_DRV_TEST_)
	drvTest();
#endif
			
	dly_tsk( 1000/MSEC );
	
	MdCngMode( MD_POWER_ON );			// 装置モードをパワーオンモードへ
	ercd = power_on_process();			// PCとのコミュニケーション開始、機器モード設定、初期値設定
	
	// 画面表示の開始
	if ( MdGetMode() == MD_INITIAL ){ 	// 初期登録画面の時は、画面１へ。
		
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN1 );	// ID番号入力画面へ。	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN1 );	// 画面番号　<-　次の画面
		}
				
	} else if ( MdGetMode() == MD_NORMAL ){	// 通常モードの時は、画面100へ。
		
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// 通常モード入力待機画面へ。	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN100 );	// 画面番号　<-　次の画面
		}
					
	} else {
		nop();
	}
	
	// 画面切替えと、カメラ撮影処理
	for (;;) {
		ercd = wai_flg( ID_FLG_MAIN, ( /*　FPTN_START_RAWCAP 		// カメラ撮影要求の受信待ち 
									  |*/ FPTN_START_CAP			/* カメラ撮影要求の受信待ち */
									  | FPTN_LCD_CHG_REQ		/* LCD画面切替要求(LCD→メイン) */
									  ), TWF_ORW, &flgptn );
		if ( ercd != E_OK ){
			ercdStat = 7;
			break;
		}
		
		switch ( flgptn ) {
			case FPTN_LCD_CHG_REQ: // LCDタスクから、画面変更要求あり
		
				ercd = clr_flg( ID_FLG_MAIN, ~FPTN_LCD_CHG_REQ );			// フラグのクリア
				if ( ercd != E_OK ){
					ercdStat = 10;
					break;
				}
			
				ercd = NextScrn_Control();		// 次の画面表示要求フラグセット			
				
				break;
				
			case FPTN_START_CAP:	// 生体検知センサーから、カメラ撮影要求あり	
			
				ercd = clr_flg( ID_FLG_MAIN, ~FPTN_START_CAP );				// フラグのクリア
				if ( ercd != E_OK ){
					ercdStat = 8;
					break;
				}
			
				if ( ( MdGetMode() == MD_NORMAL ) 			// ノーマルモードで、画面101,102,121,141,161表示中なら
				    && ( ( GetScreenNo() == LCD_SCREEN101 ) 
					  || ( GetScreenNo() == LCD_SCREEN102 ) 
					  || ( GetScreenNo() == LCD_SCREEN121 ) 
					  || ( GetScreenNo() == LCD_SCREEN141 )
					  || ( GetScreenNo() == LCD_SCREEN161 ) ) ){
						  
//					reload_CAP_Param();			// Gain. FixShutter,IR_LEDのパラメータを、初期要求値に戻す。（CAPのRetry時に変わった可能性がある為。）

					ercd = set_flg( ID_FLG_CAMERA, FPTN_START_CAP211 );				// カメラ撮影+認証処理（コマンド211）へ。
					if ( ercd != E_OK ) break;
					
				} else if ( MdGetMode() == MD_NORMAL ){			// ノーマルモードで、画面127、画面129表示中なら
			
					if ( ( GetScreenNo() == LCD_SCREEN127 ) 
					  || ( GetScreenNo() == LCD_SCREEN129 ) ){
						
//						if ( GetScreenNo() == LCD_SCREEN127 ){
//							reload_CAP_Param();			// Gain. FixShutter,IR_LEDのパラメータを、初期要求値に戻す。（CAPのRetry時に変わった可能性がある為。）
//						} 

						ercd = set_flg( ID_FLG_CAMERA, FPTN_START_CAP204 );			// カメラ撮影+登録処理（コマンド204）へ。
						if ( ercd != E_OK ) break;
					}
				} else if ( MdGetMode() == MD_INITIAL ){		// 初期登録モードで、画面6、画面8表示中なら
			
					if ( ( GetScreenNo() == LCD_SCREEN6 ) 
					  || ( GetScreenNo() == LCD_SCREEN8 ) ){
						  
//						if ( GetScreenNo() == LCD_SCREEN6 ){
//							reload_CAP_Param();			// Gain. FixShutter,IR_LEDのパラメータを、初期要求値に戻す。（CAPのRetry時に変わった可能性がある為。）
//						}
												
						ercd = set_flg( ID_FLG_CAMERA, FPTN_START_CAP204 );			// カメラ撮影+登録処理（コマンド204）へ。
						if ( ercd != E_OK ) break;
					}
				} else if ( MdGetMode() == MD_MAINTE ){		// メンテナンスモードで、画面6、画面203表示中なら
			
					if ( GetScreenNo() == LCD_SCREEN203 ){
						
//						reload_CAP_Param();			// Gain. FixShutter,IR_LEDのパラメータを、初期要求値に戻す。（CAPのRetry時に変わった可能性がある為。）

						ercd = set_flg( ID_FLG_CAMERA, FPTN_START_CAP141 );			// カメラ撮影+フル画像処理（コマンド141）へ。
						if ( ercd != E_OK ) break;
					}
				}
				break;
/**			
			case FPTN_START_RAWCAP:	// コマンド141から、生画像カメラ撮影要求あり	
			
				ercd = clr_flg( ID_FLG_MAIN, ~FPTN_START_RAWCAP );			// フラグのクリア
				if ( ercd != E_OK ){
					ercdStat = 9;
					break;
				}
				
//				reload_CAP_Param();			// Gain. FixShutter,IR_LEDのパラメータを、初期要求値に戻す。（CAPのRetry時に変わった可能性がある為。）
				
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
	slp_tsk();		//　ここへ来る時は実装エラー
}


/*==========================================================================*/
/**
 *	次の画面表示要求フラグセット
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
		
		
	//　初期登録モード
		case LCD_SCREEN1:		// 初期登録時のID入力画面。
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( ( msg[ 0 ] == LCD_NEXT ) || ( msg[ 0 ] == LCD_USER_ID ) && ( rcv_length >= 5 ) ){
				
				yb_touroku_data.user_id[ 0 ] = msg[ 1 ];	// ユーザーID
				yb_touroku_data.user_id[ 1 ] = msg[ 2 ];
				yb_touroku_data.user_id[ 2 ] = msg[ 3 ];
				yb_touroku_data.user_id[ 3 ] = msg[ 4 ];
				yb_touroku_data.user_id[ 4 ] = ',';
				
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN2 );	// 初期登録/メンテナンスの選択画面へ。	
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN2 );		// 画面番号　<-　次の画面
				}
			} else {
				nop();	
			}
			break;

		case LCD_SCREEN2:		// 初期登録 or メンテナンスボタン選択画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
						
			if ( ( msg[ 0 ] == LCD_INIT_INPUT ) && ( rcv_length >= 1 ) ){	// 	初期登録ボタンが押された
				
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN3 );	// 責任者番号　選択画面へ。	
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN3 );		// 画面番号　<-　次の画面
				}
				
			} else if ( ( msg[ 0 ] == LCD_MAINTE ) && ( rcv_length >= 1 ) ){// 	メンテナンス・ボタンが押された
				
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN201 );	// メンテナンス画面・責任者番号　選択画面へ。	
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN201 );		// 画面番号　<-　次の画面
					MdCngMode( MD_MAINTE );				// 装置モードをメンテナンス・モードへ
//					ercd = SndCmdCngMode( MD_MAINTE );	// PCへ	メンテナンス・モード切替え通知を送信
					if ( ercd != E_OK ){
						nop();		// エラー処理の記述	
					}
				}
				
			} else {
				nop();	
			}
			break;

		case LCD_SCREEN3:		// 責任者指番号　選択画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( ( msg[ 0 ] == LCD_YUBI_ID ) && ( rcv_length >= 4 ) ){	
			
				yb_touroku_data.yubi_seq_no[ 0 ] = msg[ 1 ];	// 責任者指番号
				yb_touroku_data.yubi_seq_no[ 1 ] = msg[ 2 ];
				yb_touroku_data.yubi_seq_no[ 2 ] = msg[ 3 ];
				yb_touroku_data.yubi_seq_no[ 3 ] = ',';
				
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN4 );	// 氏名　入力画面へ
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN4 );		// 画面番号　<-　次の画面
				}
				
			} else if ( ( msg[ 0 ] == LCD_CANCEL ) && ( rcv_length >= 1 ) ){
			
				befor_scrn_no = FPTN_LCD_SCREEN3;
				befor_scrn_for_ctrl = LCD_SCREEN3;
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN12 );	// 	「中止しても..」画面へ。
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN12 );	// 画面番号　<-　次の画面
				}	
								
			} else {
				nop();	
			}
			break;

		case LCD_SCREEN4:		// 氏名　入力画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( ( msg[ 0 ] == LCD_NAME ) && ( rcv_length >= 25 ) ){	

				for(i = 0 ; i < 24 ; i++ ){					// 氏名
					yb_touroku_data.name[ i ] = msg[ 1 + i ];
				}
				
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN5 );	// 	指種別画面へ。
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN5 );		// 画面番号　<-　次の画面
				}
				
			} else if ( ( msg[ 0 ] == LCD_CANCEL ) && ( rcv_length >= 1 ) ){
			
				befor_scrn_no = FPTN_LCD_SCREEN4;
				befor_scrn_for_ctrl = LCD_SCREEN4;
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN12 );	// 	「中止しても..」画面へ。
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN12 );	// 画面番号　<-　次の画面
				}	
								
			} else {
				nop();	
			}
			break;

		case LCD_SCREEN5:		// 指種別選択画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( ( msg[ 0 ] == LCD_YUBI_SHUBETU ) && ( rcv_length >= 3 ) ){	
			
				yb_touroku_data.yubi_no[ 0 ] = msg[ 1 ];	// 指種別
				yb_touroku_data.yubi_no[ 1 ] = msg[ 2 ];
				yb_touroku_data.yubi_no[ 2 ] = ',';
				
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN6 );	// 	「指をセットして..」画面へ。
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN6 );		// 画面番号　<-　次の画面
				}
				
			} else if ( ( msg[ 0 ] == LCD_CANCEL ) && ( rcv_length >= 1 ) ){
			
				befor_scrn_no = FPTN_LCD_SCREEN5;
				befor_scrn_for_ctrl = LCD_SCREEN5;
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN12 );	// 	「中止しても..」画面へ。
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN12 );	// 画面番号　<-　次の画面
				}	
								
			} else {
				nop();	
			}
			break;			
		
		case LCD_SCREEN6:		// 「指をセットして..」画面。
			break;				// 登録の生体センサー検知ON待ち。LCDタスクは、ここへのchange Reqは出さないはず。

		case LCD_SCREEN7:		// 「指を抜いて下さい..」画面。
			break;				// 登録の生体センサー検知OFF待ち。LCDタスクは、ここへのchange Reqは出さないはず。

		case LCD_SCREEN8:		// 「もう一度指をセットして..」画面。
			break;				// 登録の生体センサー検知待ち。LCDタスクは、ここへのchange Reqは出さないはず。

		case LCD_SCREEN9:		// 登録完了○画面。
			
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN11 );	// 「登録を続けますか」画面へ。
			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN11 );		// 画面番号　<-　次の画面
			}		
			break;		

		case LCD_SCREEN10:		// 登録失敗×画面。
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN2 );	// 「初期登録メニュー」画面へ。
			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN2 );			// 画面番号　<-　次の画面
			}
//			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN11 );	// 「登録を続けますか」画面へ。
//			if ( ercd == E_OK ){
//				ChgScreenNo( LCD_SCREEN11 );		// 画面番号　<-　次の画面
//			}
			break;		

		case LCD_SCREEN11:		// 登録続行の「はい」「いいえ」選択画面。
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_YES ){			// はい
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN3 );	// 責任者番号選択画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN3 );		// 画面番号　<-　次の画面
					}
				} else if ( msg[ 0 ] == LCD_NO ){	// いいえ
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// 通常画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN100 );		// 画面番号　<-　次の画面
						MdCngMode( MD_NORMAL );				// 装置モードを通常モードへ
//						ercd = SndCmdCngMode( MD_NORMAL );	// PCへ	通常モード切替え通知を送信
						if ( ercd != E_OK ){
							nop();		// エラー処理の記述	
						}
					}
				} else {
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN11 );	// 「登録を続けますか」画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN11 );		// 画面番号　<-　次の画面
					}		
				}					
			}
			break;		

		case LCD_SCREEN12:		// 中止の「はい」「いいえ」選択画面。
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_YES ){			// はい
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN2 );	// ユーザーID番号選択画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN2 );		// 画面番号　<-　次の画面
					}
				} else if ( msg[ 0 ] == LCD_NO ){	// いいえ
					ercd = set_flg( ID_FLG_LCD, ( FLGPTN )befor_scrn_no );	// 通常画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( ( UB )befor_scrn_for_ctrl );		// 画面番号　<-　次の画面
					}
				}					
			}			
			break;		



	// 通常モード				
		case LCD_SCREEN100:		// ブランク画面。
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN101 );	// 通常モードの待機画面
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN101 );		// 画面番号　<-　次の画面
				}
			}	
			break;
		
		case LCD_SCREEN101:		// 通常モードのメニュー画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_TOUROKU ){		// 登録ボタン押下
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN120 );	// 通常モード登録画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN120 );		// 画面番号　<-　次の画面
					}
				} else if ( msg[ 0 ] == LCD_SAKUJYO ){	// 削除ボタン押下
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN140 );	// 通常モード削除画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN140 );		// 画面番号　<-　次の画面
					}
				} else if ( msg[ 0 ] == LCD_KINKYUU_SETTEI ){	// "緊急番号設定"ボタンが押された
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN160 );	// 緊急番号設定・初期画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN160 );		// 画面番号　<-　次の画面
					}					
				} else if ( msg[ 0 ] == LCD_KINKYUU_KAIJYOU ){	//  "緊急解錠"ボタンが押された"
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN180 );	// 緊急開錠・初期画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN180 );		// 画面番号　<-　次の画面
					}													
				} else if ( msg[ 0 ] == LCD_MAINTE ){	// メンテナンス・ボタン押下
						
					send_donguru_chk_Wait_Ack_Retry();	// PCへ、ドングルの有無確認を送信。						
						
					// メンテナンスモード画面移行は、コマンド002受信処理内で、OK受信した場合に実行する。

				}
			}
			break;
		
		case LCD_SCREEN102:		// 通常モードの待機画面のブランク表示
			break;				// LCDタスクは、ここへのchange Reqは出さないはず。
			
		
		case LCD_SCREEN103:		// 認証完了○画面。		
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN101 );	// 通常モードの待機画面へ。
			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN101 );		// 画面番号　<-　次の画面
				MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ（必要無いが念のため）
			}
			break;
		
		case LCD_SCREEN104:		// 認証完了×画面。
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN101 );	// 通常モードのメニュー画面へ。
			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN101 );		// 画面番号　<-　次の画面
				MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ（必要無いが念のため）
			}
			break;
		
	// 通常モード・登録
		case LCD_SCREEN120:		// 通常モード・登録のブランク画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN121 );	// 
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN121 );		// 画面番号　<-　次の画面
				}
			}
			break;
			
		case LCD_SCREEN121:		// 登録時・責任者の認証画面
			break;				// 生体センサー検知待ち。LCDタスクは、ここへのchange Reqは出さないはず。
			
		case LCD_SCREEN122:		// 責任者の認証成功○画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN124);	// 責任者・一般者の登録番号選択画面へ
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN124 );		// 画面番号　<-　次の画面
				}
			}
			break;
			
		case LCD_SCREEN123:		// 責任者の認証失敗×画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// 通常モードのブランク画面
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN100 );		// 画面番号　<-　次の画面
				}
			}
			break;
			
		case LCD_SCREEN124:		// 責任者・一般者の登録番号選択画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( ( msg[ 0 ] == LCD_YUBI_ID ) && ( rcv_length >= 4 ) ){	
			
				yb_touroku_data.yubi_seq_no[ 0 ] = msg[ 1 ];	// 責任者指番号
				yb_touroku_data.yubi_seq_no[ 1 ] = msg[ 2 ];
				yb_touroku_data.yubi_seq_no[ 2 ] = msg[ 3 ];
				yb_touroku_data.yubi_seq_no[ 3 ] = ',';
				
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN125 );	// 氏名　入力画面へ
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN125 );		// 画面番号　<-　次の画面
				}
				
			} else if ( ( msg[ 0 ] == LCD_CANCEL ) && ( rcv_length >= 1 ) ){
			
				befor_scrn_no = FPTN_LCD_SCREEN124;
				befor_scrn_for_ctrl = LCD_SCREEN124;
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN132 );	// 	「中止しても..」画面へ。
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN132 );	// 画面番号　<-　次の画面
				}	
								
			} else {
				nop();	
			}	
			break;
			
		case LCD_SCREEN125:		// 名前入力画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( ( msg[ 0 ] == LCD_NAME ) && ( rcv_length >= 25 ) ){	

				for(i = 0 ; i < 24 ; i++ ){					// 氏名
					yb_touroku_data.name[ i ] = msg[ 1 + i ];
				}
				
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN126 );	// 	指種別画面へ。
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN126 );		// 画面番号　<-　次の画面
				}
				
			} else if ( ( msg[ 0 ] == LCD_CANCEL ) && ( rcv_length >= 1 ) ){
			
				befor_scrn_no = FPTN_LCD_SCREEN125;
				befor_scrn_for_ctrl = LCD_SCREEN125;
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN132 );	// 	「中止しても..」画面へ。
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN132 );	// 画面番号　<-　次の画面
				}	
								
			} else {
				nop();	
			}
			break;
			
		case LCD_SCREEN126:		// 指種別選択画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( ( msg[ 0 ] == LCD_YUBI_SHUBETU ) && ( rcv_length >= 3 ) ){	
			
				yb_touroku_data.yubi_no[ 0 ] = msg[ 1 ];	// 指種別
				yb_touroku_data.yubi_no[ 1 ] = msg[ 2 ];
				yb_touroku_data.yubi_no[ 2 ] = ',';
				
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN127 );	// 	「指をセットして..」画面へ。
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN127 );		// 画面番号　<-　次の画面
				}
				
			} else if ( ( msg[ 0 ] == LCD_CANCEL ) && ( rcv_length >= 1 ) ){
			
				befor_scrn_no = FPTN_LCD_SCREEN126;
				befor_scrn_for_ctrl = LCD_SCREEN126;
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN132 );	// 	「中止しても..」画面へ。
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN132 );	// 画面番号　<-　次の画面
				}	
								
			} else {
				nop();	
			}
			break;
			
		case LCD_SCREEN127:		// 「指をセットして...」画面
			break;				// 生体センサー検知待ち。LCDタスクは、ここへのchange Reqは出さないはず。
			
		case LCD_SCREEN128:		// 「指を抜いて下さい」画面
			break;				// 生体センサー検知待ち。LCDタスクは、ここへのchange Reqは出さないはず。
			
		case LCD_SCREEN129:		// 「もう一度、指を...」画面
			break;				// 生体センサー検知待ち。LCDタスクは、ここへのchange Reqは出さないはず。
			
		case LCD_SCREEN130:		// 登録成功○画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN124);	// 責任者・一般者の登録番号選択画面へ
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN124 );		// 画面番号　<-　次の画面
				}
			}	
			break;
			
		case LCD_SCREEN131:		// 登録失敗×画面

			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN124);	// 責任者・一般者の登録番号選択画面へ
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN124 );		// 画面番号　<-　次の画面
				}
			}
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			break;
			
		case LCD_SCREEN132:		// 中止の「はい」「いいえ」選択画面。
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_YES ){			// はい
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// ユーザーID番号選択画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN100 );		// 画面番号　<-　次の画面
					}
				} else if ( msg[ 0 ] == LCD_NO ){	// いいえ
					//仮対応　20130510 Miya
//					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN124 );	// ユーザーID番号選択画面へ。
//					if ( ercd == E_OK ){
//						ChgScreenNo( LCD_SCREEN124 );		// 画面番号　<-　次の画面
//					}

					ercd = set_flg( ID_FLG_LCD, ( FLGPTN )befor_scrn_no );	// 通常画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( ( UB )befor_scrn_for_ctrl );		// 画面番号　<-　次の画面
					}

				}					
			}			
			break;
			
		
	// 通常モード・削除
		case LCD_SCREEN140:		// 通常モード・削除のブランク画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			//20130620_Miya デモ用に削除画面遷移変更
/*
			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN145 );	// 通常モード削除時・責任者の認証画面
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN145 );		// 画面番号　<-　次の画面
				}
			}
*/

/*　20130620_Miya デモ用に削除画面遷移変更　元 */
			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN141 );	// 通常モード削除時・責任者の認証画面
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN141 );		// 画面番号　<-　次の画面
				}
			}
/**/

			break;
			
		case LCD_SCREEN141:		// 削除時・責任者の認証画面
			break;				// 生体センサー検知待ち。LCDタスクは、ここへのchange Reqは出さないはず。
			
		case LCD_SCREEN142:		// 責任者の認証成功○画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN144);	// 責任者・一般者の削除番号選択画面へ
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN144 );		// 画面番号　<-　次の画面
				}
			}	
			break;
			
		case LCD_SCREEN143:		// 責任者の認証失敗×画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100);	// 通常モードブランク画面へ
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN100 );		// 画面番号　<-　次の画面
				}
			}
			break;
			
		case LCD_SCREEN144:		// 責任者・一般者の削除番号選択画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
		
			if ( ( msg[ 0 ] == LCD_YUBI_ID ) && ( rcv_length >= 4 ) ){	
			
				yb_touroku_data.yubi_seq_no[ 0 ] = msg[ 1 ];	// 削除候補の指番号
				yb_touroku_data.yubi_seq_no[ 1 ] = msg[ 2 ];
				yb_touroku_data.yubi_seq_no[ 2 ] = msg[ 3 ];
				yb_touroku_data.yubi_seq_no[ 3 ] = ',';
				
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN145 );	// 「削除してもよろしいですか」画面へ
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN145 );		// 画面番号　<-　次の画面
				}
				
			} else if ( ( msg[ 0 ] == LCD_CANCEL ) && ( rcv_length >= 1 ) ){
			
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN146 );	// 	「中止しても..」画面へ。
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN146 );	// 画面番号　<-　次の画面
				}	
								
			} else {
				nop();	
			}	
			break;
			
		case LCD_SCREEN145:		// 「削除してもよろしいですか」画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );


			if ( rcv_length >= 1 ){
/*
				//20130620_Miya デモ用に削除画面遷移変更
				if ( msg[ 0 ] == LCD_YES ){			// はい
//					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// 削除番号選択画面へ。
//					if ( ercd == E_OK ){
//						ChgScreenNo( LCD_SCREEN100 );		// 画面番号　<-　次の画面
						//send_touroku_delete_Wait_Ack_Retry();	// PCへ、指登録情報１件の削除要求送信。
						send_touroku_init_Wait_Ack_Retry();		// PCへ、登録データの初期化要求送信。
					if ( ercd == E_OK ){
						req_restart = 1;			// パワーオンプロセスの再起動要求フラグのセット。
					}
						if ( ercd == E_OK ){
							req_restart = 1;		// パワーオンプロセスの再起動要求フラグのセット。
						}
//					}

				} else if ( msg[ 0 ] == LCD_NO ){	// いいえ
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// 中止の「はい」「いいえ」選択画面
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN100 );		// 画面番号　<-　次の画面
					}
				}					
*/				
				
/*　20130620_Miya デモ用に削除画面遷移変更　元 */
				if ( msg[ 0 ] == LCD_YES ){			// はい
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN144 );	// 削除番号選択画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN144 );		// 画面番号　<-　次の画面
						send_touroku_delete_Wait_Ack_Retry();	// PCへ、指登録情報１件の削除要求送信。
					}

				} else if ( msg[ 0 ] == LCD_NO ){	// いいえ
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN144 );	// 中止の「はい」「いいえ」選択画面
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN144 );		// 画面番号　<-　次の画面
					}
				}	
/**/				
			}
			break;
			
		case LCD_SCREEN146:		// 中止の「はい」「いいえ」選択画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_YES ){			// はい
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// ユーザーID番号選択画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN100 );		// 画面番号　<-　次の画面
					}
				} else if ( msg[ 0 ] == LCD_NO ){	// いいえ
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN144 );	// 通常画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN144 );		// 画面番号　<-　次の画面
					}
				}					
			}	
			break;

	// 通常モード・緊急開錠番号設定
		case LCD_SCREEN160:		// 通常モード・緊急開錠番号設定のブランク画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN161 );	// 「責任者の指を...」画面
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN161 );		// 画面番号　<-　次の画面
				}
			}
			break;

		case LCD_SCREEN161:		// 通常モード・緊急開錠番号設定・「責任者の指を...」画面
			break;				// 生体センサー検知待ち。LCDタスクは、ここへのchange Reqは出さないはず。
			
		case LCD_SCREEN162:		// 通常モード・緊急開錠番号設定・責任者の認証成功○画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN164 );	// 緊急開錠番号の設定画面
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN164 );		// 画面番号　<-　次の画面
				}
			}
			break;
			
		case LCD_SCREEN163:		// 通常モード・緊急開錠番号設定・責任者の認証失敗×画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// 通常モード・初期画面
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN100 );		// 画面番号　<-　次の画面
				}
			}
			break;

		case LCD_SCREEN164:		// 通常モード・緊急開錠番号の設定画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			if ( rcv_length >= 1 ){			
				if ( ( ( msg[ 0 ] == LCD_NEXT ) || ( msg[ 0 ] == LCD_KINKYUU_BANGOU ) ) && ( rcv_length >= 5 ) ){
				
					kinkyuu_touroku_no[ 0 ] = msg[ 1 ];		// 緊急開錠番号
					kinkyuu_touroku_no[ 1 ] = msg[ 2 ];
					kinkyuu_touroku_no[ 2 ] = msg[ 3 ];
					kinkyuu_touroku_no[ 3 ] = msg[ 4 ];
					kinkyuu_touroku_no[ 4 ] = 0;
				
					send_kinkyuu_touroku_Wait_Ack_Retry();	// PCへ、緊急開錠番号通知送信。
								
					// 次画面（通常モード・初期画面）への遷移は、緊急開錠番号通知送信のOK結果を受信後、その受信コマンド処理で実行する。	

				} else if ( msg[ 0 ] == LCD_CANCEL ){		// 「中止」ボタンが押された
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN165 );	// 通常モード初期画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN165 );		// 画面番号　<-　次の画面
					}	
				}
			}
			break;
			
		case LCD_SCREEN165:		// 通常モード・緊急開錠番号「中止してもよろしいですか？」画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_YES ){					// 「はい」が押された
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// 通常モード・初期画面
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN100 );		// 画面番号　<-　次の画面
					}
										
				} else if ( msg[ 0 ] == LCD_NO ){			// 「いいえ」が押された
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN164 );	// 通常モード・緊急開錠番号の設定画面
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN164 );		// 画面番号　<-　次の画面
					}					
				}
			}
			break;			

			
	// 通常モード・緊急開錠
		case LCD_SCREEN180:		// 通常モード・緊急開錠のブランク画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN181 );	// 「コールセンターに連絡して...」画面
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN181 );		// 画面番号　<-　次の画面
				}					
			}
			break;

		case LCD_SCREEN181:		// 通常モード・緊急開錠の「コールセンターに連絡して...」画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 0 ){
				
				if ( msg[ 0 ] == LCD_NEXT ){			// 「ページ」ボタンが押された
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN182 );	// 「ID番号を入力して...」画面
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN182 );	// 画面番号　<-　次の画面
					}
					
				} else if ( msg[ 0 ] == LCD_CANCEL ){	// 「中止」ボタンが押された
					befor_scrn_no = FPTN_LCD_SCREEN181;
					befor_scrn_for_ctrl = LCD_SCREEN181;
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN188 );	// 通常モード・緊急開錠「中止しますか？」画面
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN188 );	// 画面番号　<-　次の画面
					}	
				}					
			}
			break;

		case LCD_SCREEN182:		// 通常モード・緊急開錠の「ID番号を入力して...」画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( ( ( msg[ 0 ] == LCD_NEXT ) || ( msg[ 0 ] == LCD_USER_ID ) ) && ( rcv_length >= 5 ) ){
					if ( ( yb_touroku_data.user_id[ 0 ] == msg[ 1 ] )
					  && ( yb_touroku_data.user_id[ 1 ] == msg[ 2 ] )
					  && ( yb_touroku_data.user_id[ 2 ] == msg[ 3 ] )
					  && ( yb_touroku_data.user_id[ 3 ] == msg[ 4 ] ) ){	// ID番号が一致した時。
						
						send_kinkyuu_8keta_Wait_Ack_Retry();	// PCへ、緊急８桁番号データ要求送信、Ack・Nack待ちとリトライ付き
						
						// 次画面への遷移は、緊急８桁番号データ要求の応答で、番号通知を受け取った時に受信処理内で行う。
						
					} else {	// 登録済みユーザーID番号と、入力した番号が不一致の時、
						
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN187 );	// 認証失敗×画面
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN187 );		// 画面番号　<-　次の画面
						}						
					}
					 
				} else if ( msg[ 0 ] == LCD_CANCEL ){	// 「中止」ボタンが押された
					befor_scrn_no = FPTN_LCD_SCREEN182;
					befor_scrn_for_ctrl = LCD_SCREEN182;
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN188 );	// 通常モード・緊急開錠「中止しますか？」画面
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN188 );		// 画面番号　<-　次の画面
					}	
				}
			}
			break;

		case LCD_SCREEN183:		// 通常モード・緊急開錠の「番号をコールセンターへ...」画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_NEXT ){			// 「ページ」ボタンが押された
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN184 );	// 「８桁番号を入力して...」画面
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN184 );	// 画面番号　<-　次の画面
					}			
				
				} else if ( msg[ 0 ] == LCD_CANCEL ){	// 「中止」ボタンが押された
					befor_scrn_no = FPTN_LCD_SCREEN183;
					befor_scrn_for_ctrl = LCD_SCREEN183;
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN188 );	// 通常モード・緊急開錠「中止しますか？」画面
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN188 );		// 画面番号　<-　次の画面
					}	
				}
			}
			break;			


		case LCD_SCREEN184:		// 通常モード・緊急開錠の「８桁番号を入力して下さい...」画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( ( ( msg[ 0 ] == LCD_NEXT ) || ( msg[ 0 ] == LCD_KINKYUU_KAIJYOU_BANGOU ) ) && ( rcv_length >= 9 ) ){								
					kinkyuu_kaijyo_no[ 0 ] = msg[ 1 ];		// 緊急８桁番号
					kinkyuu_kaijyo_no[ 1 ] = msg[ 2 ];
					kinkyuu_kaijyo_no[ 2 ] = msg[ 3 ];
					kinkyuu_kaijyo_no[ 3 ] = msg[ 4 ];
					kinkyuu_kaijyo_no[ 4 ] = msg[ 5 ];
					kinkyuu_kaijyo_no[ 5 ] = msg[ 6 ];
					kinkyuu_kaijyo_no[ 6 ] = msg[ 7 ];
					kinkyuu_kaijyo_no[ 7 ] = msg[ 8 ];
					kinkyuu_kaijyo_no[ 8 ] = 0;

					send_kinkyuu_kaijyou_Wait_Ack_Retry();	// PCへ、緊急開錠番号送信、Ack・Nack待ちとリトライ付き
								
					// 次画面（緊急番号入力画面）への遷移は、緊急開錠番号８桁送信のOK結果を受信後、その受信コマンド処理で実行する。			
				
				} else if ( msg[ 0 ] == LCD_CANCEL ){	// 「中止」ボタンが押された
					befor_scrn_no = FPTN_LCD_SCREEN184;
					befor_scrn_for_ctrl = LCD_SCREEN184;
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN188 );	// 通常モード・緊急開錠「中止しますか？」画面
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN188 );		// 画面番号　<-　次の画面
					}	
				}
			}
			break;


		case LCD_SCREEN185:		// 通常モード・「緊急番号を入力して下さい...」画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){	
				if ( ( ( msg[ 0 ] == LCD_NEXT ) || ( msg[ 0 ] == LCD_KINKYUU_BANGOU ) ) && ( rcv_length >= 5 ) ){								
					kinkyuu_input_no[ 0 ] = msg[ 1 ];		// 入力された緊急番号
					kinkyuu_input_no[ 1 ] = msg[ 2 ];
					kinkyuu_input_no[ 2 ] = msg[ 3 ];
					kinkyuu_input_no[ 3 ] = msg[ 4 ];
					kinkyuu_input_no[ 4 ] = 0;
					
					send_kinkyuu_input_Wait_Ack_Retry();	// PCへ、緊急番号の妥当性問い合わせ確認要求送信、Ack・Nack待ちとリトライ付き
								
					// 次画面（OK/NG画面）への遷移は、緊急番号妥当性確認要求送信のOK結果を受信後、その受信コマンド処理で実行する。			
				
				} else if ( msg[ 0 ] == LCD_CANCEL ){	// 「中止」ボタンが押された
					befor_scrn_no = FPTN_LCD_SCREEN185;
					befor_scrn_for_ctrl = LCD_SCREEN185;
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN188 );	// 通常モード・緊急開錠「中止しますか？」画面
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN188 );		// 画面番号　<-　次の画面
					}	
				}
			}
			break;

		case LCD_SCREEN186:		// 通常モード・緊急開錠の認証成功○画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// 通常モード・初期画面へ。
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN100 );		// 画面番号　<-　次の画面
				}
			}
			break;
			
		case LCD_SCREEN187:		// 通常モード・緊急開錠の認証失敗×画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// 通常モード・初期画面へ。
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN100 );		// 画面番号　<-　次の画面
				}
			}
			break;

			
		case LCD_SCREEN188:		// 通常モード・緊急開錠時「中止しますか？」画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_YES ){					// 「はい」が押された
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// 通常モード・初期画面
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN100 );		// 画面番号　<-　次の画面
					}
										
				} else if ( msg[ 0 ] == LCD_NO ){			// 「いいえ」が押された
					ercd = set_flg( ID_FLG_LCD, ( FLGPTN )befor_scrn_no );	// 前画面へ戻る
					if ( ercd == E_OK ){
						ChgScreenNo( ( UB )befor_scrn_for_ctrl );	// 画面番号　<-　次の画面
					}					
				}
			}
			break;			

		
	// メンテナンス画面
		case LCD_SCREEN200:		// メンテナンス・モードのブランク画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN201 );	// メンテナンス・モードのID番号入力画面
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN201 );		// 画面番号　<-　次の画面
				}					
			}
			break;

		case LCD_SCREEN201:		// メンテナンス画面のパスワード番号入力画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){			
				if ( ( ( msg[ 0 ] == LCD_NEXT ) || ( msg[ 0 ] == LCD_USER_ID ) ) && ( rcv_length >= 5 ) ){
				
					mainte_password[ 0 ] = msg[ 1 ];	// パスワード番号
					mainte_password[ 1 ] = msg[ 2 ];
					mainte_password[ 2 ] = msg[ 3 ];
					mainte_password[ 3 ] = msg[ 4 ];
					mainte_password[ 4 ] = 0;
				
					send_password_chk_Wait_Ack_Retry();	// PCへ、パスワードの一致確認要求送信。
								
					// メンテナンス・メニュー画面への遷移は、パスワードの一致確認要求送信の結果がOKだった時、その受信コマンド処理で実行する。。	

				} else if ( msg[ 0 ] == LCD_CANCEL ){	// 「中止」ボタンが押された
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// 通常モード初期画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN100 );		// 画面番号　<-　次の画面
						MdCngMode( MD_NORMAL );				// 装置モードを通常モードへ
//						ercd = SndCmdCngMode( MD_NORMAL );	// PCへ	通常モード切替え通知を送信
					}	
				}
			}
			break;
			
		case LCD_SCREEN202:		// メンテナンス・メニュー画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_FULL_PIC_SEND_REQ ){	// フル画像送信ボタン
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN203 );	// 「指をセットして...」画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN203 );		// 画面番号　<-　次の画面
					}
					
				} else if ( msg[ 0 ] == LCD_MAINTE_SHOKIKA_REQ ){	// 初期化ボタン
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN206 );	// 初期化移行「削除してもよろしいですか？」画面へ
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN206 );		// 画面番号　<-　次の画面
					}			
				
				} else if ( msg[ 0 ] == LCD_MAINTE_END ){	// 終了ボタン
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// 通常モード画面へ。
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN100 );		// 画面番号　<-　次の画面
						MdCngMode( MD_NORMAL );				// 装置モードを通常モードへ
//						ercd = SndCmdCngMode( MD_NORMAL );	// PCへ	通常モード切替え通知を送信
					}
				}		
			break;
			
		case LCD_SCREEN203:		// フル画像送信・「指をセットして...」画面
			break;				// 生体センサー検知待ち。LCDタスクは、ここへのchange Reqは出さないはず。

			
		case LCD_SCREEN204:		// フル画像送信・指画像の取得成功○画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN202 );	// メンテナンス・メニュー画面へ
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN202 );		// 画面番号　<-　次の画面
				}
			}
			break;
			
		case LCD_SCREEN205:		// フル画像送信・指画像の取得失敗×画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN202 );	// メンテナンス・メニュー画面へ
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN202 );		// 画面番号　<-　次の画面
				}
			}
			break;
			
		case LCD_SCREEN206:		// 初期化移行「削除してもよろしいですか？」画面
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_YES ){				// 「はい」が押された

					send_touroku_init_Wait_Ack_Retry();		// PCへ、登録データの初期化要求送信。
					if ( ercd == E_OK ){
						req_restart = 1;			// パワーオンプロセスの再起動要求フラグのセット。
					}
										
				} else if ( msg[ 0 ] == LCD_NO ){		// 「いいえ」が押された
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN202 );	// メンテナンス・メニュー選択画面
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN202 );		// 画面番号　<-　次の画面
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
 *	PCとのコミュニケーション開始、各種初期値設定要求メイン
 */
/*==========================================================================*/
static ER power_on_process( void )
{
	ER ercd;
	int Retry;
	
	/** PCへWakeUp確認送信	**/
	MdCngSubMode( SUB_MD_WAKEUP );			// サブモードを、WakeUp問い合わせへ設定
	
	while ( 1 ){							// WakeUp問い合わせのPCからの応答完了が来るまで、１秒周期で繰り返し。
	
		ercd = send_WakeUp();				// WakeUpを問い合わせ。
		if ( ercd == E_OK ){				// Ack応答あり
			nop();
			break;

		} else if ( ercd == CODE_NACK ){	// Nak応答あり
			dly_tsk( 1000/MSEC );
			continue;
							
		} else if ( ercd == E_TMOUT ){		// １秒タイムアウト
			nop();
			continue;
			
		} else if ( ercd == E_OBJ ){		// Ack/Nak以外を受信
			nop();
			continue;
			
		} else {
			PrgErrSet();
			slp_tsk();		//　ここへ来る時は実装エラー
		}
	}
	
	
	/** カメラ・パラメータの初期値要求の送信	**/
	while ( MdGetSubMode() != SUB_MD_IDLE ){	// WakeUp問い合わせのPCからの応答完了を待つ。
		dly_tsk( 25/MSEC );	
	}
	
	MdCngSubMode( SUB_MD_CAMERA_PARAM_REQ );	// サブモードを、カメラパラメータの初期値要求中へ設定
	
	Retry = 0;
	while ( Retry <= 3 ){
		ercd = send_camera_param_req();		// カメラ・パラメータの初期値要求の送信
		if ( ercd == E_OK ){				// Ack応答あり
			nop();
			break;
		} else if ( ercd == CODE_NACK ){	// Nak応答あり
			dly_tsk( 1000/MSEC );
			continue;
							
		} else if ( ercd == E_TMOUT ){		// １秒タイムアウト
			nop();
			
		} else if ( ercd == E_OBJ ){		// Ack/Nak以外を受信
			nop();
			continue;
			
		} else {
			PrgErrSet();
			slp_tsk();		//　ここへ来る時は実装エラー
		}
		Retry++;
	}

	
	/** 画像処理の初期値要求の送信	**/	
	while ( MdGetSubMode() != SUB_MD_IDLE ){	// カメラ・パラメータの初期値要求のPCからの応答完了を待つ。
		dly_tsk( 25/MSEC );	
	}
	
	MdCngSubMode( SUB_MD_CAP_PARAM_REQ );	// サブモードを、画像処理の初期値要求中へ設定
	
	Retry = 0;
	while ( Retry <= 3 ){
		ercd = send_cap_param_req();		// 画像処理の初期値要求の送信
		if ( ercd == E_OK ){				// Ack応答あり
			nop();
			break;
		} else if ( ercd == CODE_NACK ){	// Nak応答あり
			dly_tsk( 1000/MSEC );
			continue;
							
		} else if ( ercd == E_TMOUT ){		// １秒タイムアウト
			nop();
			
		} else if ( ercd == E_OBJ ){		// Ack/Nak以外を受信
			nop();
			continue;
			
		} else {
			PrgErrSet();
			slp_tsk();		//　ここへ来る時は実装エラー
		}
		Retry++;
	}

	
	/** LED光量数値の初期値要求の送信	**/	
	while ( MdGetSubMode() != SUB_MD_IDLE ){	// 画像処理の初期値要求のPCからの応答完了を待つ。
		dly_tsk( 25/MSEC );	
	}
	
	MdCngSubMode( SUB_MD_LED_PARAM_REQ );	// サブモードを、LED光量数値の初期値要求中へ設定
	
	Retry = 0;
	while ( Retry <= 3 ){
		ercd = send_Led_param_req();		// LED光量数値の初期値要求の送信
		if ( ercd == E_OK ){				// Ack応答あり
			nop();
			break;
		} else if ( ercd == CODE_NACK ){	// Nak応答あり
			dly_tsk( 1000/MSEC );
			continue;
							
		} else if ( ercd == E_TMOUT ){		// １秒タイムアウト
			nop();
			
		} else if ( ercd == E_OBJ ){		// Ack/Nak以外を受信
			nop();
			continue;
			
		} else {
			PrgErrSet();
			slp_tsk();		//　ここへ来る時は実装エラー
		}
		Retry++;
	}

	
	/** 登録データの初期値要求の送信	**/	
	while ( MdGetSubMode() != SUB_MD_IDLE ){	// LED光量数値の初期値要求のPCからの応答完了を待つ。
		dly_tsk( 25/MSEC );	
	}
	
	MdCngSubMode( SUB_MD_TOUROKU_PARAM_REQ );	// サブモードを、登録データの初期値要求中へ設定
	
	Retry = 0;
	while ( Retry <= 3 ){
		ercd = send_touroku_param_req();	// 登録データの初期値要求の送信
		if ( ercd == E_OK ){				// Ack応答あり
			nop();
			break;
		} else if ( ercd == CODE_NACK ){	// Nak応答あり
			dly_tsk( 1000/MSEC );
			continue;
							
		} else if ( ercd == E_TMOUT ){		// １秒タイムアウト
			nop();
			
		} else if ( ercd == E_OBJ ){		// Ack/Nak以外を受信
			nop();
			continue;
			
		} else {
			PrgErrSet();
			slp_tsk();		//　ここへ来る時は実装エラー
		}
		Retry++;
	}

	while ( MdGetSubMode() != SUB_MD_IDLE ){	// 登録データの初期値要求のPCからの応答完了を待つ。
		dly_tsk( 25/MSEC );	
	}
	return ercd;
	
}

/*==========================================================================*/
/**
 *	モード切替通知の送信（各種モード移行時）
 */
/*==========================================================================*/
static ER SndCmdCngMode( UINT stat )	// PCへモード切替え通知を送信
{
	ER ercd = E_OK;

	switch ( stat ){
		case MD_POWER_OFF:		///< 電源OFF
		
			break;
				
		case MD_INITIAL:		///< 初期登録モード	
			
			break;
			
		case MD_MAINTE:			///< メンテナンスモード	

			ercd = send_meinte_mode_Wait_Ack_Retry();

	    	break;
			
		case MD_NORMAL:			///< 通常モード

			ercd = send_nomal_mode_Wait_Ack_Retry();

	    	break;
			
		case MD_POWER_FAIL:		///< 停電時動作モード
			
	    	break;
			
		case MD_PANIC:			///< 非常時開錠モード
			
	    	break;

		default:
			break;
	}
	return ercd;
}

/*==========================================================================*/
//	モード切替通知の送信、Ack・Nack待ちとリトライ付き（通常モード移行時）
/*==========================================================================*/
static ER send_nomal_mode_Wait_Ack_Retry( void )
{
	UINT Retry = 0;
	ER ercd;
	
	while ( Retry <= 3 ){
		ercd = send_nomal_mode();			// モード切替通知の送信（通常モード移行時）
		if ( ercd == E_OK ){				// Ack応答あり
			MdCngSubMode( SUB_MD_CHG_NORMAL );	//　サブモード・ステータスを、”ノーマルモードへ移行中”へ。
			nop();
			break;
		} else if ( ercd == CODE_NACK ){	// Nak応答あり
			dly_tsk( 1000/MSEC );
			continue;
							
		} else if ( ercd == E_TMOUT ){		// １秒タイムアウト
			nop();
			
		} else if ( ercd == E_OBJ ){		// Ack/Nak以外を受信
			nop();
			continue;
			
		} else {
			PrgErrSet();
			slp_tsk();		//　ここへ来る時は実装エラー
		}
		Retry++;
	}
	return ercd;
}

/*==========================================================================*/
/**
 *	モード切替通知の送信（通常モード移行時）
 */
/*==========================================================================*/
static ER send_nomal_mode( void )
{
	ER ercd = E_TMOUT;
	int count;
	char com_data[ 15 ];
	
	/** コマンドデータの準備	**/
	com_data[ 0 ] = 0x23;			//　ヘッダ　1Byte　ASCII
	com_data[ 1 ] = 0;				//　データ長　２バイトBinary
	com_data[ 2 ] = 0x0d;
	com_data[ 3 ] = 0x31;			//　送信元種別　1Byte　ASCII
	com_data[ 4 ] = '2';			//　コマンド番号　３桁ASCII
	com_data[ 5 ] = '0';
	com_data[ 6 ] = '0';
	com_data[ 7 ] = '0';			//　ブロック番号 ４桁ASCII
	com_data[ 8 ] = '0';
	com_data[ 9 ] = '0';
	com_data[ 10 ] = '0';
	com_data[ 11 ] = ' ';			//　伝送データ　２桁ASCII
	com_data[ 12 ] = ' ';
	com_data[ 13 ] = CODE_CR;		//　終端コード
	com_data[ 14 ] = CODE_LF;
	
	/** コマンド送信	**/
	SendBinaryData( &com_data, ( 13 + 2 ) );
	
	ercd = Wait_Ack_forBlock();		// コマンドまたはブロックデータ送信後のAck応答待ち
	count = 0;
	while( count <= 100 ){			//　ACK/NAK応答待ち（最大１秒間）
		
		if ( ercd == E_OK ){
			nop();			// Ack受信
			break;
		} else if ( ercd == CODE_NACK ){
			nop();			// Nak受信	
			break;
		} else if ( ercd == E_TMOUT ){
			nop();			// タイムアウト	
			break;
		} else if ( ercd == E_OBJ ){
			nop();			// Ack/Nak以外を受信	
			break;
		}
		dly_tsk( 10/MSEC );
		count++;
	}
	
	rxtid = rxtid_org;						// GET_UDPの為に本来の受信タスクID（tsk_rcv）を再セット
	
	return ercd;
}



/*==========================================================================*/
//	指（登録）データの削除要求の送信、Ack・Nack待ちとリトライ付き
/*==========================================================================*/
static ER send_touroku_delete_Wait_Ack_Retry( void )
{
	UINT Retry = 0;
	ER ercd;
	
	while ( Retry <= 3 ){
		ercd = send_touroku_delete_req();	// 指登録データ削除要求の送信
		if ( ercd == E_OK ){				// Ack応答あり
			nop();
			break;
		} else if ( ercd == CODE_NACK ){	// Nak応答あり
			dly_tsk( 1000/MSEC );
			continue;
							
		} else if ( ercd == E_TMOUT ){		// １秒タイムアウト
			nop();
			
		} else if ( ercd == E_OBJ ){		// Ack/Nak以外を受信
			nop();
			continue;
			
		} else {
			PrgErrSet();
			slp_tsk();		//　ここへ来る時は実装エラー
		}
		Retry++;
	}
	return ercd;
}

/*==========================================================================*/
/**
 *	指（登録）データの削除要求の送信（通常モード時）
 */
/*==========================================================================*/
static ER send_touroku_delete_req( void )
{
	ER ercd = E_TMOUT;
	int count;
	char com_data[ 24 ];
	
	/** コマンドデータの準備	**/
	com_data[ 0 ] = 0x23;			//　ヘッダ　1Byte　ASCII
	com_data[ 1 ] = 0;				//　データ長　２バイトBinary
	com_data[ 2 ] = 0x15;
	com_data[ 3 ] = 0x31;			//　送信元種別　1Byte　ASCII
	com_data[ 4 ] = '2';			//　コマンド番号　３桁ASCII
	com_data[ 5 ] = '0';
	com_data[ 6 ] = '9';
	com_data[ 7 ] = '0';			//　ブロック番号 ４桁ASCII
	com_data[ 8 ] = '0';
	com_data[ 9 ] = '0';
	com_data[ 10 ] = '0';
	
	//　伝送データ　
	com_data[ 11 ] = yb_touroku_data.tou_no[ 0 ];	//　棟番号
	com_data[ 12 ] = yb_touroku_data.tou_no[ 1 ];
	com_data[ 13 ] = ',';

	com_data[ 14 ] = yb_touroku_data.user_id[ 0 ];	//　ユーザーID
	com_data[ 15 ] = yb_touroku_data.user_id[ 1 ];
	com_data[ 16 ] = yb_touroku_data.user_id[ 2 ];
	com_data[ 17 ] = yb_touroku_data.user_id[ 3 ];
	com_data[ 18 ] = ',';

	com_data[ 19 ] = yb_touroku_data.yubi_seq_no[ 0 ];	//　責任者／一般者の登録指情報（指登録番号０～１００）
	com_data[ 20 ] = yb_touroku_data.yubi_seq_no[ 1 ];
	com_data[ 21 ] = yb_touroku_data.yubi_seq_no[ 2 ];

	com_data[ 22 ] = CODE_CR;		//　終端コード
	com_data[ 23 ] = CODE_LF;
	
	/** コマンド送信	**/
	SendBinaryData( &com_data, ( 22 + 2 ) );
	
	ercd = Wait_Ack_forBlock();		// コマンドまたはブロックデータ送信後のAck応答待ち
	count = 0;
	while( count <= 100 ){			//　ACK/NAK応答待ち（最大１秒間）
		
		if ( ercd == E_OK ){
			nop();			// Ack受信
			break;
		} else if ( ercd == CODE_NACK ){
			nop();			// Nak受信	
			break;
		} else if ( ercd == E_TMOUT ){
			nop();			// タイムアウト	
			break;
		} else if ( ercd == E_OBJ ){
			nop();			// Ack/Nak以外を受信	
			break;
		}
		dly_tsk( 10/MSEC );
		count++;
	}
	
	rxtid = rxtid_org;						// GET_UDPの為に本来の受信タスクID（tsk_rcv）を再セット
	
	return ercd;
}

/*==========================================================================*/
//	緊急開錠番号通知送信（通常モード時）、Ack・Nack待ちとリトライ付き
/*==========================================================================*/
static ER send_kinkyuu_touroku_Wait_Ack_Retry( void )	// PCへ、緊急開錠番号通知送信、Ack・Nack待ちとリトライ付き
{
	UINT Retry = 0;
	ER ercd;
	
	while ( Retry <= 3 ){
		ercd = send_kinkyuu_touroku_req();	// 緊急開錠番号通知送信
		if ( ercd == E_OK ){				// Ack応答あり
			nop();
			MdCngSubMode( SUB_MD_KINKYU_TOUROKU );	//　サブモード・ステータスを、”緊急番号登録処理シーケンス中”へ。
			break;
			
		} else if ( ercd == CODE_NACK ){	// Nak応答あり
			dly_tsk( 1000/MSEC );
			continue;
							
		} else if ( ercd == E_TMOUT ){		// １秒タイムアウト
			nop();
			
		} else if ( ercd == E_OBJ ){		// Ack/Nak以外を受信
			nop();
			continue;
			
		} else {
			PrgErrSet();
			slp_tsk();		//　ここへ来る時は実装エラー
		}
		Retry++;
	}
	return ercd;
}


/*==========================================================================*/
/**
 *	緊急開錠番号通知送信（通常モード時）
 */
/*==========================================================================*/
static ER send_kinkyuu_touroku_req( void )		// PCへ、緊急開錠番号通知送信
{
	ER ercd = E_TMOUT;
	int count;
	char com_data[ 17 ];
	
	/** コマンドデータの準備	**/
	com_data[ 0 ] = 0x23;			//　ヘッダ　1Byte　ASCII
	com_data[ 1 ] = 0;				//　データ長　２バイトBinary
	com_data[ 2 ] = 15;
	com_data[ 3 ] = 0x31;			//　送信元種別　1Byte　ASCII
	com_data[ 4 ] = '2';			//　コマンド番号　３桁ASCII
	com_data[ 5 ] = '7';
	com_data[ 6 ] = '0';
	com_data[ 7 ] = '0';			//　ブロック番号 ４桁ASCII
	com_data[ 8 ] = '0';
	com_data[ 9 ] = '0';
	com_data[ 10 ] = '0';
	
	//　伝送データ　
	com_data[ 11 ] = kinkyuu_touroku_no[ 0 ];	//　緊急開錠番号４桁
	com_data[ 12 ] = kinkyuu_touroku_no[ 1 ];
	com_data[ 13 ] = kinkyuu_touroku_no[ 2 ];
	com_data[ 14 ] = kinkyuu_touroku_no[ 3 ];
	
	com_data[ 15 ] = CODE_CR;		//　終端コード
	com_data[ 16 ] = CODE_LF;
	
	/** コマンド送信	**/
	SendBinaryData( &com_data, ( 15 + 2 ) );
	
	ercd = Wait_Ack_forBlock();		// コマンドまたはブロックデータ送信後のAck応答待ち
	count = 0;
	while( count <= 100 ){			//　ACK/NAK応答待ち（最大１秒間）
		
		if ( ercd == E_OK ){
			nop();			// Ack受信
			break;
		} else if ( ercd == CODE_NACK ){
			nop();			// Nak受信	
			break;
		} else if ( ercd == E_TMOUT ){
			nop();			// タイムアウト	
			break;
		} else if ( ercd == E_OBJ ){
			nop();			// Ack/Nak以外を受信	
			break;
		}
		dly_tsk( 10/MSEC );
		count++;
	}
	
	rxtid = rxtid_org;						// GET_UDPの為に本来の受信タスクID（tsk_rcv）を再セット
	
	return ercd;	
}

/*==========================================================================*/
//	緊急８桁番号データ要求送信（通常モード時）、Ack・Nack待ちとリトライ付き
/*==========================================================================*/
static ER send_kinkyuu_8keta_Wait_Ack_Retry( void )	// PCへ、緊急８桁番号データ要求送信、Ack・Nack待ちとリトライ付き
{
	UINT Retry = 0;
	ER ercd;
	
	while ( Retry <= 3 ){
		ercd = send_kinkyuu_8keta_req();	// 緊急８桁番号データ要求
		if ( ercd == E_OK ){				// Ack応答あり
			nop();
			MdCngSubMode( SUB_MD_KINKYU_8KETA_REQ );	//　サブモード・ステータスを、”緊急８桁番号要求通信中”へ。
			break;
			
		} else if ( ercd == CODE_NACK ){	// Nak応答あり
			dly_tsk( 1000/MSEC );
			continue;
							
		} else if ( ercd == E_TMOUT ){		// １秒タイムアウト
			nop();
			
		} else if ( ercd == E_OBJ ){		// Ack/Nak以外を受信
			nop();
			continue;
			
		} else {
			PrgErrSet();
			slp_tsk();		//　ここへ来る時は実装エラー
		}
		Retry++;
	}
	return ercd;
}


/*==========================================================================*/
/**
 *	緊急８桁番号データ要求送信（通常モード時）
 */
/*==========================================================================*/
static ER send_kinkyuu_8keta_req( void )			// PCへ、緊急８桁番号データ要求送信。
{
	ER ercd = E_TMOUT;
	int count;
	char com_data[ 22 ];
	
	/** コマンドデータの準備	**/
	com_data[ 0 ] = 0x23;			//　ヘッダ　1Byte　ASCII
	com_data[ 1 ] = 0;				//　データ長　２バイトBinary
	com_data[ 2 ] = 20;
	com_data[ 3 ] = 0x31;			//　送信元種別　1Byte　ASCII
	com_data[ 4 ] = '2';			//　コマンド番号　３桁ASCII
	com_data[ 5 ] = '7';
	com_data[ 6 ] = '1';
	com_data[ 7 ] = '0';			//　ブロック番号 ４桁ASCII
	com_data[ 8 ] = '0';
	com_data[ 9 ] = '0';
	com_data[ 10 ] = '0';
	
	//　伝送データ　
	com_data[ 11 ] = kinkyuu_hyouji_no[ 0 ];	//　緊急８桁番号表示データ
	com_data[ 12 ] = kinkyuu_hyouji_no[ 1 ];
	com_data[ 13 ] = kinkyuu_hyouji_no[ 2 ];
	com_data[ 14 ] = kinkyuu_hyouji_no[ 3 ];
	com_data[ 15 ] = kinkyuu_hyouji_no[ 4 ];
	com_data[ 16 ] = kinkyuu_hyouji_no[ 5 ];
	com_data[ 17 ] = kinkyuu_hyouji_no[ 6 ];
	com_data[ 18 ] = kinkyuu_hyouji_no[ 7 ];
	com_data[ 19 ] = kinkyuu_hyouji_no[ 8 ];

	com_data[ 20 ] = CODE_CR;		//　終端コード
	com_data[ 21 ] = CODE_LF;
	
	/** コマンド送信	**/
	SendBinaryData( &com_data, ( 20 + 2 ) );
	
	ercd = Wait_Ack_forBlock();		// コマンドまたはブロックデータ送信後のAck応答待ち
	count = 0;
	while( count <= 100 ){			//　ACK/NAK応答待ち（最大１秒間）
		
		if ( ercd == E_OK ){
			nop();			// Ack受信
			break;
		} else if ( ercd == CODE_NACK ){
			nop();			// Nak受信	
			break;
		} else if ( ercd == E_TMOUT ){
			nop();			// タイムアウト	
			break;
		} else if ( ercd == E_OBJ ){
			nop();			// Ack/Nak以外を受信	
			break;
		}
		dly_tsk( 10/MSEC );
		count++;
	}
	
	rxtid = rxtid_org;						// GET_UDPの為に本来の受信タスクID（tsk_rcv）を再セット
	
	return ercd;	
}


/*==========================================================================*/
//	緊急開錠番号送信（通常モード時）、Ack・Nack待ちとリトライ付き
/*==========================================================================*/
static ER send_kinkyuu_kaijyou_Wait_Ack_Retry( void )	// PCへ、緊急開錠番号送信、Ack・Nack待ちとリトライ付き
{
	UINT Retry = 0;
	ER ercd;
	
	while ( Retry <= 3 ){
		ercd = send_kinkyuu_kaijyou_no();	// 緊急開錠番号送信要求送信
		if ( ercd == E_OK ){				// Ack応答あり
			nop();
			MdCngSubMode( SUB_MD_KINKYU_KAIJYO_SEND );	//　サブモード・ステータスを、”緊急開錠番号通知通信中”へ。
			break;
			
		} else if ( ercd == CODE_NACK ){	// Nak応答あり
			dly_tsk( 1000/MSEC );
			continue;
							
		} else if ( ercd == E_TMOUT ){		// １秒タイムアウト
			nop();
			
		} else if ( ercd == E_OBJ ){		// Ack/Nak以外を受信
			nop();
			continue;
			
		} else {
			PrgErrSet();
			slp_tsk();		//　ここへ来る時は実装エラー
		}
		Retry++;
	}
	return ercd;
	
}


/*==========================================================================*/
/**
 *	緊急開錠番号送信（通常モード時）
 */
/*==========================================================================*/
static ER send_kinkyuu_kaijyou_no( void)			// PCへ、緊急開錠番号送信要求送信。
{
	ER ercd = E_TMOUT;
	int count;
	char com_data[ 22 ];
	
	/** コマンドデータの準備	**/
	com_data[ 0 ] = 0x23;			//　ヘッダ　1Byte　ASCII
	com_data[ 1 ] = 0;				//　データ長　２バイトBinary
	com_data[ 2 ] = 20;
	com_data[ 3 ] = 0x31;			//　送信元種別　1Byte　ASCII
	com_data[ 4 ] = '2';			//　コマンド番号　３桁ASCII
	com_data[ 5 ] = '7';
	com_data[ 6 ] = '3';
	com_data[ 7 ] = '0';			//　ブロック番号 ４桁ASCII
	com_data[ 8 ] = '0';
	com_data[ 9 ] = '0';
	com_data[ 10 ] = '0';
	
	//　伝送データ　
	com_data[ 11 ] = kinkyuu_kaijyo_no[ 0 ];	//　緊急開錠番号８桁
	com_data[ 12 ] = kinkyuu_kaijyo_no[ 1 ];
	com_data[ 13 ] = kinkyuu_kaijyo_no[ 2 ];
	com_data[ 14 ] = kinkyuu_kaijyo_no[ 3 ];
	com_data[ 15 ] = kinkyuu_kaijyo_no[ 4 ];
	com_data[ 16 ] = kinkyuu_kaijyo_no[ 5 ];
	com_data[ 17 ] = kinkyuu_kaijyo_no[ 6 ];
	com_data[ 18 ] = kinkyuu_kaijyo_no[ 7 ];
	com_data[ 19 ] = kinkyuu_kaijyo_no[ 8 ];	
	
	com_data[ 20 ] = CODE_CR;		//　終端コード
	com_data[ 21 ] = CODE_LF;
	
	/** コマンド送信	**/
	SendBinaryData( &com_data, ( 20 + 2 ) );
	
	ercd = Wait_Ack_forBlock();		// コマンドまたはブロックデータ送信後のAck応答待ち
	count = 0;
	while( count <= 100 ){			//　ACK/NAK応答待ち（最大１秒間）
		
		if ( ercd == E_OK ){
			nop();			// Ack受信
			break;
		} else if ( ercd == CODE_NACK ){
			nop();			// Nak受信	
			break;
		} else if ( ercd == E_TMOUT ){
			nop();			// タイムアウト	
			break;
		} else if ( ercd == E_OBJ ){
			nop();			// Ack/Nak以外を受信	
			break;
		}
		dly_tsk( 10/MSEC );
		count++;
	}
	
	rxtid = rxtid_org;						// GET_UDPの為に本来の受信タスクID（tsk_rcv）を再セット
	
	return ercd;		
}


/*==========================================================================*/
//	緊急番号の妥当性問い合わせ確認要求送信（通常モード時）、Ack・Nack待ちとリトライ付き
/*==========================================================================*/
static ER send_kinkyuu_input_Wait_Ack_Retry( void )	// PCへ、緊急番号の妥当性問い合わせ確認要求送信、Ack・Nack待ちとリトライ付き
{
	UINT Retry = 0;
	ER ercd;
	
	while ( Retry <= 3 ){
		ercd = send_kinkyuu_input_no();	// 緊急番号の妥当性問い合わせ確認要求送信
		if ( ercd == E_OK ){				// Ack応答あり
			nop();
			MdCngSubMode( SUB_MD_KINKYU_NO_CHK_REQ );	//　サブモード・ステータスを、”緊急番号妥当性確認要求通信中”へ。
			break;
			
		} else if ( ercd == CODE_NACK ){	// Nak応答あり
			dly_tsk( 1000/MSEC );
			continue;
							
		} else if ( ercd == E_TMOUT ){		// １秒タイムアウト
			nop();
			
		} else if ( ercd == E_OBJ ){		// Ack/Nak以外を受信
			nop();
			continue;
			
		} else {
			PrgErrSet();
			slp_tsk();		//　ここへ来る時は実装エラー
		}
		Retry++;
	}
	return ercd;
		
}


/*==========================================================================*/
/**
 *	緊急番号の妥当性問い合わせ確認要求送信（通常モード時）
 */
/*==========================================================================*/
static ER send_kinkyuu_input_no( void )			// PCへ、緊急番号の妥当性問い合わせ確認要求送信。
{
	ER ercd = E_TMOUT;
	int count;
	char com_data[ 17 ];
	
	/** コマンドデータの準備	**/
	com_data[ 0 ] = 0x23;			//　ヘッダ　1Byte　ASCII
	com_data[ 1 ] = 0;				//　データ長　２バイトBinary
	com_data[ 2 ] = 15;
	com_data[ 3 ] = 0x31;			//　送信元種別　1Byte　ASCII
	com_data[ 4 ] = '2';			//　コマンド番号　３桁ASCII
	com_data[ 5 ] = '7';
	com_data[ 6 ] = '4';
	com_data[ 7 ] = '0';			//　ブロック番号 ４桁ASCII
	com_data[ 8 ] = '0';
	com_data[ 9 ] = '0';
	com_data[ 10 ] = '0';
	
	//　伝送データ　
	com_data[ 11 ] = kinkyuu_input_no[ 0 ];	//　入力された緊急開錠番号４桁
	com_data[ 12 ] = kinkyuu_input_no[ 1 ];
	com_data[ 13 ] = kinkyuu_input_no[ 2 ];
	com_data[ 14 ] = kinkyuu_input_no[ 3 ];	
	
	com_data[ 15 ] = CODE_CR;		//　終端コード
	com_data[ 16 ] = CODE_LF;
	
	/** コマンド送信	**/
	SendBinaryData( &com_data, ( 15 + 2 ) );
	
	ercd = Wait_Ack_forBlock();		// コマンドまたはブロックデータ送信後のAck応答待ち
	count = 0;
	while( count <= 100 ){			//　ACK/NAK応答待ち（最大１秒間）
		
		if ( ercd == E_OK ){
			nop();			// Ack受信
			break;
		} else if ( ercd == CODE_NACK ){
			nop();			// Nak受信	
			break;
		} else if ( ercd == E_TMOUT ){
			nop();			// タイムアウト	
			break;
		} else if ( ercd == E_OBJ ){
			nop();			// Ack/Nak以外を受信	
			break;
		}
		dly_tsk( 10/MSEC );
		count++;
	}
	
	rxtid = rxtid_org;						// GET_UDPの為に本来の受信タスクID（tsk_rcv）を再セット
	
	return ercd;			
}

/*==========================================================================*/
//	ドングルの有無確認を送信、Ack・Nack待ちとリトライ付き
/*==========================================================================*/
static ER send_donguru_chk_Wait_Ack_Retry( void )
{
	UINT Retry = 0;
	ER ercd;
	
	while ( Retry <= 3 ){
		ercd = send_donguru_chk_req();	// ドングルの有無確認の送信
		if ( ercd == E_OK ){				// Ack応答あり
			nop();
			MdCngSubMode( SUB_MD_DONGURU_CHK );	//　サブモード・ステータスを、”ドングルの有無確認中”へ。
			break;
			
		} else if ( ercd == CODE_NACK ){	// Nak応答あり
			dly_tsk( 1000/MSEC );
			continue;
							
		} else if ( ercd == E_TMOUT ){		// １秒タイムアウト
			nop();
			
		} else if ( ercd == E_OBJ ){		// Ack/Nak以外を受信
			nop();
			continue;
			
		} else {
			PrgErrSet();
			slp_tsk();		//　ここへ来る時は実装エラー
		}
		Retry++;
	}
	return ercd;
}


/*==========================================================================*/
/**
 *	ドングルの有無確認の送信（通常モード時）
 */
/*==========================================================================*/
static ER send_donguru_chk_req( void )
{
	ER ercd = E_TMOUT;
	int count;
	char com_data[ 15 ];
	
	/** コマンドデータの準備	**/
	com_data[ 0 ] = 0x23;			//　ヘッダ　1Byte　ASCII
	com_data[ 1 ] = 0;				//　データ長　２バイトBinary
	com_data[ 2 ] = 13;
	com_data[ 3 ] = 0x31;			//　送信元種別　1Byte　ASCII
	com_data[ 4 ] = '2';			//　コマンド番号　３桁ASCII
	com_data[ 5 ] = '1';
	com_data[ 6 ] = '2';
	com_data[ 7 ] = '0';			//　ブロック番号 ４桁ASCII
	com_data[ 8 ] = '0';
	com_data[ 9 ] = '0';
	com_data[ 10 ] = '0';
	
	//　伝送データ　
	com_data[ 11 ] = ' ';			//　スペース２桁
	com_data[ 12 ] = ' ';

	com_data[ 13 ] = CODE_CR;		//　終端コード
	com_data[ 14 ] = CODE_LF;
	
	/** コマンド送信	**/
	SendBinaryData( &com_data, ( 13 + 2 ) );
	
	ercd = Wait_Ack_forBlock();		// コマンドまたはブロックデータ送信後のAck応答待ち
	count = 0;
	while( count <= 100 ){			//　ACK/NAK応答待ち（最大１秒間）
		
		if ( ercd == E_OK ){
			nop();			// Ack受信
			break;
		} else if ( ercd == CODE_NACK ){
			nop();			// Nak受信	
			break;
		} else if ( ercd == E_TMOUT ){
			nop();			// タイムアウト	
			break;
		} else if ( ercd == E_OBJ ){
			nop();			// Ack/Nak以外を受信	
			break;
		}
		dly_tsk( 10/MSEC );
		count++;
	}
	
	rxtid = rxtid_org;						// GET_UDPの為に本来の受信タスクID（tsk_rcv）を再セット
	
	return ercd;
}


/*==========================================================================*/
//	メンテナンス・モード移行時のパスワード確認要求の送信、Ack・Nack待ちとリトライ付き
/*==========================================================================*/
static ER send_password_chk_Wait_Ack_Retry( void )
{
	UINT Retry = 0;
	ER ercd;
	
	while ( Retry <= 3 ){
		ercd = send_password_chk_req();		// メンテナンス・モード移行時のパスワード確認要求の送信
		if ( ercd == E_OK ){				// Ack応答あり
			nop();
			MdCngSubMode( SUB_MD_PASSWORD_CHK );	//　サブモード・ステータスを、”パスワード確認中”へ。			
			break;
			
		} else if ( ercd == CODE_NACK ){	// Nak応答あり
			dly_tsk( 1000/MSEC );
			continue;
							
		} else if ( ercd == E_TMOUT ){		// １秒タイムアウト
			nop();
			
		} else if ( ercd == E_OBJ ){		// Ack/Nak以外を受信
			nop();
			continue;
			
		} else {
			PrgErrSet();
			slp_tsk();		//　ここへ来る時は実装エラー
		}
		Retry++;
	}
	return ercd;
}


/*==========================================================================*/
/**
 *	メンテナンス・モード移行時のパスワード確認要求の送信（通常モード時）
 */
/*==========================================================================*/
static ER send_password_chk_req( void )
{
	ER ercd = E_TMOUT;
	int count;
	char com_data[ 17 ];
	
	/** コマンドデータの準備	**/
	com_data[ 0 ] = 0x23;			//　ヘッダ　1Byte　ASCII
	com_data[ 1 ] = 0;				//　データ長　２バイトBinary
	com_data[ 2 ] = 15;
	com_data[ 3 ] = 0x31;			//　送信元種別　1Byte　ASCII
	com_data[ 4 ] = '2';			//　コマンド番号　３桁ASCII
	com_data[ 5 ] = '1';
	com_data[ 6 ] = '3';
	com_data[ 7 ] = '0';			//　ブロック番号 ４桁ASCII
	com_data[ 8 ] = '0';
	com_data[ 9 ] = '0';
	com_data[ 10 ] = '0';
	
	//　伝送データ　
	com_data[ 11 ] = mainte_password[ 0 ];	//　パスワード４桁
	com_data[ 12 ] = mainte_password[ 1 ];
	com_data[ 13 ] = mainte_password[ 2 ];
	com_data[ 14 ] = mainte_password[ 3 ];
	
	com_data[ 15 ] = CODE_CR;		//　終端コード
	com_data[ 16 ] = CODE_LF;
	
	/** コマンド送信	**/
	SendBinaryData( &com_data, ( 15 + 2 ) );
	
	ercd = Wait_Ack_forBlock();		// コマンドまたはブロックデータ送信後のAck応答待ち
	count = 0;
	while( count <= 100 ){			//　ACK/NAK応答待ち（最大１秒間）
		
		if ( ercd == E_OK ){
			nop();			// Ack受信
			break;
		} else if ( ercd == CODE_NACK ){
			nop();			// Nak受信	
			break;
		} else if ( ercd == E_TMOUT ){
			nop();			// タイムアウト	
			break;
		} else if ( ercd == E_OBJ ){
			nop();			// Ack/Nak以外を受信	
			break;
		}
		dly_tsk( 10/MSEC );
		count++;
	}
	
	rxtid = rxtid_org;						// GET_UDPの為に本来の受信タスクID（tsk_rcv）を再セット
	
	return ercd;
}


/*==========================================================================*/
//	モード切替通知の送信、Ack・Nack待ちとリトライ付き（メンテナンス・モード移行時）
/*==========================================================================*/
static ER send_meinte_mode_Wait_Ack_Retry( void )
{
	UINT Retry = 0;
	ER ercd;
	
	while ( Retry <= 3 ){
		ercd = send_meinte_mode();			// モード切替通知の送信（通常モード移行時）
		if ( ercd == E_OK ){				// Ack応答あり
			MdCngSubMode( SUB_MD_CHG_MAINTE );	//　サブモード・ステータスを、”メンテナンスモードへ移行中”へ。
			nop();
			break;
		} else if ( ercd == CODE_NACK ){	// Nak応答あり
			dly_tsk( 1000/MSEC );
			continue;
							
		} else if ( ercd == E_TMOUT ){		// １秒タイムアウト
			nop();
			
		} else if ( ercd == E_OBJ ){		// Ack/Nak以外を受信
			nop();
			continue;
			
		} else {
			PrgErrSet();
			slp_tsk();		//　ここへ来る時は実装エラー
		}
		Retry++;
	}
	return ercd;
}


/*==========================================================================*/
/**
 *	モード切替通知の送信（メンテナンス・モード移行時）
 */
/*==========================================================================*/
static ER send_meinte_mode( void )
{
	ER ercd = E_TMOUT;
	int count;
	char com_data[ 15 ];
	
	/** コマンドデータの準備	**/
	com_data[ 0 ] = 0x23;			//　ヘッダ　1Byte　ASCII
	com_data[ 1 ] = 0;				//　データ長　２バイトBinary
	com_data[ 2 ] = 0x0d;
	com_data[ 3 ] = 0x31;			//　送信元種別　1Byte　ASCII
	com_data[ 4 ] = '1';			//　コマンド番号　３桁ASCII
	com_data[ 5 ] = '0';
	com_data[ 6 ] = '0';
	com_data[ 7 ] = '0';			//　ブロック番号 ４桁ASCII
	com_data[ 8 ] = '0';
	com_data[ 9 ] = '0';
	com_data[ 10 ] = '0';
	com_data[ 11 ] = ' ';			//　伝送データ　２桁ASCII
	com_data[ 12 ] = ' ';
	com_data[ 13 ] = CODE_CR;		//　終端コード
	com_data[ 14 ] = CODE_LF;
	
	/** コマンド送信	**/
	SendBinaryData( &com_data, ( 13 + 2 ) );
	
	ercd = Wait_Ack_forBlock();		// コマンドまたはブロックデータ送信後のAck応答待ち
	count = 0;
	while( count <= 100 ){			//　ACK/NAK応答待ち（最大１秒間）
		
		if ( ercd == E_OK ){
			nop();			// Ack受信
			break;
		} else if ( ercd == CODE_NACK ){
			nop();			// Nak受信	
			break;
		} else if ( ercd == E_TMOUT ){
			nop();			// タイムアウト	
			break;
		} else if ( ercd == E_OBJ ){
			nop();			// Ack/Nak以外を受信	
			break;
		}
		dly_tsk( 10/MSEC );
		count++;
	}
	
	rxtid = rxtid_org;						// GET_UDPの為に本来の受信タスクID（tsk_rcv）を再セット
	
	return ercd;
}


/*==========================================================================*/
//　登録データ初期化要求の送信、Ack・Nack待ちとリトライ付き（メンテナンス・モード時）
/*==========================================================================*/
static ER send_touroku_init_Wait_Ack_Retry( void )
{
	UINT Retry = 0;
	ER ercd;
	
	while ( Retry <= 3 ){
		ercd = send_touroku_init_req();		// 登録データ初期化要求の送信（メンテナンス・モード時）
		if ( ercd == E_OK ){				// Ack応答あり
			nop();			
			break;
			
		} else if ( ercd == CODE_NACK ){	// Nak応答あり
			dly_tsk( 1000/MSEC );
			continue;
							
		} else if ( ercd == E_TMOUT ){		// １秒タイムアウト
			nop();
			
		} else if ( ercd == E_OBJ ){		// Ack/Nak以外を受信
			nop();
			continue;
			
		} else {
			PrgErrSet();
			slp_tsk();		//　ここへ来る時は実装エラー
		}
		Retry++;
	}
	return ercd;
}



/*==========================================================================*/
/**
 *	登録データ初期化要求の送信（メンテナンス・モード時）
 */
/*==========================================================================*/
static ER send_touroku_init_req( void )
{
	ER ercd = E_TMOUT;
	int count;
	char com_data[ 15 ];
	
	/** コマンドデータの準備	**/
	com_data[ 0 ] = 0x23;			//　ヘッダ　1Byte　ASCII
	com_data[ 1 ] = 0;				//　データ長　２バイトBinary
	com_data[ 2 ] = 0x0d;
	com_data[ 3 ] = 0x31;			//　送信元種別　1Byte　ASCII
	com_data[ 4 ] = '1';			//　コマンド番号　３桁ASCII
	com_data[ 5 ] = '1';
	com_data[ 6 ] = '1';
	com_data[ 7 ] = '0';			//　ブロック番号 ４桁ASCII
	com_data[ 8 ] = '0';
	com_data[ 9 ] = '0';
	com_data[ 10 ] = '0';
	com_data[ 11 ] = ' ';			//　伝送データ　２桁ASCII
	com_data[ 12 ] = ' ';
	com_data[ 13 ] = CODE_CR;		//　終端コード
	com_data[ 14 ] = CODE_LF;
	
	/** コマンド送信	**/
	SendBinaryData( &com_data, ( 13 + 2 ) );
	
	ercd = Wait_Ack_forBlock();		// コマンドまたはブロックデータ送信後のAck応答待ち
	count = 0;
	while( count <= 100 ){			//　ACK/NAK応答待ち（最大１秒間）
		
		if ( ercd == E_OK ){
			nop();			// Ack受信
			break;
		} else if ( ercd == CODE_NACK ){
			nop();			// Nak受信	
			break;
		} else if ( ercd == E_TMOUT ){
			nop();			// タイムアウト	
			break;
		} else if ( ercd == E_OBJ ){
			nop();			// Ack/Nak以外を受信	
			break;
		}
		dly_tsk( 10/MSEC );
		count++;
	}
	
	rxtid = rxtid_org;						// GET_UDPの為に本来の受信タスクID（tsk_rcv）を再セット
	
	return ercd;
}

/*==========================================================================*/
/**
 *	PCへUDPを通してWakeUpの問い合わせを行う（電源ON時）
 */
/*==========================================================================*/
static ER send_WakeUp( void )
{
	ER ercd = E_TMOUT;
	int count;
	char com_data[ 15 ];
	
	/** コマンドデータの準備	**/
	com_data[ 0 ] = 0x23;			//　ヘッダ　1Byte　ASCII
	com_data[ 1 ] = 0;				//　データ長　２バイトBinary
	com_data[ 2 ] = 0x0d;
	com_data[ 3 ] = 0x31;			//　送信元種別　1Byte　ASCII
	com_data[ 4 ] = '0';			//　コマンド番号　３桁ASCII
	com_data[ 5 ] = '2';
	com_data[ 6 ] = '1';
	com_data[ 7 ] = '0';			//　ブロック番号 ４桁ASCII
	com_data[ 8 ] = '0';
	com_data[ 9 ] = '0';
	com_data[ 10 ] = '0';
	com_data[ 11 ] = ' ';			//　伝送データ　２桁ASCII
	com_data[ 12 ] = ' ';
	com_data[ 13 ] = CODE_CR;		//　終端コード
	com_data[ 14 ] = CODE_LF;
	
	/** コマンド送信	**/
	SendBinaryData( &com_data, ( 13 + 2 ) );
	
	ercd = Wait_Ack_forBlock();		// コマンドまたはブロックデータ送信後のAck応答待ち
	count = 0;
	while( count <= 100 ){			//　ACK/NAK応答待ち（最大１秒間）
		
		if ( ercd == E_OK ){
			nop();			// Ack受信
			break;
		} else if ( ercd == CODE_NACK ){
			nop();			// Nak受信	
			break;
		} else if ( ercd == E_TMOUT ){
			nop();			// タイムアウト	
			break;
		} else if ( ercd == E_OBJ ){
			nop();			// Ack/Nak以外を受信	
			break;
		}
		dly_tsk( 10/MSEC );
		count++;
	}
	
	rxtid = rxtid_org;						// GET_UDPの為に本来の受信タスクID（tsk_rcv）を再セット
	
	return ercd;
}



/*==========================================================================*/
/**
 *	カメラ・パラメータの初期値要求の送信（電源ON時）
 */
/*==========================================================================*/
static ER send_camera_param_req( void )
{
	ER ercd = E_TMOUT;
	int count;
	char com_data[ 15 ];
	
	/** コマンドデータの準備	**/
	com_data[ 0 ] = 0x23;			//　ヘッダ　1Byte　ASCII
	com_data[ 1 ] = 0;				//　データ長　２バイトBinary
	com_data[ 2 ] = 0x0d;
	com_data[ 3 ] = 0x31;			//　送信元種別　1Byte　ASCII
	com_data[ 4 ] = '0';			//　コマンド番号　３桁ASCII
	com_data[ 5 ] = '1';
	com_data[ 6 ] = '6';
	com_data[ 7 ] = '0';			//　ブロック番号 ４桁ASCII
	com_data[ 8 ] = '0';
	com_data[ 9 ] = '0';
	com_data[ 10 ] = '0';
	com_data[ 11 ] = ' ';			//　伝送データ　２桁ASCII
	com_data[ 12 ] = ' ';
	com_data[ 13 ] = CODE_CR;		//　終端コード
	com_data[ 14 ] = CODE_LF;
	
	/** コマンド送信	**/
	SendBinaryData( &com_data, ( 13 + 2 ) );
	
	ercd = Wait_Ack_forBlock();		// コマンドまたはブロックデータ送信後のAck応答待ち
	count = 0;
	while( count <= 100 ){			//　ACK/NAK応答待ち（最大１秒間）
		
		if ( ercd == E_OK ){
			nop();			// Ack受信
			break;
		} else if ( ercd == CODE_NACK ){
			nop();			// Nak受信	
			break;
		} else if ( ercd == E_TMOUT ){
			nop();			// タイムアウト	
			break;
		} else if ( ercd == E_OBJ ){
			nop();			// Ack/Nak以外を受信	
			break;
		}
		dly_tsk( 10/MSEC );
		count++;
	}
	
	rxtid = rxtid_org;						// GET_UDPの為に本来の受信タスクID（tsk_rcv）を再セット
	
	return ercd;
}


/*==========================================================================*/
/**
 *	画像処理の初期値要求の送信（電源ON時）
 */
/*==========================================================================*/
static ER send_cap_param_req( void )
{
	ER ercd = E_TMOUT;
	int count;
	char com_data[ 15 ];
	
	/** コマンドデータの準備	**/
	com_data[ 0 ] = 0x23;			//　ヘッダ　1Byte　ASCII
	com_data[ 1 ] = 0;				//　データ長　２バイトBinary
	com_data[ 2 ] = 0x0d;
	com_data[ 3 ] = 0x31;			//　送信元種別　1Byte　ASCII
	com_data[ 4 ] = '0';			//　コマンド番号　３桁ASCII
	com_data[ 5 ] = '1';
	com_data[ 6 ] = '8';
	com_data[ 7 ] = '0';			//　ブロック番号 ４桁ASCII
	com_data[ 8 ] = '0';
	com_data[ 9 ] = '0';
	com_data[ 10 ] = '0';
	com_data[ 11 ] = ' ';			//　伝送データ　２桁ASCII
	com_data[ 12 ] = ' ';
	com_data[ 13 ] = CODE_CR;		//　終端コード
	com_data[ 14 ] = CODE_LF;
	
	/** コマンド送信	**/
	SendBinaryData( &com_data, ( 13 + 2 ) );
	
	ercd = Wait_Ack_forBlock();		// コマンドまたはブロックデータ送信後のAck応答待ち
	count = 0;
	while( count <= 100 ){			//　ACK/NAK応答待ち（最大１秒間）
		
		if ( ercd == E_OK ){
			nop();			// Ack受信
			break;
		} else if ( ercd == CODE_NACK ){
			nop();			// Nak受信	
			break;
		} else if ( ercd == E_TMOUT ){
			nop();			// タイムアウト	
			break;
		} else if ( ercd == E_OBJ ){
			nop();			// Ack/Nak以外を受信	
			break;
		}
		dly_tsk( 10/MSEC );
		count++;
	}
	
	rxtid = rxtid_org;						// GET_UDPの為に本来の受信タスクID（tsk_rcv）を再セット
	
	return ercd;
}


/*==========================================================================*/
/**
 *	LED光量数値の初期値要求の送信（電源ON時）
 */
/*==========================================================================*/
static ER send_Led_param_req( void )
{
	ER ercd = E_TMOUT;
	int count;
	char com_data[ 15 ];
	
	/** コマンドデータの準備	**/
	com_data[ 0 ] = 0x23;			//　ヘッダ　1Byte　ASCII
	com_data[ 1 ] = 0;				//　データ長　２バイトBinary
	com_data[ 2 ] = 0x0d;
	com_data[ 3 ] = 0x31;			//　送信元種別　1Byte　ASCII
	com_data[ 4 ] = '0';			//　コマンド番号　３桁ASCII
	com_data[ 5 ] = '1';
	com_data[ 6 ] = '7';
	com_data[ 7 ] = '0';			//　ブロック番号 ４桁ASCII
	com_data[ 8 ] = '0';
	com_data[ 9 ] = '0';
	com_data[ 10 ] = '0';
	com_data[ 11 ] = ' ';			//　伝送データ　２桁ASCII
	com_data[ 12 ] = ' ';
	com_data[ 13 ] = CODE_CR;		//　終端コード
	com_data[ 14 ] = CODE_LF;
	
	/** コマンド送信	**/
	SendBinaryData( &com_data, ( 13 + 2 ) );
	
	ercd = Wait_Ack_forBlock();		// コマンドまたはブロックデータ送信後のAck応答待ち
	count = 0;
	while( count <= 100 ){			//　ACK/NAK応答待ち（最大１秒間）
		
		if ( ercd == E_OK ){
			nop();			// Ack受信
			break;
		} else if ( ercd == CODE_NACK ){
			nop();			// Nak受信	
			break;
		} else if ( ercd == E_TMOUT ){
			nop();			// タイムアウト	
			break;
		} else if ( ercd == E_OBJ ){
			nop();			// Ack/Nak以外を受信	
			break;
		}
		dly_tsk( 10/MSEC );
		count++;
	}
	
	rxtid = rxtid_org;						// GET_UDPの為に本来の受信タスクID（tsk_rcv）を再セット
	
	return ercd;
}


/*==========================================================================*/
/**
 *	登録データの初期値要求の送信（電源ON時）
 */
/*==========================================================================*/
static ER send_touroku_param_req( void )
{
	ER ercd = E_TMOUT;
	int count;
	char com_data[ 15 ];
	
	/** コマンドデータの準備	**/
	com_data[ 0 ] = 0x23;			//　ヘッダ　1Byte　ASCII
	com_data[ 1 ] = 0;				//　データ長　２バイトBinary
	com_data[ 2 ] = 0x0d;
	com_data[ 3 ] = 0x31;			//　送信元種別　1Byte　ASCII
	com_data[ 4 ] = '0';			//　コマンド番号　３桁ASCII
	com_data[ 5 ] = '1';
	com_data[ 6 ] = '9';
	com_data[ 7 ] = '0';			//　ブロック番号 ４桁ASCII
	com_data[ 8 ] = '0';
	com_data[ 9 ] = '0';
	com_data[ 10 ] = '0';
	com_data[ 11 ] = ' ';			//　伝送データ　２桁ASCII
	com_data[ 12 ] = ' ';
	com_data[ 13 ] = CODE_CR;		//　終端コード
	com_data[ 14 ] = CODE_LF;
	
	/** コマンド送信	**/
	SendBinaryData( &com_data, ( 13 + 2 ) );
	
	ercd = Wait_Ack_forBlock();		// コマンドまたはブロックデータ送信後のAck応答待ち
	count = 0;
	while( count <= 100 ){			//　ACK/NAK応答待ち（最大１秒間）
		
		if ( ercd == E_OK ){
			nop();			// Ack受信
			break;
		} else if ( ercd == CODE_NACK ){
			nop();			// Nak受信	
			break;
		} else if ( ercd == E_TMOUT ){
			nop();			// タイムアウト	
			break;
		} else if ( ercd == E_OBJ ){
			nop();			// Ack/Nak以外を受信	
			break;
		}
		dly_tsk( 10/MSEC );
		count++;
	}
	
	rxtid = rxtid_org;						// GET_UDPの為に本来の受信タスクID（tsk_rcv）を再セット
	
	return ercd;
}


/*==========================================================================*/
/**
 *	VA300 システム制御パラメータのInitial
 */
/*==========================================================================*/
void SystemParamInit( void )
{
	ercdStat = 0;					// エラーコード記憶
	sys_ScreenNo = FPTN_LCD_INIT;	// 現在のスクリーンNo
	sys_Mode = MD_POWER_ON;			// 最新の装置モード
	sys_SubMode = SUB_MD_IDLE;		// 状態変数（サブ）
	rcv_ack_nak = 0;				// UDP通信　ACK/NAK受信フラグ
	
	s_CapResult	= CAP_JUDGE_IDLE;				// 指認証の結果
	s_DongleResult = DONGURU_JUDGE_IDLE;		// ドングルの有無確認の結果
	s_PasswordResult = PASSWORD_JUDGE_IDLE;		// パスワード確認の結果
	
//	SUB_MD_SETTING		// 最新の装置サブモード
	
}	

/*==========================================================================*/
/**
 *	最新の画面遷移状態(番号)を返す
 */
/*==========================================================================*/
UB GetScreenNo(void)
{
	return sys_ScreenNo;
}

/*==========================================================================*/
/**
 *	画面遷移状態(番号)を更新する
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
 * 	Gain. FixShutter,IR_LEDのパラメータを、Hostからの初期要求値に戻す。
 * （CAPのRetry時に変わった可能性がある為。）
 */
/*==========================================================================*/
void reload_CAP_Param( void )
{
	UB ercd;
	
	cmrGain = ini_cmrGain;						// カメラ・ゲイン値初期値　
	cmrFixShutter1 = ini_cmrFixShutter1;		// Fix Shutter Control値初期値(１回目)　
	cmrFixShutter2 = ini_cmrFixShutter2;		// Fix Shutter Control値初期値(２回目)			同上
	cmrFixShutter3 = ini_cmrFixShutter3;		// Fix Shutter Control値初期値(３回目）

//	ercd = set_flg( ID_FLG_CAMERA, FPTN_SETREQ_GAIN );	// カメラタスクに、直接のゲイン設定値を設定を依頼（FPGAを経由しない）	
//	if ( ercd != E_OK ){
//		ErrCodeSet( ercd );
//	}
		
	irDuty2 = ini_irDuty2;		// IR Duty値irDuty2　初期値;
	irDuty3 = ini_irDuty3;
	irDuty4 = ini_irDuty4;
	irDuty5 = ini_irDuty5;
	
	ercd = set_flg( ID_FLG_CAMERA, FPTN_SETREQ_SHUT1 );	// カメラタスクに、直接の露出３設定値を設定を依頼（FPGAを経由しない）	
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
		ethernet_addr[0] = ini_mac_addr[0];	/* 暫定的に即値!! */
		ethernet_addr[1] = ini_mac_addr[1];
		ethernet_addr[2] = ini_mac_addr[2];
		ethernet_addr[3] = ini_mac_addr[3];
		ethernet_addr[4] = ini_mac_addr[4];
		ethernet_addr[5] = ini_mac_addr[5];
	}
	return TRUE;
}


/**
 * Initialize peripherals on evaluation board（★ Please customize !!）
 *
 */
void init_peripheral(void)
{
	
}

/**
 * Initialize port（★ Please customize !!）
 *
 */

void ini_pio(void)
{
	sfr_outw(BSC_GPIOIC,0x0000);
	sfr_outw(BSC_PDTRB, 0x0000);		// 
	sfr_outl(BSC_PCTRB, 0x00000014);	// PORT17,18(TEST Pin1,2)出力
	
	TP_CLR(1);							// テストピン1初期化
	TP_CLR(2);							// テストピン2初期化
	
}

/**
 * Initialize interrupt（★ Please customize !!）
 *
 */

static void ini_intr(void)
{
	FpgaInit();							// FPGA関連初期化

	sfr_outw(INTC_IPRD, 0x0000);
	sfr_setw(INTC_ICR,  0x0000);
}

/*****************************************************************************
* TELNET のパスワードチェック
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
* TELNET のコマンド処理
*
******************************************************************************/

BOOL telnetd_callback(T_TERMINAL *t, B *s)
{
    return GetCmd( s );
}

/**
 * TELNET送信
 */
 
void telnetd_send(B *s)
{
	terminal_print((T_TERMINAL *)&telnetd, s);
}

/**
 * TELNETバイナリ送信
 */
 
void telnetd_send_bin(B *s, INT len)
{
	terminal_sendbin((T_TERMINAL *)&telnetd, s, len);
}


/**
 * IP Address の入力
 *
 * @retval TRUE 読出し成功
 * @retval FALSE 読出し失敗
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
		default_ipaddr[0] = ini_ipaddr[0];	/* 別ファイルで定義 */
		default_ipaddr[1] = ini_ipaddr[1];
		default_ipaddr[2] = ini_ipaddr[2];
		default_ipaddr[3] = ini_ipaddr[3];
	}
		
	if (flg == FALSE) {
#if 0
		if (EepGetSNo( &serial_no) == E_OK) {
			if (serial_no && serial_no != 0xff) {	// シリアル番号読み出せたときはIPアドレスの最下位を変更する。
				default_ipaddr[3] = (UB)(serial_no & 0xff);
			}
		}
#endif

		subnet_mask[0] = ini_mask[0];		/* 暫定的に即値!! */
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
 *	変更履歴
 *		04/08/23	OYOモニタプログラムに変更
 *
 */
TASK RcvTask(INT ch)
{
	char	code;
	BOOL	stx_flag;

	/* initialize */


//	ini_sio(ch, (B *)"115200 B8 PN S1");		//☆モニタプログラム用
	ini_sio(ch, (B *)"38400 B8 PN S1");		//☆モニタプログラム用
	
	ctl_sio(ch, TSIO_RXE|TSIO_TXE|TSIO_RTSON);
	memset( cSioRcvBuf, 0, RCV_BUF_SIZE);		// 受信バッファクリア
	usSioRcvCount = 0;							// シリアル受信でー多数クリア

	for (;;)
	{
		stx_flag = FALSE;						// STX受信フラグクリア
		
		for(;;) {
			get_sio( ch, (UB*)&code, TMO_FEVR);	// 受信データ待ち

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
 * @param ch チャンネル番号
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
			
    	} else {							/* コーディングエラー */
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

   /* Initialize processor（★ Please customize !!）*/

	init_peripheral();

	
	/* Initialize system */

    sysini();

	ini_clk();

	ini_pio();

	/* Create tasks */

	cre_tsk(TSK_MAIN,      &ctsk_main);
	cre_tsk(TSK_CMD_LAN,   &ctsk_lancmd);
	cre_tsk(TSK_SND1,      &ctsk_snd1);
	cre_tsk(TSK_COMMUNICATE,  &ctsk_urcv);//電文処理
	cre_tsk(TSK_UDP_SEND,    &ctsk_usnd);//UDP送信

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


	//2013.05.08 Miya メッセージNGのため共用メモリで対応する
	g_LcdmsgData.LcdMsgSize = 0;
	memset(&g_LcdmsgData.LcdMsgBuff[0], 0x20, 1024);

	//FIFO用にエリアを確保して設定する。but LcdCmd_buf[]にストアーされなかった
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
