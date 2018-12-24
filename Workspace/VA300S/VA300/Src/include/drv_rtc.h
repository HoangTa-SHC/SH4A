/**
*	VA-300�e�X�g�v���O����
*
*	@file drv_rtc.h
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2010/01/14
*	@brief  RTC��`���(���Č����痬�p)
*
*	Copyright (C) 2010, OYO Electric Corporation
*/
#ifndef	_DRV_RTC_H_
#define	_DRV_RTC_H_

// ��`
										//=== �b�A���[�����W�X�^ ===
#define	RSECAR_ENB		0x80			///< �C�l�[�u��

										//=== ���A���[�����W�X�^ ===
#define	RMINAR_ENB		0x80			///< �C�l�[�u��

										//=== ���A���[�����W�X�^ ===
#define	RHRAR_ENB		0x80			///< �C�l�[�u��

										//=== �j���A���[�����W�X�^ ===
#define	RWKAR_ENB		0x80			///< �C�l�[�u��

										//=== ���A���[�����W�X�^ ===
#define	RDAYAR_ENB		0x80			///< �C�l�[�u��

										//=== ���A���[�����W�X�^ ===
#define	RMONAR_ENB		0x80			///< �C�l�[�u��

										//=== RTC�R���g���[�����W�X�^1 ===
#define	RCR1_CF			0x80			///< ���グ�t���O
#define	RCR1_CIE		0x10			///< ���グ���荞�݃C�l�[�u���t���O
#define	RCR1_AIE		0x08			///< �A���[�����荞�݃C�l�[�u���t���O
#define	RCR1_AF			0x01			///< �A���[���t���O

										//=== RTC�R���g���[�����W�X�^2 ===
#define	RCR2_PEF		0x80			///< �������荞�݃t���O
#define	RCR2_PES		0x70			///< �������荞�݃C�l�[�u���t���O
#define	RCR2_RTCEN		0x08			///< ���U��L��
#define	RCR2_ADJ		0x04			///< 30�b����
#define	RCR2_RESET		0x02			///< ���Z�b�g
#define	RCR2_START		0x01			///< START�r�b�g
										
										//=== RTC�j���J�E���^���W�X�^ ===
#define	RWKCNT_SUN		0x00			///< ���j��
#define	RWKCNT_MON		0x01			///< ���j��
#define	RWKCNT_TUE		0x02			///< �Ηj��
#define	RWKCNT_WED		0x03			///< ���j��
#define	RWKCNT_THU		0x04			///< �ؗj��
#define	RWKCNT_FRI		0x05			///< ���j��
#define	RWKCNT_SAT		0x06			///< �y�j��

/// �[�N�`�F�b�N
#define leapYear(y) (((y) % 4) == 0 && (((y) % 100) != 0 || ((y) % 400) == 0))

// �\���̒�`
typedef union {
	struct {
		unsigned char	enb: 1;			///< �A���[���̂Ƃ��̔�r���r�b�g
		unsigned char	high:3;			///< ��ʌ�
		unsigned char	low	:4;			///< ���ʌ�
	}bcd;

	unsigned char	byte;				///< �o�C�g�^�A�N�Z�X�p
} BCD_B;

typedef union {
	struct {
		unsigned char	h_h:4;			///< �ŏ�ʌ�
		unsigned char	h_l:4;			///< ��ʌ�
		unsigned char	l_h:4;			///< ���ʌ�
		unsigned char	l_l:4;			///< �ŉ��ʌ�
	} bcd;

	struct
	{
		unsigned char	high;			///< �o�C�g�^�A�N�Z�X�p���
		unsigned char	low;			///< �o�C�g�^�A�N�Z�X�p����
	}byte;

	unsigned short	word;				///< ���[�h�^�A�N�Z�X�p
}BCD_W;

// RTC�\����
typedef struct {
	BCD_W	Year;						///< �N
	BCD_B	Month;						///< ��
	BCD_B	Day;						///< ��
	BCD_B	Week;						///< �j��
	BCD_B	Hour;						///< ��
	BCD_B	Minute;						///< ��
	BCD_B	Sec;						///< �b
}ST_RTC;

// �v���g�^�C�v�錾
#if defined(_DRV_RTC_C_)
void rtcInit(void);							// RTC������
ER rtcWrite(ST_RTC*);						// RTC���W�X�^�ւ̐ݒ�
ER rtcRead(ST_RTC*);						// RTC���W�X�^����Ǐo��
void Sec2Rtc(unsigned long, ST_RTC*);		// �b����RTC�p�̒l�ɕϊ�
void Rtc2Sec(ST_RTC*, unsigned long*);		// RTC�p�̒l����b�ɕϊ�
void RtcStructClear(ST_RTC*);				// RTC�p�\���̂̏�����
void rtcAlarmClear(void);					// RTC�A���[���N���A
BOOL rtcParm2Rtc(UH, UB, UB, UB, UB, UB, UB, ST_RTC*);	// �p�����[�^��RTC�^�ɕϊ�����
BOOL rtcReadDate(UH*, UB*, UB*);			// RTC����N�������擾
BOOL rtcReadTime(UB*, UB*, UB*);			// RTC���玞�Ԃ��擾
#else
extern void rtcInit(void);
extern ER rtcWrite(ST_RTC*);				// RTC���W�X�^�ւ̐ݒ�
extern ER rtcRead(ST_RTC*);					// RTC���W�X�^����Ǐo��
extern void Sec2Rtc(unsigned long, ST_RTC*); // �b����RTC�p�̒l�ɕϊ�
extern void Rtc2Sec(ST_RTC*, unsigned long*);// RTC�p�̒l����b�ɕϊ�
extern void RtcStructClear(ST_RTC*);		// RTC�p�\���̂̏�����
extern void rtcAlarmClear(void);			// RTC�A���[���N���A
extern BOOL rtcParm2Rtc(UH, UB, UB, UB, UB, UB, UB, ST_RTC*);	// �p�����[�^��RTC�^�ɕϊ�����
extern BOOL rtcReadDate(UH*, UB*, UB*);		// RTC����N�������擾
extern BOOL rtcReadTime(UB*, UB*, UB*);		// RTC���玞�Ԃ��擾
#endif
#endif										/* end of _DRV_RTC_H_				*/
