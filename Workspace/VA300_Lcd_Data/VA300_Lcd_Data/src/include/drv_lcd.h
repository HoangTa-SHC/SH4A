/**
*	LCD�h���C�o
*
*	@file drv_lcd.c
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2013/03/09
*	@brief  LCD�h���C�o
*
*	Copyright (C) 2007, OYO Electric Corporation
*/

#ifndef	_DRV_LCD_H_
#define	_DRV_LCD_H_

#include <kernel.h>


// �萔��`
#define SCREEN_WIDTH	480		///< LCD��ʉ���
#define SCREEN_HEIGHT	272		///< LCD��ʏc��

#define SDRAM_BUF_SIZE	0x60000	///< SDRAM ���C�g�o�b�t�@�������T�C�Y 384K�ɐݒ�
								///< 480*272*3(byte)��16KB�P�ʂɐ؂�グ(0x60000)

// EXTERN�錾�̒�`
#ifdef EXTERN
#undef EXTERN
#endif

#if defined(_DRV_LCD_C_)
#define	EXTERN
#else
#define	EXTERN extern
#endif

// �v���g�^�C�v�錾
EXTERN void LcdTest(void);
EXTERN ER LcdcInit(ID idSem);				///< LCD������
EXTERN ER LcdcDisplayWindowSet( UB bufNum,
								UB colorEnb,
								UB keyR,
								UB keyG,
								UB keyB,
								UH Xstart,
								UH Ystart,
								UH Xend,
								UH Yend);	///< �f�B�X�v���C�E�B���h�E�̓��͉摜�����̐ݒ�
EXTERN ER LcdImageWrite(UH val);			///< �f�B�X�v���C�E�B���h�E��16�r�b�g�摜�f�[�^����������
EXTERN ER LcdcDisplayWindowFinish(void);	///< �f�B�X�v���C�E�B���h�E�̂��߂̐ݒ������
EXTERN ER LcdcAlphablendSet(UB mode, 
							UB alpha,
							UH width,
							UH height,
							UB src1bufnum,
							UH src1Xstart,
							UH src1Ystart,
							UB src2bufnum,
							UH src2Xstart,
							UH src2Ystart,						
							UB dstbufnum,
							UH dstXstart,
							UH dstYstart);		///< �A���t�@�u�����f�B���O�ݒ�
EXTERN ER LcdcAlphablendEnd( UB *result );		///< �A���t�@�u�����f�B���O�I��
EXTERN ER LcdcPictureInPictureSet(UB pipnum,
							UH width_pip, 	
							UH height_pip,	
							UB srcbufnum_pip,
							UH srcXstart_pip,
							UH srcYstart_pip,
							UH dstXstart_pip,
							UH dstYstart_pip);	///< PIP�ݒ�
EXTERN ER LcdcDisplayModeSet( UB bufNum, UB pipSel);	///< ���C����ʕ\���o�b�t�@��I��
EXTERN ER LcdcBackLightOn(void);				///< �o�b�N���C�g�_��
EXTERN ER LcdcBackLightOff(void);				///< �o�b�N���C�g����
EXTERN ER LcdcTestColorBarSet(void);			///< �J���[�o�[�e�X�g

#endif