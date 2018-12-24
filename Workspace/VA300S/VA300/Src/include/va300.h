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

#define NEWCMR 1
#define AUTHTEST 1			//0:OFF 1:登録あり 2:登録なし //20160613Miya 認証テスト(Flashに画像を保存する)
#define KOUJYOUCHK	0		//20160606Miya 工場チェック用
#define	FORDEMO	0			//20160711Miya デモ機

//=== 定義　（本定義は、ファイル"va300.h"、"id.h"の２か所で同じ定義を行うこと。）
#define VA300S	1		// =0 : VA300,   =1 : VA300S,  =2 : VA300Sの場合でデバッグ用に画像データをLANでPCへ送る。
#define VA300ST 1		// =0 : VA300,VA300S   =1 : VA300ST(ユタカ仕様)

#define NEWALGO		1	//NewAlgo 
						//2015.05.31 1:位置による減点の緩和
						//             追加減点見直し

#define FPGA_HI		1	//20160902Miya FPGA高速化 0:従来速度 1:高速
#define BPWRCTRL	1	//20160905Miya B-PWR制御

#define PCCTRL		0	//20160930Miya PCからVA300Sを制御する
#define	CMRSENS		1	//20170424Miya 	カメラ感度対応(FAR/FRR対応)

#define FREEZTEST	0	//20170706Miya

enum COM_CH {								///< 通信チャンネル定義
	COM_CH_MON,								///< モニタチャンネル
	COM_CH_LAN,								///< LANチャンネル
};

enum SYS_SPEC {								///< システム設定状態問い合わせ
	SYS_SPEC_MANTION = 0,					// マンション・占有部仕様
	SYS_SPEC_OFFICE,						// １対１仕様（オフィス仕様）
	SYS_SPEC_ENTRANCE,						// マンション・共用部仕様
	SYS_SPEC_OFFICE_NO_ID,					// １対多仕様（オフィス・ID番号無し仕様）
	SYS_SPEC_KOUJIM,						//20160112Miya FinKeyS //工事モード(マンション・占有部)
	SYS_SPEC_KOUJIO,						//20160112Miya FinKeyS //工事モード(オフィス)
	SYS_SPEC_KOUJIS,						//20160112Miya FinKeyS //工事モード(マート制御盤)
	SYS_SPEC_SMT,							//20160112Miya FinKeyS //スマート制御盤対応
};

enum YUTAKA_NYUTAI_KUBUN {					///< ユタカ仕様　入退室区分
	NYUTAI_NON = 0,							//  未入退
	NYUSHITU,								//  入室
	TAISHITU								//　退室
};

enum SYS_SPEC_DEMO {						///< システム設定状態問い合わせ(DEMO仕様か否か)
	SYS_SPEC_NOMAL = 0,						// DEMOでない場合
	SYS_SPEC_DEMO							// DEMO仕様
};

enum PFAIL_SENSE_STATUS {					///< 停電検知受信の有無
	PFAIL_SENSE_NO = 0,
	PFAIL_SENSE_ON							///< ON
};

enum SHUTDOWN_OK_STATUS {					///< シャットダウンOK受信の有無
	SHUTDOWN_NO = 0,						///< NO
	SHUTDOWN_OK								///< OK
};

enum CAP_STATUS {							///< 指認証結果
	CAP_JUDGE_IDLE,
	CAP_JUDGE_OK,							///< 指認証OK
	CAP_JUDGE_NG,							///< 指認証NG
	CAP_JUDGE_RT,							///< 指認証リトライ要求
	CAP_JUDGE_E1,							///< 指認証画像異常（ヒストグラム異常）（撮影画像が２分割された異常画像）
	CAP_JUDGE_E2,							///< 指認証画像異常（スリット異常）（指が歪んで挿入された場合）
	CAP_JUDGE_RT2							///< 指認証リトライ要求
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

enum ID_NO_CHECK_STATUS {					///< ID番号確認の結果（１対１仕様）
	ID_NO_JUDGE_IDLE = 0,
	ID_NO_JUDGE_OK,							///< OK
	ID_NO_JUDGE_NG							///< NG
};

enum ID_AUTHORITY_CHECK_STATUS {			///< ID権限確認の結果（１対１仕様）
	ID_AUTHORITY_JUDGE_IDLE = 0,
	ID_AUTHORITY_JUDGE_OK,					///< OK
	ID_AUTHORITY_JUDGE_NG,					///< NG
	ID_AUTHORITY_JUDGE_CN					///< CANSEL
};

enum KINKYU_TOUROKU_STATUS {				///< 緊急番号登録確認の結果
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
	MD_PFAIL,								///< 停電時動作モード
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
	SUB_MD_ID_NO_CHECK_REQ,					///< ID番号問合せの送信中
	SUB_MD_ID_AUTHORITY_CHECK_REQ,			///< ID権限問合せの送信中
	SUB_MD_DONGURU_CHK,						///< PCへドングルの有無確認の確認通信中
	SUB_MD_PASSWORD_CHK,					///< PCへパスワードの確認通信中
	SUB_MD_KINKYU_TOUROKU,					///< 緊急番号登録処理シーケンス中
	SUB_MD_KINKYU_8KETA_REQ,				///< 緊急８桁番号要求通信中
	SUB_MD_KINKYU_KAIJYO_SEND,				///< 緊急開錠番号通知通信中
	SUB_MD_KINKYU_NO_CHK_REQ,				///< 緊急番号妥当性確認要求通信中
	SUB_MD_CHG_MAINTE,						///< メンテナンスモードへ移行中
	SUB_MD_CHG_NORMAL,						///< 通常モードへ移行中
	SUB_MD_CHG_PFAIL,						///< 停電モードへ移行中
	SUB_MD_REQ_SHUTDOWN,					///< シャットダウン要求中
	SUB_MD_UNLOCK,							///< 解錠要求中
	SUB_MD_NINSHOU,							///< 認証完了送信中（VA300ｓ）
	SUB_MD_TOUROKU,							///< 登録完了送信中（VA300ｓ）
	SUB_MD_KAKAIHOU_TIME,					///< 過開放時間設定コマンド送信中（VA300ｓ）
	SUB_MD_INIT_TIME,						///< 時刻の初期設定コマンド送信中（VA300ｓ）
	SUB_MD_MEMCHK,							///< メモリチェック中
	SUB_MD_ALLDEL,							///< 一括削除送信中（VA300ｓ）
		
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
	LCD_SCREEN105,		// 画面１０５表示中 //20140423Miya 認証リトライ追加
	LCD_SCREEN106,		// 画面１０６表示中 //20140423Miya 認証リトライ追加
	LCD_SCREEN107,		// 画面１０７表示中 //20140925Miya password_open	//画面101と併用につき未使用
	LCD_SCREEN108,		// 画面１０８表示中 //20140925Miya password_open	//画面108内で2面切替あり
	LCD_SCREEN109,		// 画面１０９表示中 //20140925Miya password_open	//エラー表示
	LCD_SCREEN110,		// 画面１１０表示中 //20140925Miya password_open	//しばらくお待ち下さい

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
	LCD_SCREEN133,		// 画面133表示中

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

	LCD_SCREEN170,		// 画面１８０表示中　通常モード（緊急開錠）
	LCD_SCREEN171,		// 画面１８１表示中
	LCD_SCREEN172,		// 画面１８２表示中
	LCD_SCREEN173,		// 画面１８３表示中
	LCD_SCREEN174,		// 画面１８４表示中
	LCD_SCREEN175,		// 画面１８５表示中
	LCD_SCREEN176,		// 画面１８６表示中
	LCD_SCREEN177,		// 画面１８７表示中
	LCD_SCREEN178,		// 画面１８８表示中

	LCD_SCREEN180,		// 画面１９０表示中　通常モード（パスワード変更）
	LCD_SCREEN181,		// 画面１９１表示中
	LCD_SCREEN182,		// 画面１９２表示中
	LCD_SCREEN183,		// 画面１９３表示中
	LCD_SCREEN184,		// 画面１９４表示中
	LCD_SCREEN185,		// 画面１９５表示中
	LCD_SCREEN186,		// 画面１９６表示中
	LCD_SCREEN187,		// 画面１９７表示中
	LCD_SCREEN188,		// 画面198表示中	//20160108Miya FinKeyS

	LCD_SCREEN200,		// 画面２００表示中　メンテナンス・モード
	LCD_SCREEN201,		// 画面２０１表示中
	LCD_SCREEN202,		// 画面２０２表示中
	LCD_SCREEN203,		// 画面２０３表示中
	LCD_SCREEN204,		// 画面２０４表示中
	LCD_SCREEN205,		// 画面２０５表示中
	LCD_SCREEN206,		// 画面２０６表示中
	LCD_SCREEN207,		// 画面２０７表示中
	LCD_SCREEN208,		// 画面２０８表示中

	LCD_SCREEN220,		// 画面２２０表示中　メンテナンス・モード(情報)
	LCD_SCREEN221,		// 画面２２１表示中
	LCD_SCREEN222,		// 画面２２２表示中
	LCD_SCREEN223,		// 画面２２３表示中
	LCD_SCREEN224,		// 画面２２４表示中
	LCD_SCREEN225,		// 画面２２５表示中

	LCD_SCREEN240,		// 画面２４０表示中　メンテナンス・モード(設定変更)
	LCD_SCREEN241,		// 画面２４１表示中
	LCD_SCREEN242,		// 画面２４２表示中
	LCD_SCREEN243,		// 画面２４３表示中
	LCD_SCREEN244,		// 画面２４４表示中
	LCD_SCREEN245,		// 画面２４５表示中
	LCD_SCREEN246,		// 画面２４６表示中
	LCD_SCREEN247,		// 画面２４７表示中
	LCD_SCREEN248,		// 画面２４８表示中
	LCD_SCREEN249,		// 画面２４９表示中

	LCD_SCREEN260,		// 画面２６０表示中　メンテナンス・モード(技術)
	LCD_SCREEN261,		// 画面２６１表示中
	LCD_SCREEN262,		// 画面２６２表示中
	LCD_SCREEN263,		// 画面２６３表示中
	LCD_SCREEN264,		// 画面２６４表示中
	LCD_SCREEN265,		// 画面２６５表示中
	LCD_SCREEN266,		// 画面２６６表示中
	LCD_SCREEN267,		// 画面２６７表示中
	
	LCD_SCREEN400,		// 画面４００表示中　１対１仕様：初期登録モード
	LCD_SCREEN401,		// 画面４０１表示中
	LCD_SCREEN402,		// 画面４０２表示中
	LCD_SCREEN403,		// 画面４０３表示中
	LCD_SCREEN404,		// 画面４０４表示中
	LCD_SCREEN405,		// 画面４０５表示中
	LCD_SCREEN406,		// 画面４０６表示中
	LCD_SCREEN407,		// 画面４０７表示中
	LCD_SCREEN408,		// 画面４０８表示中
	LCD_SCREEN409,		// 画面４０９表示中
	LCD_SCREEN410,		// 画面４１０表示中
	LCD_SCREEN411,		// 画面４１１表示中
	
	LCD_SCREEN500,		// 画面５００表示中　１対１仕様：通常モード
	LCD_SCREEN501,		// 画面５０１表示中	
	LCD_SCREEN502,		// 画面５０２表示中	
	LCD_SCREEN503,		// 画面５０３表示中	
	LCD_SCREEN504,		// 画面５０４表示中	
	LCD_SCREEN505,		// 画面５０５表示中	
	LCD_SCREEN506,		// 画面５０６表示中	
	
	LCD_SCREEN520,		// 画面５２０表示中　１対１仕様：登録モード
	LCD_SCREEN521,		// 画面５２１表示中
	LCD_SCREEN522,		// 画面５２２表示中
	LCD_SCREEN523,		// 画面５２３表示中
	LCD_SCREEN524,		// 画面５２４表示中
	LCD_SCREEN525,		// 画面５２５表示中
	LCD_SCREEN526,		// 画面５２６表示中
	LCD_SCREEN527,		// 画面５２７表示中
	LCD_SCREEN528,		// 画面５２８表示中
	LCD_SCREEN529,		// 画面５２９表示中
	LCD_SCREEN530,		// 画面５３０表示中
	LCD_SCREEN531,		// 画面５３１表示中
	LCD_SCREEN532,		// 画面５３２表示中
	LCD_SCREEN533,		// 画面５３３表示中
	LCD_SCREEN534,		// 画面５３４表示中
	LCD_SCREEN535,		// 画面５３５表示中
	LCD_SCREEN536,		// 画面５３６表示中
	LCD_SCREEN537,		// 画面５３７表示中
	LCD_SCREEN538,		// 画面５３８表示中
	
	LCD_SCREEN542,		// 画面５４２表示中　１対１仕様：削除モード
	LCD_SCREEN543,		// 画面５４３表示中
	LCD_SCREEN544,		// 画面５４４表示中
	LCD_SCREEN545,		// 画面５４５表示中
	LCD_SCREEN546,		// 画面５４６表示中
	LCD_SCREEN547,		// 画面５４７表示中
	LCD_SCREEN548,		// 画面５４８表示中
	LCD_SCREEN549,		// 画面５４９表示中
	LCD_SCREEN550,		// 画面５５０表示中
	LCD_SCREEN551,		// 画面５５１表示中

//20160108Miya FinKeyS ->
	LCD_SCREEN600,		// 画面600表示中	 通常モード2（FinKeyS）
	LCD_SCREEN601,		// 画面601表示中
	LCD_SCREEN602,		// 画面602表示中
	LCD_SCREEN603,		// 画面603表示中
	LCD_SCREEN604,		// 画面604表示中
	LCD_SCREEN605,		// 画面605表示中
	LCD_SCREEN606,		// 画面606表示中
	LCD_SCREEN607,		// 画面607表示中
	LCD_SCREEN608,		// 画面608表示中
	LCD_SCREEN609,		// 画面609表示中
	LCD_SCREEN610,		// 画面610表示中
	LCD_SCREEN611,		// 画面611表示中
	LCD_SCREEN612,		// 画面612表示中
	LCD_SCREEN613,		// 画面613表示中

	LCD_SCREEN620,		// 画面620表示中	 工事モード
	LCD_SCREEN621,		// 画面621表示中
	LCD_SCREEN622,		// 画面622表示中
	LCD_SCREEN623,		// 画面623表示中
	LCD_SCREEN624,		// 画面624表示中
	LCD_SCREEN625,		// 画面625表示中
//20160108Miya FinKeyS <-
	
	LCD_SCREEN999,		// 画面９９９表示中　「シャットダウンします」
		
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
	LCD_PASSWORD_OPEN,			// パスワード開錠ボタンが押された	//20140925Miya password_open
	LCD_KEYIN_PASSWORD,			// 開錠パスワードが入力された		//20140925Miya password_open
	LCD_ODEKAKE,				// おでかけボタンが押された	//20160108Miya FinKeyS
	LCD_ORUSUBAN,				// お留守番ボタンが押された	//20160108Miya FinKeyS
	LCD_SETTEI,					// 設定ボタンが押された	//20160108Miya FinKeyS
	
	LCD_USER_ID,				// "ユーザーID"が入力された
	LCD_YUBI_ID,				// "責任者/一般者"の指IDが選択された
	LCD_NAME,					// "氏名"が入力された
	LCD_YUBI_SHUBETU,			// "指種別"が選択された
	LCD_MAINTE_END,				// "終了"ボタンが押された
	LCD_KINKYUU_KAIJYOU_BANGOU,	// "緊急開錠番号　８桁"が入力された
	LCD_KINKYUU_BANGOU,			// "ユーザー入力の緊急番号　４桁が入力された

	LCD_PASS_HENKOU_REQ,		// ”パスワード変更”ボタンが押された
	LCD_PASS_SETTEI_HENKOU_REQ,	// ”パスワード開錠設定変更”ボタンが押された

	LCD_JYOUHOU_REQ,			// ”情報”ボタンが押された
	LCD_SETTEI_HENKOU_REQ,		// ”設定変更”ボタンが押された
	LCD_SINDAN_REQ,				// ”診断”ボタンが押された
	LCD_SYOKI_SETTEI_REQ,		// ”初期設定”ボタンが押された
	LCD_VERSION_REQ,			// ”バージョン”ボタンが押された
	LCD_ERROR_REREKI_REQ,		// ”エラー履歴”ボタンが押された
	LCD_NINSYOU_JYOUKYOU_REQ,	// ”認証状況”ボタンが押された
	LCD_JIKOKU_HYOUJI_REQ,		// ”時刻確認”ボタンが押された
	LCD_PASS_KAIJYOU_REQ,		// ”パスワード開錠”ボタンが押された
	LCD_CALLCEN_TEL_REQ,		// ”コールセンターTEL”ボタンが押された
	LCD_JIKOKU_AWASE_REQ,		// ”時刻合わせ”ボタンが押された
	LCD_ITI_TYOUSEI_REQ,		// ”LCD位置調整”ボタンが押された
	LCD_MAINTE_SHOKIKA_REQ,		// "初期化”ボタンが押された
	LCD_SPEC_CHG_REQ,			// ”仕様切替”ボタンが押された
	LCD_IMAGE_KAKUNIN_REQ,		// ”画像確認”ボタンが押された
	LCD_FULL_PIC_SEND_REQ,		// "フル画像送信"ボタンが押された
	
	LCD_ENTER,					// "確定"ボタンが押された
	LCD_OK,						// "確認"ボタンが押された
	LCD_MENU,					// "MENU"ボタンが押された
	LCD_KANTOKU,				// "監督者"ボタンが押された
	LCD_KANRI,					// "管理者"ボタンが押された
	LCD_IPPAN,					// "一般者"ボタンが押された	

	LCD_NOUSE,					// "未使用"ボタンが押された	

	LCD_ERR_REQ,				// 通常モード待機中にエラーを検出した	//20140925Miya add err

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

enum SIO_MODE {
	SIO_RCV_IDLE_MODE,		// RS-232 コマンド受信/送信アイドル
	SIO_RCV_ENQ_MODE,		// RS-232 ENQ受信中状態
	SIO_RCV_WAIT_MODE,		// RS-232 制御Boxからのコマンド受信待ち。
	SIO_RCV_CMD_MODE,		// RS-232 制御Boxからのコマンド受信中状態。
	SIO_SEND_MODE,			// コマンド送信中。
	SIO_SEND_ACK_WAIT		// コマンド送信済みで、ACK/NAKの受信待ち、および受付中。
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

enum RCV_COM_STAT {
	ERR_FIRST_CODE = -6,	// "@"で無い時。
	ERR_CKSUM = -3,			// Ckeck Sumの不一致
	ERR_END_CODE = -2,		// 終端コードの不一致
	ERR_DATA_LENG = -1,		// データ長の不一致
	
	COMM_DATA_IDLE = 0,		// データ受信IDLE
	RCV_TEST_COM,			// テストコマンドの受信	
	RCV_NINSHOU_DATA,		// 認証データの受信
	RCV_TOUROKU_REQ,		// 制御Boxから登録要求あり
	RCV_NINSHOU_OK,			// 制御Boxから認証許可を受信（事前に、端末から認証確認コマンドを送信済みで、それに対する応答コマンド）
	RCV_TEIDEN_STAT,		// 制御Boxから停電状態の応答を受信（事前に、端末から停電状態要求コマンドを送信済みで、それに対する応答コマンド）
	RCV_SHUTDOWN_OK,		// 制御Boxからシャットダウン要求応答を受信（事前に、端末から　同要求コマンドを送信済みで、それに対する応答コマンド）
	RCV_TEIDEN_EVENT,		// 停電状態イベント報告の受信
	RCV_ALARM_STAT,			// 警報発生・消去イベント報告の受信
	RCV_ADJUST_TIME,		// 時刻合わせコマンドの受信

	//20160930Miya PCからVA300Sを制御する
	RCV_TANMATU_INFO,		// A0 端末情報取得
	RCV_REGDATA_DNLD_STA,	// A1 登録データダウンロード開始
	RCV_REGDATA_DNLD_GET,	// A2 登録データダウンロード中
	RCV_REGDATA_DNLD_END,	// A3 登録データダウンロード終了
	RCV_REGDATA_UPLD_STA,	// A4 登録データアップロード開始
	RCV_REGDATA_UPLD_GET,	// A5 登録データアップロード中
	RCV_REGDATA_UPLD_END,	// A6 登録データアップロード終了
	RCV_REGPROC,			// A7 登録操作
	RCV_AUTHPROC,			// A8 認証操作
	

	
	KEY_UNNOWN_REQ			// 不明な命令の受信
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
#define RE_SIZE_Y		560				//20131210Miya cng // 縮小前のYサイズ
//#define RE_SIZE_Y		320				// 縮小前のYサイズ


#if(AUTHTEST >= 1)	//20160613Miya
#define ADDR_OKIMG1			0x040000		//認証OK画像保存開始アドレス1(Flash)
#define ADDR_OKIMG2			0x060000		//認証OK画像保存開始アドレス2(Flash)
#define ADDR_OKIMG3			0x160000		//認証NG画像保存開始アドレス1(Flash)
#define ADDR_OKIMG4			0x180000		//認証NG画像保存開始アドレス2(Flash)
#endif

#define ADDR_REGAUTHDATA	0x1A0000		//認証パラメータ保存開始アドレス1(Flash)
#define ADDR_REGIMG1		0x120000		//登録画像保存開始アドレス1(Flash)
#define ADDR_REGIMG2		0x140000		//登録画像保存開始アドレス2(Flash)

#define ADDR_REGAUTHDATA2	0x1C0000		//認証パラメータ保存開始アドレス2(Flash) //20161031Miya Ver2204
#define ADDR_ADDLCDGAMEN	0x1E0000		//LCD追加画面(Flash) //20161031Miya Ver2204

//20140925Miya password_open
#define FLG_ON	1
#define FLG_OFF 0

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


#ifndef NO_DEBUG 
#define Pfail_start_time   2400				// 停電検知後の２分間監視タイマー値
#define EOT_WAIT_TIME		100				// EOT待ちタイムアウト時間（１秒）
#define ACK_WAIT_TIME		100				// Ack待ちタイムアウト時間（１秒）

#else
#define Pfail_start_time    1000				// 停電検知後の２分間監視タイマー値
#define EOT_WAIT_TIME		1000				// EOT待ちタイムアウト時間（１秒）
#define ACK_WAIT_TIME		1000				// Ack待ちタイムアウト時間（１秒）

#endif

#define NINSHOU_WAIT_TIME	1000 * 3		// 指認証失敗時の画面待ち時間（30秒）

#define ENQ_SENDING_MAX 9999

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
	//UB	ybCapBuf[ ( ( RE_SIZE_X * RE_SIZE_Y ) * 3 ) ];	// 指画像データ（縮小画像３枚分）
	UB	ybCapBuf[ 640 * 480 ];	// 指画像データ（縮小画像３枚分）//20160601Miya forDebug
} T_YBDATA;


// 追加　20130510 Miya
typedef struct t_ybdata20{		//=== 登録情報の定義 ===
	UB	yubi_seq_no[ 4 ];		// 登録指番号（責任者＋一般者）３桁＋","
	UB	kubun[ 2 ];				// 責任者/一般者の区分１桁＋","
	UB	yubi_no[ 3 ];			// 指の識別番号（右、左、どの指）２桁＋","
	UB	name[ 25 ];				// 登録者指名２４桁＋","
	//UB	ybCapBuf[ ( ( RE_SIZE_X * RE_SIZE_Y ) * 3 ) ];	// 指画像データ（縮小画像３枚分）
} T_YBDATA20;


/*ユーザ情報*/
typedef	struct STRUCT_REGUSERINFO{
	unsigned short	RegSts;				//登録状況確認　0：未登録　1：登録済み
	unsigned short	BlockNum;			//棟番号　"00"?"99"
	unsigned short	UserId;				//ユーザID（部屋番号）　"0000"?"9999"
	unsigned short	RegNum;				//血流登録番号　"001"?"100"
	unsigned short	TotalNum;			//ユーザ指登録本数
	unsigned char	MainteTelNum[16];	//緊急番号(4ケタ)
	unsigned char	KinkyuNum[4];		//緊急番号(4ケタ)
	unsigned short	RegImageSens;		//登録時の撮影感度 0:感度up(ｽｰﾊﾟｰﾍﾋﾞｰ) 1:感度up(ﾍﾋﾞｰ) 2:標準 3:感度down(女性・子供)
	//unsigned short	RegDensityMax[4];	//登録時4分割の濃度値MAX
	//unsigned short	RegDensityMin[4];	//登録時4分割の濃度値MIN
	//unsigned short	RegSlitInfo;		//登録時スリット情報 0:スリット無し 1:上スリット 2:下スリット 3:両スリット
	unsigned short	RegHiDensityDev[4];	//登録時4分割の濃度値AVE
	unsigned short	RegDensityAve[3];	//H/M/Lの濃度値　0:H 1:M 2:L
	unsigned short	RegFingerSize;		//登録時指太さ(未対応) 0:ｽｰﾊﾟｰﾍﾋﾞｰ 1:ﾍﾋﾞｰ 2:標準 3:細い 4:極細
	unsigned short	CapNum;				//1回目2回目区別
	unsigned short	kinkyu_times;
	unsigned short	r1;					//認証スコアー(R1)	//20140925Miya add log
	unsigned short	r2;					//認証スコアー(R1)	//20140925Miya add log
	unsigned short	lbp_lvl;			//20140905Miya lbp追加　認証時のLBPレベル
	unsigned short	lbp_pls;			//20140905Miya lbp追加　認証時のLBPによる減点緩和実施したかどうか
	unsigned short	xsft;				//20140910Miya XSFT
	unsigned short	trim_sty;				//FLASHは4byteアライメントにつきdummy追加 //20140925 add log
}RegUserInfoData;

/*登録血流画像タグ情報*/
typedef	struct STRUCT_REGBVATAG{
	unsigned short	CapNum;				//1回目2回目区別
	unsigned short	RegInfoFlg;			//登録状況確認　0：未登録　1：登録済み　2?：学習画像あり　0xFF：削除
	unsigned short	BlockNum;			//棟番号　"00"?"99"
	unsigned short	UserId;				//ユーザID（部屋番号）　ASCII4文字　"0000"?"9999"
	unsigned short	RegNum;				//血流登録番号　"001"?"100"
	unsigned short	Level;				//登録者レベル　"0"：監督者　"1"：管理者　"2"：一般者
	unsigned short	RegFinger;			//登録指　"01"：右親指　"02"：右人差指　"03"：右中指　"04"：右薬指　"05"：右小指
										//        "11"：左親指　"12"：左人差指　"13"：左中指　"14"：左薬指　"15"：左小指
	unsigned char	Name[24];			//登録者名前
	unsigned short	RegImageSens;		//登録時の撮影感度 0:感度up(ｽｰﾊﾟｰﾍﾋﾞｰ) 1:感度up(ﾍﾋﾞｰ) 2:標準 3:感度down(女性・子供)
	//unsigned short	RegDensityMax[4];	//登録時4分割の濃度値MAX
	//unsigned short	RegDensityMin[4];	//登録時4分割の濃度値MIN
	//unsigned short	RegSlitInfo;		//登録時スリット情報 0:スリット無し 1:上スリット 2:下スリット 3:両スリット
	unsigned short	RegHiDensityDev[4];	//登録時4分割の濃度値AVE
	unsigned short	RegDensityAve[3];	//H/M/Lの濃度値　0:H 1:M 2:L
	unsigned short	RegFingerSize;		//登録時指太さ(未対応) 0:ｽｰﾊﾟｰﾍﾋﾞｰ 1:ﾍﾋﾞｰ 2:標準 3:細い 4:極細
	unsigned char	MinAuthImg[2][200];	//極小画像
	unsigned short	end_code;
}RegBloodVesselTagData;

/*画像キャプチャー条件パラメータ*/
typedef	struct STRUCT_CAPIMGPARA{
	short		PictureSizeX;		//カメラ撮影サイズ（X)
	short		PictureSizeY;		//カメラ撮影サイズ（Y)
	short		TrimStPosX;			//トリミング開始座標（X)
	short		TrimStPosY;			//トリミング開始座標（Y)
	short		TrimSizeX;			//トリミング画像サイズ（X)
	short		TrimSizeY;			//トリミング画像サイズ（Y)
	short		ResizeMode;			//画像圧縮率　0：辺1/2　1：辺1/4　2：辺1/8　3：辺1/1（等倍）
	short		CapSizeX;			//キャプチャーサイズ（X)
	short		CapSizeY;			//キャプチャーサイズ（Y)
	unsigned char	DataLoadFlg;	//パラメータのロード先 0：未調整　1：調整済み　2：INI強制
}CapImgData;

/*画像＆認証処理パラメータ*/
typedef	struct STRUCT_IMGPROCAUTHPROCPARA{
	unsigned char	Proc;				//登録状況確認用フラグ　0：未登録　1：登録済み　2?：学習画像あり　0xFF：削除
	short		InpSizeX;				//入力サイズ（X)
	short		InpSizeY;				//入力サイズ（Y)
	short		IrLedPos[8];			//画像分割領域を設定 入力サイズ（X)基準
										//[0]：LED1-LED2間　[1]：LED2-LED3間　[2]：LED3-LED4間
	unsigned short AuthLvlPara;			//認証アルゴの閾値
	short	LearningPara;				//学習機能　０：OFF　１：簡易学習　２：通常学習
	int		LvlCameraErrHi;				//ヒストグラム輝度平均値の判定(カメラエラー上限)
	int		LvlCameraErrLo;				//ヒストグラム輝度平均値の判定(カメラエラー下限)
	int		LvlHistAveHi;				//ヒストグラム輝度平均値の判定(高感度画像しきい値)
	int		LvlHistAveLo;				//ヒストグラム輝度平均値の判定(低感度画像しきい値)
	int		LvlMeiryouLoAll;			//細線化明瞭度の下限（全体）
	int		LvlMeiryouLoPart;			//細線化明瞭度の下限（部分）
	int		LvlMeiryouHiAll;			//細線化明瞭度の上限（全体）
	int		LvlMeiryouHiPart;			//細線化明瞭度の上限（部分）
	int		ThLowPart1;					//認証スコアー減点閾値1(250点以下)
	int		ThLowPart2;					//認証スコアー減点閾値2(200点以下)
	int		WeightLowPart1;				//認証スコアー減点係数1(1/100値を使用する)
	int		WeightLowPart2;				//認証スコアー減点係数1(1/100値を使用する)
	unsigned short LvlBrightLo;			//明るさ判定（暗レベル）閾値
	unsigned short LvlBrightLo2;		//明るさ判定（暗レベル）画素数
	unsigned short LvlBrightHi;			//明るさ判定（明レベル）閾値
	unsigned short LvlBrightHiNum;		//明るさ判定（明レベル）画素数
	unsigned short LvlFingerTop;		//指先コントラスト判定閾値(20)
	unsigned short LvlSlitCheck;		//スリット判定端部レベル(20)
	unsigned short NumSlitCheck;		//スリット判定端部画素数(20)
	unsigned short LvlSlitSensCheck;	//スリット判定生体センサー検知レベル(60)
	unsigned short StSlitSensCheck;		//スリット判定生体センサー検知開始画素(30)
	unsigned short EdSlitSensCheck;		//スリット判定生体センサー検知終了画素(60)
	unsigned short NgImageChkFlg;		//画像異常判定(0:なし 1:あり 2:メモリ異常のみ 3:指抜きのみ)
	unsigned short WeightXzure;			//X方向ずれ減点重み係数(20->0.2)
	unsigned short WeightYzure;			//Y方向ずれ減点重み係数(30->0.3)
	unsigned short WeightYmuki;			//Y方向ずれ減点向き係数(20->0.2)
	unsigned short WeightYhei;			//Y方向ずれ平行ずれ係数(20->0.2)
	unsigned short WeightScore400;		//認証スコアー400以下減点重み係数(90->0.9)
	unsigned short WeightScore300;		//認証スコアー300以下減点重み係数(90->0.9)
	unsigned short WeightScore200;		//認証スコアー200以下減点重み係数(75->0.75)
	unsigned short WeightScore200a;		//認証スコアー200以下減点係数(25->0.0025)
	unsigned short WeightScore200b;		//認証スコアー200以下減点切片(25->0.25)
	unsigned short WeightXzureSHvy;		//スーパーヘビー級X方向ずれ減点重み係数(8->0.08)
	unsigned short WeightYzureSHvy;		//スーパーヘビー級Y方向ずれ減点重み係数(8->0.08)
	unsigned short WeightYmukiSHvy;		//スーパーヘビー級Y方向ずれ減点向き係数(100->1)
	unsigned short WeightYheiSHvy;		//スーパーヘビー級Y方向ずれ平行ずれ係数(20->0.2)
	unsigned short WeightScore400SHvy;	//スーパーヘビー級認証スコアー400以下減点重み係数(90->0.9)
	unsigned short WeightScore300SHvy;	//スーパーヘビー級認証スコアー300以下減点重み係数(90->0.9)
	unsigned short WeightScore200SHvy;	//スーパーヘビー級認証スコアー200以下減点重み係数(75->0.75)
	unsigned short WeightScore200aSHvy;	//スーパーヘビー級認証スコアー200以下減点係数(25->0.0025)
	unsigned short WeightScore200bSHvy;	//スーパーヘビー級認証スコアー200以下減点切片(25->0.25)
}ImgAndAuthProcSetData;

//20160312Miya 極小精度UP
typedef	struct STRUCT_REGIMGDATAADD{
	unsigned short	RegImageSens;			//登録時の撮影感度 0:感度up(ｽｰﾊﾟｰﾍﾋﾞｰ) 1:感度up(ﾍﾋﾞｰ) 2:標準 3:感度down(女性・子供)
	unsigned short	RegFingerSize;			//登録時指太さ(未対応) 0:ｽｰﾊﾟｰﾍﾋﾞｰ 1:ﾍﾋﾞｰ 2:標準 3:細い 4:極細
	unsigned short	Sel1stRegNum;			//1位に選ばれたの登録番号
	unsigned short	RegContrast[2][2][16];		//16分割コントラスト比 [登録/学習][R1/R2][データ]
	UB				RegLbpImg[2][80*40];	//[登録/学習][画素数]
	UB				RegR1Img[2][80*40];		//[登録/学習][画素数]
	UB				RegMiniR1Img[2][20*10];	//[登録/学習][画素数]
}RegImgDataAdd;

/*画像＆認証処理ステータス情報*/
typedef	struct STRUCT_STSIMGPROCAUTHPROC{
	unsigned char	AuthResult;		//画像処理＆認証処理の結果 0：正常　1：Retry　2?：異常
	short		RetruCnt;			//画像処理＆認証処理のリトライ回数
	short		CmrGain;			//カメラゲインの設定 0：なし　1：+補正　-1：-補正
	short		L_ImgSSvalue;		//シャッター速度の設定 0：なし　1：+補正　-1：-補正
	short		N_ImgSSvalue;		//シャッター速度の設定 0：なし　1：+補正　-1：-補正
	short		H_ImgSSvalue;		//シャッター速度の設定 0：なし　1：+補正　-1：-補正
	short		IrLedSw[8];			//LED点灯設定 0：変更無し　1：点灯　-1：消灯
									//[0]：LED1　[1]：LED2　[2]：LED3　[3]：LED4"
	unsigned short	StsErr;			//エラー状況把握 0：正常 1?：異常
}StsImgAndAuthProc;

/*ユーザー使用番号*/
typedef struct STRUCT_USEPROCNUM{
	int		MainteUsb;				//メンテUSB有無チェック　0:なし 1:あり
	char	SvMaintePassWord[4];	//メンテナンスパスワード(登録番号)
	char	InMaintePassWord[4];	//メンテナンスパスワード(入力番号)
	char	OpenKeyNum[8];			//緊急解錠キー8ケタ番号
	char	CalOpenCode[8];			//緊急解錠コード
	char	InOpenCode[8];			//緊急解錠コード
	char	InKinkyuNum[4];			//緊急番号(入力番号)
	char	InTouNum[2];			//棟番号(部署番号)
	char	InUserIdNum[4];			//ユーザID
	char	InChkRegLvl[1];			//責任者区分 0:監督者 1:管理者 2:一般者
}UseProcNum;

//パスワード開錠	//20140925Miya password_open
typedef struct STRUCT_PASSWORDOPEN{
	char	sw;				//パスワード開錠SW 0:OFF 1:ON
	char	hide_num;		//非表示SW 0:OFF 1:ON(非表示実施、｢*｣表示)
	char	kigou_inp;		//記号入力SW 0:OFF 1:ON
	char	random_key;		//キーボードランダム表示SW 0:OFF 1:ON
	short	keta;			//入力ケタ数(4～8)
	char	password[10];	//パスワード
}PasswordOpen;

//パスワード開錠2	//20160108Miya FinKeyS
typedef struct STRUCT_PASSWORDOPEN2{
	short	family_sw;			//家族分用意 0:しない 1:する
	short	num;
	short	keta[20];			//入力ケタ数(4～8)
	char	password[20][10];	//パスワード
	char	dummy[4];
}PasswordOpen2;

//技術メンテ　//20140925Miya add mainte
typedef struct STRUCT_TECHMENUDATA{
	short	SysSpec;		//仕様 0:専有 1:共用 2:法人
	short	DemoSw;			//デモモード 0:OFF 1:ON
	//short	YslitSw;			//デモモード 0:OFF 1:ON
	short	HijyouRemocon;	//非常リモコン使用 0:OFF 1:ON
	short	DebugHyouji;	//デバッグ用表示 0:OFF 1:ON
	short	CamKando;		//カメラ感度 0:弱 1:中 2:強
	short	CmrCenter;		//20160610Miya カメラセンター(NEWCMR) //LED明るさ 0:OFF 1:中 2:強
}TechMenuData;

//認証ログ	//20140925Miya add log
typedef struct STRUCT_AUTHLOG{
	unsigned long	ok_cnt;
	unsigned long	ng_cnt;
	unsigned long	ok_cnt1st;
	unsigned long	ok_cnt2nd;
	unsigned long	ok_cnt3rd;
	unsigned long	ok_finger_cnt[20];
	short			wcnt;
	short			now_result[8][4];	//[回数][内容]　内容 0:登録番号 1:R1 2:R2 3:LBP
	unsigned short	now_seq_num;
}AuthLog;

//メンテログ //20140925Miya add log
typedef struct STRUCT_MAINTELOG{
	unsigned short	err_wcnt;
	unsigned short	err_rcnt;
	unsigned short	err_buff[128][5];	//[回数][内容] 内容 0:エラー番号 1:時 2:分 3:秒 4:状態(0:解除1:発生中)
	unsigned short	now_hour;
	unsigned short	now_min;
	unsigned short	now_sec;
	unsigned short	st_hour;
	unsigned short	st_min;
	unsigned short	st_sec;
	unsigned short	diag_cnt1;
	unsigned short	diag_cnt2;	//ライフカウンター
	unsigned short	chk_num;	//異物チェックレベル
	unsigned short	chk_ave;	//異物チェックレベル
	unsigned char	diag_buff[8][16];	//[回数][診断内容]  0xff:NG 1:OK
	//unsigned short	dmy;				//FLASHは4byteアライメントにつきdummy追加 //20140925 add log
	unsigned short	cmr_seq_err_f;		//カメラシーケンスエラー発生フラグ(エラー番号39)
	unsigned short	end_code;
}MainteLog;


//20150531Miya
typedef struct POSGENTEN{
	int 	zure_sum_x;		//ずれ量SUM値
	double	ttl_dat_x;		//累計SUM値
	int 	zure_sum_y;		//ずれ量SUM値
	double 	ttl_dat_y;		//累計SUM値
	int		pm_zure_y;		//ずれが+方向、-方向にばらついているか 0:ばらつきなし 1:ばらつきあり
	int		parallel_y;		//平行ずれがMAXかどうか 0:MAXでない 1:MAX
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
	int		LedPosiSetFlg;		//LED位置設定実施フラグ　0x1234:実施
	int		LedPosi;			//0:通常 1:5mm下げ
	int		LcdAdjFlg;			//LCD調整実施フラグ　0x1234:実施
	int		LcdAdjBaseX[3];		//目標座標X
	int		LcdAdjBaseY[3];		//目標座標Y
	int		LcdAdjInputX[3];	//入力座標X
	int		LcdAdjInputY[3];	//入力座標Y
	int		LiveCnt;			//起動カウント
	int		InitRetryCnt;		//初期起動リトライカウント
}BkDataNoClear;

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

extern int g_CmrParaSet;				//カメラパラメータ設定フラグ
extern UB g_IrLed_SW;					//20160312Miya 極小精度UP

extern ID	rxtid_org;		// GET_UDPの為の元々のタスクID
extern ID	rxtid;			// GET_UDPの為のタスクID

extern UB RegImgBuf[20][2][2][80*40];	//[登録数][学習][ソーベル][認証サイズ]

extern UINT timer_10ms, count_1sec, count_1min, count_1hour;	// 時分秒カウント


extern UB CapImgBuf[2][2][80*40];		//[登録数][ソーベル][認証サイズ] //20160906Miya



//@}

/// @name プロトタイプ宣言
//@{

// va300.c
EXTERN UB sys_kindof_spec;	 // マンション仕様/１対１仕様　切替えフラグ　0:マンション(占有部)仕様、1:1対１（法人）仕様
						       // 			、2：マンション（共用部）仕様。3:デモ機・マンション(占有部)仕様、4:デモ機・1対１（法人）仕様 
							   // 			、5：デモ機・マンション（共用部）仕様。
EXTERN UB nyuu_shutu_kubun;	//  ユタカ仕様・入退室区分　0：未入退、1：入口、2：出口。

EXTERN UH g_DipSwCode;
							   
EXTERN int g_AuthCnt;	//20140423Miya 認証リトライ回数

EXTERN void telnetd_send(B *s);					///< TELNET送信
EXTERN void telnetd_send_bin(B *s, INT len);	///< TELNETバイナリ送信
EXTERN UB GetScreenNo(void);					// 現在表示スクリーン番号の取得
EXTERN void ChgScreenNo( UB NewScreenNo );		// 画面遷移状態(番号)を更新する
EXTERN void reload_CAP_Param( void );			// Gain. FixShutter,IR_LEDのパラメータを、初期要求値に戻す。（CAPのRetry時に変わった可能性がある為。）
EXTERN ER send_shutdown_req_Wait_Ack_Retry( void ); // シャットダウン要求の送信、Ack・Nack待ちとリトライ付き
EXTERN ER send_touroku_delete_Wait_Ack_Retry( void );	// 指（登録）データの削除要求の送信、Ack・Nack待ちとリトライ付き
EXTERN ER send_ID_No_check_req_Wait_Ack_Retry( void );	// ID番号問合せの送信、Ack・Nack待ちとリトライ付き
EXTERN ER send_ID_Authority_check_req_Wait_Ack_Retry( void );	// ID権限問合せの送信、Ack・Nack待ちとリトライ付き
EXTERN ER send_donguru_chk_Wait_Ack_Retry( void );		// ドングルの有無確認を送信、Ack・Nack待ちとリトライ付き
EXTERN ER send_password_chk_Wait_Ack_Retry( void );		// メンテナンス・モード移行時のパスワード確認要求の送信、Ack・Nack待ちとリトライ付き
EXTERN ER send_meinte_mode_Wait_Ack_Retry( void );		// モード切替通知の送信、Ack・Nack待ちとリトライ付き（メンテナンス・モード移行時）
EXTERN ER send_touroku_init_Wait_Ack_Retry( void );		// 登録データ初期化要求の送信、Ack・Nack待ちとリトライ付き（メンテナンス・モード時）
EXTERN ER send_kinkyuu_touroku_Wait_Ack_Retry( void );	// PCへ、緊急開錠番号通知送信、Ack・Nack待ちとリトライ付き
EXTERN ER send_kinkyuu_8keta_Wait_Ack_Retry( void );	// PCへ、緊急８桁番号データ要求送信、Ack・Nack待ちとリトライ付き
EXTERN ER send_kinkyuu_kaijyou_Wait_Ack_Retry( void );	// PCへ、緊急開錠番号送信、Ack・Nack待ちとリトライ付き
EXTERN ER send_kinkyuu_input_Wait_Ack_Retry( void );	// PCへ、緊急番号の妥当性問い合わせ確認要求送信、Ack・Nack待ちとリトライ付き
EXTERN UB GetSysSpec( void );		// マンション仕様/１対１仕様　切替えフラグ　0:マンション(占有部)仕様、1:1対１（法人）仕様、2：マンション（共用部）仕様。 
EXTERN ER set_reg_param_for_Lcd( void );
EXTERN void SetError(int err);	//20140930Miya

EXTERN void init_wdt( void );					// WDT(ウォッチドッグ・タイマー)の初期設定
EXTERN void reset_wdtc( void );					// WDT(ウォッチドッグ・タイマー)カウンタのクリア
EXTERN void stop_wdt( void );					// WDT(ウォッチドッグ・タイマー)の無効化設定
EXTERN void reset_wdt_cnt( void );			// WDTカウンタ用メモリのダイレクト・リセット処理(フラッシュ・メモリ・ドライバ処理専用)
											// 各TaskのWDTクリア・フラグを無視して、カウンタをクリアするので、
											// ドライバ内での使用に限る。（乱用すると、タスク単位でのWDT機能の意味がなくなる。）

EXTERN UINT sys_ScreenNo;		// 現在のスクリーンNo
EXTERN UB s_CapResult;			// 指認証の結果
EXTERN UB s_DongleResult;		// ドングルの有無確認の結果
EXTERN UB s_PasswordResult;		// パスワード確認の結果
EXTERN UB s_ID_NO_Result;		// ID番号確認の結果
EXTERN UB s_ID_Authority_Result;	// ID権限確認の結果
EXTERN UB s_ID_Authority_Level; // ID権限問合せコマンドで問合せたユーザーIDの権限レベル 。  ASCIIコード。 
EXTERN UB s_Kantoku_num[ 2 ];	// ID権限問合せ応答コマンドで得た監督者の総数。 ASCIIコード。
EXTERN UB s_Kanri_num[ 2 ];		// ID権限問合せ応答コマンドで得た管理者の総数。 ASCIIコード。
EXTERN UB s_Ippan_num[ 6 ];		// ID権限問合せ応答コマンドで得た一般者の総数。 ASCIIコード。
EXTERN UB s_KinkyuuTourokuResult;	// 緊急登録通知の結果

EXTERN UINT rcv_ack_nak;		// ack受信フラグ　=0：未受信、=1:ACK受信、=-1：nak受信
EXTERN void LcdPosAdj(int calc);	//20161031Miya Ver2204 LCDADJ
EXTERN ER	SndCmdCngMode( UINT stat );		// PCへモード切替え通知を送信
EXTERN FLGPTN befor_scrn_no;	// 画面遷移用変数。「中止しますか？」「いいえ」の時の戻り先のFLG_PTN番号。
EXTERN UB  befor_scrn_for_ctrl; // 画面遷移用変数。「中止しますか？」「いいえ」の時の戻り先の画面番号。
EXTERN UB req_restart;			// パワー・オン・プロセスの再起動要求フラグ　=0:要求無し、=1:要求あり
EXTERN UB Pfail_mode_count;	// 停電モードの場合の起動回数(他モードで起動した場合は、resetされる。)
EXTERN UINT Pfail_sense_flg;	// 停電検知通知受信フラグ　0:受信無し、1:受信あり
EXTERN UINT sys_demo_flg;		// デモ仕様フラグ
EXTERN UINT g_CapTimes;			// 1:撮影1回目 2:再撮影	//20131210Miya add

EXTERN UH Flbuf[0x10000];		//フラッシュ退避用バッファ(1セクション分)

EXTERN UINT door_open_over_time;	// ドア過開放設定時間
EXTERN UINT Test_1sec_timer;	// 1秒サイクリック・カウンタ・テスト用
EXTERN UINT Pfail_start_timer;	// 停電タイマーサイクリック・カウント用
EXTERN unsigned long WDT_counter;		// WDTタイマー用サイクリック・カウンタ
EXTERN unsigned long Ninshou_wait_timer;	// 指認証失敗時の画面待ち時間（30秒）

EXTERN unsigned short FpgaVerNum; //20140905Miya lbp追加 FPGAバージョンアップ
EXTERN UB f_fpga_ctrl;				//20140915Miya FPGA制御フラグ 0:V1.001用　1:V1.002用
EXTERN int g_key_arry[10];	//20140925Miya password open
EXTERN int g_Diagnosis_start;	//20140930Miya	//診断開始フラグ

EXTERN UINT main_TSK_wdt;		// mainタスク　WDTリセット・リクエスト・フラグ
EXTERN UINT camera_TSK_wdt;		// カメラタスク　WDTリセット・リクエスト・フラグ
EXTERN UINT ts_TSK_wdt;			// タッチセンサ・タスク　WDTリセット・リクエスト・フラグ
EXTERN UINT lcd_TSK_wdt;		// LCDタスク　WDTリセット・リクエスト・フラグ
EXTERN UINT sio_rcv_TSK_wdt;	// SIO受信タスク　WDTリセット・リクエスト・フラグ
EXTERN UINT sio_snd_TSK_wdt;	// SIO送信タスク　WDTリセット・リクエスト・フラグ

EXTERN int	g_pcproc_f;	//20160930Miya PCからVA300Sを制御する
EXTERN int	g_capallow; //20160930Miya PCからVA300Sを制御する
EXTERN int	g_pc_authnum; //20160930Miya PCからVA300Sを制御する
EXTERN int	g_pc_authtime; //20160930Miya PCからVA300Sを制御する

EXTERN BkDataNoClear g_BkDataNoClear;	//20161031Miya Ver2204 LCDADJ
EXTERN int	g_LedCheck;					//20161031Miya Ver2204 

EXTERN int dbg_dbg1;

//20170315Miya 400Finger ->
//EXTERN int g_RegBlockNum;		// 登録ブロック番号(1 ～ 7) -1:登録なし
EXTERN int g_RegAddrNum;		// 登録番地(0 ～ 239) -1:登録なし
//EXTERN int g_RegTotalYubiNum;	// 登録されている指の総数
EXTERN int g_BufAuthScore[240][20];	// 極小認証のスコアー
//EXTERN unsigned char g_taikyo_flg;		//20170320Miya 400FingerM2 静脈退去フラグ
//20170315Miya 400Finger <-
EXTERN unsigned short g_FPGA_ErrFlg;	//20170706Miya FPGAフリーズ対策


// Next_screen_ctrl.c
EXTERN ER NextScrn_Control_mantion( void );		// 次の画面コントロール（マンション(占有部)仕様の場合）
EXTERN ER NextScrn_Control_office( void );		// 次の画面コントロール（１対１仕様の場合）
EXTERN ER NextScrn_Control( void );				// 次の画面コントロール
EXTERN UB Chk_shutdown_ok( void );				// 認証操作中かどうかをチェックする
EXTERN ER Pfail_shutdown( void );		// 停電モード、停電検知通知受信の場合の、シャットダウン実行

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
extern int dbg_Auth_hcnt;	//20141014Miya
extern int g_lcdpos[3][2];	//20161031Miya Ver2204 LCDADJ

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
extern int IrLedOnOffSet(int sw, UH duty2, UH duty3, UH duty4, UH duty5);	//20160312Miya 極小精度UP
extern T_YBDATA yb_touroku_data;				// 指登録情報（１指分）
extern T_YBDATA20 yb_touroku_data20[21];		// 指登録情報（20指分）//追加　20130510 Miya

extern UB kinkyuu_tel_no[17];					// 緊急開錠電話番号１６桁（配列最終番目は区切り記号”,”）
extern UB kinkyuu_touroku_no[5];				// 緊急開錠の緊急登録番号４桁（配列最終番目は区切り記号　NUL　）　
extern UB kinkyuu_hyouji_no[9];					// 緊急開錠の緊急番号８桁表示データ（配列最終番目は区切り記号　NUL　）　
extern UB kinkyuu_kaijyo_no[9];					// 緊急開錠の開錠番号８桁データ（配列最終番目は区切り記号　NUL　）
extern UB kinkyuu_input_no[5];					// 緊急開錠時に入力された番号４桁（配列最終番目は区切り記号　NUL　）

extern UB mainte_password[5];					// メンテナンス・モード移行時の確認用パスワード４桁。（配列最終番目は区切り記号　NUL　）　

//extern UB g_ubCapBuf[ ( ( 160 * 140 ) * 3 ) ];//20131210Miya cng	///< 縮小画像取得用バッファ
extern UB g_ubCapBuf[ 640 * 480 ];	//20160601Miya

#if (NEWCMR == 1)
EXTERN ER NCmrI2C_Send( UB *pAddr, UB *pData, int iSendSize );		// カメラ・コマンドのI2C送信
EXTERN ER NCmrI2C_Rcv( UB *pAddr, UB *pData, int iRcvSize );		// カメラ・コマンドのI2C受信
#endif

extern UB g_ubHdrBuf[ 160 * 80 ];
//extern UB g_ubResizeBuf[ 80 * 40 ];
//extern UB g_ubResizeSvBuf[ 80 * 40 ];	//20140423miya 認証向上
extern UB g_ubResizeBuf[ 100 * 40 ];	//20140910Miya XSFT
extern UB g_ubResizeSvBuf[ 100 * 40 ];	//20140910Miya XSFT //20140905Miya LBP追加
extern UB g_ubSobelR1Buf[ 80 * 40 ];
extern UB g_ubSobelR2Buf[ 80 * 40 ];
extern UB g_ubSobelR3Buf[ 20 * 10 ];
extern UB g_ubSobelR1SvBuf[ 80 * 40 ];
extern UB g_ubSobelR2SvBuf[ 80 * 40 ];
extern UB g_ubSobelR3SvBuf[ 20 * 10 ];
extern unsigned short g_hdr_blend[ 160 * 80 ];
extern UB g_ubLbpBuf[ 80 * 40 ];	//20140905Miya LBP追加
extern UB g_ubResizeSvBuf2[ 80 * 40 ];	//20140905Miya LBP追加
extern UB g_ubSobelR3DbgBuf[ 20 * 10 ];	//20160312Miya 極小精度UP

extern unsigned short CmrCmdManualGetParameter( UB para );
extern ER CmrCmdFixShutterCtl( UB FSC_val );
extern ER CmrCmdManualGainCtl( UB gain_val );
extern ER Wait_Ack_forBlock( void );			// コマンドまたはブロックデータ送信後のAck応答待ち

extern int Check_Cap_Raw_flg;					// 生画像撮影チェック実行中フラグ　Added T.Nagai 
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
extern unsigned short usSioRcvCount;	// シリアル受信データ数
#define	RCV_BUF_SIZE	1024 + 4
extern UB cSioRcvBuf[ RCV_BUF_SIZE ];	// シリアル受信データバッファ

extern UB send_Ack_buff[ 4 ];  // ACK送信コードデータ・バッファ
extern UB send_Nack_buff[ 4 ]; // ACK送信コードデータ・バッファ
extern UB send_Enq_buff[ 4 ];  // ACK送信コードデータ・バッファ
extern UINT ENQ_sending_cnt;	 // ENQ再送カウンタ
extern UINT sio_rcv_comm_len;	 // 受信中コマンドの指定レングス
extern UINT sio_rcv_block_num;	 // 受信中コマンドのブロック番号

#define SND_SIO_BUF_SIZE 1024 + 4	 // シリアル送信コマンドデータ・バッファ
extern UB send_sio_buff[ SND_SIO_BUF_SIZE ];	 // シリアル送信コマンドデータ・バッファ
extern UINT sio_snd_comm_len;	 // 送信中コマンドの指定レングス
extern UINT sio_snd_block_num;	 // 送信中コマンドのブロック番号

extern unsigned short usSioBunkatuNum;	//20160930Miya PCからVA300Sを制御する

EXTERN int chk_rcv_cmddata( UB *Buf, int cmd_len  );	// 受信コマンド２０バイトの解析処理。

EXTERN  int send_Comm_Ack( INT ch );
EXTERN  int send_Comm_Nack( INT ch );
EXTERN  int send_Comm_EOT( INT ch );
EXTERN  void util_i_to_char( unsigned int i, char * buff );
EXTERN  void util_i_to_char( unsigned int i, char * buff );

EXTERN UB KeyIO_board_soft_VER[ 4 ];		// 制御BOX　錠制御SH2　ソフトウェア・バージョン番号
											// VA-300, VA-300s兼用。
EXTERN  UB dip_sw_data[ 4 ];	// VA-300sの制御Box内Dip SW状態 Bit0,　0：OFF、1：ON、　左桁からBit0,1,2,3

EXTERN UINT DBG_send_cnt;		// ソフトウェア・Debug用

// tsk_snd_serial.c
extern void SendSerialBinaryData( UB *data, UINT cnt );	// Serialデータの送信（汎用）
extern void send_sio_WakeUp( void );			// VA300S制御BoxへシリアルでWakeUpの問い合わせを行う（Test	コマンド、電源ON時）
extern void send_sio_ShutDown( void );			// VA300S制御Boxへシリアルでシャット・ダウン要求コマンドを送信する。
extern void send_sio_Touroku( void );			// VA300S制御Boxへシリアルで登録完了コマンド(01)を送信。
extern void send_sio_Touroku_Init( int j );		// VA300S制御Boxへシリアルで登録完了コマンド(01)を送信(電源ON時の一括送信)。
extern void send_sio_Touroku_InitAll( void );	// VA300S制御Boxへシリアルで登録完了コマンド(01)を送信メイン(電源ON時の一括送信)。
//extern void send_sio_Ninshou( UB result );		// VA300S制御Boxへシリアルで認証完了コマンド(03)を送信。
extern void send_sio_Ninshou( UB result, UB auth_type, UB info_type );		//20160108Miya FinKeyS // VA300S制御Boxへシリアルで認証完了コマンド(03)を送信。
extern void send_sio_Touroku_Del( void );		// VA300S制御Boxへシリアルで登録データ削除コマンド(04)を送信。
extern void send_sio_Touroku_AllDel( void );	// VA300S制御Boxへシリアルで登録データ初期化（一括削除）コマンド(05)を送信。
extern void send_sio_Kakaihou_time( void );		// VA300s制御Boxへシリアルで過開放時間の設定要求コマンドを送信する。
extern void send_sio_init_time( void );			// VA300s制御Boxへ初期時刻の設定要求コマンド(10)を送信する。
extern void send_sio_force_lock_close( void );	// VA300s制御Boxへ強制解錠コマンド(11)を送信する。
extern void send_sio_BPWR( int sw );		//20160905Miya B-PWR制御
extern void send_sio_VA300Reset( void );	//20161031Miya Ver2204 端末リセット
extern void send_sio_STANDARD( void );			//20160905Miya PCからVA300Sを制御する
extern void send_sio_TANMATU_INFO( void );		//20160905Miya PCからVA300Sを制御する
extern void send_sio_REGDATA_UPLD_STA( void );	//20160905Miya PCからVA300Sを制御する
extern void send_sio_REGDATA_UPLD_GET( void );	//20160905Miya PCからVA300Sを制御する
extern void send_sio_REGPROC( int rslt );		//20160905Miya PCからVA300Sを制御する
extern void send_sio_AUTHPROC( int rslt );		//20160905Miya PCからVA300Sを制御する

extern UB IfImageBuf[2*80*40 + 20*10];

// tsk_ninshou.c

#if( FREEZTEST )
EXTERN UB DoAuthProc( UB num );
#endif

EXTERN TASK NinshouTask( void );
extern void SendNinshouData( char *data, int cnt );	// データのタスク間送信（汎用・認証処理専用）
EXTERN void InitImgAuthPara( void );
EXTERN UB InitFlBkAuthArea( void );
EXTERN UB InitBkAuthData( void );
EXTERN UB SaveBkAuthDataTmpArea( void );
EXTERN UB SaveBkAuthDataFl( void );	//画像処理&認証処理バックアップデータ保存
EXTERN UB ReadBkAuthData( void );	//画像処理&認証処理バックアップデータ読込

EXTERN UB SaveBkDataNoClearFl( void );	//20161031Miya Ver2204 LCDADJ
EXTERN UB ReadBkDataNoClearFl( void );	//20161031Miya Ver2204 LCDADJ

EXTERN UB SetImgAuthPara( UB *data );
EXTERN UB DelImgAuthPara( void );
EXTERN UB SaveRegImgTmpArea( UB proc );
EXTERN UB SaveRegImgFlArea( int num );
EXTERN UB ReadRegImgArea( int num );
EXTERN UB AddRegImgFromRegImg( int sw, int num );	//20160312Miya 極小精度UP
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

//20160902Miya FPGA高速化
EXTERN int WrtImgToRam(int ts, int sbl);
EXTERN int Cal4Ave(int ts, int sbl, int *rt1, int *rt2, int *rt3, int *rt4);
EXTERN void MakeTestImg(void);
EXTERN void AutoMachingFPGA(UB num, int proc, double *scr1, double *scr2);


extern UB g_RegFlg;
extern int g_AuthOkCnt;	//認証OK回数　50回で学習画像保存
extern RegUserInfoData			g_RegUserInfoData;				//ユーザ情報
extern RegBloodVesselTagData	g_RegBloodVesselTagData[20];	//登録血流画像タグ情報
extern UseProcNum 				g_UseProcNum;					//ユーザー使用番号
extern PasswordOpen				g_PasswordOpen;					//パスワード開錠
extern PasswordOpen				g_InPasswordOpen;					//パスワード開錠
extern TechMenuData				g_TechMenuData;					//技術メンテ
extern AuthLog					g_AuthLog;						//認証ログ		//20140925Miya add log
extern MainteLog				g_MainteLog;					//メンテログ	//20140925Miya add log
extern RegImgDataAdd			g_RegImgDataAdd[20];		//20160312Miya 極小精度UP 登録画像(R1,LBP,R1極小)
extern PasswordOpen2			g_PasswordOpen2;				//20160108Miya FinKeyS //パスワード開錠

#if(AUTHTEST >= 1)	//20160613Miya
extern unsigned short g_sv_okcnt;
extern unsigned short g_sv_okcnt0;
extern unsigned short g_sv_okcnt1;
extern unsigned short g_sv_okcnt2;
extern unsigned short g_sv_okcnt3;
extern unsigned short g_sv_okcnt4;
extern UB TstAuthImgBuf[8][100*40];	//[OK/NG][8個][シフト前サイズ]
extern unsigned short g_cmr_err;
extern unsigned short g_imgsv_f;
#endif

// tsk_log.c
EXTERN TASK LogTask( void );
extern void SendLogData( char *data, int cnt );	// データのタスク間送信（汎用・ロギング処理専用）

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

extern int	g_SameImgGet;	//20151118Miya 同画像再撮影
extern int	g_SameImgCnt;	//20160115Miya 同画像再撮影
extern int	g_AuthType;		//20160120Miya 0:指認証 1:パスワード認証 2:緊急開錠
extern int	ode_oru_sw;						//20160108Miya FinKeyS おでかけ・お留守番SW
extern int g_TestCap_start;
extern UW	dbg_flwsize;
extern UW	dbg_flwsizeIn;

extern int	dbg_ts_flg;
extern int	dbg_cam_flg;
extern int	dbg_nin_flg;
extern int	dbg_cap_flg;
extern int g_MainteLvl;	//20160711Miya デモ機

extern PosGenten g_PosGenten[2];

#if ( NEWCMR == 1 )
extern int g_cmr_dbgcnt;
#endif


//@}
#endif										// end of _VA300_H_
