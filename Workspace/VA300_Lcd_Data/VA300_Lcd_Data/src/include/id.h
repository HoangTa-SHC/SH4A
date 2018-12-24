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

//========================= iTronのための定数宣言 ===============================
//xxx_NULL～xxx_MAX == xxxID_MAX+1
//===============================================================================

/********************************************************************
	Task ID の宣言
********************************************************************/
enum tskid{
	TSK_NULL,
	TSK_MAIN,							///< VA300 端末制御メインタスク
	TSK_CMD_LAN,						///< LANコマンド処理タスク
	TSK_DISP,							///< 表示タスク
	TSK_UDPRCV,							///< UDP 受信タスク
	TSK_COMMUNICATE,					///< UDPからの電文処理
	TSK_UDP_SEND,						///< UDP送信
	TSK_WDT,							///< ウォッチドッグタスク
	TSK_IO,								///< I/O検知タスク
	TSK_EXKEY,							///< 外部キー入力タスク
	TSK_LCD,							///< LCD表示タスク
	TSK_BUZ,							///< ブザーテストタスク
	TSK_TS,								///< 生態検知センサタスク
	TSK_CAMERA,							///< カメラ撮影制御タスク
//	TSK_CMD_MON,						///< モニタタスク
//	TSK_RCV_MON,						///< モニタタスク
//	TSK_RCV1,							///< シリアルCH0受信タスク
	TSK_SND1,							///< シリアルCH0送信タスク
//	TSK_MODE,							///< モード管理タスク
//	TSK_TELNETD,						///< TELNETタスク
//	TSK_SHELL1,							///< シェルタスク

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
	MPF_COM,							///< for some task memory pool
	MPF_MON,							///< for monitor task memory pool
	MPF_DISP,							///< for Display task memory pool
	MPF_LRES,							///< for LAN response memory pool
	
	MPF_MAX
};

/********************************************************************
	mailBox ID の宣言
********************************************************************/
enum mbxid{
	MBX_NULL,
	MBX_MODE,							///< モードタスクのメールボックス
	MBX_DISP,							///< 表示タスクのメールボックス
	MBX_RESSND,							///< レスポンス送信用メールボックス
	MBX_SND,							///< シリアル送信用メールボックス
	MBX_CMD_LAN,						///< LANコマンドインタープリタ 10/02/19
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
	ID_FLG_LRCV,					///< UDPコマンド受信コントロール
	ID_FLG_IO,						///< I/O検知用
	ID_FLG_EXKEY,					///< 外部キー検知用
	ID_FLG_BUZ,						///< ブザー用

	FLG_MAX
};

/********************************************************************
	flg の宣言（wai_flgの引数"flgptn"　フラグパターン）
********************************************************************/
// To メインタスク（ID_FLG_MAIN）
#define FPTN_START_CAP			0x10000001L // カメラキャプチャ開始要求（トリミング+縮小画像）
#define FPTN_START_RAWCAP		0x10000002L // カメラキャプチャ開始要求（生画像）
#define FPTN_LCD_CHG_REQ		0x10000004L // LCD画面切替要求(LCD→メイン)

// To カメラタスク（ID_FLG_CAMERA）
#define FPTN_START_CAP204		0x20000001L // カメラキャプチャ開始要求（登録画像、コマンド204）
#define FPTN_START_CAP211		0x20000002L // カメラキャプチャ開始要求（認証用画像、コマンド211）
#define FPTN_START_CAP141		0x20000004L // カメラキャプチャ開始要求（生画像、コマンド141）
#define FPTN_SETREQ_GAIN		0x20000008L // カメラ・ゲインの初期値設定（コマンド022の実行）
#define FPTN_SETREQ_SHUT1		0x20000010L // カメラ・シャッタースピード１の初期値設定（コマンド022の実行）

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

#define FPTN_LCD_SCREEN180		0x4000003dL // 画面１８０表示要求(メイン→LCD)　通常モード（緊急開錠）
#define FPTN_LCD_SCREEN181		0x4000003eL // 画面１８１表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN182		0x4000003fL // 画面１８２表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN183		0x40000040L // 画面１８３表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN184		0x40000041L // 画面１８４表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN185		0x40000042L // 画面１８５表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN186		0x40000043L // 画面１８６表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN187		0x40000044L // 画面１８７表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN188		0x40000045L // 画面１８８表示要求(メイン→LCD)

#define FPTN_LCD_SCREEN200		0x40000046L // 画面２００表示要求(メイン→LCD)　メンテナンス・モード
#define FPTN_LCD_SCREEN201		0x40000047L // 画面２０１表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN202		0x40000048L // 画面２０２表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN203		0x40000049L // 画面２０３表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN204		0x4000004aL // 画面２０４表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN205		0x4000004bL // 画面２０５表示要求(メイン→LCD)
#define FPTN_LCD_SCREEN206		0x4000004cL // 画面２０６表示要求(メイン→LCD)

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

#endif								// end of _ID_H_