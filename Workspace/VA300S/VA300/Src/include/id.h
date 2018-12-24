/**
*	VA-300プログラム
*
*	@file id.h
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/08/01
*	@brief  タスクID定義情報(他案件から流用)
*
*	Copyright (C) 2010, OYO Electric Corporation
*/
#ifndef	_ID_H_
#define	_ID_H_

//=== 定義　（本定義は、ファイル"va300.h"、"id.h"の２か所で同じ定義を行うこと。）
#define VA300S	1		// =0 : VA300,   =1 : VA300S  =2 : VA300Sの場合でデバッグ用に画像データをLANでPCへ送る。
#define VA300ST 1		// =0 : VA300,VA300S   =1 : VA300ST(ユタカ仕様)

//========================= iTronのための定数宣言 ===============================
//xxx_NULL～xxx_MAX == xxxID_MAX+1
//===============================================================================

/********************************************************************
	Task ID の宣言
********************************************************************/
enum tskid{
	TSK_NULL,
	TSK_MAIN,							///< VA300 端末制御メインタスク
	TSK_DISP,							///< 表示タスク
#if ( VA300S == 0 )
	TSK_CMD_LAN,						///< LANコマンド処理タスク
	TSK_UDPRCV,							///< UDP 受信タスク
	TSK_COMMUNICATE,					///< UDPからの電文処理
	TSK_UDP_SEND,						///< UDP送信
#endif
#if ( VA300S == 1 )
	TSK_NINSHOU,						///< 認証タスク
	TSK_LOG,							///< ロギングタスク
#endif
#if ( VA300S == 2 )
	TSK_CMD_LAN,						///< LANコマンド処理タスク
	TSK_UDPRCV,							///< UDP 受信タスク
	TSK_COMMUNICATE,					///< UDPからの電文処理
	TSK_UDP_SEND,						///< UDP送信
	TSK_NINSHOU,						///< 認証タスク
	TSK_LOG,							///< ロギングタスク
#endif

	TSK_WDT,							///< ウォッチドッグタスク
	TSK_IO,								///< I/O検知タスク
	TSK_EXKEY,							///< 外部キー入力タスク
	TSK_LCD,							///< LCD表示タスク
	TSK_BUZ,							///< ブザーテストタスク
	TSK_TS,								///< 生態検知センサタスク
	TSK_CAMERA,							///< カメラ撮影制御タスク
//	TSK_CMD_MON,						///< モニタタスク
//	TSK_RCV_MON,						///< モニタタスク
	TSK_RCV1,							///< シリアルCH0受信タスク
	TSK_SND1,							///< シリアルCH0送信タスク
//	TSK_MODE,							///< モード管理タスク
//	TSK_TELNETD,						///< TELNETタスク
//	TSK_SHELL1,							///< シェルタスク

    TSK_LEARN_DATA,

	TSK_MAX
};

/********************************************************************
	セマフォ ID の宣言
********************************************************************/
enum semid{
	SEM_NULL,
	SEM_LED,							///< LED
	SEM_7SEG,							///< 7SEG
	SEM_RTC,							///< RTC
	SEM_ERR,							///< エラーステータス
	SEM_SROM,							///< Serial EEPROM
	SEM_FPGA,							///< FPGA
	SEM_TIM,							///< タイマ
	SEM_BUZ,							///< ブザー
	SEM_CMR,							///< カメラ
	SEM_IRLED,							///< 赤外線LED
	SEM_TPL,							///< タッチパネル
	SEM_LCD,							///< LCD
	SEM_FL,								///< フラッシュメモリ
	SEM_SPF,							///< sprintf
	SEM_STKN,							///< strtok
	SEM_STL,							///< strtol
	
	SEM_MAX
};

/********************************************************************
	周期ハンドラ IDの宣言
********************************************************************/
enum cycid{
	CYC_NULL,
	CYC_TMO,							///< タイマー用周期ハンドラ
	CYC_LAN,							///< LAN用周期ハンドラ(予約)

	CYC_MAX
};

/********************************************************************
	メッセージバッファ ID の宣言、固定長
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
	メモリープール ID の宣言、固定長
********************************************************************/
enum mpfid{
	MPF_NULL,
	MPF_COM,
	MPF_SND_SIO,						///< for some task memory pool
#if ( VA300S == 0 )
	MPF_LRES,							///< for LAN response memory pool
#endif
#if ( VA300S == 1 )
	MPF_SND_NINSHOU,					///< 認証処理データ用メモリ・プール
	MPF_LOG_DATA,						///< 認証処理データ用メモリ・プール
#endif
#if ( VA300S == 2 )
	MPF_LRES,							///< for LAN response memory pool
	MPF_SND_NINSHOU,					///< 認証処理データ用メモリ・プール
	MPF_LOG_DATA,						///< 認証処理データ用メモリ・プール
#endif
	MPF_MON,							///< for monitor task memory pool
	MPF_DISP,							///< for Display task memory pool
	
	MPF_MAX
};

/********************************************************************
	mailBox ID の宣言
********************************************************************/
enum mbxid{
	MBX_NULL,
	MBX_MODE,							///< モードタスクのメールボックス
	MBX_DISP,							///< 表示タスクのメールボックス
	MBX_SND_SIO,						///< シリアル送信用メールボックス
#if ( VA300S == 0 )
	MBX_RESSND,							///< レスポンス送信用メールボックス
	MBX_CMD_LAN,						///< LANコマンドインタープリタ 10/02/19
#endif
#if ( VA300S == 1 )
	MBX_SND_NINSHOU,					///< 認証処理データのタスク間送信用メール・ボックス
	MBX_LOG_DATA	,					///< ロギング処理データのタスク間送信用メール・ボックス
#endif
#if ( VA300S == 2 )
	MBX_RESSND,							///< レスポンス送信用メールボックス
	MBX_CMD_LAN,						///< LANコマンドインタープリタ 10/02/19
	MBX_SND_NINSHOU,					///< 認証処理データのタスク間送信用メール・ボックス
	MBX_LOG_DATA	,					///< ロギング処理データのタスク間送信用メール・ボックス
#endif
	MBX_CMD_LBUS,						///< ローカルバスコマンドインタープリタ 10/02/19
	MBX_MON,							///< モニタ向け

	MBX_MAX
};

/********************************************************************
	alh ID の宣言
********************************************************************/
enum alhid{
	ALH_NULL,
	ALH_TIM,							///< タイマー用
	
	ALH_MAX
};

/********************************************************************
	flg ID の宣言（wai_flgの引数"flg_id"　イベントフラグID）
********************************************************************/
enum flgid{
	ID_FLG_NULL,
	ID_FLG_MAIN,					///< メインコントロール
	ID_FLG_CAMERA,					///< カメラコントロール 
	ID_FLG_TS,						///< 生体検知用
	ID_FLG_LCD,						///< LCDコントロール
	ID_FLG_IO,						///< I/O検知用
	ID_FLG_EXKEY,					///< 外部キー検知用
	ID_FLG_BUZ,						///< ブザー用
#if ( VA300S == 0 )
	ID_FLG_LRCV,					///< UDPコマンド受信コントロール
#endif
#if ( VA300S == 1 )
	ID_FLG_NINSHOU,					///< 認証処理用（使用しないかも？）
	ID_FLG_LOG,						///< ロギング処理用（使用しないかも？）
#endif
#if ( VA300S == 2 )
	ID_FLG_LRCV,					///< UDPコマンド受信コントロール
	ID_FLG_NINSHOU,					///< 認証処理用（使用しないかも？）
	ID_FLG_LOG,						///< ロギング処理用（使用しないかも？）
#endif
	FLG_MAX
};

/********************************************************************
	flg の宣言（wai_flgの引数"flgptn"　フラグパターン）
********************************************************************/
// To メインタスク（ID_FLG_MAIN）
#define FPTN_START_CAP			0x10000001L // カメラキャプチャ開始要求（トリミング+縮小画像）
#define FPTN_START_RAWCAP		0x10000002L // カメラキャプチャ開始要求（生画像）
#define FPTN_LCD_CHG_REQ		0x10000004L // LCD画面切替要求(LCD→メイン)
#define FPTN_SEND_REQ_MAINTE_CMD 0x10000008L 	// 受信コマンド解析タスク→メインTaskへ、メンテナンスモード切替え通知送信を依頼。

// To カメラタスク（ID_FLG_CAMERA）
#define FPTN_START_CAP204		0x20000001L // カメラキャプチャ開始要求（登録画像、コマンド204）
#define FPTN_START_CAP211		0x20000002L // カメラキャプチャ開始要求（認証用画像、コマンド211）
#define FPTN_START_CAP141		0x20000004L // カメラキャプチャ開始要求（生画像、コマンド141）
#define FPTN_CHECK_IMAGE		0x20000006L	// カメラキャプチャメモリチェック
#define FPTN_SETREQ_GAIN		0x20000008L // カメラ・ゲインの初期値設定（コマンド022の実行）
#define FPTN_SETREQ_SHUT1		0x20000009L // カメラ・シャッタースピード１の初期値設定（コマンド022の実行）
#define FPTN_CMR_INIT			0x2000000aL // カメラWeakUP //20140930Miya Bio FPGA
#define FPTN_REROAD_PARA		0x2000000bL // カメラパラメータリロード//20150930Miya


// To 生体検知タスク（ID_FLG_TS）
#define FPTN_CMR_TS				0x30000001L // カメラ撮影終了、LEDのブリンク開始へ。
#define FPTN_END_TS				0x30000002L // 生体検知処理の終了、再検知待ちへ。
#define FPTN_WAIT_CHG_SCRN		0x30000004L // 画面が「指を抜いて下さい」または、失敗画面になるのを待つ。

// To LCDタスク（ID_FLG_LCD）
#define FPTN_LCD_INIT			0x40000001L // LCD初期画面表示要求(メイン→LCD、ノーマルモード移行の時)

#define FPTN_LCD_SCREEN1		0x40000002L // 画面１表示要求(メイン→LCD) 初期登録モード
#define FPTN_LCD_SCREEN2		0x40000003L // 画面２表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN3		0x40000004L // 画面３表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN4		0x40000005L // 画面４表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN5		0x40000006L // 画面５表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN6		0x40000007L // 画面６表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN7		0x40000008L // 画面７表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN8		0x40000009L // 画面８表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN9		0x4000000aL // 画面９表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN10		0x4000000bL // 画面１０表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN11		0x4000001cL // 画面１１表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN12		0x4000001dL // 画面１２表示要求(メイン→LCD)

#define FPTN_LCD_SCREEN100		0x4000001eL // 画面１００表示要求(メイン→LCD)　通常モード
#define FPTN_LCD_SCREEN101		0x4000001fL // 画面１０１表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN102		0x40000020L // 画面１０２表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN103		0x40000021L // 画面１０３表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN104		0x40000022L // 画面１０４表示要求(メイン→LCD)

#define FPTN_LCD_SCREEN120		0x40000023L // 画面１２０表示要求(メイン→LCD)　通常モード（登録）
#define FPTN_LCD_SCREEN121		0x40000024L // 画面１２１表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN122		0x40000025L // 画面１２２表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN123		0x40000026L // 画面１２３表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN124		0x40000027L // 画面１２４表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN125		0x40000028L // 画面１２５表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN126		0x40000029L // 画面１２６表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN127		0x4000002aL // 画面１２７表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN128		0x4000002bL // 画面１２８表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN129		0x4000002cL // 画面１２９表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN130		0x4000002dL // 画面１３０表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN131		0x4000002eL // 画面１３１表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN132		0x4000002fL // 画面１３２表示要求(メイン→LCD)

#define FPTN_LCD_SCREEN140		0x40000030L // 画面１４０表示要求(メイン→LCD)　通常モード（削除）
#define FPTN_LCD_SCREEN141		0x40000031L // 画面１４１表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN142		0x40000032L // 画面１４２表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN143		0x40000033L // 画面１４３表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN144		0x40000034L // 画面１４４表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN145		0x40000035L // 画面１４５表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN146		0x40000036L // 画面１４６表示要求(メイン→LCD)

#define FPTN_LCD_SCREEN160		0x40000037L // 画面１６０表示要求(メイン→LCD)　通常モード（緊急開錠番号設定）
#define FPTN_LCD_SCREEN161		0x40000038L // 画面１６１表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN162		0x40000039L // 画面１６２表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN163		0x4000003aL // 画面１６３表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN164		0x4000003bL // 画面１６４表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN165		0x4000003cL // 画面１６５表示要求(メイン→LCD)

#define FPTN_LCD_SCREEN170		0x4000003dL // 画面表示要求(メイン→LCD)　通常モード（緊急開錠）
#define FPTN_LCD_SCREEN171		0x4000003eL // 画面表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN172		0x4000003fL // 画面表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN173		0x40000040L // 画面表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN174		0x40000041L // 画面表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN175		0x40000042L // 画面表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN176		0x40000043L // 画面表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN177		0x40000044L // 画面表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN178		0x40000045L // 画面表示要求(メイン→LCD)

#define FPTN_LCD_SCREEN200		0x40000046L // 画面２００表示要求(メイン→LCD)　メンテナンス・モード
#define FPTN_LCD_SCREEN201		0x40000047L // 画面２０１表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN202		0x40000048L // 画面２０２表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN203		0x40000049L // 画面２０３表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN204		0x4000004aL // 画面２０４表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN205		0x4000004bL // 画面２０５表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN206		0x4000004cL // 画面２０６表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN207		0x4000004dL // 画面２０７表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN208		0x4000004eL // 画面２０８表示要求(メイン→LCD)

#define FPTN_LCD_SCREEN400		0x4000004fL // 画面４００表示要求(メイン→LCD)　１対１仕様：初期登録モード
#define FPTN_LCD_SCREEN401		0x40000050L // 画面４０１表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN402		0x40000051L // 画面４０２表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN403		0x40000052L // 画面４０３表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN404		0x40000053L // 画面４０４表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN405		0x40000054L // 画面４０５表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN406		0x40000055L // 画面４０６表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN407		0x40000056L // 画面４０７表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN408		0x40000057L // 画面４０８表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN409		0x40000058L // 画面４０９表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN410		0x40000059L // 画面４１０表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN411		0x4000005aL // 画面４１１表示要求(メイン→LCD)
	
#define FPTN_LCD_SCREEN500		0x4000005bL // 画面５００表示要求(メイン→LCD)　１対１仕様：通常モード
#define FPTN_LCD_SCREEN501		0x4000005cL // 画面５０１表示要求(メイン→LCD)	
#define FPTN_LCD_SCREEN502		0x4000005dL // 画面５０２表示要求(メイン→LCD)	
#define FPTN_LCD_SCREEN503		0x4000005eL // 画面５０３表示要求(メイン→LCD)	
#define FPTN_LCD_SCREEN504		0x4000005fL // 画面５０４表示要求(メイン→LCD)	
#define FPTN_LCD_SCREEN505		0x40000060L // 画面５０５表示要求(メイン→LCD)	
#define FPTN_LCD_SCREEN506		0x40000061L // 画面５０６表示要求(メイン→LCD)	
	
#define FPTN_LCD_SCREEN520		0x40000062L // 画面５２０表示要求(メイン→LCD)　１対１仕様：登録モード
#define FPTN_LCD_SCREEN521		0x40000063L // 画面５２１表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN522		0x40000064L // 画面５２２表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN523		0x40000065L // 画面５２３表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN524		0x40000066L // 画面５２４表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN525		0x40000067L // 画面５２５表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN526		0x40000068L // 画面５２６表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN527		0x40000069L // 画面５２７表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN528		0x4000006aL // 画面５２８表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN529		0x4000006bL // 画面５２９表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN530		0x4000006cL // 画面５３０表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN531		0x4000006dL // 画面５３１表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN532		0x4000006eL // 画面５３２表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN533		0x4000006fL // 画面５３３表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN534		0x40000070L // 画面５３４表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN535		0x40000071L // 画面５３５表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN536		0x40000072L // 画面５３６表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN537		0x40000073L // 画面５３７表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN538		0x40000074L // 画面５３８表示要求(メイン→LCD)
	
#define FPTN_LCD_SCREEN542		0x40000075L // 画面５４２表示要求(メイン→LCD)　１対１仕様：削除モード
#define FPTN_LCD_SCREEN543		0x40000076L // 画面５４３表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN544		0x40000077L // 画面５４４表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN545		0x40000078L // 画面５４５表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN546		0x40000079L // 画面５４６表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN547		0x4000007aL // 画面５４７表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN548		0x4000007bL // 画面５４８表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN549		0x4000007cL // 画面５４９表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN550		0x4000007dL // 画面５５０表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN551		0x4000007eL // 画面５５１表示要求(メイン→LCD)

#define FPTN_LCD_SCREEN105		0x4000007fL // 画面１０５表示要求(メイン→LCD) 通常モード //20140423Miya 認証リトライ追加
#define FPTN_LCD_SCREEN106		0x40000080L // 画面１０６表示要求(メイン→LCD) 通常モード //20140423Miya 認証リトライ追加

#define FPTN_LCD_SCREEN108		0x4000008aL // 画面１０８表示要求(メイン→LCD) 通常モード //20140925Miya password_open
#define FPTN_LCD_SCREEN109		0x4000008bL // 画面１０９表示要求(メイン→LCD) 通常モード //20140925Miya password_open
#define FPTN_LCD_SCREEN110		0x4000008cL // 画面１０９表示要求(メイン→LCD) 通常モード //20140925Miya password_open

#define FPTN_LCD_SCREEN180		0x4000008dL // 画面１９０表示要求(メイン→LCD) 通常モード //20140925Miya password_open
#define FPTN_LCD_SCREEN181		0x4000008eL // 画面１９１表示要求(メイン→LCD) 通常モード //20140925Miya password_open
#define FPTN_LCD_SCREEN182		0x4000008fL // 画面１９２表示要求(メイン→LCD) 通常モード //20140925Miya password_open
#define FPTN_LCD_SCREEN183		0x40000090L // 画面１９３表示要求(メイン→LCD) 通常モード //20140925Miya password_open
#define FPTN_LCD_SCREEN184		0x40000091L // 画面１９４表示要求(メイン→LCD) 通常モード //20140925Miya password_open
#define FPTN_LCD_SCREEN185		0x40000092L // 画面１９５表示要求(メイン→LCD) 通常モード //20140925Miya password_open
#define FPTN_LCD_SCREEN186		0x40000093L // 画面１９６表示要求(メイン→LCD) 通常モード //20140925Miya password_open
#define FPTN_LCD_SCREEN187		0x40000094L // 画面１９７表示要求(メイン→LCD) 通常モード //20140925Miya password_open

#define FPTN_LCD_SCREEN220		0x40000095L // 画面２２０表示要求(メイン→LCD) 通常モード //20140925Miya password_open
#define FPTN_LCD_SCREEN221		0x40000096L // 画面２２１表示要求(メイン→LCD) 通常モード //20140925Miya password_open
#define FPTN_LCD_SCREEN222		0x40000097L // 画面２２２表示要求(メイン→LCD) 通常モード //20140925Miya password_open
#define FPTN_LCD_SCREEN223		0x40000098L // 画面２２３表示要求(メイン→LCD) 通常モード //20140925Miya password_open
#define FPTN_LCD_SCREEN224		0x40000099L // 画面２２４表示要求(メイン→LCD) 通常モード //20140925Miya password_open
#define FPTN_LCD_SCREEN225		0x4000009aL // 画面２２５表示要求(メイン→LCD) 通常モード //20140925Miya password_open

#define FPTN_LCD_SCREEN240		0x4000009bL // 画面２４０表示要求(メイン→LCD) 通常モード //20140925Miya password_open
#define FPTN_LCD_SCREEN241		0x4000009cL // 画面２４１表示要求(メイン→LCD) 通常モード //20140925Miya password_open
#define FPTN_LCD_SCREEN242		0x4000009dL // 画面２４２表示要求(メイン→LCD) 通常モード //20140925Miya password_open
#define FPTN_LCD_SCREEN243		0x4000009eL // 画面２４３表示要求(メイン→LCD) 通常モード //20140925Miya password_open
#define FPTN_LCD_SCREEN244		0x4000009fL // 画面２４４表示要求(メイン→LCD) 通常モード //20140925Miya password_open
#define FPTN_LCD_SCREEN245		0x400000a0L // 画面２４５表示要求(メイン→LCD) 通常モード //20140925Miya password_open
#define FPTN_LCD_SCREEN246		0x400000a1L // 画面２４６表示要求(メイン→LCD) 通常モード //20140925Miya password_open
#define FPTN_LCD_SCREEN247		0x400000a2L // 画面２４７表示要求(メイン→LCD) 通常モード //20140925Miya password_open
#define FPTN_LCD_SCREEN248		0x400000a3L // 画面２４８表示要求(メイン→LCD) 通常モード //20140925Miya password_open
#define FPTN_LCD_SCREEN249		0x400000a4L // 画面２４９表示要求(メイン→LCD) 通常モード //20140925Miya password_open

#define FPTN_LCD_SCREEN260		0x400000a5L // 画面２６０表示要求(メイン→LCD) 通常モード //20140925Miya password_open
#define FPTN_LCD_SCREEN261		0x400000a6L // 画面２６１表示要求(メイン→LCD) 通常モード //20140925Miya password_open
#define FPTN_LCD_SCREEN262		0x400000a7L // 画面２６２表示要求(メイン→LCD) 通常モード //20140925Miya password_open
#define FPTN_LCD_SCREEN263		0x400000a8L // 画面２６３表示要求(メイン→LCD) 通常モード //20140925Miya password_open
#define FPTN_LCD_SCREEN264		0x400000a9L // 画面２６４表示要求(メイン→LCD) 通常モード //20140925Miya password_open
#define FPTN_LCD_SCREEN265		0x400000aaL // 画面２６４表示要求(メイン→LCD) 通常モード //20140925Miya password_open
#define FPTN_LCD_SCREEN266		0x400000abL // 画面２６４表示要求(メイン→LCD) 通常モード //20140925Miya password_open
#define FPTN_LCD_SCREEN267		0x400000acL // 画面２６４表示要求(メイン→LCD) 通常モード //20140925Miya password_open

//20160108Miya FinKeyS ->
#define FPTN_LCD_SCREEN600		0x400000adL // 通常モード2 (FinKeyS)
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

#define FPTN_LCD_SCREEN620		0x400000bbL // 工事モード
#define FPTN_LCD_SCREEN621		0x400000bcL // 
#define FPTN_LCD_SCREEN622		0x400000bdL // 
#define FPTN_LCD_SCREEN623		0x400000beL // 
#define FPTN_LCD_SCREEN624		0x400000bfL // 
#define FPTN_LCD_SCREEN625		0x400000c0L // 

#define FPTN_LCD_SCREEN188		0x400000c1L // パスワード変更　追加(家族分)
#define FPTN_LCD_SCREEN133		0x400000c2L // 画面１３２表示要求(メイン→LCD)
//20160108Miya FinKeyS <-

#define FPTN_LCD_SCREEN999		0x400000999 // 画面９９９表示要求(メイン→LCD)　「シャットダウンします」

// To I/O
#define	FPTN_INIT				0x50000001L	// 初期化
#define	FPTN_ERR				0x50010002L	// エラー発生
#define	FPTN_CAP_TR_END			0x51000004L	// DMA転送完了
#define	FPTN_DEBUG				0x50000008L	// デバッグ

// To ブザータスク（s_idFlg）
#define	FPTN_EMG_ON				0x60000001L	// 警報用ブザーON
#define	FPTN_EMG_OFF			0x60000002L	// 警報用ブザーOFF
#define	FPTN_TPL_OK				0x60000010L	// タッチパネル用ブザー(ピッ)
#define	FPTN_TPL_NG				0x60000020L	// タッチパネル用ブザー(ピピッ)
#define	FPTN_TPL_OFF			0x60000040L	// タッチパネル用ブザーOFF

// To 外部キー
#define	FPTN_KEY				0x70000001L	// キー入力

#if ( VA300s == 1 )
// To 認証タスク
#define FPTN_START_NINSHOU		0x80000001L	// 認証開始要求（使用しないかも？）

// To ロギングタスク
#define FPTN_START_LOG			0x90000001L	// １レコード分のロギング要求（使用しないかも？）

#endif


#endif								// end of _ID_H_