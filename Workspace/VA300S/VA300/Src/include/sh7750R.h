/**
*	VA-300�v���O����
*
*	@file sh7750R.h
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/08/31
*	@brief  SH7750R�p���W�X�^��`���
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
#ifndef	SH7750R_H
#define	SH7750R_H

/* ���䃌�W�X�^���o�̓}�N�� */

#define IO_RBASE ((volatile unsigned char *)0xf0000000)

#define sfrr_in(n)       (*(IO_RBASE+(n)))
#define sfrr_out(n,c)    (*(IO_RBASE+(n))=(c))
#define sfrr_inw(n)      (*(volatile unsigned short *)(IO_RBASE+(n)))
#define sfrr_outw(n,c)   (*(volatile unsigned short *)(IO_RBASE+(n))=(c))
#define sfrr_inl(n)      (*(volatile unsigned long *)(IO_RBASE+(n)))
#define sfrr_outl(n,c)   (*(volatile unsigned long *)(IO_RBASE+(n))=(c))
#define sfrr_set(n,c)    (*(IO_RBASE+(n))|=(c))
#define sfrr_clr(n,c)    (*(IO_RBASE+(n))&=~(c))
#define sfrr_setw(n,c)   (*(volatile unsigned short *)(IO_RBASE+(n))|=(c))
#define sfrr_clrw(n,c)   (*(volatile unsigned short *)(IO_RBASE+(n))&=~(c))
#define sfrr_clrl(n,c)   (*(volatile unsigned long *)(IO_RBASE+(n))&=~(c))
#define sfrr_setl(n,c)   (*(volatile unsigned long *)(IO_RBASE+(n))|=(c))

/* ���䃌�W�X�^�̃I�t�Z�b�g�A�h���X */

#define INTC_INTPRI00   0xe080000    /* ���荞�ݗD�惌�x���ݒ背�W�X�^00 */
#define INTC_INTREQ00   0xe080020    /* ���荞�ݗv�����W�X�^00 */
#define INTC_INTMSK00   0xe080040    /* ���荞�݃}�X�N���W�X�^00 */
#define INTC_INTMSKCLR00 0xe080060   /* ���荞�݃}�X�N�N���A���W�X�^ */

#define TMU_TCOR3       0xe100008    /* �^�C�}�R���X�^���g���W�X�^3 */
#define TMU_TCNT3       0xe10000c    /* �^�C�}�J�E���^3 */
#define TMU_TCR3        0xe100010    /* �^�C�}�R���g���[�����W�X�^3 */
#define TMU_TCOR4       0xe100014    /* �^�C�}�R���X�^���g���W�X�^4 */
#define TMU_TCNT4       0xe100018    /* �^�C�}�J�E���^4 */
#define TMU_TCR4        0xe10001c    /* �^�C�}�R���g���[�����W�X�^4 */

#endif										/* end of SH7750R_H				*/
