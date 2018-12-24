/**
*	VA-300 �Z���T�[���v���O����
*
*	@file drv_exkey.h
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/09/24
*	@brief  10�L�[��`���
*
*	Copyright (C) 2010, OYO Electric Corporation
*/
#ifndef	_DRV_EXKEY_H_
#define	_DRV_EXKEY_H_

// ��`(DSW2��FPGA�̒�`�t�@�C���Œ�`)
enum KEY_CODE {							///< �L�[�R�[�h
	KEY_NONE     = 0,					///< �L�[�������Ă��Ȃ�
	KEY_ENTER    = 0x0D,				///< [�ďo]
	KEY_UNDEF    = 0x15,				///< ����`�L�[
	KEY_DEL      = 0x18,				///< [���]
	KEY_0        = '0',					///< 0
	KEY_1,								///< 1
	KEY_2,								///< 2
	KEY_3,								///< 3
	KEY_4,								///< 4
	KEY_5,								///< 5
	KEY_6,								///< 6
	KEY_7,								///< 7
	KEY_8,								///< 8
	KEY_9,								///< 9
	KEY_ASTERISK = '*',					///< [*]
};

// EXTERN�錾�̒�`
#ifdef EXTERN
#undef EXTERN
#endif

#if defined(_DRV_EXKEY_C_)
#define	EXTERN
#else
#define	EXTERN extern
#endif

// �v���g�^�C�v�錾
EXTERN ER ExKeyInit(BOOL bIntEnable, ID idTsk);			///< �O���L�[������
EXTERN enum KEY_CODE ExtKeyPol(void);					///< �L�[���̓|�[�����O
EXTERN ER ExtKeyGet(enum KEY_CODE *peKey, TMO tmout);	///< �L�[���͎擾
#endif										/* end of _DRV_EXKEY_H_				*/
