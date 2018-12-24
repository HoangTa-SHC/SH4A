/**
*	VA-300プログラム
*
*	@file drv_rtc.c
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2010/01/13
*	@brief  RTCモジュール
*
*	Copyright (C) 2010, OYO Electric Corporation
*/
#define	_DRV_RTC_C_
#include "kernel.h"
#include "sh7750.h"
#include "drv_rtc.h"
#include "id.h"

								   //    1   2   3   4   5   6   7   8   9  10  11  12
static const unsigned char DayMax[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

static const ST_RTC RtcParamClear = {
	2, 0, 1, 0,						// Year
	0, 0, 1,						// Month
	0, 0 ,1,						// Day
	RWKCNT_FRI,						// Week
	0, 0, 0,						// Hour
	0, 0, 0,						// Minute
	0, 0, 0							// Secount
};

// プロトタイプ宣言
static BOOL rtcAlarmWrite(ST_RTC* pstRtc);				// アラームレジスタへの設定
static BOOL rtcCheck(ST_RTC* pstRtc, BOOL bAlarm);		// RTC設定値のチェック
static unsigned char Bcd2Byte(unsigned char bcdcode);	// BCD→BYTE変換
static unsigned short Bcd2Word(unsigned short bcdcode);	// BCD→WORD変換
static unsigned char Byte2Bcd(unsigned char code);		// BYTE→BCD変換
static unsigned short Word2Bcd(unsigned short code);	// WORD→BCD変換
#if 0
static void int_alarm(void);							// アラーム割込み処理
#endif
/*==========================================================================*/
/**
 * RTC初期化
 */
/*==========================================================================*/
void rtcInit(void)
{
	ST_RTC	rtc, rtcAlarm;
	int	i;
	
	// 仮設定
	rtc = RtcParamClear;
	
	rtcAlarm = RtcParamClear;
	rtcAlarm.Year.word   = 0;
	rtcAlarm.Week.byte   = RWKCNT_SUN;
	
	// RTC
	sfr_clr(CPG_STBCR, 0x02);			// RTC動作させる
	
	// RTC制御
	sfr_out(RTC_RCR1, 0x00);
	sfr_out(RTC_RCR2, RCR2_RTCEN);
	
	// アラーム初期化
	rtcAlarmClear();
	
	// RTC初期化
	if (rtcWrite(&rtc) != E_OK) {
		
	}
	
	sfr_set(RTC_RCR2, RCR2_START);		// RTC動作開始
}

/*==========================================================================*/
/**
 * アラームをクリア
 */
/*==========================================================================*/
void rtcAlarmClear(void)
{
	ST_RTC	rtcAlarm;
	
	rtcAlarm.Month.byte  = 1;
	rtcAlarm.Day.byte    = 1;
	rtcAlarm.Week.byte   = RWKCNT_SUN;
	rtcAlarm.Hour.byte   = 0;
	rtcAlarm.Minute.byte = 0;
	rtcAlarm.Sec.byte    = 0;
	
	rtcAlarmWrite( &rtcAlarm);				// 初期化した値を設定
	sfr_clr(RTC_RCR1, (RCR1_AF|RCR1_AIE));	// アラームクリア
}

/*==========================================================================*/
/**
 * アラームを設定
 *
 * @param bEnable TRUE...アラーム有効
 */
/*==========================================================================*/
void rtcAlarmEnable(BOOL bEnable)
{
	UINT psw;

	psw = vdis_psw();
	if (bEnable) {						// アラーム有効
		sfr_set(RTC_RMONAR, RMONAR_ENB);
		sfr_set(RTC_RDAYAR, RDAYAR_ENB);
		sfr_clr(RTC_RWKAR,  RWKAR_ENB);	// 曜日は無効にしておく
		sfr_set(RTC_RHRAR,  RHRAR_ENB);
		sfr_set(RTC_RMINAR, RMINAR_ENB);
		sfr_set(RTC_RSECAR, RSECAR_ENB);
	} else {							// アラーム無効
		sfr_clr(RTC_RMONAR, RMONAR_ENB);
		sfr_clr(RTC_RDAYAR, RDAYAR_ENB);
		sfr_clr(RTC_RWKAR,  RWKAR_ENB);
		sfr_clr(RTC_RHRAR,  RHRAR_ENB);
		sfr_clr(RTC_RMINAR, RMINAR_ENB);
		sfr_clr(RTC_RSECAR, RSECAR_ENB);
	}
	vset_psw(psw);
}

/*==========================================================================*/
/* 時間設定																	*/
/*==========================================================================*/
ER rtcWrite(ST_RTC* pstRtc)
{
	ER	ercd;
	
	if (rtcCheck(pstRtc, FALSE) == FALSE) {
		return E_PAR;
	}
	// 設定
	ercd = twai_sem( SEM_RTC, (10/MSEC));			// RTCロック
	if (ercd != E_OK) {
		return ercd;
	}
	do {
		sfr_clr( RTC_RCR1, RCR1_CF);

		sfr_outw( RTC_RYRCNT, pstRtc->Year.word);	// 年設定
		sfr_out( RTC_RMONCNT, pstRtc->Month.byte);	// 月設定
		sfr_out( RTC_RDAYCNT, pstRtc->Day.byte);	// 日設定
		sfr_out( RTC_RWKCNT,  pstRtc->Week.byte);	// 曜日設定
		sfr_out( RTC_RHRCNT,  pstRtc->Hour.byte);	// 時設定
		sfr_out( RTC_RMINCNT, pstRtc->Minute.byte);	// 分設定
		sfr_out( RTC_RSECCNT, pstRtc->Sec.byte);	// 秒設定
	}while(sfr_in( RTC_RCR1) & RCR1_CF);

	ercd = sig_sem( SEM_RTC);						// RTC解放
	if ( ercd != E_OK) {							// 
		return E_SYS;								// システムエラー
	}
	
	return E_OK;
}

/*==========================================================================*/
/* 時間読出し																*/
/*==========================================================================*/
ER rtcRead(ST_RTC* pstRtc)
{
	ER ercd;
	
	// 設定
	ercd = twai_sem( SEM_RTC, (10/MSEC));			// RTCロック
	if (ercd != E_OK) {
		return ercd;
	}
	
	do {
		sfr_clr( RTC_RCR1, RCR1_CF);

		pstRtc->Year.word   = sfr_inw( RTC_RYRCNT);	// 年設定
		pstRtc->Month.byte  = sfr_in( RTC_RMONCNT);	// 月設定
		pstRtc->Day.byte    = sfr_in( RTC_RDAYCNT);	// 日設定
		pstRtc->Week.byte   = sfr_in( RTC_RWKCNT);	// 曜日設定
		pstRtc->Hour.byte   = sfr_in( RTC_RHRCNT);	// 時設定
		pstRtc->Minute.byte = sfr_in( RTC_RMINCNT);	// 分設定
		pstRtc->Sec.byte    = sfr_in( RTC_RSECCNT);	// 秒設定
	}while(sfr_in( RTC_RCR1) & RCR1_CF);
		
	if (sig_sem( SEM_RTC) != E_OK) {				// RTC解放
		return E_SYS;								// システムエラー
	}
	return E_OK;
}

/*==========================================================================*/
/* アラーム時間設定															*/
/*==========================================================================*/
static BOOL rtcAlarmWrite(ST_RTC* pstRtc)
{
	UINT	psw;
	unsigned char	aie_bkup;
	
	if (rtcCheck(pstRtc, TRUE) == FALSE) {
		return FALSE;
	}
	// 設定
	psw = vdis_psw();
	aie_bkup = sfr_in(RTC_RCR1) & ~RCR1_AIE;
	sfr_out( RTC_RMONAR, pstRtc->Month.byte);		// 月設定
	sfr_out( RTC_RDAYAR, pstRtc->Day.byte);			// 日設定
	sfr_out( RTC_RWKAR,  pstRtc->Week.byte);		// 曜日設定
	sfr_out( RTC_RHRAR,  pstRtc->Hour.byte);		// 時設定
	sfr_out( RTC_RMINAR, pstRtc->Minute.byte);		// 分設定
	sfr_out( RTC_RSECAR, pstRtc->Sec.byte);			// 秒設定
	sfr_set(RTC_RCR1, aie_bkup);
	vset_psw(psw);
	
	return TRUE;
}

/*==========================================================================*/
/* 時間設定値の確認															*/
/*==========================================================================*/
static BOOL rtcCheck(ST_RTC* pstRtc, BOOL bAlarm)
{
	unsigned short	wBinYear;
	unsigned char	byBinMonth, byBinDay, byDayInMonth;

	/* 設定値の異常値チェック */
	// 年
	if (bAlarm != TRUE) {				// アラームでないときだけチェックする
		if ((pstRtc->Year.bcd.h_h > 0x09) || (pstRtc->Year.bcd.h_l > 0x09)
			|| (pstRtc->Year.bcd.l_h > 0x09) || (pstRtc->Year.bcd.l_l > 0x09)) {
			return FALSE;
		}
	}
	// 月
	byBinMonth = Bcd2Byte(pstRtc->Month.byte);
	if ((pstRtc->Month.bcd.high > 0x01) || (pstRtc->Month.bcd.low > 0x09) 
		|| (byBinMonth > 12) || (byBinMonth < 1)) {
		return FALSE;
	}
	// 日
	byBinMonth--;
	byDayInMonth = DayMax[ byBinMonth ];
	if (byBinMonth == 1) {				// 閏年処理
		if (bAlarm == TRUE) {			// アラームのときは年指定がないので無条件で29日
			byDayInMonth++;
		} else {
			wBinYear = Bcd2Word(pstRtc->Year.word);
			if (leapYear(wBinYear)) {	// うるう年
				byDayInMonth++;
			}
		}
	}
	
	byBinDay = Bcd2Byte(pstRtc->Day.byte);
	if ((pstRtc->Day.bcd.high > 0x03) || (pstRtc->Day.bcd.low > 0x09) 
		|| (byBinDay > byDayInMonth) || (byBinDay < 1)) {
		return FALSE;
	}
	
	// 曜日
	if ((pstRtc->Week.bcd.low > 0x06) || pstRtc->Week.bcd.high){
		return FALSE;
	}
	// 時
	if ((pstRtc->Hour.bcd.high > 0x02) || (pstRtc->Hour.bcd.low > 0x09) 
		|| (pstRtc->Hour.byte > 0x23)) {
		return FALSE;
	}
	// 分
	if ((pstRtc->Minute.bcd.high > 0x05) || (pstRtc->Minute.bcd.low > 0x09) 
		|| (pstRtc->Minute.byte > 0x59)) {
		return FALSE;
	}
	// 秒
	if ((pstRtc->Sec.bcd.high > 0x05) || (pstRtc->Sec.bcd.low > 0x09) 
		|| (pstRtc->Sec.byte > 0x59)) {
		return FALSE;
	}
	return TRUE;
}

//-----------------------------------------------------------------------------
// BCD コードの変換(BCD->BYTE)
//-----------------------------------------------------------------------------
static unsigned char Bcd2Byte(unsigned char bcdcode)
{
	BCD_B	temp;

	temp.byte = bcdcode;

	return temp.bcd.high * 10 + temp.bcd.low;
}

//-----------------------------------------------------------------------------
// BCD コードの変換(BCD->WORD)
//-----------------------------------------------------------------------------
static unsigned short Bcd2Word(unsigned short bcdcode)
{
	BCD_W	temp;

	temp.word = bcdcode;

	return	temp.bcd.h_h * 1000 +
			temp.bcd.h_l *  100 +
			temp.bcd.l_h *   10 +
			temp.bcd.l_l;
}

//-----------------------------------------------------------------------------
// BCD コードへの変換(BYTE->BCD)
//-----------------------------------------------------------------------------
static unsigned char Byte2Bcd(unsigned char code)
{
	BCD_B	temp;

	temp.bcd.high = code / 10;
	temp.bcd.low  = code % 10;

	return temp.byte;
}

//-----------------------------------------------------------------------------
// BCD コードへの変換(WORD->BCD)
//-----------------------------------------------------------------------------
static unsigned short Word2Bcd(unsigned short code)
{
	BCD_W	temp;

	temp.bcd.l_l = code % 10;	code /= 10;
	temp.bcd.l_h = code % 10;	code /= 10;
	temp.bcd.h_l = code % 10;	code /= 10;
	temp.bcd.h_h = code % 10;
	
	return	temp.word;
}

/*==========================================================================*/
/**
 * 秒からRTC用の値に変換
 *
 * @param ulSec 秒
 * @param rtc カレンダ日付
 */
/*==========================================================================*/
void Sec2Rtc(unsigned long ulSec, ST_RTC* pstRtc)
{
	unsigned short wYear;
	unsigned char byMonth;
	
	pstRtc->Sec.byte    = Byte2Bcd( ulSec % 60);	ulSec /= 60;
	pstRtc->Minute.byte = Byte2Bcd( ulSec % 60);	ulSec /= 60;
	pstRtc->Hour.byte   = Byte2Bcd( ulSec % 24);	ulSec /= 24;
	pstRtc->Week.byte   = Byte2Bcd(( ulSec + RWKCNT_FRI) % 7);
	
	wYear  = 2010;						// 基準年は2010年にしておく
	byMonth = 0;
	
	for(;;) {
		unsigned char daysInMonth;

		daysInMonth = DayMax[ byMonth ];

		// うるう年処理
		if (byMonth == 1 && leapYear (wYear)) {
			++daysInMonth;
		}

		if (ulSec < daysInMonth) {
			break;
		}

		ulSec -= daysInMonth;
		if (++byMonth == 12) {
			byMonth = 0;
			++wYear;
		}
	}

	pstRtc->Day.byte   = Byte2Bcd( ulSec + 1);
	pstRtc->Month.byte = Byte2Bcd( byMonth + 1);
	pstRtc->Year.word  = Word2Bcd( wYear);
}

/*==========================================================================*/
/**
 * RTC用の値から秒に変換
 *
 * @param pstRtc カレンダ日付
 * @param pulSec 秒
 */
/*==========================================================================*/
void Rtc2Sec(ST_RTC* pstRtc, unsigned long* pulSec)
{
	unsigned short wYear, wBinYear;
	unsigned long ulDay;
	unsigned long ulSec;
	unsigned char byBinMonth, byMonth;
	
	*pulSec = 0;
	
	wBinYear = Bcd2Word( pstRtc->Year.word);	
	if ( wBinYear <= 2100) {								// 2100年まで
		ulSec  = Bcd2Byte(pstRtc->Sec.byte);
		ulSec += ((unsigned long)Bcd2Byte(pstRtc->Minute.byte) * 60);
		ulSec += ((unsigned long)Bcd2Byte(pstRtc->Hour.byte) * 3600);
	
		ulDay = 0;
		for (wYear = 2010;wYear < wBinYear; wYear++) {		// 基準年は2010年にしておく
			if (leapYear(wYear)) {
				ulDay += 366;
			} else {
				ulDay += 365;
			}
		}
	
		byBinMonth = Bcd2Byte(pstRtc->Month.byte);
		byMonth = 0;
	
		for(byMonth = 0;byMonth < byBinMonth - 1;byMonth++) {
			unsigned char daysInMonth;

			daysInMonth = DayMax[ byMonth ];

			// うるう年処理
			if (byMonth == 1 && leapYear (wBinYear)) {
				++daysInMonth;
			}
			ulDay += daysInMonth;
		}

		ulDay += (Bcd2Byte(pstRtc->Day.byte) - 1);
		ulSec += ulDay * (3600 * 24);
		*pulSec = ulSec;
	}
}

/*==========================================================================*/
/**
 * RTC用構造体クリア
 *
 * @param pstRtc カレンダ日付
 */
/*==========================================================================*/
void RtcStructClear(ST_RTC* pstRtc)
{
	*pstRtc = RtcParamClear;
}
