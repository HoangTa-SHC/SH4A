//=============================================================================
/**
*	VA-300プログラム
*
*	@file va300.h
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2011/11/04
*	@brief  共通定義情報(他案件から流用)
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

//=== 定義
enum COM_CH {								///< 通信チャンネル定義
	COM_CH_MON,								///< モニタチャンネル
	COM_CH_LAN,								///< LANチャンネル
};

enum CAP_STATUS {							///< 指認証結果
	CAP_JUDGE_IDLE,
	CAP_JUDGE_OK,							///< 指認証OK
	CAP_JUDGE_NG,							///< 指認証NG
	CAP_JUDGE_RT,							///< 指認証リトライ要求
	CAP_JUDGE_E1,							///< 指認証画像異常（ヒストグラム異常）（撮影画像が２分割された異常画像）
	CAP_JUDGE_E2							///< 指認証画像異常（スリット異常）（指が歪んで挿入された場合）
};

enum DONBURU_STATUS {						///< ドングルの有無確認の結果
	DONGURU_JUDGE_IDLE = 0,
	DONGURU_JUDGE_OK,						///< OK
	DONGURU_JUDGE_NG						///< NG
};

enum PASSWORD_STATUS {						///< パスワード確認の結果
	PASSWORD_JUDGE_IDLE = 0,
	PASSWORD_JUDGE_OK,						///< OK
	PASSWORD_JUDGE_NG						///< NG
};

enum KINKYU_TOUROKU_STATUS {				///< パスワード確認の結果
	KINKYU_TOUROKU_JUDGE_IDLE = 0,
	KINKYU_TOUROKU_JUDGE_OK,				///< OK
	KINKYU_TOUROKU_JUDGE_NG					///< NG
};

enum MODE_STATUS {							///<=== 状態定義 ===
	MD_POWER_ON = 0,						///< 電源ON（初期値)
	MD_POWER_OFF,							///< 電源OFF
	MD_INITIAL,								///< 初期登録モード
	MD_MAINTE,								///< メンテナンスモード
	MD_NORMAL,								///< 通常モード
	MD_CAP,									///< 認証処理中
	MD_POWER_FAIL,							///< 停電時動作モード
	MD_PANIC,								///< 非常時開錠モード
	MD_SELFTEST,							///< セルフテスト状態
	MD_ERROR,								///< エラー発生状態
	MD_MAX
};

enum SUBMODE_STATUS {						///<=== サブ状態定義 ===
	SUB_MD_IDLE = 0,						///< アイドル状態（以下のどれにも当て嵌まらない時））
	SUB_MD_WAKEUP,							///< WakeUp確認中
	SUB_MD_CAMERA_PARAM_REQ,				///< カメラパラメータの初期値要求中
	SUB_MD_CAP_PARAM_REQ,					///< 画像処理の初期値要求中
	SUB_MD_LED_PARAM_REQ,					///< LED光量数値の初期値要求中
	SUB_MD_TOUROKU_PARAM_REQ,				///< 登録データの初期値要求中
	SUB_MD_DELETE,							///< 削除モード
	SUB_MD_DONGURU_CHK,						///< PCへドングルの有無確認の確認通信中
	SUB_MD_PASSWORD_CHK,					///< PCへパスワードの確認通信中
	SUB_MD_KINKYU_TOUROKU,					///< 緊急番号登録処理シーケンス中
	SUB_MD_KINKYU_8KETA_REQ,				///< 緊急８桁番号要求通信中
	SUB_MD_KINKYU_KAIJYO_SEND,				///< 緊急開錠番号通知通信中
	SUB_MD_KINKYU_NO_CHK_REQ,				///< 緊急番号妥当性確認要求通信中
	SUB_MD_CHG_MAINTE,						///< メンテナンスモードへ移行中
	SUB_MD_CHG_NORMAL,						///< 通常モードへ移行中
	
	SUB_MD_MAX
};

enum SCREEN_NO {
	LCD_INIT,			/// LCD初期画面
	LCD_SCREEN1,		// 画面１表示中		初期登録画面
	LCD_SCREEN2,		// 画面２表示中
	LCD_SCREEN3,		// 画面３表示中
	LCD_SCREEN4,		// 画面４表示中
	LCD_SCREEN5,		// 画面５表示中
	LCD_SCREEN6,		// 画面６表示中
	LCD_SCREEN7,		// 画面７表示中
	LCD_SCREEN8,		// 画面８表示中
	LCD_SCREEN9,		// 画面９表示中
	LCD_SCREEN10,		// 画面１０表示中
	LCD_SCREEN11,		// 画面１１表示中
	LCD_SCREEN12, 		// 画面１２表示中

	LCD_SCREEN100,		// 画面１００表示中	 通常モード（認証）
	LCD_SCREEN101,		// 画面１０１表示中
	LCD_SCREEN102,		// 画面１０２表示中
	LCD_SCREEN103,		// 画面１０３表示中
	LCD_SCREEN104,		// 画面１０４表示中

	LCD_SCREEN120,		// 画面１２０表示中　通常モード（登録）
	LCD_SCREEN121,		// 画面１２１表示中
	LCD_SCREEN122,		// 画面１２２表示中
	LCD_SCREEN123,		// 画面１２３表示中
	LCD_SCREEN124,		// 画面１２４表示中
	LCD_SCREEN125,		// 画面１２５表示中
	LCD_SCREEN126,		// 画面１２６表示中
	LCD_SCREEN127,		// 画面１２７表示中
	LCD_SCREEN128,		// 画面１２８表示中
	LCD_SCREEN129,		// 画面１２９表示中
	LCD_SCREEN130,		// 画面１３０表示中
	LCD_SCREEN131,		// 画面１３１表示中
	LCD_SCREEN132,		// 画面１３２表示中

	LCD_SCREEN140,		// 画面１４０表示中　通常モード（削除）
	LCD_SCREEN141,		// 画面１４１表示中
	LCD_SCREEN142,		// 画面１４２表示中
	LCD_SCREEN143,		// 画面１４３表示中
	LCD_SCREEN144,		// 画面１４４表示中
	LCD_SCREEN145,		// 画面１４５表示中
	LCD_SCREEN146,		// 画面１４６表示中

	LCD_SCREEN160,		// 画面１６０表示中　通常モード（緊急開錠番号設定）
	LCD_SCREEN161,		// 画面１６１表示中
	LCD_SCREEN162,		// 画面１６２表示中
	LCD_SCREEN163,		// 画面１６３表示中
	LCD_SCREEN164,		// 画面１６４表示中
	LCD_SCREEN165,		// 画面１６５表示中

	LCD_SCREEN180,		// 画面１８０表示中　通常モード（緊急開錠）
	LCD_SCREEN181,		// 画面１８１表示中
	LCD_SCREEN182,		// 画面１８２表示中
	LCD_SCREEN183,		// 画面１８３表示中
	LCD_SCREEN184,		// 画面１８４表示中
	LCD_SCREEN185,		// 画面１８５表示中
	LCD_SCREEN186,		// 画面１８６表示中
	LCD_SCREEN187,		// 画面１８７表示中
	LCD_SCREEN188,		// 画面１８８表示中

	LCD_SCREEN200,		// 画面２００表示中　メンテナンス・モード
	LCD_SCREEN201,		// 画面２０１表示中
	LCD_SCREEN202,		// 画面２０２表示中
	LCD_SCREEN203,		// 画面２０３表示中
	LCD_SCREEN204,		// 画面２０４表示中
	LCD_SCREEN205,		// 画面２０５表示中
	LCD_SCREEN206,		// 画面２０６表示中
		
	LCD_SCREEN_MAX
};

enum LCD_SCREEN_REQ {		// メッセージ･バッファ"MBF_LCD_DATA" の先頭バイトで使用する。
	LCD_NEXT,					// 次の通常画面への遷移を要求
	LCD_YES,					// "はい"が押された
	LCD_NO,						// "いいえ"が押された
	LCD_BACK,					// "戻る"が押された
	LCD_CANCEL,					// "中止"が押された
	LCD_INIT_INPUT,				// "初期登録"ボタンが押された
	LCD_TOUROKU,				// "登録"ボタンが押された
	LCD_SAKUJYO,				// "削除"ボタンが押された
	LCD_MAINTE,					// "メンテナンス"ボタンが押された
	LCD_KINKYUU_SETTEI,			// "緊急番号設定"ボタンが押された
	LCD_KINKYUU_KAIJYOU,		// "緊急解錠"ボタンが押された"
	
	LCD_USER_ID,				// "ユーザーID"が入力された
	LCD_YUBI_ID,				// "責任者/一般者"の指IDが選択された
	LCD_NAME,					// "氏名"が入力された
	LCD_YUBI_SHUBETU,			// "指種別"が選択された
	LCD_FULL_PIC_SEND_REQ,		// "フル画像送信"ボタンが押された
	LCD_MAINTE_SHOKIKA_REQ,		// "初期化”ボタンが押された
	LCD_MAINTE_END,				// "メンテナンスモード終了"ボタンが押された
	LCD_KINKYUU_KAIJYOU_BANGOU,	// "緊急開錠番号　８桁"が入力された
	LCD_KINKYUU_BANGOU,			// "ユーザー入力の緊急番号　４桁が入力された			
	
	LCD_REQ_MAX	
};

//通信の状態
enum EmfStatus {
	ST_COM_STANDBY = 0,		// 待機(0)
	ST_COM_RECEIVE,		// 受信中
		//以下は受信中の中の分類
	ST_COM_WAIT_LEN,	// データ長待ち
	ST_COM_WAIT_DATA,	// データ待ち
	ST_COM_WAIT_CR,		// CR待ち
	ST_COM_WAIT_LF,		// LF待ち
	ST_COM_WAIT_EOT,	// EOT待ち
	ST_COM_WAIT_ACK_NAK,	// ACK/NAK待ち
	
	ST_COM_SLEEP,		// 受信停止
	
	ST_COM_MAX			// 状態数
};

//
enum LBUS_FMT_CHK {							///< コマンドのフォーマット確認
	FMT_PKT_ERR = -3,						///< パケットエラー
	FMT_SUM_ERR = -2,						///< チェックサムエラー
	FMT_CMD_ERR = -1,						///< コマンドエラー
	FMT_CMD_NONE = 0,						///< コマンドなし
	FMT_CMD_P2P,							///< 自アドレス宛パケット
	FMT_CMD_BC,								///< ブロードキャストパケット
};

enum RCV_ERR_CODE {							///< 受信エラーコード
	RCV_ERR_NONE = 0,						///< 受信エラーなし
	RCV_ERR_STR_LIM,						///< 受信文字数上限オーバー
	RCV_ERR_PRTY,							///< パリティチェックエラー
	RCV_ERR_CHKSUM,							///< チェックサムエラー
	RCV_ERR_OP,								///< オペランドエラー
	RCV_ERR_ACK,							///< コマンドAck受信エラー
	RCV_ERR_RETRY,							///< 再送信回数オーバー
	RCV_ERR_NAK,							///< コマンドNak受信エラー
	RCV_ERR_EXE,							///< コマンドExe受信エラー
	RCV_ERR_COMM							///< コマンド受信エラー
};

// 手順
enum PR_TYPE {
	PR_TYPE_A,								///< 手順A
	PR_TYPE_B,								///< 手順B
	PR_TYPE_C,								///< 手順C
	PR_TYPE_D,								///< 手順D
	PR_TYPE_E,								///< 手順E
};

enum TERM_NUMBER {							///< 端末番号の定義
	TERM_NO_1 = 1,							///< 端末1
	TERM_NO_2,								///< 端末2
};

enum SOUND_CTRL {							///< サウンド制御
	SOUND_CTRL_EMG_ON,						///< 警報ON
	SOUND_CTRL_EMG_OFF,						///< 警報OFF
	SOUND_CTRL_TPL_OK,						///< タッチパネル用ブザーOK音
	SOUND_CTRL_TPL_NG,						///< タッチパネル用ブザーNG音
};

// カメラ撮影関連
enum CAP_COMMAND {						// キャプチャー実行命令コマンドの種別
	COMMAND_NONE,
	COMMAND204,
	COMMAND210,
	COMMAND141
};	

#define TRIM_START_X	200				// トリミングの開始X座標（左上を原点として）
#define TRIM_START_Y	180				// トリミングの開始Y座標
#define TRIM_SIZE_X		640				// トリミングのXサイズ（元サイズは、1280×640）
#define TRIM_SIZE_Y		320				// トリミングのYサイズ
#define RE_SIZE_X		640				// 縮小前のXサイズ
#define RE_SIZE_Y		320				// 縮小前のYサイズ

/**
 *	モードタスクへのコマンド
 *	 000～ 999:通信インターフェースのコマンド
 *	1000～    :モードタスクへのコマンド
 */
enum MD_CMD {								///< モードタスクへのコマンド)
	MD_CMD_ACK_RCV  = 1000,					///< ACK受信
	MD_CMD_EOT_RCV,							///< EOT受信
	MD_CMD_SND_FAIL,						///< 受信失敗通知
	
	MD_CMD_FIN_SET,							///< 指挿入通知
	MD_CMD_KEY_ON,							///< タッチパネル、外部キー入力
	
	MD_CMD_POL_REQ = 2000,					///< ポーリング要求
	MD_CMD_RCV,								///< コマンド受信通知
};

/// @name マクロ定義
//@{
//=== 制御フラグ ===
#define	FLAG_VERIFY		0x01

#ifndef	STX
#define	STX				0x02				///< 通信STX定義
#endif
#ifndef	ETX
#define	ETX				0x03				///< 通信ETX定義
#endif
#define	CRLF			"\x0D\x0A"			///< 通信終端定義

#ifndef	TRUE
	#define	TRUE			1				///< TRUE定義
#endif
#ifndef	FALSE
	#define	FALSE			0				///< FALSE定義
#endif

/// リセット
#define	reset()				(*(void (*)(void))0xA0000000)()

/// 
#define	LAN_BUF_SIZE		128				///< LANの送受信バッファサイズ
#define	LBUS_BUF_SIZE		256				///< ローカルバスの送受信バッファサイズ
//#define	MAX_SND_LAN			1450			///< 最大LAN送信サイズ(UDPは最大1472バイト)
//#define	MAX_SND_LAN			65535			///< 最大LAN送信サイズ(MispoのUDP転送最大サイズ)
#define	MAX_SND_LAN			8192			///< 最大LAN送信サイズ(Winsock最大受信サイズ)
#define	MSG_SIZE	1024+1	///< タスク間メッセージバッファサイズ

#define	LBUS_RTRY_MAX		3				///< ローカルバスの再送最大数

// バックアップ
#define	HEAD_MARK			0x4F796F00		///< 有効コード

// テストピン定義
#define	TP_REV(n)			sfr_outw(BSC_PDTRB, (sfr_inw(BSC_PDTRB) ^ (n << 1)))
#define	TP_CLR(n)			sfr_outw(BSC_PDTRB, (sfr_inw(BSC_PDTRB) & ~(n << 1)))

//=== デバッグ用 ===
#if defined( _DEBUG_)
	#define DPRINT	DebugOut
#else
	#define DPRINT
#endif
//@}

/// @name 型定義
//@{
typedef struct st_mcn_status {			///< 装置ステータス 
	ER	erLan;							///< LAN状態ステータス
	
} ST_MCN_STS;

typedef struct st_param{				///< パラメータ情報構造体
	UW   uwMark;						///< 固定値(値有効チェック)
	
	
	UH   uhChkSum;						///< チェックサム
} ST_PARAM;

typedef struct st_mcn_param{			///< 装置パラメータ情報構造体
	UW	uwMark;							///< 固定値(値有効チェック)
	TMO	tmRcvTmout;						///< 受信タイムアウト(ms)
	int	iSendRetry;						///< 送信リトライ回数
	TMO	tmPoling;						///< ポーリング間隔(ms)
	TMO	tmFirstTmout;					///< 初期登録がないときのタイムアウト時間(ms)
	TMO	tmLCDdelay;						///< 自動画面切替時間(ms)
	int	iStartX;						///< 切り出し開始X座標
	int	iStartY;						///< 切り出し開始Y座標
	TMO	tmPowOff;						///< 停電時電源OFF時間(ms)
	enum HOST_TYPE	eTermNo;			///< 端末番号
	
	UH	uhChkSum;						///< チェックサム
} ST_MCN_PARAM;

typedef struct st_ctrl {				///< 制御構造体
	UW   uwMark;						///< 固定値(値有効チェック)
	
//	ST_RTC stTime;						///< 日時
	UH   uhFlag;						///< 測定制御フラグ
	UH   uhChkSum;						///< チェックサム
} ST_CTRL;

typedef struct t_commsg{				//=== タスク間メッセージの定義 ===
	struct t_commsg *next;				///< 先頭4ﾊﾞｲﾄは OS で使用(4)
	INT	msgpri;							// メッセージ優先度
	UH	cnt;							///< データ数
	UB	buf[ MSG_SIZE ];				///< データ
}T_COMMSG;

typedef struct t_lancmdmsg{
	struct t_lancmdmsg *next;			///< 先頭4ﾊﾞｲﾄは OS で使用(4)
	INT		ch;							///< チャンネル番号
	BOOL	rflg;						///< レスポンス要求
	UH		cnt;						///< データ数
	B		buf[ LAN_BUF_SIZE ];
} T_LANCMDMSG;

typedef struct t_lanresmsg{
	struct t_lanresmsg *next;			///< 先頭4ﾊﾞｲﾄは OS で使用(4)
	INT		msgpri;						///< メッセージ優先度
	UH		cnt;						///< データ数
	B		buf[ MAX_SND_LAN ];
} T_LANRESMSG;

typedef struct t_mdcmdmsg{
	struct t_mdcmdmsg *next;			///< 先頭4ﾊﾞｲﾄは OS で使用(4)
	INT		msgpri;						///< メッセージ優先度
	UH		uhCmd;						///< コマンド
	UH		uhBlkNo;					///< ブロック番号
	ID		idOrg;						///< ID
	UH		cnt;						///< データ数
	B		cData[ MAX_SND_LAN ];
} T_MDCMDMSG;


typedef struct t_ybdata{		//=== 登録情報の定義 ===
	UB	tou_no[ 3 ];			// 棟番号２桁＋","
	UB	user_id[ 5 ];			// ユーザーID４桁＋","
	UB	yubi_seq_no[ 4 ];		// 登録指番号（責任者＋一般者）３桁＋","
	UB	kubun[ 2 ];				// 責任者/一般者の区分１桁＋","
	UB	yubi_no[ 3 ];			// 指の識別番号（右、左、どの指）２桁＋","
	UB	name[ 25 ];				// 登録者指名２４桁＋","
	UB	ybCapBuf[ ( ( RE_SIZE_X * RE_SIZE_Y ) * 3 ) ];	// 指画像データ（縮小画像３枚分）
} T_YBDATA;


// 追加　20130510 Miya
typedef struct t_ybdata20{		//=== 登録情報の定義 ===
	UB	yubi_seq_no[ 4 ];		// 登録指番号（責任者＋一般者）３桁＋","
	UB	kubun[ 2 ];				// 責任者/一般者の区分１桁＋","
	UB	yubi_no[ 3 ];			// 指の識別番号（右、左、どの指）２桁＋","
	UB	name[ 25 ];				// 登録者指名２４桁＋","
	UB	ybCapBuf[ ( ( RE_SIZE_X * RE_SIZE_Y ) * 3 ) ];	// 指画像データ（縮小画像３枚分）
} T_YBDATA20;




//@}

/// @name 広域変数宣言
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
//	保存しないデータ
//-----------------------------------------------------------------------------
EXTERN ST_MCN_STS stMcnSts;
// network
EXTERN UB ethernet_addr[6];
EXTERN UB default_ipaddr[4];
EXTERN UB default_gateway[4];
EXTERN UB subnet_mask[4];
EXTERN UH udp_portno;
EXTERN UH telnet_portno;						///< TELNETのポート番号
//-----------------------------------------------------------------------------
//	保存するデータ
//-----------------------------------------------------------------------------
#pragma section PARAM
EXTERN ST_PARAM stParam;
EXTERN ST_MCN_PARAM stMcnParam;
EXTERN ST_CTRL stCtrl;
#pragma section

extern UH cmrGain;			// カメラ・ゲイン値　PCからUDPコマンド経由で与えられた値を、通信の指示に従ってその都度カメラに設定する。
extern UH cmrFixShutter1;	// Fix Shutter Control値(１回目)　実際にFPGA経由でカメラコントロールの為に参照する値
extern UH cmrFixShutter2;	// Fix Shutter Control値(２回目)			同上
extern UH cmrFixShutter3;	// Fix Shutter Control値(３回目＝シャッタースピード、初期値を受けたタイミングで、カメラにじかに設定する)　同上

extern UH ini_cmrGain;					// カメラ・ゲイン値初期値　
extern UH ini_cmrFixShutter1;			// Fix Shutter Control値初期値(１回目)　
extern UH ini_cmrFixShutter2;			// Fix Shutter Control値初期値(２回目)			同上
extern UH ini_cmrFixShutter3;			// Fix Shutter Control値初期値(３回目）

extern int iStartX, iStartY, iSizeX, iSizeY;// キャプチャ画像のトリミング開始座標とサイズ
extern int iReSizeX, iReSizeY;				// 縮小画像のXサイズ、Yサイズ
extern int iReSizeMode;						// 縮小画像の縮小率　0:辺1/2、1:辺1/4、2:辺1/8

extern UB irDuty2;			// IR LED Duty値irDuty2;
extern UB irDuty3;
extern UB irDuty4;
extern UB irDuty5;

extern UB ini_irDuty2;		// IR Duty値irDuty2　初期値;
extern UB ini_irDuty3;
extern UB ini_irDuty4;
extern UB ini_irDuty5;

extern ID	rxtid_org;		// GET_UDPの為の元々のタスクID
extern ID	rxtid;			// GET_UDPの為のタスクID

//@}

/// @name プロトタイプ宣言
//@{

// va300.c
EXTERN void telnetd_send(B *s);					///< TELNET送信
EXTERN void telnetd_send_bin(B *s, INT len);	///< TELNETバイナリ送信
EXTERN UB GetScreenNo(void);					// 現在表示スクリーン番号の取得
EXTERN void ChgScreenNo( UB NewScreenNo );		// 画面遷移状態(番号)を更新する
EXTERN void reload_CAP_Param( void );			// Gain. FixShutter,IR_LEDのパラメータを、初期要求値に戻す。（CAPのRetry時に変わった可能性がある為。）
EXTERN UB sys_ScreenNo;
EXTERN UB s_CapResult;		// 指認証の結果
EXTERN UB s_DongleResult;	// ドングルの有無確認の結果
EXTERN UB s_PasswordResult;	// パスワード確認の結果
EXTERN UB s_KinkyuuTourokuResult;	// 緊急登録通知の結果
EXTERN UINT rcv_ack_nak;	// ack受信フラグ　=0：未受信、=1:ACK受信、=-1：nak受信
EXTERN ER	SndCmdCngMode( UINT stat );		// PCへモード切替え通知を送信

// tsk_mode.c
EXTERN TASK ModeTask();							///< モードタスク
EXTERN UB MdGetMode(void);						///< モード参照
EXTERN void MdCngMode( UINT eNextMode );		///< モード変更
EXTERN UB MdGetSubMode(void);					///< サブモード参照
EXTERN void MdCngSubMode( UINT eNextMode );		///< サブモード変更
EXTERN ER MdSendMsg(ID idOrg, UH uhCmd, UH uhBlkno, B *p, int iCount);	///< モードタスクへのコマンドメッセージ送信

// tsk_disp.c
EXTERN void DispLed(int, int);					///< LED表示
//EXTERN void Disp7Seg(enum LED_7SEG_SEL eSelect7Seg, int, enum CTRL7SEG);	///< ７SEG表示
EXTERN TASK DispTask(void);						///< 表示タスク

// tsk_lcmd.c
EXTERN TASK LanCmdTask(INT ch);					///< LANコマンド処理タスク
EXTERN void LanCmdSend(enum PR_TYPE eProt, const UH uhCmd, UH uhBlkno, B *pData);	///< コマンド送信
EXTERN void LanReqSend(void);							///< 問い合わせコマンド送信
EXTERN void LanResSend(enum PR_TYPE eProt, B *pData);	///< 応答コマンドの送信
EXTERN void LanFingerDataSend(B *pId);					///< 指データの送信
EXTERN void LanParmErrLog(char* pCmd, char* pComment, long lData);	///< パラメータエラーをログに残す
EXTERN UB CAPGetResult( void );					///< 最新の指認証結果を返す（指認証結果参照）
EXTERN UINT sys_Mode;				// 状態変数
EXTERN UINT sys_SubMode;			// 状態変数(サブ）


// tsk_lrcv.c
EXTERN TASK UdpRcvTask(INT ch);					///< UDP受信タスク
EXTERN void CmmSendAck(void);					///< Ack送信
EXTERN void CmmSendEot(void);					///< EOT送信
EXTERN void CmmSendNak(void);					///< NAK送信
EXTERN void CmmChangeStatus( int newStatus );	///< UDP受信状態変更
EXTERN BOOL CmmSendData(enum PR_TYPE eProt, const UH uhCmd, UH uhBlkno, char *cData, int iCnt);	///< コマンド送信
EXTERN enum HEADER_CODE HeaderCodeGet(enum PR_TYPE eProt);	///< ヘッダコードに変換


// tsk_io.c
EXTERN TASK IoTask(void);						///< I/O検知タスク
EXTERN INTHDR PcsStartInt(void);				///< PCS測定開始割り込み
EXTERN INTHDR PcsStopInt(void);					///< PCS測定終了割込み
EXTERN INTHDR IrStartInt(void);					///< IR測定開始割り込み
EXTERN INTHDR IrStopInt(void);					///< IR測定終了割込み
EXTERN INTHDR IrTrendInt(void);					///< 測定データ転送完了割込み
EXTERN INTHDR FpgaErrInt(void);					///< FPGAエラー割込み
EXTERN INTHDR DmaStopInt(void);					///< DMA転送完了割込み

// tsk_exkey.c
EXTERN ER ExKeyInit(ID tskid, ID flgid);		///< 外部キータスク初期化
EXTERN TASK ExKeyTask(void);					///< 外部キー制御タスク

// tsk_lcd.c
EXTERN TASK LcdTask(void);						///< LCD表示タスク
EXTERN ER LcdTaskInit(ID idTsk);				///< LCD表示タスクの初期化

// tsk_buz.c
EXTERN ER SoundInit(ID tskid, ID flgid);		///< サウンドタスク初期化
EXTERN TASK SoundTask(void);					///< サウンドタスク
EXTERN void SoundCtrl(enum SOUND_CTRL eCtrl);	///< サウンド制御

// tsk_ts.c
EXTERN ER TsTaskInit(ID tskid);					///< 生態検知タスク初期化
EXTERN TASK TsTask(void);						///< 生態検知タスク

// tsk_ctlmain.c
//EXTERN ER CtlMainTaskInit(ID idTsk);			///< 端末コントロール・メイン・タスク初期化
//EXTERN TASK CtlMain( void );					///< 端末コントロール・メイン・タスク

// tsk_camera.c
EXTERN ER CameraTaskInit(ID idTsk);				///< カメラ撮影コントロール・タスク初期化
EXTERN TASK CameraTask( void );					///< カメラ撮影コントロール・タスク
extern void yb_init_all( void );				// 指登録情報のデータ初期化(all)
extern T_YBDATA yb_touroku_data;				// 指登録情報（１指分）
extern T_YBDATA20 yb_touroku_data20[21];		// 指登録情報（20指分）//追加　20130510 Miya

extern UB kinkyuu_tel_no[17];					// 緊急開錠電話番号１６桁（配列最終番目は区切り記号”,”）
extern UB kinkyuu_touroku_no[5];				// 緊急開錠の緊急登録番号４桁（配列最終番目は区切り記号　NUL　）　
extern UB kinkyuu_hyouji_no[9];					// 緊急開錠の緊急番号８桁表示データ（配列最終番目は区切り記号　NUL　）　
extern UB kinkyuu_kaijyo_no[9];					// 緊急開錠の開錠番号８桁データ（配列最終番目は区切り記号　NUL　）
extern UB kinkyuu_input_no[5];					// 緊急開錠時に入力された番号４桁（配列最終番目は区切り記号　NUL　）

extern UB mainte_password[5];					// メンテナンス・モード移行時の確認用パスワード４桁。（配列最終番目は区切り記号　NUL　）　

extern ER CmrCmdFixShutterCtl( UB FSC_val );
extern ER CmrCmdManualGainCtl( UB gain_val );
extern ER Wait_Ack_forBlock( void );			// コマンドまたはブロックデータ送信後のAck応答待ち

// selftest.c
EXTERN BOOL SelfTest(void);						///< セルフテスト

// prm_func.c
EXTERN void McnParamInit(void);					///< 装置パラメータを初期化する
EXTERN TMO McnParamRcvTmoutGet(void);			///< 受信タイムアウト時間を取得する
EXTERN int McnParamSendRetryGet(void);			///< 送信リトライ回数を取得する
EXTERN TMO McnParamPolingGet(void);				///< ポーリング時間を取得する
EXTERN TMO McnParamFirstTmoutGet(void);			///< 初期登録がないときのタイムアウト時間を取得する
EXTERN TMO McnParamLcdDelayGet(void);			///< 自動画面切替時間を取得する
EXTERN int McnParamStartXGet(void);				///< 切り出し開始X座標を取得する
EXTERN int McnParamStartYGet(void);				///< 切り出し開始Y座標を取得する
EXTERN TMO McnParamPowerOffGet(void);			///< 停電時電源OFF時間を取得する
EXTERN enum HOST_TYPE McnParamTermNumberGet(void);	///< 端末番号を取得する
EXTERN BOOL IsMcnParamEnable(void);				///< パラメータの有効性を確認する

// sub_func.c
EXTERN int _sprintf(char *s, const char *control, ...);	///< sprintfのセマフォ版
EXTERN BOOL MpfUseDisplayLine(int iLine, char *pBuf);	///< メモリプール使用量表示データ取得


//@}
#endif										// end of _VA300_H_
