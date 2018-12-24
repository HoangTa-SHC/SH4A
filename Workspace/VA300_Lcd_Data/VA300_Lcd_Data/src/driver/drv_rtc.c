/**
*	VA-300�v���O����
*
*	@file drv_rtc.c
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2010/01/13
*	@brief  RTC���W���[��
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

// �v���g�^�C�v�錾
static BOOL rtcAlarmWrite(ST_RTC* pstRtc);				// �A���[�����W�X�^�ւ̐ݒ�
static BOOL rtcCheck(ST_RTC* pstRtc, BOOL bAlarm);		// RTC�ݒ�l�̃`�F�b�N
static unsigned char Bcd2Byte(unsigned char bcdcode);	// BCD��BYTE�ϊ�
static unsigned short Bcd2Word(unsigned short bcdcode);	// BCD��WORD�ϊ�
static unsigned char Byte2Bcd(unsigned char code);		// BYTE��BCD�ϊ�
static unsigned short Word2Bcd(unsigned short code);	// WORD��BCD�ϊ�
#if 0
static void int_alarm(void);							// �A���[�������ݏ���
#endif
/*==========================================================================*/
/**
 * RTC������
 */
/*==========================================================================*/
void rtcInit(void)
{
	ST_RTC	rtc, rtcAlarm;
	int	i;
	
	// ���ݒ�
	rtc = RtcParamClear;
	
	rtcAlarm = RtcParamClear;
	rtcAlarm.Year.word   = 0;
	rtcAlarm.Week.byte   = RWKCNT_SUN;
	
	// RTC
	sfr_clr(CPG_STBCR, 0x02);			// RTC���삳����
	
	// RTC����
	sfr_out(RTC_RCR1, 0x00);
	sfr_out(RTC_RCR2, RCR2_RTCEN);
	
	// �A���[��������
	rtcAlarmClear();
	
	// RTC������
	if (rtcWrite(&rtc) != E_OK) {
		
	}
	
	sfr_set(RTC_RCR2, RCR2_START);		// RTC����J�n
}

/*==========================================================================*/
/**
 * �A���[�����N���A
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
	
	rtcAlarmWrite( &rtcAlarm);				// �����������l��ݒ�
	sfr_clr(RTC_RCR1, (RCR1_AF|RCR1_AIE));	// �A���[���N���A
}

/*==========================================================================*/
/**
 * �A���[����ݒ�
 *
 * @param bEnable TRUE...�A���[���L��
 */
/*==========================================================================*/
void rtcAlarmEnable(BOOL bEnable)
{
	UINT psw;

	psw = vdis_psw();
	if (bEnable) {						// �A���[���L��
		sfr_set(RTC_RMONAR, RMONAR_ENB);
		sfr_set(RTC_RDAYAR, RDAYAR_ENB);
		sfr_clr(RTC_RWKAR,  RWKAR_ENB);	// �j���͖����ɂ��Ă���
		sfr_set(RTC_RHRAR,  RHRAR_ENB);
		sfr_set(RTC_RMINAR, RMINAR_ENB);
		sfr_set(RTC_RSECAR, RSECAR_ENB);
	} else {							// �A���[������
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
/* ���Ԑݒ�																	*/
/*==========================================================================*/
ER rtcWrite(ST_RTC* pstRtc)
{
	ER	ercd;
	
	if (rtcCheck(pstRtc, FALSE) == FALSE) {
		return E_PAR;
	}
	// �ݒ�
	ercd = twai_sem( SEM_RTC, (10/MSEC));			// RTC���b�N
	if (ercd != E_OK) {
		return ercd;
	}
	do {
		sfr_clr( RTC_RCR1, RCR1_CF);

		sfr_outw( RTC_RYRCNT, pstRtc->Year.word);	// �N�ݒ�
		sfr_out( RTC_RMONCNT, pstRtc->Month.byte);	// ���ݒ�
		sfr_out( RTC_RDAYCNT, pstRtc->Day.byte);	// ���ݒ�
		sfr_out( RTC_RWKCNT,  pstRtc->Week.byte);	// �j���ݒ�
		sfr_out( RTC_RHRCNT,  pstRtc->Hour.byte);	// ���ݒ�
		sfr_out( RTC_RMINCNT, pstRtc->Minute.byte);	// ���ݒ�
		sfr_out( RTC_RSECCNT, pstRtc->Sec.byte);	// �b�ݒ�
	}while(sfr_in( RTC_RCR1) & RCR1_CF);

	ercd = sig_sem( SEM_RTC);						// RTC���
	if ( ercd != E_OK) {							// 
		return E_SYS;								// �V�X�e���G���[
	}
	
	return E_OK;
}

/*==========================================================================*/
/* ���ԓǏo��																*/
/*==========================================================================*/
ER rtcRead(ST_RTC* pstRtc)
{
	ER ercd;
	
	// �ݒ�
	ercd = twai_sem( SEM_RTC, (10/MSEC));			// RTC���b�N
	if (ercd != E_OK) {
		return ercd;
	}
	
	do {
		sfr_clr( RTC_RCR1, RCR1_CF);

		pstRtc->Year.word   = sfr_inw( RTC_RYRCNT);	// �N�ݒ�
		pstRtc->Month.byte  = sfr_in( RTC_RMONCNT);	// ���ݒ�
		pstRtc->Day.byte    = sfr_in( RTC_RDAYCNT);	// ���ݒ�
		pstRtc->Week.byte   = sfr_in( RTC_RWKCNT);	// �j���ݒ�
		pstRtc->Hour.byte   = sfr_in( RTC_RHRCNT);	// ���ݒ�
		pstRtc->Minute.byte = sfr_in( RTC_RMINCNT);	// ���ݒ�
		pstRtc->Sec.byte    = sfr_in( RTC_RSECCNT);	// �b�ݒ�
	}while(sfr_in( RTC_RCR1) & RCR1_CF);
		
	if (sig_sem( SEM_RTC) != E_OK) {				// RTC���
		return E_SYS;								// �V�X�e���G���[
	}
	return E_OK;
}

/*==========================================================================*/
/* �A���[�����Ԑݒ�															*/
/*==========================================================================*/
static BOOL rtcAlarmWrite(ST_RTC* pstRtc)
{
	UINT	psw;
	unsigned char	aie_bkup;
	
	if (rtcCheck(pstRtc, TRUE) == FALSE) {
		return FALSE;
	}
	// �ݒ�
	psw = vdis_psw();
	aie_bkup = sfr_in(RTC_RCR1) & ~RCR1_AIE;
	sfr_out( RTC_RMONAR, pstRtc->Month.byte);		// ���ݒ�
	sfr_out( RTC_RDAYAR, pstRtc->Day.byte);			// ���ݒ�
	sfr_out( RTC_RWKAR,  pstRtc->Week.byte);		// �j���ݒ�
	sfr_out( RTC_RHRAR,  pstRtc->Hour.byte);		// ���ݒ�
	sfr_out( RTC_RMINAR, pstRtc->Minute.byte);		// ���ݒ�
	sfr_out( RTC_RSECAR, pstRtc->Sec.byte);			// �b�ݒ�
	sfr_set(RTC_RCR1, aie_bkup);
	vset_psw(psw);
	
	return TRUE;
}

/*==========================================================================*/
/* ���Ԑݒ�l�̊m�F															*/
/*==========================================================================*/
static BOOL rtcCheck(ST_RTC* pstRtc, BOOL bAlarm)
{
	unsigned short	wBinYear;
	unsigned char	byBinMonth, byBinDay, byDayInMonth;

	/* �ݒ�l�ُ̈�l�`�F�b�N */
	// �N
	if (bAlarm != TRUE) {				// �A���[���łȂ��Ƃ������`�F�b�N����
		if ((pstRtc->Year.bcd.h_h > 0x09) || (pstRtc->Year.bcd.h_l > 0x09)
			|| (pstRtc->Year.bcd.l_h > 0x09) || (pstRtc->Year.bcd.l_l > 0x09)) {
			return FALSE;
		}
	}
	// ��
	byBinMonth = Bcd2Byte(pstRtc->Month.byte);
	if ((pstRtc->Month.bcd.high > 0x01) || (pstRtc->Month.bcd.low > 0x09) 
		|| (byBinMonth > 12) || (byBinMonth < 1)) {
		return FALSE;
	}
	// ��
	byBinMonth--;
	byDayInMonth = DayMax[ byBinMonth ];
	if (byBinMonth == 1) {				// �[�N����
		if (bAlarm == TRUE) {			// �A���[���̂Ƃ��͔N�w�肪�Ȃ��̂Ŗ�������29��
			byDayInMonth++;
		} else {
			wBinYear = Bcd2Word(pstRtc->Year.word);
			if (leapYear(wBinYear)) {	// ���邤�N
				byDayInMonth++;
			}
		}
	}
	
	byBinDay = Bcd2Byte(pstRtc->Day.byte);
	if ((pstRtc->Day.bcd.high > 0x03) || (pstRtc->Day.bcd.low > 0x09) 
		|| (byBinDay > byDayInMonth) || (byBinDay < 1)) {
		return FALSE;
	}
	
	// �j��
	if ((pstRtc->Week.bcd.low > 0x06) || pstRtc->Week.bcd.high){
		return FALSE;
	}
	// ��
	if ((pstRtc->Hour.bcd.high > 0x02) || (pstRtc->Hour.bcd.low > 0x09) 
		|| (pstRtc->Hour.byte > 0x23)) {
		return FALSE;
	}
	// ��
	if ((pstRtc->Minute.bcd.high > 0x05) || (pstRtc->Minute.bcd.low > 0x09) 
		|| (pstRtc->Minute.byte > 0x59)) {
		return FALSE;
	}
	// �b
	if ((pstRtc->Sec.bcd.high > 0x05) || (pstRtc->Sec.bcd.low > 0x09) 
		|| (pstRtc->Sec.byte > 0x59)) {
		return FALSE;
	}
	return TRUE;
}

//-----------------------------------------------------------------------------
// BCD �R�[�h�̕ϊ�(BCD->BYTE)
//-----------------------------------------------------------------------------
static unsigned char Bcd2Byte(unsigned char bcdcode)
{
	BCD_B	temp;

	temp.byte = bcdcode;

	return temp.bcd.high * 10 + temp.bcd.low;
}

//-----------------------------------------------------------------------------
// BCD �R�[�h�̕ϊ�(BCD->WORD)
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
// BCD �R�[�h�ւ̕ϊ�(BYTE->BCD)
//-----------------------------------------------------------------------------
static unsigned char Byte2Bcd(unsigned char code)
{
	BCD_B	temp;

	temp.bcd.high = code / 10;
	temp.bcd.low  = code % 10;

	return temp.byte;
}

//-----------------------------------------------------------------------------
// BCD �R�[�h�ւ̕ϊ�(WORD->BCD)
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
 * �b����RTC�p�̒l�ɕϊ�
 *
 * @param ulSec �b
 * @param rtc �J�����_���t
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
	
	wYear  = 2010;						// ��N��2010�N�ɂ��Ă���
	byMonth = 0;
	
	for(;;) {
		unsigned char daysInMonth;

		daysInMonth = DayMax[ byMonth ];

		// ���邤�N����
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
 * RTC�p�̒l����b�ɕϊ�
 *
 * @param pstRtc �J�����_���t
 * @param pulSec �b
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
	if ( wBinYear <= 2100) {								// 2100�N�܂�
		ulSec  = Bcd2Byte(pstRtc->Sec.byte);
		ulSec += ((unsigned long)Bcd2Byte(pstRtc->Minute.byte) * 60);
		ulSec += ((unsigned long)Bcd2Byte(pstRtc->Hour.byte) * 3600);
	
		ulDay = 0;
		for (wYear = 2010;wYear < wBinYear; wYear++) {		// ��N��2010�N�ɂ��Ă���
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

			// ���邤�N����
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
 * RTC�p�\���̃N���A
 *
 * @param pstRtc �J�����_���t
 */
/*==========================================================================*/
void RtcStructClear(ST_RTC* pstRtc)
{
	*pstRtc = RtcParamClear;
}
