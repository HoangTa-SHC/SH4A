/**
*	LCD�h���C�o
*
*	@file drv_lcd.c
*	@version 1.00
*	@version 1.01 LCD�̂�����ɑΉ�
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2013/03/09
*	@brief  LCD�h���C�o
*
*	Copyright (C) 2007, OYO Electric Corporation
*/
#define	_DRV_LCD_C_
#include <kernel.h>
#include "drv_fpga.h"
#include "drv_lcd.h"

#define	LCD_BASE		0xA8000000	///LCD�R���g���[�����W�X�^�̃x�[�X�A�h���X
#define LCD_REG_ADD     0x00000000  ///LCD���W�X�^�̃A�h���X
#define LCD_REG_DATA    0x00000002  ///LCD���W�X�^�̃f�[�^

#define	lcdPreBufSet()	(*(volatile UH *)(LCD_BASE+LCD_REG_ADD) = 0x66)	///< �A�h���X���W�X�^�Ƀo�b�t�@��ݒ�

// �v���g�^�C�v�錾
static void lcdcRegSet(int iAddr, UH uhData);	///< LCD�R���g���[�����W�X�^������
static UH lcdcRegGet(int iAddr);				///< LCD�R���g���[�����W�X�^�ǂݏo��
static void lcdcBufSet(UH uhData);				///< LCD�R���g���[���f�[�^���W�X�^������
static ER lcdcInit(void);						///< LCD������(�{��)

// �ϐ��錾
const T_CSEM csem_lcd   = { TA_TFIFO, 1, 1, (B *)"sem_lcd" };
static ID s_idSem;								///< �Z�}�t�HID
extern const unsigned short imgWord[];

/*==========================================================================*/
/*
 * LCD�R���g���[�����W�X�^��������
 * @param  iAddr  ���W�X�^�A�h���X
 * @param  uhData ���W�X�^�f�[�^
*/
/*==========================================================================*/
static void lcdcRegSet(int iAddr, UH uhData)
{
	volatile UH *puhReg;

	puhReg  = (volatile UH*)(LCD_BASE+LCD_REG_ADD);
	*puhReg = (UH)iAddr;
	puhReg++;
	
	*puhReg = uhData;
}
/*==========================================================================*/
/*
 * LCD�R���g���[�����W�X�^�ǂݍ���
 * @param  iAddr ���W�X�^�A�h���X
 * @return ���W�X�^�f�[�^
*/
/*==========================================================================*/
static UH lcdcRegGet(int iAddr)
{
	volatile UH *puhReg;
	volatile UH uhRet;

	puhReg  = (volatile UH*)(LCD_BASE+LCD_REG_ADD);
	*puhReg = (UH)iAddr;
	puhReg++;
	
	uhRet = *puhReg;
	
	return uhRet; 
}
/*==========================================================================*/
/*
 * LCD�R���g���[�����W�X�^�}�X�N�t����������
 * @param  add ���W�X�^�A�h���X
 * @param  mask �t���O�p�����[�^(1�̕������������ݑΏ�)
 * @param  data ���W�X�^�f�[�^
*/
/*==========================================================================*/
static void lcdcRegSetMask(int add,UH mask,UH data)
{
	UH dat;
			
	dat  = lcdcRegGet(add);
	dat &= (~mask);
	dat|=data;
	lcdcRegSet(add,dat);	
	
}

/*==========================================================================*/
/*
 * LCD�R���g���[���o�b�t�@�֏�������
 * @param  uhData �摜�f�[�^
*/
/*==========================================================================*/
static void lcdcBufSet(UH uhData)
{
	volatile UH *puhData;

	puhData = (volatile UH *)(LCD_BASE+LCD_REG_DATA);
	*puhData = uhData;
}

/*==========================================================================*/
/*
 * �@�@LCD�e�X�g�i����`�F�b�N�p�v���O�����j
 *
*/
/*==========================================================================*/
void LcdTest(void)
{
	int lop;	
	volatile UH *p;
	
	lcdcRegSet(0x04,0x1a);
	lcdcRegSet(0x06,0x01);	
	lcdcRegSet(0x08,0x01);
	lcdcRegSet(0x0c,0x23);		
	lcdcRegSet(0x0e,0x3f);
	lcdcRegSet(0x12,0x00);	
	lcdcRegSet(0x04,0x9a);	
	
	//50ms �҂�
	dly_tsk(50/MSEC);

	lcdcRegSet(0x12,0x00);	

	// ���̓f�[�^�^�C�v��24bppmode2�ɐݒ�
	lcdcRegSet(0x14, 0x02);

	//LCD�^�C�~���O�ݒ� 480 * 272
	lcdcRegSet(0x16, 59);
	lcdcRegSet(0x18, 109);
	lcdcRegSet(0x1A, 0x0E);
	lcdcRegSet(0x1C, 0x01);
	lcdcRegSet(0x1E, 6);
	lcdcRegSet(0x20, 40);
	lcdcRegSet(0x22, 177);
	lcdcRegSet(0x24, 9);
	lcdcRegSet(0x26, 2);
	lcdcRegSet(0x28, 0x00);
	
	//  �g�p����SDRAM �̎�ނ�ݒ肷��B128mbit
	lcdcRegSet(0x82, 0x03);
	
	// SDRAM ���t���b�V���J�E���^
	lcdcRegSet(0x8C, 0x00);
//	lcdcRegSet(0x8C, 0xFF);
	lcdcRegSet(0x8E, 0x01);
//	lcdcRegSet(0x8E, 0x03);
	
	// SDRAM ���C�g�o�b�t�@�������T�C�Y 384KB�ɐݒ�  SDRAM_BUF_SIZE=0x60000
	// 480*272*3(byte)��16KB�P�ʂɐ؂�グ(0x60000)
	lcdcRegSet(0x90, SDRAM_BUF_SIZE >> 14);	
			
	
	// �\�t�g�E�F�A���Z�b�g
	lcdcRegSet(0x68, 0xE8);
	
	// ���Z�b�g����
	lcdcRegSet(0x68, 0x00);
	
	// SDCLK���C�l�[�u��
	lcdcRegSet(0x68, 0x01);

	
	// SDRAM�̃C�j�V�����C�Y
	lcdcRegSet(0x84, 0x82);
	
	for(;;){//0x86�A�h���X��bit1=1�҂�
		if( (lcdcRegGet(0x86)&0x02)!=0)break;
	}

	// PWM�̔䗦
	lcdcRegSet(0x72, 0xFF);	
	lcdcRegSet(0x7A, 0xFF);	

	// PWM�̃C�j�V�����C�Y
//	lcdcRegSet(0x70, 0x05);	
	lcdcRegSet(0x70, 0x86);	

//-----------------------------
	
	//�\���I�t �V���O���o�b�t�@�I��
	lcdcRegSet(0x2a, 0x00);

	//�A�b�v�f�[�g�@�s�v�H�H
	lcdcRegSet(0x50, 0x80);
	lcdcRegSet(0x50, 0x00);	
	
	//�������݃o�b�t�@�P�I��
	lcdcRegSet(0x52, 0x00);

	//���ߐFR�̒l
	lcdcRegSet(0x54, 0x00);
	
	//���ߐFR�̒l
	lcdcRegSet(0x56, 0x00);	
	
	//���ߐFB�̒l
	lcdcRegSet(0x58, 0x00);		
		
	//X�J�n�ʒu//1
	lcdcRegSet(0x5a, 0x00);
	
	//Y�J�n�ʒu//1
	lcdcRegSet(0x5c, 0x00);			
	lcdcRegSet(0x5e, 0x00);

	//X�I���ʒu//480
	lcdcRegSet(0x60, 118);

	//Y�I���ʒu//272
//	lcdcRegSet(0x62, 0x43);			
//	lcdcRegSet(0x64, 0x03);   	
	lcdcRegSet(0x62, 0x0F);			
	lcdcRegSet(0x64, 0x01);   	
	
//-----------------------------	
	p = imgWord;
	lcdcRegSet(0x66, *p);		// Debug
	p++;
	for(lop=1;lop<=480*272*2;lop++, p++){//���f�[�^��������(16bit bus)
		lcdcBufSet( *p );		// Debug
//		lcdcBufSet( 0x00FF );	// Debug
	}
//-----------------------------	

	//�o�b�t�@�P�\��ON	
	lcdcRegSet(0x2a, 0x01);
	
	//�A�b�v�f�[�g�@�s�v�H�H		
	lcdcRegSetMask(0x50,0x80,0x80);	
	lcdcRegSetMask(0x50,0x80,0x00);	
	
	//IO����i�\���j
	lcdcRegSet(0x6e, 0x08);		
		
}
/*==========================================================================*/
/*
 *	LCD��������
 *	@param idSem LCD�p�Z�}�t�HID
 *	@return E_OK ����
*/
/*==========================================================================*/
ER LcdcInit(ID idSem)
{
	ER ercd;
	
	s_idSem = 0;
	ercd = cre_sem(idSem, &csem_lcd);	// Create semaphore
	if (ercd == E_OK) {
		s_idSem = idSem;
		ercd = lcdcInit();
	}
	return ercd;
}

/*==========================================================================*/
/*
 *	LCD��������(�{��)
 *	@param idSem LCD�p�Z�}�t�HID
 *	@return E_OK ����
*/
/*==========================================================================*/
static ER lcdcInit(void)
{
	volatile UH dat;
	int	iTmout;
	ER	ercd;

	lcdcRegSet(0x04,0x1a);
	lcdcRegSet(0x06,0x01);	
	lcdcRegSet(0x08,0x01);
	lcdcRegSet(0x0c,0x23);		
	lcdcRegSet(0x0e,0x3f);
	lcdcRegSet(0x12,0x00);	
	lcdcRegSet(0x04,0x9a);	
	
	//20ms Wait
	dly_tsk(20/MSEC);
	
	lcdcRegSet(0x12,0x00);	
	
	//  �g�p����LCD �p�l���̃^�C�~���O�A�ɐ���ݒ肷��B
	// (HDISP+HNDP)(dot)*(VDISP+VNDP)(dot)*60(frame/sec)=11992800��12MHz=SYSCLK(PCLK)

	// ���̓f�[�^�^�C�v��24bppmode2�ɐݒ�
	lcdcRegSet(0x14, 0x02);

	// HDISP��480�ɐݒ�
	lcdcRegSet(0x16, (SCREEN_WIDTH / 8)- 1);

	// HNDP��220�ɐݒ�
	lcdcRegSet(0x18, 109);

	// VDISP��272�ɐݒ�
	lcdcRegSet(0x1A, (SCREEN_HEIGHT & 0xFF)- 1);	// 15(+1)
	lcdcRegSet(0x1C, SCREEN_HEIGHT / 256);			// +256

	// VNDP��14�ɐݒ�
	lcdcRegSet(0x1E, 6);

	// HSW��41�ɐݒ�
	lcdcRegSet(0x20, 40);

	// HPS��177�ɐݒ�
	lcdcRegSet(0x22, 177);


	// VSW��10�ɐݒ�
	lcdcRegSet(0x24, 9);

	// VPS��2�ɐݒ�
	lcdcRegSet(0x26, 2);


	// PCLK�ɐ��𗧂�������ɐݒ�
	lcdcRegSet(0x28, 0x00);
	
	
	//  �g�p����SDRAM �̎�ނ�ݒ肷��B
	// SDRAM 128Mbit�ɐݒ�
	lcdcRegSet(0x82, 0x03);
	
	// SDRAM ���t���b�V���J�E���^��14.2us(=(1/36MHz) x 512)�ɐݒ�
	lcdcRegSet(0x8C, 0x00);
	lcdcRegSet(0x8E, 0x02);
	
	// SDRAM ���t���b�V���J�E���^��1 . us(=(1/27MHz) x 420)�ɐݒ�
	lcdcRegSet(0x8C, 0xA0);
	lcdcRegSet(0x8E, 0x01);
	
	// SDRAM ���C�g�o�b�t�@�������T�C�Y 384KB�ɐݒ�
	// 480*272*3(byte)��16KB�P�ʂɐ؂�グ(0x60000)
	lcdcRegSet(0x90, SDRAM_BUF_SIZE >> 14);	
			
	
	// �\�t�g�E�F�A���Z�b�g
	lcdcRegSet(0x68, 0xE8);
	
	// ���Z�b�g����
	lcdcRegSet(0x68, 0x00);
	
	// SDCLK���C�l�[�u��
	lcdcRegSet(0x68, 0x01);
	
	// SDRAM�̃C�j�V�����C�Y
	lcdcRegSet(0x84, 0x82);
	
	
	// �����������҂�
	ercd = E_TMOUT;
	for(iTmout = 0;iTmout < 50;iTmout++){
		if ((lcdcRegGet(0x86) & 0x02) == 0) {
			dly_tsk((10/MSEC));
		} else {
			ercd = E_OK;
			break;
		}
	}

	// PWM�̃C�j�V�����C�Y
	/// MODIFY_BEGIN LCD�_�� '13/5/1
	lcdcRegSet(0x72, 0x6B);
	lcdcRegSet(0x74, 0x00);
	lcdcRegSet(0x76, 0x00);
	lcdcRegSet(0x78, 0x00);

	lcdcRegSet(0x7A, 0x0B);
	lcdcRegSet(0x7C, 0x00);
	lcdcRegSet(0x7E, 0x00);
	lcdcRegSet(0x80, 0x00);
	
//	lcdcRegSet(0x70, 0x05);	
	lcdcRegSet(0x70, 0x86);	
	/// MODIFY_END LCD�_��
	
	/// INSERT_BEGIN LCD�\�����Ȃ� '13/5/8
	lcdcRegSetMask(0x2a, 0x0f, 0x0B);
	lcdcRegSetMask(0x50, 0x80, 0x80);
	lcdcRegSetMask(0x50, 0x80, 0x00);
	lcdcRegSet(0x6e, 0x08);	
	/// INSERT_END LCD�\�����Ȃ� '13/5/8

	return ercd;
}
/*========================================================================*/
/*
 * �f�B�X�v���C�E�B���h�E�̓��͉摜�����̐ݒ�
 *
 * @param colorEnb ���ߐF�C�l�[�u��(0:�f�B�Z�[�u���A1:�C�l�[�u��)
 * @param keyR     ���ߐFR(0�`255)
 * @param keyG     ���ߐFG(0�`255)
 * @param keyB     ���ߐFB(0�`255)
 * @param Xstart   X�J�n�ʒu(0,8,16,���472)
 * @param Ystart   Y�J�n�ʒu(0�`271)
 * @param Xend     X�I���ʒu(7,15,23���479)
 * @param Yend     Y�I���ʒu(0�`271)
 * @retval E_OK ����
*/
/*========================================================================*/
ER LcdcDisplayWindowSet( UB bufNum,		// ��ʃo�b�t�@(0�`15)
					 UB colorEnb,	// ���ߐF�C�l�[�u��(0:�f�B�Z�[�u���A1:�C�l�[�u��)
					 UB keyR,		// ���ߐFR(0�`255)
					 UB keyG,		// ���ߐFG(0�`255)
					 UB keyB, 		// ���ߐFB(0�`255)
					 UH Xstart,		// X�J�n�ʒu(0,8,16,���472)
					 UH Ystart,		// Y�J�n�ʒu(0�`271)
					 UH Xend,		// X�I���ʒu(7,15,23���479)
					 UH Yend)		// Y�I���ʒu(0�`271)
{

	ER ercd;

	ercd = twai_sem(s_idSem, (100/MSEC));	// �Z�}�t�H�̎擾	
	if (ercd == E_OK) {
	 	/// DELETE_BEGIN LCD����� '13/5/7
//		lcdcRegSet(0x2a, 0x0f);
//		lcdcRegSetMask(0x50, 0x80, 0x80);	
	 	/// DELETE_BEGIN LCD����� '13/5/7
		lcdcRegSetMask(0x50, 0x80, 0x00);			
		
		lcdcRegSetMask(0x52, 0xf0, bufNum << 4);	// Input Mode Register�A�h���X

		// ���ߐF�ݒ�
		if (colorEnb == 0) {
			lcdcRegSetMask(0x52, 0x08, 0x00);	// Input Mode Register�A�h���X
		} else {
			lcdcRegSetMask(0x52, 0x08, 0x08);	// Input Mode Register�A�h���X
		}
		//���ߐFR�̒l
		lcdcRegSet(0x54, keyR);
	
		//���ߐFR�̒l
		lcdcRegSet(0x56, keyG);	
	
		//���ߐFB�̒l
		lcdcRegSet(0x58, keyB);		
		
		//X�J�n�ʒu
		lcdcRegSet(0x5a, (Xstart >> 2) & 0xfe);
	
		//Y�J�n�ʒu
		lcdcRegSet(0x5c, (Ystart >> 2) & 0xff);			
		lcdcRegSet(0x5e, Ystart        & 3   );

		//X�I���ʒu
		lcdcRegSet(0x60, ((Xend +1 - 8) >> 2) & 0xfe);

		//Y�I���ʒu
		lcdcRegSet(0x62, ((Xend +1 - 8) >> 2) & 0xfe);
		lcdcRegSet(0x64, (Yend) & 3               );
		
		// �A�h���X���W�X�^�Ƀo�b�t�@��ݒ�
//		lcdPreBufSet();
	}

	return ercd;
 	
}
/*========================================================================*/
/*	�f�B�X�v���C�E�B���h�E��16�r�b�g�摜�f�[�^����������
 *	���f�[�^�̘A�������݂Ȃ̂ŃZ�}�t�H�͂����Ȃ�
 * @param  val �摜�f�[�^
*/ 
/*========================================================================*/
ER LcdImageWrite(UH val)
{
	lcdcBufSet( val );

	return E_OK;
}
/*========================================================================*/
/*   �f�B�X�v���C�E�B���h�E�̂��߂̐ݒ������
 *   @retval E_OK ����
*/ 
/*========================================================================*/
ER LcdcDisplayWindowFinish(void)
{
	ER ercd;	
	
	ercd = pol_sem( s_idSem );				// �Z�}�t�H�擾�ς݃`�F�b�N	
	if (ercd == E_TMOUT) {
		/// DELETE_BEGIN LCD����� '13/5/7
//		lcdcRegSet(0x2a, 0x00);		
//		lcdcRegSet(0x2a, 0x01);	
//		lcdcRegSetMask(0x50, 0x80, 0x80);	
//		lcdcRegSetMask(0x50, 0x80, 0x00);	
	
//		lcdcRegSet(0x6e, 0x08);		
		/// DELETE_END LCD����� '13/5/7

		ercd = sig_sem(s_idSem);			// �Z�}�t�H�ԋp
	} else if (ercd == E_OK) {
		ercd = E_OBJ;
	}

	return ercd;
}
/*==========================================================================*/
/*
 * �A���t�@�u�����f�B���O�̂��߂̕\��������ݒ�
 * @param mode ���͉摜�̑I��(0:���͉摜�̂݁A1:���͉摜1�Ɠ��͉摜2)
 * @param alpha �A���t�@�l(0�`32)
 * @param width �����摜�T�C�Y(8,16,24...480)
 * @param height �����摜�T�C�Y(1�`280)
 * @param src1bufnum ���͉摜1�̉�ʃo�b�t�@No(0�`15)
 * @param src1Xstart ���͉摜1��X�J�n�ʒu(0,8,16,24...472)
 * @param src1Ystart ���͉摜1��Y�J�n�ʒu(0�`271)
 * @param src2bufnum ���͉摜2�̉�ʃo�b�t�@No(0�`15)
 * @param src2Xstart ���͉摜2��X�J�n�ʒu(0,8,16,24...472)
 * @param src2Ystart ���͉摜2��Y�J�n�ʒu(0�`271)							
 * @param dstbufnum �o�͉摜�̉�ʃo�b�t�@No(0�`15)
 * @param dstXstart �o�͉摜��X�J�n�ʒu(0,8,16,24...472)
 * @param dstYstart �o�͉摜��Y�J�n�ʒu(0�`271)
 * @return  E_OK ����
 */
/*==========================================================================*/
ER LcdcAlphablendSet(UB mode,       //���͉摜�̑I��(0:���͉摜�̂݁A1:���͉摜1�Ɠ��͉摜2)
						UB alpha,      //�A���t�@�l(0�`32)
						UH width,      //�����摜�T�C�Y(8,16,24...480)
						UH height,     //�����摜�T�C�Y(1�`280)
						UB src1bufnum, //���͉摜1�̉�ʃo�b�t�@No(0�`15)
						UH src1Xstart, //���͉摜1��X�J�n�ʒu(0,8,16,24...472)
						UH src1Ystart, //���͉摜1��Y�J�n�ʒu(0�`271)
						UB src2bufnum, //���͉摜2�̉�ʃo�b�t�@No(0�`15)
						UH src2Xstart, //���͉摜2��X�J�n�ʒu(0,8,16,24...472)
						UH src2Ystart, //���͉摜2��Y�J�n�ʒu(0�`271)							
						UB dstbufnum,  //�o�͉摜�̉�ʃo�b�t�@No(0�`15)
						UH dstXstart,  //�o�͉摜��X�J�n�ʒu(0,8,16,24...472)
						UH dstYstart) //�o�͉摜��Y�J�n�ʒu(0�`271)
{

	UB	setting;

	UW	src1adr, src2adr, dstadr;
	ER ercd;
	
	ercd = twai_sem(s_idSem, (100/MSEC));	// �Z�}�t�H�̎擾
	
	if (ercd == E_OK) {
		//�A���t�@�u�����f�B���O �����摜�T�C�Y
		lcdcRegSet(0x98, (width >> 3) - 1);	// �l��ݒ�

		//�A���t�@�u�����f�B���O �����摜�T�C�Y
		lcdcRegSet(0x9a, (height - 1) & 0xff);	// �l��ݒ�			
		lcdcRegSet(0x9c, (height - 1) >> 8);	// �l��ݒ�	


		//�A���t�@�u�����f�B���O�ݒ�l
		if (mode == 0) {
			setting = 0xc0;	// ���͉摜1�̂�
		} else {
			setting = 0x80;	// ���͉摜1+���͉摜2
		}
		setting |= (alpha & 0x3f);

		//�A���t�@�u�����f�B���O�ݒ背�W�X�^
		lcdcRegSet(0x9e, setting);	// �l��ݒ�			


		// src1,src2,dst�̃X�^�[�g�A�h���X���v�Z	
		// SDRAM�A�h���X = �o�b�t�@n�̃A�h���X + (YS * X�t���T�C�Y) * 3�o�C�g + XS * 3�o�C�g
		src1adr = src1bufnum * SDRAM_BUF_SIZE + (src1Ystart * SCREEN_WIDTH + src1Xstart)* 3;
		src2adr = src2bufnum * SDRAM_BUF_SIZE + (src2Ystart * SCREEN_WIDTH + src2Xstart)* 3;
		dstadr  = dstbufnum  * SDRAM_BUF_SIZE + (dstYstart  * SCREEN_WIDTH + dstXstart )* 3;
			
		//�A���t�@�u�����f�B���O ���͉摜�P �������J�n�A�h���X
		lcdcRegSet(0xa0, src1adr & 0xf8);	// �l��ݒ�			
		src1adr >>= 8;
		lcdcRegSet(0xa2, src1adr & 0xff);	// �l��ݒ�			
		src1adr >>= 8;
		lcdcRegSet(0xa4, src1adr & 0xff);	// �l��ݒ�			

		//�A���t�@�u�����f�B���O ���͉摜�Q �������J�n�A�h���X
		lcdcRegSet(0xa6, src2adr & 0xf8);	// �l��ݒ�	
		src2adr >>= 8;
		lcdcRegSet(0xa8, src2adr & 0xff);	// �l��ݒ�		
		src2adr >>= 8;
		lcdcRegSet(0xaa, src2adr & 0xff);	// �l��ݒ�			

		//�A���t�@�u�����f�B���O �o�͉摜 �������J�n�A�h���X
		lcdcRegSet(0xac, dstadr & 0xf8);	// �l��ݒ�			
		dstadr >>= 8;
		lcdcRegSet(0xae, dstadr & 0xff);	// �l��ݒ�			
		dstadr >>= 8;
		lcdcRegSet(0xb0, dstadr & 0xff);	// �l��ݒ�		

		//Interrupt Control Register�̊����݂��C�l�[�u���ɂ���
		lcdcRegSet(0xB2, 0x01);

		// Interrupt Status Register�̃t���O���N���A����
		lcdcRegSet(0xB6, 0x01);
		lcdcRegSet(0xB6, 0x0);
	
		// �A���t�@�u�����f�B���O�X�^�[�g
		lcdcRegSet(0x94, 1);
		lcdcRegSet(0x94, 0);

		sig_sem(s_idSem);						// �Z�}�t�H�ԋp
	}
	return ercd;

}
/*========================================================================*/
/* �A���t�@�u�����f�B���O�R�s�[����������s�Ȃ�
 * @param  *result   0:�R�s�[����   1:�R�s�[��
 * @return  E_OK ����
*/
/*========================================================================*/
// �A���t�@�u�����f�B���O�@�\�ɂ��f�[�^�R�s�[������������A�t���O��߂�
//	Interrupt Status Register�̃t���O���`�F�b�N
ER LcdcAlphablendEnd( UB *result )
{
	ER ercd;
	UH uhVal;
	
	ercd = E_OK;
	
	*result = 1;
	
	//	Interrupt Status Register�̃t���O���`�F�b�N
	uhVal = lcdcRegGet(0xB4);
	if (uhVal & 0x01) {
		*result = 0;
		ercd = twai_sem(s_idSem, (100/MSEC));	// �Z�}�t�H�̎擾
		if (ercd == E_OK) {
			// �A���t�@�u�����f�B���O�@�\�ɂ��f�[�^�R�s�[������
			// Interrupt Status Register�̃t���O���N���A����
			lcdcRegSet(0xB6, 0x01);
			lcdcRegSet(0xB6, 0x0);
			
			sig_sem(s_idSem);		// �Z�}�t�H�ԋp
		}
	}

	return ercd;
}
/*========================================================================*/
/*  Picture In Picture�\���̂��߂̏�����ݒ�
 *	@return  E_OK ����
*/
/*========================================================================*/
ER LcdcPictureInPictureSet(UB pipnum,				// 0�FPIP1�A1�FPIP2
						UH width_pip, 		        //�����摜�T�C�Y(8,16,24...480)
						UH height_pip,		        //�����摜�T�C�Y(1�`272)
						UB srcbufnum_pip,		    // ���͉摜�̉�ʃo�b�t�@No�i0�`15)
						UH srcXstart_pip,		    //���͉摜��X�J�n�ʒu(0,8,16,24...472)
						UH srcYstart_pip,		    //���͉摜��Y�J�n�ʒu(0�`271)
						UH dstXstart_pip,		    //���C����� X�J�n�ʒu(0,8,16,24...472)
						UH dstYstart_pip)		    //���C����� Y�J�n�ʒu(0�`271)
{
	UW	srcadr;
	UB	regadr;
	ER	ercd;

	ercd = twai_sem(s_idSem, (100/MSEC));	// �Z�}�t�H�̎擾
	
	if (ercd == E_OK) {
		if (pipnum == 0) {
			/* REG[2Ch] PIP1 Display Memory Start Address Register 0�̃A�h���X��ݒ� */
			regadr = 0x2C;	//
		} else if (pipnum == 1) {
			/* REG[3Eh] PIP2 Display Memory Start Address Register 0�̃A�h���X��ݒ� */
			regadr = 0x3E;	//
		} else {
			return ercd;
		}

		//PIP1�܂���PIP2 �������J�n�A�h���X���W�X�^�̃A�h���X
		// src�̃X�^�[�g�A�h���X���v�Z	
		// SDRAM�A�h���X = �o�b�t�@n�̃A�h���X + (YS * X�t���T�C�Y) * 3�o�C�g + XS * 3�o�C�g
		srcadr = srcbufnum_pip * SDRAM_BUF_SIZE + (srcYstart_pip * SCREEN_WIDTH + srcXstart_pip)* 3;
			
		//�������J�n�A�h���X
		lcdcRegSet(regadr, srcadr & 0xf8);	// �l��ݒ�
		regadr += 2;
		srcadr >>= 8;
		lcdcRegSet(regadr, srcadr & 0xff);	// �l��ݒ�
		regadr += 2;
		srcadr >>= 8;
		lcdcRegSet(regadr, srcadr & 0xff);	// �l��ݒ�
		regadr += 2;

		// X�J�n�ʒu
		lcdcRegSet(regadr, (dstXstart_pip >> 2) & 0xfe);	// �l��ݒ�
		regadr += 2;
		

		// Y�J�n�ʒu
		lcdcRegSet(regadr, (dstYstart_pip >> 2) & 0xff);	// �l��ݒ�
		regadr += 2;
		lcdcRegSet(regadr,  (dstYstart_pip) & 3);			// �l��ݒ�
		regadr += 2;
			
		// X�I���ʒu
		lcdcRegSet(regadr,  ((dstXstart_pip + width_pip - 1 + 1 - 8) >> 2) & 0xfe);	// �l��ݒ�
		regadr += 2;

		// Y�I���ʒu
		lcdcRegSet(regadr,  ((dstYstart_pip + height_pip - 1) >> 2) & 0xff);	// �l��ݒ�
		regadr += 2;
		lcdcRegSet(regadr,  ((dstYstart_pip + height_pip - 1)) & 3);			// �l��ݒ�
		regadr += 2;
		
		sig_sem(s_idSem);						// �Z�}�t�H�ԋp
	}
	return ercd;
}
/*========================================================================*/
/*  ���C����ʕ\���o�b�t�@��I��
 *  @param  bufNum ���C����ʕ\���o�b�t�@(0�`15)
 *  @param  pipSel PIP��ʕ\���I��(0:PIP�\���Ȃ��@1:PIP1   2:PIP2   3:PIP1&PIP2  )
 *	@return  E_OK ����
*/
/*========================================================================*/
ER LcdcDisplayModeSet( UB bufNum, UB pipSel)
{
	ER ercd;

	// ���W�X�^�ɂ��킹�����u
	if (pipSel) {
		pipSel &= 0x03;
		pipSel++;
	}

	ercd = twai_sem(s_idSem, (100/MSEC));	// �Z�}�t�H�̎擾
	
	if (ercd == E_OK) {
		lcdcRegSet(0x2A, ((bufNum & 0xf) << 4) | (pipSel << 1) | 1);	// ���̉�ʁE����PIP�o�͑I����ݒ肷��
		lcdcRegSet(0x50, 0x80);										// �\���ݒ背�W�X�^(0x2A�`0x4E)�X�V
	
		sig_sem(s_idSem);					// �Z�}�t�H�ԋp
	}
	
	return ercd;
}
/*==========================================================================*/
/*
 * LCD�p�l���̃o�b�N���C�g��_��
 * @return E_OK ����
 */
/*==========================================================================*/
ER LcdcBackLightOn(void)
{
	ER ercd;

	ercd = twai_sem(s_idSem, (100/MSEC));	// �Z�}�t�H�̎擾	
	if (ercd == E_OK) {	
		/// MODIFY_BEGIN LCD�_�� '13/5/1
//		lcdcRegSetMask(0x70, 0x07, 0x05);
		lcdcRegSetMask(0x70, 0x07, 0x06);
		/// MODIFY_END LCD�_��
		sig_sem(s_idSem);					// �Z�}�t�H�ԋp
	}
	
	return ercd;
	
}
/*==========================================================================*/
/*
 * LCD�p�l���̃o�b�N���C�g������
 * @return E_OK ����
 */
/*==========================================================================*/
ER LcdcBackLightOff(void)
{
	ER ercd;

	ercd = twai_sem(s_idSem, (100/MSEC));	// �Z�}�t�H�̎擾	
	if (ercd == E_OK) {	
		/// MODIFY_BEGIN �o�b�N���C�gOFF�ł��Ȃ��ɑΉ� '13/5/7
//		lcdcRegSetMask(0x70, 0x07, 0x00);
		lcdcRegSetMask(0x70, 0x07, 0x04);
		lcdcRegSetMask(0x70, 0x04, 0x00);
		/// MODIFY_END �o�b�N���C�gOFF�ł��Ȃ��ɑΉ� '13/5/7
		sig_sem(s_idSem);					// �Z�}�t�H�ԋp
	}		

	return ercd;
}
/*==========================================================================*/
/*
 * LCD�p�l���ɃJ���[�o�[��\��
 * @return E_OK ����
 */
/*==========================================================================*/
ER LcdcTestColorBarSet(void)
{
	ER ercd;

	ercd = twai_sem(s_idSem, (100/MSEC));	// �Z�}�t�H�̎擾	
	if (ercd == E_OK) {	
		lcdcRegSetMask(0x2a, 0x0f, 0x0f);
		lcdcRegSetMask(0x50, 0x80, 0x80);
		lcdcRegSetMask(0x50, 0x80, 0x00);
	
		lcdcRegSet(0x6e, 0x08);		
	
		sig_sem(s_idSem);					// �Z�}�t�H�ԋp
	}
	
	return ercd;
}


