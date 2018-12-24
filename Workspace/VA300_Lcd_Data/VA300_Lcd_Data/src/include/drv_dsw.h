/**
*	VA-300�v���O����
*
*	@file drv_dsw.h
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2013/02/23
*	@brief  DSW��`���
*
*	Copyright (C) 2010, OYO Electric Corporation
*/
#ifndef	_DRV_DSW_H_
#define	_DRV_DSW_H_

// ��`(DSW2��FPGA�̒�`�t�@�C���Œ�`)
#define	DSW1_MON		(0x01)				///< ���j�^SW
#define	DSW1_2			(0x02)				///< DSW2
#define	DSW1_3			(0x04)				///< DSW3
#define	DSW1_4			(0x08)				///< DSW4
#define	DSW1_5			(0x10)				///< DSW5
#define	DSW1_6			(0x20)				///< DSW6
#define	DSW1_7			(0x40)				///< DSW7
#define	DSW1_8			(0x80)				///< DSW8
#define	DSW1_ALL		(0xFF)				///< CPU�ɐڑ�����Ă���SDSW

// �v���g�^�C�v�錾
#if defined(_DRV_DSW_C_)
UH DswGet(void);							// DSW��Ԏ擾
#else
extern UH DswGet(void);
#endif
#endif										/* end of _DRV_DSW_H_				*/
