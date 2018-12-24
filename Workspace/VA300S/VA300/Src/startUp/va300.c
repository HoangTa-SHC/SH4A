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

// サイクリック・ハンドラ定義
static void cycle1_hdr(void);
const T_CCYC ccyc_tim1  = { TA_HLNG|TA_STA, NULL, cycle1_hdr, (10/MSEC), (10/MSEC)};

//2013.05.08 Miya メッセージNGのため共用メモリで対応する
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

static ER lan_get_sys_spec( UB *pval );		// VA300　仕様切替えフラグの内容を、EEPROMから読み出す。

static void init_wdt( void );				// WDT(ウォッチドッグ・タイマー)の初期設定
static void reset_wdtc( void );				// WDT(ウォッチドッグ・タイマー)カウンタのクリア
static void stop_wdt( void );				// WDT(ウォッチドッグ・タイマー)の無効化設定
static void reset_wdtmem( void );			// WDTカウンタ用メモリのリセット処理
static void reset_wdt_cnt( void );			// WDTカウンタ用メモリのダイレクト・リセット処理(フラッシュ・メモリ・ドライバ処理専用)
											// 各TaskのWDTクリア・フラグを無視して、カウンタをクリアするので、
											// ドライバ内での使用に限る。（乱用すると、タスク単位でのWDT機能の意味がなくなる。）
#if ( VA300S == 1 || VA300S == 2 )
static ER lan_get_Pfail_cnt( UB *pval );	// VA300S　停電繰り返しカウンタの内容を、EEPROMから読み出す
#endif

static void SystemParamInit( void );		// VA300 システム制御パラメータのInitial
static UB GetScreenNo(void);				// 現在表示スクリーン番号の取得
static void ChgScreenNo( UB NewScreenNo );	// 画面遷移状態(番号)を更新する

static ER self_diagnosis( char mode );	//20140930Miya
static void Init_Cmr_Param(void);	//20140930Miya
static void SetError(int err);			//20140930Miya
ER static power_on_process( void );		// PCとのコミュニケーション開始、初期値設定要求
static ER set_initial_param_for_Lcd( void );
static ER set_reg_param_for_Lcd( void );
static ER set_initial_param( void );		// VA-300sの場合の、各種パラメータの初期設定。

static void LcdPosAdj(int calc);		//20161031Miya Ver2204 LCDADJ

#if ( VA300S == 0 || VA300S == 2 )
ER static send_WakeUp( void );			/** PCへWakeUp確認送信	**/
ER static send_camera_param_req( void );	/** カメラ・パラメータの初期値要求の送信	**/
ER static send_cap_param_req( void );		/** 画像処理の初期値要求の送信	**/	
ER static send_Led_param_req( void );		/** LED光量数値の初期値要求の送信	**/	
ER static send_touroku_param_req( void );	/** 登録データの初期値要求の送信	**/	

static ER	SndCmdCngMode( UINT stat );		// PCへモード切替え通知を送信

static ER send_nomal_mode_Wait_Ack_Retry( void ); // モード切替通知の送信、Ack・Nack待ちとリトライ付き（通常モード移行時）
static ER send_nomal_mode( void );			// モード切替通知の送信（通常モード移行時）
static ER send_Pfail_mode_Wait_Ack_Retry( void ); // モード切替通知の送信、Ack・Nack待ちとリトライ付き（停電モード移行時）
static ER send_Pfail_mode( void );			// モード切替通知の送信（停電モード移行時）
static ER send_shutdown_req_Wait_Ack_Retry( void ); // シャットダウン要求の送信、Ack・Nack待ちとリトライ付き
static ER send_shutdown_req( void );			// シャットダウン要求の送信
static ER send_touroku_delete_Wait_Ack_Retry( void );	// 指（登録）データの削除要求の送信、Ack・Nack待ちとリトライ付き
static ER send_touroku_delete_req( void );	// 指（登録）データの削除要求の送信（通常モード時）
static ER send_ID_No_check_req_Wait_Ack_Retry( void );	// ID番号問合せの送信、Ack・Nack待ちとリトライ付き
static ER send_ID_No_check_req( void );		// ID番号問合せの送信（通常モード時）
static ER send_ID_Authority_check_req_Wait_Ack_Retry( void );	// ID権限問合せの送信、Ack・Nack待ちとリトライ付き
static ER send_ID_Authority_check_req( void );	// ID権限問合せの送信（通常モード時）
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
#endif

static void reload_CAP_Param( void );		// Gain. FixShutter,IR_LEDのパラメータを、初期要求値に戻す。（CAPのRetry時に変わった可能性がある為。）
static UB GetSysSpec( void );		// マンション仕様/１対１仕様　状態を返す。0:マンション(占有部)仕様、1:1対１仕様。

static UB sys_kindof_spec = 0; // マンション仕様/１対１仕様　切替えフラグ　0:マンション(占有部)仕様、1:1対１（法人）仕様
						       // 			、2：マンション（共用部）仕様。3:デモ機・マンション(占有部)仕様、4:デモ機・1対１（法人）仕様 
							   // 			、5：デモ機・マンション（共用部）仕様。
static UB nyuu_shutu_kubun = 0;	// ユタカ仕様・入退室区分　0：未入退、1：入口、2：出口							   

static UH g_DipSwCode;	
						   
static int g_AuthCnt;	//20140423Miya 認証リトライ回数

static int ercdStat = 0;		// エラーコード記憶
static UINT sys_ScreenNo;		// 現在のスクリーンNo
static UINT sys_Mode;			// 状態変数
static UINT sys_SubMode;		// 状態変数（サブ）
static UINT sys_demo_flg;		// デモ仕様フラグ
static 	UB	s_CapResult;		// 指認証の結果
static UB s_DongleResult;		// ドングルの有無確認の結果
static UB s_PasswordResult;		// パスワード確認の結果
static UB s_ID_NO_Result;		// ID番号確認の結果
static UB s_ID_Authority_Result;  // ID権限確認の結果
static UB s_ID_Authority_Level; // ID権限問合せコマンドで問合せたユーザーIDの権限レベル。  ASCIIコード。
static UB s_Kantoku_num[ 2 ];	// ID権限問合せ応答コマンドで得た監督者の総数。  ASCIIコード。
static UB s_Kanri_num[ 2 ];		// ID権限問合せ応答コマンドで得た管理者の総数。 ASCIIコード。
static UB s_Ippan_num[ 6 ];		// ID権限問合せ応答コマンドで得た一般者の総数。 ASCIIコード。
static UB s_KinkyuuTourokuResult; // 緊急登録通知の結果
static FLGPTN befor_scrn_no;	// 画面遷移用変数。「中止しますか？」「いいえ」の時の戻り先のFLG_PTN番号。
static UB  befor_scrn_for_ctrl; // 画面遷移用変数。「中止しますか？」「いいえ」の時の戻り先の画面番号。

static UINT rcv_ack_nak;		// ack受信フラグ　=0：未受信、=1:ACK受信、=-1：nak受信

static UB req_restart = 0;		// パワー・オン・プロセスの再起動要求フラグ　=0:要求無し、=1:要求あり
static UB Pfail_mode_count;		// 停電モードの場合の起動回数(他モードで起動した場合は、resetされる。)
static UINT Pfail_sense_flg;	// 停電検知通知受信フラグ　0:受信無し、1:受信あり

static UINT g_CapTimes;			// 1:撮影1回目 2:再撮影	//20131210Miya add

static UH Flbuf[0x10000];		//フラッシュ退避用バッファ(1セクション分)

static UINT door_open_over_time;	// ドア過開放設定時間
static UINT Test_1sec_timer;		// 1秒サイクリック・カウンタ・テスト用
static UINT Pfail_start_timer;		// 停電タイマーサイクリック・カウント用
static unsigned long WDT_counter;	// WDTタイマー用サイクリック・カウンタ
static UINT main_TSK_wdt = FLG_ON;	// mainタスク　WDTリセット・リクエスト・フラグ
static UINT camera_TSK_wdt = FLG_ON;	// カメラタスク　WDTリセット・リクエスト・フラグ
static UINT ts_TSK_wdt = FLG_ON;		// タッチセンサ・タスク　WDTリセット・リクエスト・フラグ
static UINT lcd_TSK_wdt = FLG_ON;		// LCDタスク　WDTリセット・リクエスト・フラグ
static UINT sio_rcv_TSK_wdt = FLG_ON;	// SIO受信タスク　WDTリセット・リクエスト・フラグ
static UINT sio_snd_TSK_wdt = FLG_ON;	// SIO送信タスク　WDTリセット・リクエスト・フラグ

static UINT timer_10ms = 0, count_1sec = 0, count_1min = 0, count_1hour = 0;	// 時分秒カウント

static BOOL s_bMonRs485;		// モニタはRS485フラグ

static unsigned short FpgaVerNum;	//20140905Miya lbp追加 FPGAバージョンアップ
static UB f_fpga_ctrl=1;				//20140915Miya FPGA制御フラグ 0:V1.001用　1:V1.002用
static int g_key_arry[10];	//20140925Miya password open

static int g_Diagnosis_start=0;	//20140930Miya	//診断開始フラグ

#if defined(_DRV_TEST_)
// ドライバテスト
extern BOOL drvTest();

#endif

static int	g_SameImgGet;	//20151118Miya 同画像再撮影
static int	g_SameImgCnt;	//20160115Miya 同画像再撮影
static int	g_AuthType=0;		//20160120Miya 0:指認証 1:パスワード認証 2:緊急開錠
static int	ode_oru_sw;						//20160108Miya FinKeyS おでかけ・お留守番SW
static int	g_TestCap_start;
static UW	dbg_flwsize;
static UW	dbg_flwsizeIn;

static int	dbg_ts_flg;
static int	dbg_cam_flg;
static int	dbg_nin_flg;
static int	dbg_cap_flg;
static int g_MainteLvl;	//20160711Miya デモ機

static int	g_pcproc_f;	//20160930Miya PCからVA300Sを制御する //0:アイドル 1:認証 2:登録
static int	g_capallow; //20160930Miya PCからVA300Sを制御する
static int	g_pc_authnum; //20160930Miya PCからVA300Sを制御する
static int	g_pc_authtime; //20160930Miya PCからVA300Sを制御する

//20161031Miya Ver2204 LCDADJ ->
ST_POS_REV BaseT;
ST_POS_REV InputT;
ST_TPL_REV RevT;
BkDataNoClear g_BkDataNoClear;
static int	g_LedCheck;
//20161031Miya Ver2204 LCDADJ <-

static int dbg_dbg1;


//20170315Miya 400Finger ->
//static int g_RegBlockNum;		// 登録ブロック番号(1 ～ 7) -1:登録なし
static int g_RegAddrNum;		// 登録番地(0 ～ 239) -1:登録なし
//static int g_RegTotalYubiNum;	// 登録されている指の総数
static int g_BufAuthScore[240][20];	// 極小認証のスコアー
//static unsigned char g_taikyo_flg = 0;		//20170320Miya 400FingerM2 静脈退去フラグ
//20170315Miya 400Finger <-
static unsigned short g_FPGA_ErrFlg;	//20170706Miya FPGAフリーズ対策

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
	ER memerr1, memerr2;
	FLGPTN	flgptn;	

	int n, cnt, i;
	INT iMonCh, iLBusCh;
	
	UINT screen_timer;					// 画面表示タイマー
	UINT old_screenNo;					// 前回表示時の画面No
	UB ret_stat;
	UB sys_smt=0;		//20160112Miya FinKeyS
	
	int st_y;
	
	T_YBDATA *ybdata;

	int dbg_cnt1, dbg_cnt2;
	
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
	FlInit( SEM_FL );					// フラッシュメモリドライバ初期化
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
	}

#if ( VA300S == 0 || VA300S == 2 )	
	// UDP 通信初期化
	ercd = udp_ini( TSK_UDPRCV, ID_UDP_OYOCMD, udp_portno);	
//	ercd = udp_ini( 0, ID_UDP_OYOCMD, udp_portno);	
	if( ercd != E_OK) {
		ercdStat = 3;
	    ErrCodeSet( ercd );
		slp_tsk();
	}
#endif


LAN_INI_END:
	sta_tsk(TSK_DISP,    0);			// 表示タスク起動
	sta_tsk(TSK_IO, 0);					// I/O検知タスク起動
	
#if ( VA300S == 0 )		
	sta_tsk(TSK_COMMUNICATE, 0);		// UDP電文処理タスク
	
	sta_tsk(TSK_UDP_SEND, 0);			// UDP送信タスク
	sta_tsk(TSK_CMD_LAN,  0);			// コマンド処理タスク起動
#endif
#if ( VA300S == 1 )		
//	sta_tsk( TSK_NINSHOU, 0 );				// 認証タスク起動
	sta_tsk( TSK_LOG, 0 );					// ロギングタスク起動
#endif
#if ( VA300S == 2 )		
	sta_tsk(TSK_COMMUNICATE, 0);		// UDP電文処理タスク
	
	sta_tsk(TSK_UDP_SEND, 0);			// UDP送信タスク
	sta_tsk(TSK_CMD_LAN,  0);			// コマンド処理タスク起動

	sta_tsk( TSK_NINSHOU, 0 );				// 認証タスク起動
	sta_tsk( TSK_LOG, 0 );					// ロギングタスク起動
#endif
	
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

//#if ( USE_MON == 1 )
//	ercd = Mon_ini( iMonCh );			// モニタコマンドタスク初期化
//
//	sta_tsk(TSK_SND1, iMonCh);			// シリアル送信タスク起動
//#endif

	sta_tsk(TSK_SND1, 0);				// シリアル送信タスク起動
	sta_tsk(TSK_RCV1, 0);				// シリアル受信タスク起動
	
	LcdcInit( SEM_LCD );				// LCDコントローラの初期化

	CameraTaskInit( TSK_CAMERA );		// カメラコントロールタスク初期化
	LedInit(SEM_LED);					// LED初期化
	
	yb_init_all();						// 指情報(属性情報以外)、緊急開錠情報の初期化
	
	LedOut(LED_POW, LED_ON);			// 電源表示LEDをONする（橙）
	
PowerOn_Process:
	req_restart = 0;
	LedOut(LED_ERR, LED_OFF);			// 電源表示LEDをOFFする（赤）
	LedOut(LED_OK, LED_OFF);			// 電源表示LEDをOFFする（緑）

	//20160112Miya FinKeyS
	sys_smt = 0;
	ercd = lan_get_sys_spec( &sys_kindof_spec );	// VA300　仕様切替えフラグの内容を、LAN用 EEPROMから読み出す。

#if KOUJYOUCHK	//20160610Miya
	//sys_kindof_spec = 6;
#endif

	if (ercd != E_OK) {
		sys_kindof_spec = SYS_SPEC_MANTION;			// EEPROMから仕様切替えフラグ内容が読み出せない場合は、”マンション（占有部）仕様”をSet。
		sys_demo_flg = SYS_SPEC_NOMAL;
	}else{
		if(sys_kindof_spec > SYS_SPEC_SMT){
			ercd = lan_set_eep( EEP_SYSTEM_SPEC, SYS_SPEC_MANTION );	// 設定仕様情報のEEPROMへの書込み。
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
	ercd = lan_get_sys_spec( &sys_kindof_spec );	// VA300　仕様切替えフラグの内容を、LAN用 EEPROMから読み出す。
	if (ercd != E_OK) {
		sys_kindof_spec = SYS_SPEC_MANTION;			// EEPROMから仕様切替えフラグ内容が読み出せない場合は、”マンション（占有部）仕様”をSet。
		sys_demo_flg = SYS_SPEC_NOMAL;
	}	else {
		if ( ( sys_kindof_spec >= 0 ) && ( sys_kindof_spec <= 2 ) ){
			sys_demo_flg = SYS_SPEC_NOMAL;			// デモ仕様でない場合
		}	else	{
			if ( sys_kindof_spec == 3 ){
				sys_kindof_spec = SYS_SPEC_MANTION;	// マンション・占有部仕様
				sys_demo_flg = SYS_SPEC_DEMO;		// デモ仕様の場合
			} else if ( sys_kindof_spec == 4 ){
				sys_kindof_spec = SYS_SPEC_OFFICE;	// １対１仕様（オフィス仕様）
				sys_demo_flg = SYS_SPEC_DEMO;		// デモ仕様の場合
			} else if ( sys_kindof_spec == 5 ){
				sys_kindof_spec = SYS_SPEC_ENTRANCE;// マンション・共用部仕様
				sys_demo_flg = SYS_SPEC_DEMO;		// デモ仕様の場合
//			} else if ( sys_kindof_spec == 7 ){
//				sys_kindof_spec = SYS_SPEC_OFFICE_NO_ID;	// １対多仕様（オフィス・ID番号無し仕様）
//				sys_demo_flg = SYS_SPEC_DEMO;		// デモ仕様の場合
			} else {
				sys_kindof_spec = SYS_SPEC_MANTION;
				sys_demo_flg = SYS_SPEC_NOMAL;
			}	
		}
	}
*/
	
	ercd = set_flg( ID_FLG_LCD, FPTN_LCD_INIT );	// LCDの初期画面表示のリクエスト
	if ( ercd != E_OK ) ercdStat = 5;
	
#if defined(_DRV_TEST_)
	drvTest();
#endif
			
	dly_tsk( 500/MSEC );

PowerOn_Process2:
	dly_tsk( 500/MSEC );
	
	MdCngMode( MD_POWER_ON );			// 装置モードをパワーオンモードへ
	ercd = power_on_process();			// 制御Boxとのコミュニケーション開始、機器モード設定、初期値設定
	if(ercd == 1){
		goto PowerOn_Process;
	}

	//20140915Miya FPGA制御フラグ 0:V1.001用　1:V1.002用
	FpgaVerNum = *(volatile unsigned short *)(FPGA_BASE + 0x0000) ; // fpga version no.
	if( FpgaVerNum == 0x1001 ){
		f_fpga_ctrl = 0;
	}else{
		f_fpga_ctrl = 1;
	}

	g_InPasswordOpen.sw 		= FLG_OFF;	//パスワード開錠SW 0:OFF 1:ON
	g_InPasswordOpen.kigou_inp 	= FLG_OFF;	//記号入力SW 0:OFF 1:ON
	g_InPasswordOpen.hide_num 	= FLG_OFF;	//非表示SW 0:OFF 1:ON(非表示実施、｢*｣表示)
	g_InPasswordOpen.random_key = FLG_OFF;	//キーボードランダム表示SW 0:OFF 1:ON
	g_InPasswordOpen.keta 		= 4;		//入力ケタ数(4～8)
	for(i = 0 ; i < 8 ; i++ ){
		g_InPasswordOpen.password[i] = 20;	//パスワード(0～19) 初期値:20
	}
			
	main_TSK_wdt = FLG_ON;					// WDTカウンタ用メモリクリア(20秒でWDT起動 = パワーオン・リセット)
	
	// 画面表示の開始
	if ( MdGetMode() == MD_INITIAL ){ 	// 初期登録画面の時は、画面１へ。
		//20160112Miya FinKeyS スマート制御版設定から初期化につき、スマート制御版設定にする
		if( sys_smt == SYS_SPEC_SMT ){
			g_TechMenuData.SysSpec = 2;
			g_PasswordOpen.sw = 1;
			g_PasswordOpen2.family_sw = 1;
			SaveBkAuthDataFl();
			ercd = lan_set_eep( EEP_SYSTEM_SPEC, SYS_SPEC_MANTION );	// 設定仕様情報のEEPROMへの書込み。
			sys_kindof_spec = SYS_SPEC_MANTION;
			sys_demo_flg = SYS_SPEC_NOMAL;
		}
		
		if ( GetSysSpec() == SYS_SPEC_MANTION ){	// マンション(占有部)仕様の場合。
		
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN1 );	// ID番号入力画面へ。	
			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN1 );	// 画面番号　<-　次の画面
			}
			
		} else if ( GetSysSpec() == SYS_SPEC_OFFICE ){		// １対１仕様の場合。
		
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN401 );	// 初期登録メニュー画面へ。	
			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN401 );	// 画面番号　<-　次の画面
			}						
		} else if ( GetSysSpec() == SYS_SPEC_KOUJIM || GetSysSpec() == SYS_SPEC_KOUJIO || GetSysSpec() == SYS_SPEC_KOUJIS ){//20160112Miya FinKeyS		// 工事画面
			g_PasswordOpen2.family_sw = 0;
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN620 );	// 通常モード入力待機画面へ。（１対１仕様）	
			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN620 );	// 画面番号　<-　次の画面
			}						
		}
				
	} else if ( ( MdGetMode() == MD_NORMAL ) || ( MdGetMode() == MD_PFAIL ) ){	// 通常モード、停電モードの時。

		if ( GetSysSpec() == SYS_SPEC_MANTION ){	// マンション(占有部)仕様の場合。

			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// 通常モード入力待機画面へ。	
			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN100 );	// 画面番号　<-　次の画面
			}
			
		} else if ( GetSysSpec() == SYS_SPEC_OFFICE ){		// １対１仕様の場合。
		
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN500 );	// 通常モード入力待機画面へ。（１対１仕様）	
			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN500 );	// 画面番号　<-　次の画面
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

	//ercd = set_flg( ID_FLG_CAMERA, FPTN_CHECK_IMAGE );			// メモリチェック
/*	
	cnt = 0;
	Check_Cap_Raw_flg = 1;						// 2014.6.23 Added T.Nagai 撮影中フラグのセット
	while(1){
		dly_tsk( 1000/MSEC );
		if ( Check_Cap_Raw_flg == 0 ){			// 2014.6.23 Modify T.Nagai 撮影中フラグのセット
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
	//reload_CAP_Param();	//カメラの初期値設定
	
	//カメラ動作確認
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
	screen_timer = 0;							// 画面表示タイマーの初期化。
	old_screenNo = GetScreenNo();				// 現在表示中の画面Noを保存。
				
	main_TSK_wdt = FLG_ON;							// WDTカウンタ用メモリクリア(20秒でWDT起動 = パワーオン・リセット)
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
		
	// 画面切替えと、カメラ撮影処理
	for (;;) {
		ercd = twai_flg( ID_FLG_MAIN, ( FPTN_START_CAP			/* カメラ撮影要求の受信待ち */
									  | FPTN_LCD_CHG_REQ		/* LCD画面切替要求(LCD→メイン) */
									  | FPTN_SEND_REQ_MAINTE_CMD /* メンテナンスモード切替え通知送信(受信コマンド解析タスク→メインTask)　*/
									  ), TWF_ORW, &flgptn, 1000/MSEC );
		if ( ercd == E_TMOUT ){
			
			main_TSK_wdt = FLG_ON;					// WDTカウンタ用メモリクリア(20秒でWDT起動 = パワーオン・リセット)
		
			if ( GetScreenNo() == old_screenNo ){	// 同一画面が15秒間続いて、かつ、停電モードまたは停電検知通知受信の場合は、強制的にシャットダウン。
				if ( screen_timer >= 15 ){
					if ( ( Pfail_sense_flg == PFAIL_SENSE_ON ) || ( MdGetMode() == MD_PFAIL ) ){
						
						Pfail_shutdown();			// シャットダウンの実行
						
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
				
				// 停電検知通知を受信なら、認証操作中でなければシャットダウン
				ret_stat = Chk_shutdown_ok();				
				if ( ret_stat == SHUTDOWN_OK ){
						
						Pfail_shutdown();			// シャットダウンの実行
						
				}	else	{
					screen_timer = 0;
					old_screenNo = GetScreenNo();	// 現在表示中の画面Noを保存。
					continue;
				}	

			}	else	{
				screen_timer = 0;
				old_screenNo = GetScreenNo();		// 現在表示中の画面Noを保存。
				continue;
			}
			
		} else if ( ercd != E_OK ){
			ercdStat = 7;
			break;
		}
		
		screen_timer = 0;							// 画面表示タイマーの初期化。
		old_screenNo = GetScreenNo();				// 現在表示中の画面Noを保存。
		
		main_TSK_wdt = FLG_ON;						// WDTカウンタ用メモリクリア(20秒でWDT起動 = パワーオン・リセット)
		
		switch ( flgptn ) {
			case FPTN_LCD_CHG_REQ: // LCDタスクから、画面変更要求あり
				ercd = 0;	
				ercd = clr_flg( ID_FLG_MAIN, ~FPTN_LCD_CHG_REQ );			// フラグのクリア
				if ( ercd != E_OK ){
					ercdStat = 10;
					break;
				}
				
				//if ( GetSysSpec() == SYS_SPEC_MANTION ){	// マンション(占有部)仕様の場合。
				if ( GetSysSpec() == SYS_SPEC_MANTION || GetSysSpec() == SYS_SPEC_KOUJIM || GetSysSpec() == SYS_SPEC_KOUJIO || GetSysSpec() == SYS_SPEC_KOUJIS ){	// マンション(占有部)仕様の場合。
				
					ercd = NextScrn_Control_mantion();		// 次の画面表示要求フラグセット（マンション(占有部)仕様の場合）
						
				} else if ( GetSysSpec() == SYS_SPEC_OFFICE ){		// １対１仕様の場合。
				
					ercd = NextScrn_Control_office();		// 次の画面表示要求フラグセット（１対１仕様の場合）
				
				} else {
					nop();	//エラー処理の記述
				}
				
				ercd = NextScrn_Control();					// 次の画面表示要求フラグセット（共通仕様の場合）
				if(GetScreenNo() == LCD_SCREEN110 && g_Diagnosis_start == 1){
						
					ercd = self_diagnosis(2);
					g_Diagnosis_start = 0;

					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN101 );
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN101 );			// 画面番号　<-　次の画面
					}
				}
				if(GetScreenNo() == LCD_SCREEN613 && g_Diagnosis_start == 1){	//20160108Miya FinKeyS
						
					ercd = self_diagnosis(2);
					g_Diagnosis_start = 0;

					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN601 );
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN601 );			// 画面番号　<-　次の画面
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

					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN204 );	// 自己診断結果画面へ。	
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN204 );			// 画面番号　<-　次の画面
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
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN267 );	// メンテナンス・メニュー画面(設定変更)へ
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN267 );		// 画面番号　<-　次の画面
					}
				}
				
				break;
				
			case FPTN_START_CAP:	// 生体検知センサーから、カメラ撮影要求あり	
			
				ercd = clr_flg( ID_FLG_MAIN, ~FPTN_START_CAP );				// フラグのクリア
				if ( ercd != E_OK ){
					ercdStat = 8;
					break;
				}
			
				if ( ( ( MdGetMode() == MD_NORMAL ) || ( MdGetMode() == MD_PFAIL ) )			// ノーマルモードで、画面101,102,121,141,161表示中なら
				    && ( ( GetScreenNo() == LCD_SCREEN101 ) 	// マンション(占有部)仕様の場合
					  || ( GetScreenNo() == LCD_SCREEN102 ) 
					  || ( GetScreenNo() == LCD_SCREEN105 ) 	//20140423Miya 認証リトライ追加
					  || ( GetScreenNo() == LCD_SCREEN121 ) 
					  || ( GetScreenNo() == LCD_SCREEN141 )
					  || ( GetScreenNo() == LCD_SCREEN161 )
					  || ( GetScreenNo() == LCD_SCREEN181 )
					  || ( GetScreenNo() == LCD_SCREEN601 )		// 20160108Miya FinKeyS
					  || ( GetScreenNo() == LCD_SCREEN602 )		// 20160108Miya FinKeyS
					  || ( GetScreenNo() == LCD_SCREEN605 )		// 20160108Miya FinKeyS
					  || ( GetScreenNo() == LCD_SCREEN610 )		// 20160108Miya FinKeyS
					  || ( GetScreenNo() == LCD_SCREEN611 )		// 20160108Miya FinKeyS
					  || ( GetScreenNo() == LCD_SCREEN503 )		// １対１仕様の場合
					  || ( GetScreenNo() == LCD_SCREEN523 )
					  || ( GetScreenNo() == LCD_SCREEN544 ) ) ){
						  
//					reload_CAP_Param();			// Gain. FixShutter,IR_LEDのパラメータを、初期要求値に戻す。（CAPのRetry時に変わった可能性がある為。）

					ercd = set_flg( ID_FLG_CAMERA, FPTN_START_CAP211 );				// カメラ撮影+認証処理（コマンド211）へ。
					if ( ercd != E_OK ) break;
					
				} else if ( ( MdGetMode() == MD_NORMAL ) || ( MdGetMode() == MD_PFAIL ) ){			// ノーマルモードで、画面127、画面129表示中なら
			
					if ( ( GetScreenNo() == LCD_SCREEN127 )  	// マンション(占有部)仕様の場合
					  || ( GetScreenNo() == LCD_SCREEN129 )
					  || ( GetScreenNo() == LCD_SCREEN530 )		// １対１仕様の場合
					  || ( GetScreenNo() == LCD_SCREEN532 ) ){
						
//						if ( GetScreenNo() == LCD_SCREEN127 ){
//							reload_CAP_Param();			// Gain. FixShutter,IR_LEDのパラメータを、初期要求値に戻す。（CAPのRetry時に変わった可能性がある為。）
//						} 

						ercd = set_flg( ID_FLG_CAMERA, FPTN_START_CAP204 );			// カメラ撮影+登録処理（コマンド204）へ。
						if ( ercd != E_OK ) break;
					}
				} else if ( MdGetMode() == MD_INITIAL ){		// 初期登録モードで、画面6、画面8表示中なら
			
					if ( ( GetScreenNo() == LCD_SCREEN6 )   	// マンション(占有部)仕様の場合
					  || ( GetScreenNo() == LCD_SCREEN8 )
					  || ( GetScreenNo() == LCD_SCREEN405 )		// １対１仕様の場合
					  || ( GetScreenNo() == LCD_SCREEN407 ) ){
						  
//						if ( GetScreenNo() == LCD_SCREEN6 ){
//							reload_CAP_Param();			// Gain. FixShutter,IR_LEDのパラメータを、初期要求値に戻す。（CAPのRetry時に変わった可能性がある為。）
//						}
												
						ercd = set_flg( ID_FLG_CAMERA, FPTN_START_CAP204 );			// カメラ撮影+登録処理（コマンド204）へ。
						if ( ercd != E_OK ) break;
					}
				} else if ( MdGetMode() == MD_MAINTE ){		// メンテナンスモードで、画面6、画面203表示中なら
/*			
					if ( GetScreenNo() == LCD_SCREEN203 ){
						
//						reload_CAP_Param();			// Gain. FixShutter,IR_LEDのパラメータを、初期要求値に戻す。（CAPのRetry時に変わった可能性がある為。）

						ercd = set_flg( ID_FLG_CAMERA, FPTN_START_CAP141 );			// カメラ撮影+フル画像処理（コマンド141）へ。
						if ( ercd != E_OK ) break;
					}
*/
				}
				break;

			case FPTN_SEND_REQ_MAINTE_CMD:	// 受信コマンド解析タスク→メインTaskへ、メンテナンスモード切替え通知送信を依頼。

				ercd = clr_flg( ID_FLG_MAIN, ~FPTN_SEND_REQ_MAINTE_CMD );			// フラグのクリア
				if ( ercd != E_OK ){
					ercdStat = 9;
					break;
				}
				
#if ( VA300S == 0 )				
				ercd = SndCmdCngMode( (UINT)MD_MAINTE );	// PCへ	メンテナンスモード切替え通知を送信
				if ( ercd != E_OK ){
					nop();		// エラー処理の記述	
				}
#endif
				
			default:
				break;
		}

//		if(dbg_dbg1 == 0){
//			dbg_dbg1 = 1;
//				MakeTestImg();	//20160902Miya FPGA高速化 forDebug
//				WrtImgToRam(0, 0);	//[撮影][R1]
//				WrtImgToRam(1, 0);	//[登録][R1]
//				WrtImgToRam(0, 1);	//[撮影][R2]
//				WrtImgToRam(1, 1);	//[登録][R2]
//		}			
		
		if ( req_restart == 1 ) goto PowerOn_Process;
		
	}
	PrgErrSet();
	slp_tsk();		//　ここへ来る時は実装エラー
}



/*==========================================================================*/
/**
 *	自己診断処理
 *	引数	char mode;		//0:テスト撮影のみ 1:自己診断(メニュー) 2:自己診断(タイマー)
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

//撮影テスト1(カメラ感度を変えて、感度違いの画像得る)
	size = iReSizeX * iReSizeY;
	for(i = 0 ; i < loop ; i++){
		Check_Cap_Raw_flg = 1;
		ercd = set_flg( ID_FLG_CAMERA, FPTN_CHECK_IMAGE );			// メモリチェック
			 
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

//カメラ故障診断
	cnt = 0;
	for(i = 0 ; i < size/2 ; i++){
		if( Flbuf[i] == 0x0000 ){
			++cnt;
		}
	}
	if( cnt == size/2 ){
		result = result | 0x04;	//BIT3
	}

//異物チェック
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

//画像メモリ R/Wテスト
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
	if( result & 0x01 )		g_MainteLog.diag_buff[cnt][0] = 1;	//診断エラーあり(撮影異常)
	else					g_MainteLog.diag_buff[cnt][0] = 0;	//診断エラーなし

	if( result & 0x02 )		g_MainteLog.diag_buff[cnt][1] = 1;	//診断エラーあり(メモリリフレッシュ異常)
	else					g_MainteLog.diag_buff[cnt][1] = 0;	//診断エラーなし

	if( result & 0x04 )		g_MainteLog.diag_buff[cnt][2] = 1;	//診断エラーあり(カメラ異常)
	else					g_MainteLog.diag_buff[cnt][2] = 0;	//診断エラーなし

	if( result & 0x08 )		g_MainteLog.diag_buff[cnt][3] = 1;	//診断エラーあり(異物異常)
	else					g_MainteLog.diag_buff[cnt][3] = 0;	//診断エラーなし

	if( result & 0x10 )		g_MainteLog.diag_buff[cnt][4] = 1;	//診断エラーあり(メモリRW異常)
	else					g_MainteLog.diag_buff[cnt][4] = 0;	//診断エラーなし

	if( result & 0x20 )		g_MainteLog.diag_buff[cnt][4] = 1;	//診断エラーあり(メモリRW異常)
	else					g_MainteLog.diag_buff[cnt][4] = 0;	//診断エラーなし

	if( result & 0x40 )		g_MainteLog.diag_buff[cnt][4] = 1;	//診断エラーあり(メモリRW異常)
	else					g_MainteLog.diag_buff[cnt][4] = 0;	//診断エラーなし

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

	//if(mode == 2 && result > 0){	//時々撮影関連のエラーが発生するが復帰するのでエラー表示しなしい
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
	
	irDuty2 = ini_irDuty2;		// IR Duty値irDuty2　初期値;
	irDuty3 = ini_irDuty3;
	irDuty4 = ini_irDuty4;
	irDuty5 = ini_irDuty5;

	cmrGain = ini_cmrGain;						// カメラ・ゲイン値初期値　
	cmrFixShutter1 = ini_cmrFixShutter1;		// Fix Shutter Control値初期値(１回目)　
	cmrFixShutter2 = ini_cmrFixShutter2;		// Fix Shutter Control値初期値(２回目)			同上
	cmrFixShutter3 = ini_cmrFixShutter3;		// Fix Shutter Control値初期値(３回目）

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

	if( err == 39 || err == 40 ){			//カメラシーケンスエラー
		g_MainteLog.cmr_seq_err_f = g_MainteLog.cmr_seq_err_f | 0x01;
	}else if( err == 41 ){		//カメラWakeUpエラー
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
 *	制御Boxとのコミュニケーション開始、各種初期値設定要求メイン
 */
/*==========================================================================*/
static ER power_on_process( void )
{
	ER ercd = 0;
	int Retry;
	int i, j;
	
	/** 制御BoxへWakeUp確認送信	**/
	MdCngSubMode( SUB_MD_WAKEUP );			// サブモードを、WakeUp問い合わせへ設定

									// 宮内さんへ。2014.9.29
									// 以下の解錠リモコンの有無設定を、フラッシュ・メモリから読出して初期設定して下さい。	
	ercd = ReadBkAuthData();	//20141003Miya add set_initial_param()から移動

	ercd = ReadBkDataNoClearFl();	//20161031Miya Ver2204 LCDADJ
	//ercd = SaveBkDataNoClearFl();	//20161031Miya Ver2204 LCDADJ
	LcdPosAdj(0);					//20161031Miya Ver2204 LCDADJ
	
	// 錠制御基板（VA-300,VA-300ｓ兼用）バージョン番号の初期化
	//KeyIO_board_soft_VER[ 0 ] = '2';
	//KeyIO_board_soft_VER[ 1 ] = '.';
	//KeyIO_board_soft_VER[ 2 ] = '0';
	//KeyIO_board_soft_VER[ 3 ] = '2';
	KeyIO_board_soft_VER[ 0 ] = '1';
	KeyIO_board_soft_VER[ 1 ] = '.';
	KeyIO_board_soft_VER[ 2 ] = '0';
	KeyIO_board_soft_VER[ 3 ] = '0';

//20140125Miya nagaiBug修正　
//while(1)内で、send_sio_WakeUp()をコールするので、ACK成立前にコマンド送出され通信が成立しない。
//#if ( VA300S == 1 || VA300S == 2 )	
//#if ( VA300S == 1 )	//20140905Miya
//#if ( VA300S == 1 && AUTHTEST == 0 )	//20160715Miya
#if ( VA300S == 1 && AUTHTEST == 0 && PCCTRL == 0 )	//20160930Miya PCからVA300Sを制御する

	sio_mode = SIO_SEND_MODE;			// シリアル通信を、送信モードへ設定。
	for(i = 0 ; i < 3 ; i++){
	
		send_sio_WakeUp();					// VA300S制御BoxへWakeUpを問い合わせ。
		j = 0;	
		while(1){
			dly_tsk( 1200/MSEC );			// 変更2014.4.21 T.Nagai 停電からの再スタート時のTestコマンド返信受信のタイミング調整
//			dly_tsk( 1000/MSEC );
					
			if ( MdGetSubMode() == SUB_MD_IDLE ){	// WakeUp問い合わせに対して、制御BoxからのTestコマンド応答完了を待つ。
				j = 0;
				ercd = set_initial_param();		// システム・初期モード、カメラ・パラメータ、画像処理パラメータ、
											// LED光量、登録データ、などの各種パラメータの初期化を行う。
											// VA-300sの場合、制御Boxからパラメータ受信は無い為、上記を自ら設定する必要がある。


				send_sio_Touroku_InitAll();		// VA300S制御Boxへシリアルで登録完了コマンド(01)を送信(電源ON時の一括送信)。

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
		dly_tsk( 5000/MSEC );				// 変更2014.4.21 T.Nagai 停電からの再スタート時のTestコマンド返信受信のタイミング調整
//		dly_tsk( 1000/MSEC );
		ercd = 1;
		return ercd;
	}
	
	send_sio_Kakaihou_time();				// VA300s制御Boxへ過開放時間の設定要求コマンドを送信する。
	MdCngSubMode( SUB_MD_KAKAIHOU_TIME );	// サブモードを、過開放時間の設定要求送信中へ。
	j = 0;
	while( MdGetSubMode() != SUB_MD_IDLE ){	// 制御Boxからの過開放時間の設定要求コマンド応答完了を待つ。
		dly_tsk( 100/MSEC );
		j++;
		if ( j >= 30 ){						// ３秒経過後、Ack応答無い場合は、PaowerONProcess再処理へ。
			LedOut(LED_OK, LED_ON);
			LedOut(LED_ERR, LED_ON);
			ercd = 1;
			return ercd;
		}
	}
	g_MainteLvl = 0;	//20160711Miya デモ機
	if(FORDEMO == 1){	//20160711Miya デモ機
		g_TechMenuData.DemoSw = FLG_ON;
		g_PasswordOpen.sw = FLG_ON;
	}
#else
	MdCngSubMode( SUB_MD_IDLE );	//　サブモード・ステータスを、”アイドル”へ。
	ercd = set_initial_param();		// システム・初期モード、カメラ・パラメータ、画像処理パラメータ、
	dip_sw_data[0] = 1;
#endif


#if ( VA300S == 2 )	//20140905Miya
	MdCngSubMode( SUB_MD_IDLE );	//　サブモード・ステータスを、”アイドル”へ。
	ercd = set_initial_param();		// システム・初期モード、カメラ・パラメータ、画像処理パラメータ、
	dip_sw_data[0] = 1;
#endif


//#else
#if ( VA300S == 0 )	//20140905Miya
	while ( 1 ){							// WakeUp問い合わせのPCからの応答完了が来るまで、１秒周期で繰り返し。
		ercd = send_WakeUp();				// PCへWakeUpを問い合わせ。
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
#endif

/*
	while ( 1 ){							// WakeUp問い合わせのPCからの応答完了が来るまで、１秒周期で繰り返し。

#if ( VA300S == 1 || VA300S == 2 )	
		sio_mode = SIO_SEND_MODE;			// シリアル通信を、送信モードへ設定。
		send_sio_WakeUp();					// VA300S制御BoxへWakeUpを問い合わせ。
		
		dly_tsk( 1000/MSEC );
		
		if ( MdGetSubMode() == SUB_MD_IDLE ){	// WakeUp問い合わせに対して、制御BoxからのTestコマンド応答完了を待つ。
	
			ercd = set_initial_param();		// システム・初期モード、カメラ・パラメータ、画像処理パラメータ、
											// LED光量、登録データ、などの各種パラメータの初期化を行う。
			break;							// VA-300sの場合、制御Boxからパラメータ受信は無い為、上記を自ら設定する必要がある。
	
		}

#else
		ercd = send_WakeUp();				// PCへWakeUpを問い合わせ。
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
#endif
	}
*/

#if ( VA300S == 1 || VA300S == 2 )	

	//　カメラゲイン設定
	cmrGain = 7;
	ini_cmrGain = 7;
	//　露出１設定
	cmrFixShutter1 = 2;
	ini_cmrFixShutter1 = 2;
	//　露出２設定
	cmrFixShutter2 = 4;
	ini_cmrFixShutter2 = 4;
	//　露出３設定
	cmrFixShutter3 = 6;
	ini_cmrFixShutter3 = 6;

	// IR LEDの点灯初期値
	irDuty2 = 255;		
	irDuty3 = 255;
	irDuty4 = 255;
	irDuty5 = 0;
	// IR LEDの点灯初期値を記憶
	ini_irDuty2 = 255;
	ini_irDuty3 = 255;
	ini_irDuty4 = 255;
	ini_irDuty5 = 0;

#if (NEWCMR == 0)	//20160601Miya
	// 画像切り出しサイズ指定
	iSizeX = 640;
	iSizeY = 560;
	// 	トリミングの座標を設定する
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
	// 画像切り出しサイズ指定
	iSizeX = 400;
	iSizeY = 160;
	// 	トリミングの座標を設定する
	iStartX = 180;	
	iStartY = 160;	//(480 - 160) / 2 = 160

	if(g_BkDataNoClear.LedPosi == 1){	//20161031Miya Ver2204 日光対策 開始座標を奥に
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

	// リサイズの縮小率を設定する
	iReSizeMode = RSZ_MODE_1;	//< 辺1/4
	iReSizeX = iSizeX / 4;		//20131210Miya add
	iReSizeY = iSizeY / 4;		//20131210Miya add
/*
	if(g_DipSwCode != 0){
		if( g_DipSwCode == 0x10 ){
			//　カメラゲイン設定
			cmrGain = 8;
			ini_cmrGain = 8;
		}
		if( g_DipSwCode == 0x20 || g_DipSwCode == 0x30 ){
			//　カメラゲイン設定
			cmrGain = 9;
			ini_cmrGain = 9;
		}
		if( g_DipSwCode == 0x30 ){
			//　カメラゲイン設定
			cmrGain = 10;
			ini_cmrGain = 10;
		}
		
	}
*/
	InitImgAuthPara();	//認証パラメータ

	//reload_CAP_Param();	//カメラの初期値設定

	ercd = 0;
	
//#if ( VA300S == 0 ) 
#else	
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

	while ( MdGetSubMode() != SUB_MD_IDLE ){	// LED光量数値の初期値要求のPCからの応答完了を待つ。
		dly_tsk( 25/MSEC );	
	}
	

	// マンション(占有部)仕様の場合のみ、登録データの初期値要求を行う。
	if ( GetSysSpec() == SYS_SPEC_MANTION ){
		
		/** 登録データの初期値要求の送信	**/	
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
	}
#endif	
	
	return ercd;
	
}


/*==========================================================================*/
/*
 *	LCD表示用のパラメータ初期化
 */
/*==========================================================================*/
static ER set_initial_param_for_Lcd( void )
{
	ER ercd = E_OK;
	int i, j;
	
	// 緊急開錠電話番号
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

	// 棟番号
	yb_touroku_data.tou_no[ 0 ] = '0';
	yb_touroku_data.tou_no[ 1 ] = '0';
	yb_touroku_data.tou_no[ 2 ]	= ',';
	// ユーザーID
	yb_touroku_data.user_id[ 0 ] = '0';	
	yb_touroku_data.user_id[ 1 ] = '0';
	yb_touroku_data.user_id[ 2 ] = '0';
	yb_touroku_data.user_id[ 3 ] = '0';
	yb_touroku_data.user_id[ 4 ] = ',';

	for( j = 0 ; j < 21 ; j++ ){	//　受信配列情報が、２０回分→２１回分に変更。2013.7.15
		yb_touroku_data20[j].yubi_seq_no[ 0 ]	= '0';	// 責任者/一般者の登録指情報
		yb_touroku_data20[j].yubi_seq_no[ 1 ]	= '0';
		yb_touroku_data20[j].yubi_seq_no[ 2 ]	= '0';
		yb_touroku_data20[j].yubi_seq_no[ 3 ]	= ',';

		yb_touroku_data20[j].kubun[ 0 ]			= 0;	// 責任者/一般者区分、デホルトは”一般者”
		yb_touroku_data20[j].kubun[ 1 ]			= ',';

		yb_touroku_data20[j].yubi_no[ 0 ]		= '0';	// 登録指番号（指種別）
		yb_touroku_data20[j].yubi_no[ 1 ]		= '0';
		yb_touroku_data20[j].yubi_no[ 2 ]		= ',';
		
		for ( i=0; i<24; i++ ){
			yb_touroku_data20[j].name[ i ] = 0;	
		}
	}
	// 責任者/一般者の登録指情報
	yb_touroku_data.yubi_seq_no[ 0 ]	= yb_touroku_data20[ 1 ].yubi_seq_no[ 0 ];	
	yb_touroku_data.yubi_seq_no[ 1 ]	= yb_touroku_data20[ 1 ].yubi_seq_no[ 1 ];
	yb_touroku_data.yubi_seq_no[ 2 ]	= yb_touroku_data20[ 1 ].yubi_seq_no[ 2 ];
	yb_touroku_data.yubi_seq_no[ 3 ]	= ',';
	// 責任者/一般者区分、デホルトは”一般者”
	yb_touroku_data.kubun[ 0 ]			= yb_touroku_data20[ 1 ].kubun[ 0 ];	
	yb_touroku_data.kubun[ 1 ]			= ',';
	// 登録指番号（指種別）
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
 *	LCD表示用のパラメータに登録データをセットする
 */
/*==========================================================================*/
static ER set_reg_param_for_Lcd( void )
{
	ER ercd = E_OK;
	int i, j;
	unsigned short uw_dat0, uw_dat1000, uw_dat100, uw_dat10, uw_dat1;
	UB ubtmp;

	// 緊急開錠電話番号
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

	// 棟番号
	uw_dat0 = g_RegUserInfoData.BlockNum;
	uw_dat10 = uw_dat0 / 10;
	uw_dat1  = uw_dat0 - 10 * uw_dat10;
	yb_touroku_data.tou_no[ 0 ] = (UB)uw_dat10 + 0x30;
	yb_touroku_data.tou_no[ 1 ] = (UB)uw_dat1 + 0x30;
	yb_touroku_data.tou_no[ 2 ]	= ',';
	// ユーザーID
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

	//for( j = 0 ; j < 21 ; j++ ){	//　受信配列情報が、２０回分→２１回分に変更。2013.7.15
	for( j = 0 ; j < 20 ; j++ ){	//　VA300Sでは、受信配列情報が、２０回 2014.01.31
		// 責任者/一般者の登録指情報
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

		// 責任者/一般者区分、デホルトは”一般者”
		uw_dat0 = g_RegBloodVesselTagData[j].Level;
		uw_dat1 = uw_dat0;
		yb_touroku_data20[j+1].kubun[ 0 ]			= (UB)uw_dat1 + 0x30;
		yb_touroku_data20[j+1].kubun[ 1 ]			= ',';

		// 登録指番号（指種別）
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
	// 責任者/一般者の登録指情報
	yb_touroku_data.yubi_seq_no[ 0 ]	= yb_touroku_data20[ 1 ].yubi_seq_no[ 0 ];	
	yb_touroku_data.yubi_seq_no[ 1 ]	= yb_touroku_data20[ 1 ].yubi_seq_no[ 1 ];
	yb_touroku_data.yubi_seq_no[ 2 ]	= yb_touroku_data20[ 1 ].yubi_seq_no[ 2 ];
	yb_touroku_data.yubi_seq_no[ 3 ]	= ',';
	// 責任者/一般者区分、デホルトは”一般者”
	yb_touroku_data.kubun[ 0 ]			= yb_touroku_data20[ 1 ].kubun[ 0 ];	
	yb_touroku_data.kubun[ 1 ]			= ',';
	// 登録指番号（指種別）
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
 *	システム・初期モード、カメラ・パラメータ、画像処理パラメータ、
 *   LED光量、登録データ、などの各種パラメータの初期化を行う。）
 */
/*==========================================================================*/
static ER set_initial_param( void )
{
	ER ercd = E_OK;

	g_DipSwCode = DswGet();

	//ercd = InitFlBkAuthArea();//
	//ercd = ReadBkAuthData();	//20141003Miya del power_on_process()の先頭へ移動

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
#if(AUTHTEST >= 1 || PCCTRL == 1)	//20160930Miya PCからVA300Sを制御する
		if( g_RegUserInfoData.RegSts == 0 ){	//登録データなし
			ercd = InitRegImgArea();
			g_RegUserInfoData.RegSts = 1;	//20160601Miya forDebug
			SaveBkAuthDataFl();
		}
#endif

		if( g_RegUserInfoData.RegSts == 0 ){	//登録データなし
			ercd = InitRegImgArea();
			MdCngMode( MD_INITIAL );
			set_initial_param_for_Lcd();
			Pfail_mode_count = 0;
		}else{									//登録データあり
			ercd = ReadRegImgArea(0);
			ercd = ReadRegImgArea(10);
			ercd = AddRegImgFromRegImg(0, 0);		//20160312Miya 極小精度UP
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
	
	//MdCngMode( MD_INITIAL );		// システム・初期モードを、INITIALへ。
	
	g_AuthOkCnt = 0;
	CmrReloadFlg = 0;

	g_AuthType = 0;	//20160120Miya
	ode_oru_sw = 0;	//20160120Miya
	
	dbg_ts_flg = 0;
	dbg_cam_flg = 0;
	dbg_nin_flg = 0;
	dbg_cap_flg = 0;

	DBG_send_cnt = 0;
	
	ercd = lan_get_Pfail_cnt( &Pfail_mode_count );		// VA300S　停電時起動繰り返しカウンタの内容を、LAN用 EEPROMから読み出す。
	if ( Pfail_sense_flg == PFAIL_SENSE_ON ){		// Testコマンドで、停電中を受信？
		Pfail_mode_count++;
		ercd = lan_set_eep( EEP_PFAIL_CNT, Pfail_mode_count );	// EEPROMへインクリメントしたカウンタの内容を書込み。
		
		if ( MdGetMode() != MD_INITIAL ){			// 初期モード以外の場合は、停電モードへ移行。
			MdCngMode( MD_PFAIL );
			Pfail_sense_flg = PFAIL_SENSE_NO;		// 停電モード移行後は、制御Boxからの停電通知フラグをOFFする。
													// 停電通知フラグをONしたままだと、ただちにSHUTDown処理に入ってしまう為。
		}
		
	} else {										// Testコマンドで、正常電源供給中を受信
		Pfail_mode_count = 0;						// 停電モードでの起動回数を初期化。
		ercd = lan_set_eep( EEP_PFAIL_CNT, Pfail_mode_count );	// EEPROMへインクリメントしたカウンタの内容を書込み。
		dly_tsk( 100/MSEC );
	}
	
	door_open_over_time = 30;		// 	ドア過開放時間デホルト（30秒）

	g_pcproc_f = 0;	//20160930Miya PCからVA300Sを制御する
	g_capallow = 0; //20160930Miya PCからVA300Sを制御する

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
		
			Pfail_mode_count = 0;	// 停電モードの場合の起動回数
			
			break;
			
		case MD_MAINTE:			///< メンテナンスモード	

			ercd = send_meinte_mode_Wait_Ack_Retry();
			
			Pfail_mode_count = 0;	// 停電モードの場合の起動回数

	    	break;
			
		case MD_NORMAL:			///< 通常モード

			ercd = send_nomal_mode_Wait_Ack_Retry();
			
			Pfail_mode_count = 0;	// 停電モードの場合の起動回数

	    	break;
			
		case MD_PFAIL:			///< 停電時動作モード
		
			ercd = send_Pfail_mode_Wait_Ack_Retry();
			
	    	break;
			
		case MD_PANIC:			///< 非常時開錠モード
		
			Pfail_mode_count = 0;	// 停電モードの場合の起動回数
			
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
//	モード切替通知の送信、Ack・Nack待ちとリトライ付き（停電モード移行時）
/*==========================================================================*/
static ER send_Pfail_mode_Wait_Ack_Retry( void )
{
	UINT Retry = 0;
	ER ercd;
	
	while ( Retry <= 3 ){
		ercd = send_Pfail_mode();			// モード切替通知の送信（停電モード移行時）
		if ( ercd == E_OK ){				// Ack応答あり
			MdCngSubMode( SUB_MD_CHG_PFAIL );	//　サブモード・ステータスを、”停電モードへ移行中”へ。
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
 *	モード切替通知の送信（停電モード移行時）
 */
/*==========================================================================*/
static ER send_Pfail_mode( void )
{
	ER ercd = E_TMOUT;
	int count;
	char com_data[ 15 ];
	
	/** コマンドデータの準備	**/
	com_data[ 0 ] = 0x23;			//　ヘッダ　1Byte　ASCII
	com_data[ 1 ] = 0;				//　データ長　２バイトBinary
	com_data[ 2 ] = 0x0d;
	com_data[ 3 ] = 0x31;			//　送信元種別　1Byte　ASCII
	com_data[ 4 ] = '4';			//　コマンド番号　３桁ASCII
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
//	シャットダウン要求の送信、Ack・Nack待ちとリトライ付き
/*==========================================================================*/
static ER send_shutdown_req_Wait_Ack_Retry( void )
{
	UINT Retry = 0;
	ER ercd;
	
	while ( Retry <= 3 ){
		ercd = send_shutdown_req();			// シャットダウン要求の送信
		if ( ercd == E_OK ){				// Ack応答あり
			MdCngSubMode( SUB_MD_REQ_SHUTDOWN );	//　サブモード・ステータスを、”シャットダウン要求中”へ。
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
 *	シャットダウン要求の送信
 */
/*==========================================================================*/
static ER send_shutdown_req( void )
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
	com_data[ 5 ] = '5';
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

	if ( GetSysSpec() == SYS_SPEC_OFFICE ){ //  １対１認証仕様の場合
		com_data[ 19 ] = ' ';				//　責任者／一般者の登録指情報（指登録番号０～１００）
		com_data[ 20 ] = ' ';
		com_data[ 21 ] = ' ';
		
	} else { 								//  マンション(占有部)仕様の場合
		com_data[ 19 ] = yb_touroku_data.yubi_seq_no[ 0 ];	//　責任者／一般者の登録指情報（指登録番号０～１００）
		com_data[ 20 ] = yb_touroku_data.yubi_seq_no[ 1 ];
		com_data[ 21 ] = yb_touroku_data.yubi_seq_no[ 2 ];		
	}

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
//	ID番号問合せの送信、Ack・Nack待ちとリトライ付き
/*==========================================================================*/
static ER send_ID_No_check_req_Wait_Ack_Retry( void )
{
	UINT Retry = 0;
	ER ercd;
	
	while ( Retry <= 3 ){
		ercd = send_ID_No_check_req();			// ID番号問合せの送信
		if ( ercd == E_OK ){				// Ack応答あり
			nop();
			MdCngSubMode( SUB_MD_ID_NO_CHECK_REQ );	//　サブモード・ステータスを、”ID番号問合せ処理シーケンス中”へ。
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
 *	ID番号問合せの送信（通常モード時）
 */
/*==========================================================================*/
static ER send_ID_No_check_req( void )
{
	ER ercd = E_TMOUT;
	int count;
	char com_data[ 24 ];
	
	/** コマンドデータの準備	**/
	com_data[ 0 ] = 0x23;			//　ヘッダ　1Byte　ASCII
	com_data[ 1 ] = 0;				//　データ長　２バイトBinary
	com_data[ 2 ] = 0x13;
	com_data[ 3 ] = 0x31;			//　送信元種別　1Byte　ASCII
	com_data[ 4 ] = '2';			//　コマンド番号　３桁ASCII
	com_data[ 5 ] = '1';
	com_data[ 6 ] = '4';
	com_data[ 7 ] = '0';			//　ブロック番号 ４桁ASCII
	com_data[ 8 ] = '0';
	com_data[ 9 ] = '0';
	com_data[ 10 ] = '0';
	
	//　伝送データ　
	com_data[ 11 ] = yb_touroku_data.tou_no[ 0 ];	//　部署番号
	com_data[ 12 ] = yb_touroku_data.tou_no[ 1 ];
	com_data[ 13 ] = ',';

	com_data[ 14 ] = yb_touroku_data.user_id[ 0 ];	//　ユーザーID
	com_data[ 15 ] = yb_touroku_data.user_id[ 1 ];
	com_data[ 16 ] = yb_touroku_data.user_id[ 2 ];
	com_data[ 17 ] = yb_touroku_data.user_id[ 3 ];

	com_data[ 18 ] = CODE_CR;		//　終端コード
	com_data[ 19 ] = CODE_LF;
	
	/** コマンド送信	**/
	SendBinaryData( &com_data, ( 18 + 2 ) );
	
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
//	ID権限問合せの送信、Ack・Nack待ちとリトライ付き
/*==========================================================================*/
static ER send_ID_Authority_check_req_Wait_Ack_Retry( void )
{
	UINT Retry = 0;
	ER ercd;
	
	while ( Retry <= 3 ){
		ercd = send_ID_Authority_check_req();			// ID番号問合せの送信
		if ( ercd == E_OK ){				// Ack応答あり
			nop();
			MdCngSubMode( SUB_MD_ID_AUTHORITY_CHECK_REQ );	//　サブモード・ステータスを、”ID権限問合せ処理シーケンス中”へ。
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
 *	ID権限問合せの送信（通常モード時）
 */
/*==========================================================================*/
static ER send_ID_Authority_check_req( void )
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
	com_data[ 5 ] = '1';
	com_data[ 6 ] = '5';
	com_data[ 7 ] = '0';			//　ブロック番号 ４桁ASCII
	com_data[ 8 ] = '0';
	com_data[ 9 ] = '0';
	com_data[ 10 ] = '0';
	
	//　伝送データ　
	com_data[ 11 ] = yb_touroku_data.tou_no[ 0 ];	//　部署番号
	com_data[ 12 ] = yb_touroku_data.tou_no[ 1 ];
	com_data[ 13 ] = ',';

	com_data[ 14 ] = yb_touroku_data.user_id[ 0 ];	//　ユーザーID
	com_data[ 15 ] = yb_touroku_data.user_id[ 1 ];
	com_data[ 16 ] = yb_touroku_data.user_id[ 2 ];
	com_data[ 17 ] = yb_touroku_data.user_id[ 3 ];
	com_data[ 18 ] = ',';
	
	if ( GetScreenNo() == LCD_SCREEN522 ){
		com_data[ 19 ] = ' ';		// 管理者区分<--　不定。	
	} else if ( GetScreenNo() == LCD_SCREEN543 ){
		com_data[ 19 ] = ' ';		// 管理者区分<--　不定。
	} else if ( GetScreenNo() == LCD_SCREEN547 ){
		com_data[ 19 ] = s_ID_Authority_Level;  		// 管理者区分<--　コマンド216　で得た管理者区分をSet。
	} else {						// ここに来る時は、プログラムミス。
		com_data[ 19 ] = yb_touroku_data.kubun[ 0 ];	//　管理者区分		
	}

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
									//  仕様種別を設定
	if ( GetSysSpec() == SYS_SPEC_MANTION ){				// マンション・占有部仕様
		if ( sys_demo_flg != SYS_SPEC_DEMO ){
			com_data[ 12 ] = '0';
		}	else {
			com_data[ 12 ] = '3';	//	デモ仕様の場合
		}
	} else if ( GetSysSpec() == SYS_SPEC_OFFICE ){			// １対１仕様（オフィス仕様）
		if ( sys_demo_flg != SYS_SPEC_DEMO ){
			com_data[ 12 ] = '1';
		}	else {
			com_data[ 12 ] = '4';	//	デモ仕様の場合
		}	
	} else if ( GetSysSpec() == SYS_SPEC_ENTRANCE ){		// マンション・共用部仕様
		if ( sys_demo_flg != SYS_SPEC_DEMO ){
			com_data[ 12 ] = '2';
		}	else {
			com_data[ 12 ] = '5';	//	デモ仕様の場合
		}	
	} else {
		com_data[ 12 ] = '0';		// 仕様設定が不定の場合は、マンション・占有部仕様
	}
	
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

#endif


/*==========================================================================*/
/**
 *	VA300　仕様切替えフラグの内容を、EEPROMから読み出す。
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
 *	VA300S　停電繰り返しカウンタの内容を、EEPROMから読み出す。
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
	
	Pfail_mode_count = 0;			// 停電モードの場合の起動回数(他モードで起動した場合は、resetされる。)
	Pfail_sense_flg = PFAIL_SENSE_NO;			// 停電検知フラグ　OFF

//	SUB_MD_SETTING		// 最新の装置サブモード

	g_CapTimes = 0;	//20131210Miya add
	
	WDT_counter = 0;				// WDTタイマー用サイクリック・カウンタ
	main_TSK_wdt = FLG_ON;			// mainタスク　WDTカウンタリセット・リクエスト・フラグ
	camera_TSK_wdt = FLG_ON;		// カメラタスク　WDTカウンタリセット・リクエスト・フラグ
	ts_TSK_wdt = FLG_ON;			// タッチセンサ・タスク　WDTカウンタリセット・リクエスト・フラグ
	lcd_TSK_wdt = FLG_ON;			// LCDタスク　WDTカウンタリセット・リクエスト・フラグ
	sio_rcv_TSK_wdt = FLG_ON;		// SIO受信タスク　WDTカウンタリセット・リクエスト・フラグ
	sio_snd_TSK_wdt = FLG_ON;		// SIO送信タスク　WDTカウンタリセット・リクエスト・フラグ
	
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
 *	マンション仕様/１対１仕様　状態を返す。　
 *	@return 　　0:マンション(占有部)仕様、1:1対１仕様。
 */
/*==========================================================================*/
static UB GetSysSpec( void )
{
	return sys_kindof_spec;
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

#if ( NEWCMR == 1 )		//20160610Miya
	return;
#endif


//20150930Miya
	CmrReloadFlg = 1;

	cmrGain = ini_cmrGain;						// カメラ・ゲイン値初期値　
	cmrFixShutter1 = ini_cmrFixShutter1;		// Fix Shutter Control値初期値(１回目)　
	cmrFixShutter2 = ini_cmrFixShutter2;		// Fix Shutter Control値初期値(２回目)			同上
	cmrFixShutter3 = ini_cmrFixShutter3;		// Fix Shutter Control値初期値(３回目）

	irDuty2 = ini_irDuty2;		// IR Duty値irDuty2　初期値;
	irDuty3 = ini_irDuty3;
	irDuty4 = ini_irDuty4;
	irDuty5 = ini_irDuty5;

	ercd = set_flg( ID_FLG_CAMERA, FPTN_REROAD_PARA );
	if ( ercd != E_OK ){
		ErrCodeSet( ercd );
	}
	
	return;

//	ercd = set_flg( ID_FLG_CAMERA, FPTN_CMR_WKAEUP );	// カメラWeakUP //20140930Miya Bio FPGA	
//	if ( ercd != E_OK ){
//		ErrCodeSet( ercd );
//	}
	
	cmrGain = ini_cmrGain;						// カメラ・ゲイン値初期値　
	cmrFixShutter1 = ini_cmrFixShutter1;		// Fix Shutter Control値初期値(１回目)　
	cmrFixShutter2 = ini_cmrFixShutter2;		// Fix Shutter Control値初期値(２回目)			同上
	cmrFixShutter3 = ini_cmrFixShutter3;		// Fix Shutter Control値初期値(３回目）

	//dly_tsk( 500/MSEC );
	ercd = set_flg( ID_FLG_CAMERA, FPTN_SETREQ_GAIN );	// カメラタスクに、直接のゲイン設定値を設定を依頼（FPGAを経由しない）	
	if ( ercd != E_OK ){
		ErrCodeSet( ercd );
	}
		
	irDuty2 = ini_irDuty2;		// IR Duty値irDuty2　初期値;
	irDuty3 = ini_irDuty3;
	irDuty4 = ini_irDuty4;
	irDuty5 = ini_irDuty5;

	dly_tsk( 500/MSEC );
	ercd = set_flg( ID_FLG_CAMERA, FPTN_SETREQ_SHUT1 );	// カメラタスクに、直接の露出３設定値を設定を依頼（FPGAを経由しない）	
	if ( ercd != E_OK ){
		ErrCodeSet( ercd );
	}

	dly_tsk( 500/MSEC );
	ercd = CmrCmdSleep();	//20150930Miya	
	
//	dly_tsk( 500/MSEC );
//	ercd = set_flg( ID_FLG_CAMERA, FPTN_CMR_SLEEP );	// カメラSleep //20140930Miya Bio FPGA	
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
		ethernet_addr[0] = ini_mac_addr[0];	/* 暫定的に即値!! */
		ethernet_addr[1] = ini_mac_addr[1];
		ethernet_addr[2] = ini_mac_addr[2];
		ethernet_addr[3] = ini_mac_addr[3];
		ethernet_addr[4] = ini_mac_addr[4];
		ethernet_addr[5] = ini_mac_addr[5];
	}
	return TRUE;
}


/*********************************
 * Initialize peripherals on evaluation board（★ Please customize !!）
 *
 ********************************/
void init_peripheral(void)
{
	
}

/*********************************
 * Initialize port（★ Please customize !!）
 *
 ********************************/

void ini_pio(void)
{
	sfr_outw(BSC_GPIOIC,0x0000);
	sfr_outw(BSC_PDTRB, 0x0000);		// 
	sfr_outl(BSC_PCTRB, 0x00000014);	// PORT17,18(TEST Pin1,2)出力
	
	TP_CLR(1);							// テストピン1初期化
	TP_CLR(2);							// テストピン2初期化
	
}

/*********************************
 * Initialize interrupt（★ Please customize !!）
 *
 ********************************/

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


/*********************************
 * IP Address の入力
 *
 * @retval TRUE 読出し成功
 * @retval FALSE 読出し失敗
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


/*******************************************************************
* WDT(ウォッチドッグ・タイマー)の初期設定
*
********************************************************************/
static void init_wdt( void )
{
	int tmp;
	
	// ウォッチドッグタイマカウンタ・レジスタ　初期化
	reset_wdtc();
	
	// WDTコントロール・ステータス・レジスタ　初期化
	tmp = 0xa5cf;					// 0xa5 : 書き込みコマンド・データ
									// bit7 : TME =1、カウントアップ開始
									// bit6 : WT/IT =1、WDTモード
									// bit5 : RSTS = 0、パワーオン・リセット指定 
									// bit4 : WOVF = 1、WDTモード時オーバーフロー
									// bit3 : IOVF = 1、インターバルタイマーモード時オーバーフロー
									// bit2-0 : クロックセレクト　111の時　分周比1/4096、オーバーフロー周期5.25ms						 
	sfr_outw( CPG_WTCSR, tmp );
 
}

/*******************************************************************
* WDT(ウォッチドッグ・タイマー)の無効化設定
*
********************************************************************/
static void stop_wdt( void )
{
	int tmp;
	
	// ウォッチドッグタイマカウンタ・レジスタ　初期化
	reset_wdtc();
	
	// WDTコントロール・ステータス・レジスタ　初期化
	tmp = 0xa54f;					// 0xa5 : 書き込みコマンド・データ
									// bit7 : TME =0、カウントアップ停止
									// bit6 : WT/IT =1、WDTモード
									// bit5 : RSTS = 0、パワーオン・リセット指定 
									// bit4 : WOVF = 1、WDTモード時オーバーフロー
									// bit3 : IOVF = 1、インターバルタイマーモード時オーバーフロー
									// bit2-0 : クロックセレクト　111の時　分周比1/4096、オーバーフロー周期5.25ms						 
	sfr_outw( CPG_WTCSR, tmp );
 
}

/*******************************************************************
* WDT(ウォッチドッグ・タイマー)カウンタのクリア
*
********************************************************************/
static void reset_wdtc( void )
{
	int tmp;
	
	// ウォッチドッグタイマカウンタ・レジスタ　クリア
	tmp = 0x5a00;
	sfr_outw( CPG_WTCNT, tmp );
	
}

/*******************************************************************
* WDTカウンタ用メモリのリセット処理
*	各タスクからのWDTリセット・リクエストが全てANDで成立した場合に、WDTカウンタをクリアする。
********************************************************************/
static void reset_wdtmem( void )
{
	if ( ( main_TSK_wdt == FLG_ON ) &&			// mainタスク　WDTリセット・リクエスト・フラグ
		 ( camera_TSK_wdt == FLG_ON ) &&		// カメラタスク　WDTリセット・リクエスト・フラグ
		 ( ts_TSK_wdt == FLG_ON ) &&			// タッチセンサ・タスク　WDTリセット・リクエスト・フラグ
		 ( lcd_TSK_wdt == FLG_ON ) &&			// LCDタスク　WDTリセット・リクエスト・フラグ
		 ( sio_rcv_TSK_wdt == FLG_ON ) &&		// SIO受信タスク　WDTリセット・リクエスト・フラグ
		 ( sio_snd_TSK_wdt == FLG_ON ) ){		// SIO送信タスク　WDTリセット・リクエスト・フラグ
		 
	     WDT_counter = 0;						// WDTカウンタ用メモリ(20秒でWDT起動)をクリア。
		 
#if( FREEZTEST )
		 main_TSK_wdt = FLG_ON;				// 各タスクのWDTリセット・リクエスト・フラグをOFF。
		 camera_TSK_wdt = FLG_ON;
		 ts_TSK_wdt = FLG_ON;
		 lcd_TSK_wdt = FLG_ON;
		 sio_rcv_TSK_wdt = FLG_ON;
		 sio_snd_TSK_wdt = FLG_ON;
#else
		 main_TSK_wdt = FLG_OFF;				// 各タスクのWDTリセット・リクエスト・フラグをOFF。
		 camera_TSK_wdt = FLG_OFF;
		 ts_TSK_wdt = FLG_OFF;
		 lcd_TSK_wdt = FLG_OFF;
		 sio_rcv_TSK_wdt = FLG_ON;
		 sio_snd_TSK_wdt = FLG_ON;
#endif
	}
}

/*******************************************************************
* WDTカウンタ用メモリのダイレクト・リセット処理(フラッシュ・メモリ・ドライバ処理専用)
*	フラッシュ・メモリのRead/Writeドライバ関数内で、WDTカウンタをクリアする。
*	各TaskのWDTクリア・フラグを無視して、カウンタをクリアするので、
*	ドライバ内での使用に限る。（乱用すると、タスク単位でのWDT機能の意味がなくなる。）
********************************************************************/
static void reset_wdt_cnt( void )
{
	WDT_counter = 0;						// WDTカウンタ用メモリ(20秒でWDT起動)をクリア。
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
//	if ( Test_1sec_timer > 0 ) Test_1sec_timer--;		// 未使用
	if ( Pfail_start_timer > 0 ) Pfail_start_timer--;	// 未使用
	if ( Ninshou_wait_timer > 0 ) Ninshou_wait_timer--;	// 指認証失敗時の画面待ち時間（30秒）
	
	// 時分秒の計測。　初期時間は、端末LCDの画面246設定から受け取る。
	timer_10ms++;					// 10msec 刻み
	if ( timer_10ms >= 100 ){	// 1秒の計測
		timer_10ms = 0;
		count_1sec++;

		if ( count_1sec >= 60 ){			// 1分の計測
			count_1sec = 0;
			count_1min++;

			if ( count_1min >= 60 ){		// 1時間の計測
				count_1min = 0;
				count_1hour++;
				
				if ( count_1hour >= 24 ){	// 24時間の計測
					count_1hour = 0;
				}
				g_MainteLog.now_min = count_1hour;	// "時"をLCD画面表示用メモリに反映
			}
			g_MainteLog.now_hour = count_1min;		// "分"を　　↓
		}
		g_MainteLog.now_sec = count_1sec;			// "秒"を　　↓		
	}	
	
	if ( WDT_counter++ > 3000 ){			// WDTカウンタ用メモリ(30秒でWDT起動)
		init_wdt();							// WDTタイマー開始（約5.25ms後にパワーONリセットが掛かる） 	
	}
	reset_wdtmem();							// WDTカウンタ用メモリのリセット処理
}


/*********************************
 * main
 *
 ********************************/

int main(void)
{
 	int _mbf_size;

   /* Initialize processor（★ Please customize !!）*/

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
	cre_tsk(TSK_COMMUNICATE,  &ctsk_urcv);	//電文処理
	cre_tsk(TSK_UDP_SEND,    &ctsk_usnd);	//UDP送信
#endif
#if ( VA300S == 1 )
//	cre_tsk(TSK_NINSHOU,  &ctsk_ninshou);	//認証処理
	cre_tsk(TSK_LOG,    &ctsk_log);			//Logging処理
#endif
#if ( VA300S == 2 )
	cre_tsk(TSK_CMD_LAN,   &ctsk_lancmd);
	cre_tsk(TSK_COMMUNICATE,  &ctsk_urcv);	//電文処理
	cre_tsk(TSK_UDP_SEND,    &ctsk_usnd);	//UDP送信

	cre_tsk(TSK_NINSHOU,  &ctsk_ninshou);	//認証処理
	cre_tsk(TSK_LOG,    &ctsk_log);			//Logging処理
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
	//sta_tsk(TSK_MAIN, 0);
#if TSK_LEARN_DATA_TEST_ENABLE
    TaskLearnData();
#endif

	/* Start multitask system */
	intsta();                   /* Start interval timer interrupt */
	syssta();                   /* enter into multi task */
}

/* end */
