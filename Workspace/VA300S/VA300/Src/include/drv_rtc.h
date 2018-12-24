/**
*	VA-300テストプログラム
*
*	@file drv_rtc.h
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2010/01/14
*	@brief  RTC定義情報(他案件から流用)
*
*	Copyright (C) 2010, OYO Electric Corporation
*/
#ifndef	_DRV_RTC_H_
#define	_DRV_RTC_H_

// 定義
										//=== 秒アラームレジスタ ===
#define	RSECAR_ENB		0x80			///< イネーブル

										//=== 分アラームレジスタ ===
#define	RMINAR_ENB		0x80			///< イネーブル

										//=== 時アラームレジスタ ===
#define	RHRAR_ENB		0x80			///< イネーブル

										//=== 曜日アラームレジスタ ===
#define	RWKAR_ENB		0x80			///< イネーブル

										//=== 日アラームレジスタ ===
#define	RDAYAR_ENB		0x80			///< イネーブル

										//=== 月アラームレジスタ ===
#define	RMONAR_ENB		0x80			///< イネーブル

										//=== RTCコントロールレジスタ1 ===
#define	RCR1_CF			0x80			///< 桁上げフラグ
#define	RCR1_CIE		0x10			///< 桁上げ割り込みイネーブルフラグ
#define	RCR1_AIE		0x08			///< アラーム割り込みイネーブルフラグ
#define	RCR1_AF			0x01			///< アラームフラグ

										//=== RTCコントロールレジスタ2 ===
#define	RCR2_PEF		0x80			///< 周期割り込みフラグ
#define	RCR2_PES		0x70			///< 周期割り込みイネーブルフラグ
#define	RCR2_RTCEN		0x08			///< 発振器有効
#define	RCR2_ADJ		0x04			///< 30秒調整
#define	RCR2_RESET		0x02			///< リセット
#define	RCR2_START		0x01			///< STARTビット
										
										//=== RTC曜日カウンタレジスタ ===
#define	RWKCNT_SUN		0x00			///< 日曜日
#define	RWKCNT_MON		0x01			///< 月曜日
#define	RWKCNT_TUE		0x02			///< 火曜日
#define	RWKCNT_WED		0x03			///< 水曜日
#define	RWKCNT_THU		0x04			///< 木曜日
#define	RWKCNT_FRI		0x05			///< 金曜日
#define	RWKCNT_SAT		0x06			///< 土曜日

/// 閏年チェック
#define leapYear(y) (((y) % 4) == 0 && (((y) % 100) != 0 || ((y) % 400) == 0))

// 構造体定義
typedef union {
	struct {
		unsigned char	enb: 1;			///< アラームのときの比較許可ビット
		unsigned char	high:3;			///< 上位桁
		unsigned char	low	:4;			///< 下位桁
	}bcd;

	unsigned char	byte;				///< バイト型アクセス用
} BCD_B;

typedef union {
	struct {
		unsigned char	h_h:4;			///< 最上位桁
		unsigned char	h_l:4;			///< 上位桁
		unsigned char	l_h:4;			///< 下位桁
		unsigned char	l_l:4;			///< 最下位桁
	} bcd;

	struct
	{
		unsigned char	high;			///< バイト型アクセス用上位
		unsigned char	low;			///< バイト型アクセス用下位
	}byte;

	unsigned short	word;				///< ワード型アクセス用
}BCD_W;

// RTC構造体
typedef struct {
	BCD_W	Year;						///< 年
	BCD_B	Month;						///< 月
	BCD_B	Day;						///< 日
	BCD_B	Week;						///< 曜日
	BCD_B	Hour;						///< 時
	BCD_B	Minute;						///< 分
	BCD_B	Sec;						///< 秒
}ST_RTC;

// プロトタイプ宣言
#if defined(_DRV_RTC_C_)
void rtcInit(void);							// RTC初期化
ER rtcWrite(ST_RTC*);						// RTCレジスタへの設定
ER rtcRead(ST_RTC*);						// RTCレジスタから読出し
void Sec2Rtc(unsigned long, ST_RTC*);		// 秒からRTC用の値に変換
void Rtc2Sec(ST_RTC*, unsigned long*);		// RTC用の値から秒に変換
void RtcStructClear(ST_RTC*);				// RTC用構造体の初期化
void rtcAlarmClear(void);					// RTCアラームクリア
BOOL rtcParm2Rtc(UH, UB, UB, UB, UB, UB, UB, ST_RTC*);	// パラメータをRTC型に変換する
BOOL rtcReadDate(UH*, UB*, UB*);			// RTCから年月日を取得
BOOL rtcReadTime(UB*, UB*, UB*);			// RTCから時間を取得
#else
extern void rtcInit(void);
extern ER rtcWrite(ST_RTC*);				// RTCレジスタへの設定
extern ER rtcRead(ST_RTC*);					// RTCレジスタから読出し
extern void Sec2Rtc(unsigned long, ST_RTC*); // 秒からRTC用の値に変換
extern void Rtc2Sec(ST_RTC*, unsigned long*);// RTC用の値から秒に変換
extern void RtcStructClear(ST_RTC*);		// RTC用構造体の初期化
extern void rtcAlarmClear(void);			// RTCアラームクリア
extern BOOL rtcParm2Rtc(UH, UB, UB, UB, UB, UB, UB, ST_RTC*);	// パラメータをRTC型に変換する
extern BOOL rtcReadDate(UH*, UB*, UB*);		// RTCから年月日を取得
extern BOOL rtcReadTime(UB*, UB*, UB*);		// RTCから時間を取得
#endif
#endif										/* end of _DRV_RTC_H_				*/
