/**
*	VA-300プログラム
*
*	@file sh7750R.h
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/08/31
*	@brief  SH7750R用レジスタ定義情報
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
#ifndef	SH7750R_H
#define	SH7750R_H

/* 制御レジスタ入出力マクロ */

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

/* 制御レジスタのオフセットアドレス */

#define INTC_INTPRI00   0xe080000    /* 割り込み優先レベル設定レジスタ00 */
#define INTC_INTREQ00   0xe080020    /* 割り込み要因レジスタ00 */
#define INTC_INTMSK00   0xe080040    /* 割り込みマスクレジスタ00 */
#define INTC_INTMSKCLR00 0xe080060   /* 割り込みマスククリアレジスタ */

#define TMU_TCOR3       0xe100008    /* タイマコンスタントレジスタ3 */
#define TMU_TCNT3       0xe10000c    /* タイマカウンタ3 */
#define TMU_TCR3        0xe100010    /* タイマコントロールレジスタ3 */
#define TMU_TCOR4       0xe100014    /* タイマコンスタントレジスタ4 */
#define TMU_TCNT4       0xe100018    /* タイマカウンタ4 */
#define TMU_TCR4        0xe10001c    /* タイマコントロールレジスタ4 */

#endif										/* end of SH7750R_H				*/
