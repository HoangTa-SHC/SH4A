//=============================================================================
/**
*	VA-300プログラム
*
*	@file command.h
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/8/24
*	@brief  通信コマンド定義情報
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
//=============================================================================
#include <ctype.h>
#include <machine.h>
#include <string.h>
#include <stdarg.h>

#ifndef	_COMMAND_H_
#define	_COMMAND_H_

//=== 定義
// 型
#define	FMT_LEN_SIZE	2					///< データ長
#define	FMT_CMD_SIZE	3					///< コマンド番号サイズ
#define	FMT_BLKNO_SIZE	4					///< ブロック番号サイズ
#define	FMT_DATA_SIZE	2					///< データ部分の最低限のデータサイズ(ただし、実際は可変長)
typedef struct {							///< コマンドフォーマット構造体
	UB		ubType;							///< 手順タイプ
	UB		ubLen[ FMT_LEN_SIZE ];			///< データ長
	UB		ubOrg;							///< 送信元
	char	cCmd[ FMT_CMD_SIZE ];			///< コマンド
	char	cBlkNo[ FMT_BLKNO_SIZE ];		///< ブロック番号
	char	cData[ FMT_DATA_SIZE ];			///< データサイズ
} ST_CMDFMT;

// 文字コード
#define CODE_EOT	0x04					///< EOT
#define CODE_ACK	0x06					///< ACK
#define CODE_CR		0x0D					///< CR
#define CODE_LF		0x0A					///< LF
#define CODE_NACK	0x15					///< NACK

// ヘッダ
enum HEADER_CODE {
	HC_ERR = -1,							///< 規定無
	HC_A = '#',								///< 手順A
	HC_B,									///< 手順B
	HC_C,									///< 手順C
	HC_D,									///< 手順D
	HC_E,									///< 手順E
	
};

// 送信元種別
enum HOST_TYPE {
	HOST_PC = '0',							///< PCからの送信
	HOST_TERM1,								///< 端末1からの送信
	HOST_TERM2,								///< 端末2からの送信
	
};

/// コマンド
// (通信コマンド)
// 共通コマンド
#define	CMD_REQ 			000				///< コマンド問い合わせ(端末→制御盤)
#define	CMD_RES_T			001				///< 応答コマンド(端末→制御盤)
#define	CMD_RES_C			002				///< 応答コマンド(制御盤→端末)
// 電源ONと初期化処理
#define	CMD_POW_ON			010				///< 電源ON通知(制御盤→端末)
#define	CMD_POW_OFF			011				///< 電源OFF通知(制御盤→端末)
#define	CMD_CMR_DEF			012				///< カメラ・パラメータの初期値送信(制御盤→端末)
#define	CMD_LGT_DEF			013				///< LED光量数値の初期値送信(制御盤→端末)
#define	CMD_PRM_DEF			014				///< 設定データの初期値送信(制御盤→端末)
#define	CMD_CND_DEF			015				///< 撮像条件の初期値送信(制御盤→端末)

// メンテナンス・モード
#define	CMD_MD_MNT_T		100				///< モード切替通知(端末→制御盤)
#define	CMD_MD_MNT_C		101				///< モード切替通知(制御盤→端末)
#define	CMD_LCD_REQ			102				///< LCD画面表示データ転送開始要求(端末→制御盤)
#define	CMD_LCD_WRT			103				///< LCD画面表示データのブロック転送(制御盤→端末)
#define	CMD_MID_SET			104				///< 保守ID登録・変更(端末→制御盤)
#define	CMD_MID_REQ			105				///< 保守ID参照要求(端末→制御盤)
#define	CMD_MID_SND			106				///< 保守ID送信(制御盤→端末)
#define	CMD_PRM_INI			107				///< 設定データ初期化要求(端末→制御盤)
#define	CMD_PRM_SET			108				///< 設定データの登録・変更(端末→制御盤)
#define	CMD_PRM_REQ			109				///< 設定データの参照要求(端末→制御盤)
#define	CMD_PRM_SND			110				///< 設定データの送信(制御盤→端末)
#define	CMD_RGST_INI		111				///< 登録データ初期化要求(端末→制御盤)
#define	CMD_LOG_INI			112				///< 履歴データ初期化要求(端末→制御盤)
#define	CMD_CND_INI			120				///< 画像撮影条件初期化要求(端末→制御盤)
#define	CMD_CND_DAT			121				///< 画像撮影条件初期化データ送信(制御盤→端末)
#define	CMD_CND_TST			122				///< 画像撮影条件確認開始(端末→制御盤)
#define	CMD_CND_SND			123				///< 画像撮影条件通知(制御盤→端末)
#define	CMD_PIC_SND			124				///< 撮影画像送信(端末→制御盤)
#define	CMD_CND_SET			125				///< 画像撮影条件更新(制御盤→端末)

// 通常モード
#define	CMD_MD_NML_T		200				///< モード切替通知(端末→制御盤)
#define	CMD_MD_NML_C		201				///< モード切替通知(制御盤→端末)
#define	CMD_RGST_REQ		202				///< 登録データの参照要求(端末→制御盤)
#define	CMD_RGST_SND		203				///< 登録データの送信(制御盤→端末)
#define	CMD_RGST_SET		204				///< 登録データの登録(端末→制御盤)
#define	CMD_JDG_SND			205				///< 判定結果通知(制御盤→端末)
#define	CMD_CMR_RTRY		206				///< 指データの再撮影要求(制御盤→端末)
#define	CMD_CMR_SET			207				///< カメラ・パラメータの変更要求(制御盤→端末)
#define	CMD_LGT_SET			208				///< LED光量変更要求(制御盤→端末)
#define	CMD_RGST_DEL		209				///< 登録データの削除要求(端末→制御盤)
#define	CMD_FIN_SND			210				///< 指データの送信(端末→制御盤)
#define	CMD_LCD_LOCK		230				///< 開錠・閉錠通知(制御盤→端末)
#define	CMD_LCD_DOOR		231				///< ドア開閉通知(制御盤→端末)
#define	CMD_BUZ_REQ			232				///< ブザーON/OFF要求(制御盤→端末)
#define	CMD_TST_REQ			250				///< 自己診断開始要求
#define	CMD_TST_SND			251				///< 自己診断画像(ヒストグラム)送信(端末→制御盤)
#define	CMD_TST_RSLT		252				///< 自己診断結果送信(制御盤→端末)
#define	CMD_ERR_SND			260				///< エラー情報報告(端末→制御盤)
#define	CMD_ERR_REQ			261				///< エラー表示要求(制御盤→端末)
#define	CMD_ERR_CLR			262				///< エラー消去要求(制御盤→端末)
#define	CMD_ENO_SET			270				///< 緊急番号登録・変更(端末→制御盤)
#define	CMD_ENO_REQ			271				///< 緊急番号参照要求(端末→制御盤)
#define	CMD_ENO_SND			272				///< 緊急番号データ送信(制御盤→端末)
#define	CMD_RND_REQ			273				///< 緊急開錠用乱数要求(端末→制御盤)
#define	CMD_RND_SND			274				///< 緊急開錠用乱数送信(制御盤→端末)
#define	CMD_OPN_SND			275				///< 開錠番号入力受付(端末→制御盤)
#define	CMD_ENO_CHK			276				///< 緊急番号入力受付(端末→制御盤)
#define	CMD_EMG_RSLT		277				///< 緊急開錠合否判定通知(制御盤→端末)
#define	CMD_EMP_REQ			280				///< 退去状態開始要求(端末→制御盤)
#define	CMD_EMPID_SND		281				///< 退去状態開錠の保守ID入力通知(端末→制御盤)
#define	CMD_LCD_EMP			282				///< 退去状態開錠通知(制御盤→端末)
// 停電モード
#define	CMD_MD_BAT_C		301				///< モード切替通知(制御盤→端末)
#define	CMD_POW_OFF_T		302				///< 電源OFF要求(端末→制御盤)
#define	CMD_POW_OFF_C		303				///< 電源OFF要求(制御盤→端末)
// 非常時開錠モード
#define	CMD_MD_PNC_C		401				///< モード切替通知(制御盤→端末)
#define	CMD_LCD_PNC			402				///< 非常時開錠通知(制御盤→端末)


/// 固定データ
// コマンド一般
#define	CMD_GEN_DAT			"  "			///< スペース
#define	CMD_GEN_OK			"OK"			///< OK
#define	CMD_GEN_NG			"NG"			///< NG

// 応答コマンド
#define	CMD_RES_OK			"OK"			///< OK
#define	CMD_RES_NG			"NG"			///< NG
// 指データの判定結果
#define	CMD_JDG_SND_OK		"OK"			///< OK
#define	CMD_JDG_SND_NG		"NG"			///< NG
// 開錠・閉錠通知
#define	CMD_LCD_LOCK_ON		"ON"			///< ON
#define	CMD_LCD_LOCK_OFF	"OFF"			///< OFF
// ドア開閉通知
#define	CMD_LCD_DOOR_OPEN	"OPEN"			///< OPEN
#define	CMD_LCD_DOOR_CLOSE	"CLOSE"			///< CLOSE
// ブザーON・OFF要求
#define	CMD_BUZ_REQ_ON		"ON"			///< ON
#define	CMD_BUZ_REQ_OFF		"OFF"			///< OFF
// 自己診断結果送信
#define	CMD_TST_RSLT_OK		"OK"			///< OK
#define	CMD_TST_RSLT_NG		"NG"			///< NG
// 緊急開錠合否判定通知
#define	CMD_EMG_RSLT_OK		"OK"			///< OK
#define	CMD_EMG_RSLT_NG		"NG"			///< NG
// 退去状態開錠通知
#define	CMD_LCD_EMP_OPEN	"OPEN"			///< OPEN
#define	CMD_LCD_EMP_CLOSE	"CLOSE"			///< CLOSE

#endif										// end of _COMMAND_H_
