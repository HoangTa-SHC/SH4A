/*******************************************************************************
* LANコントローラドライバのボード依存部 (SH7780_EVA_BOD)                       *
*                                                                              *
*  Copyright (c) 2005 -2010, MiSPO Co., Ltd.                                   *
*  All rights reserved.                                                        *
*                                                                              *
* 2005-04-19 Created                                                        OK *
* 2005-09-21 マルチチャネル対応                                             OK *
* 2005-09-21 Modified for SH7706                                            OK *
* 2005-10-14 DMAC_DMAORのアクセスサイズbug-fix(32bit->16bit)                OK *
* 2006-05-22 Modified for SEM150                                            SZ *
* 2007-09-14 Modified for SH-2007                                           YM *
* 2010-03-25 Changed interrupt handling from handler to service routine     HE *
*******************************************************************************/

/*

DMA駆動モードの切り替え

          DMA_BURST  = 0: サイクルスティール転送   ..... default
                       1: バースト転送

          (例) shc <option> -def=DMA_BURST=1 nonethw.c
                            ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

*/

#include "kernel.h"
#include "nonet.h"
#include "nonets.h"
#include "nonethw.h"
#include "sh7750.h"
#include "drv_fpga.h"

/* I/O定義 */

#ifndef sfr_in
#define sfr_in(n)       (*(volatile unsigned char *)(n))
#define sfr_out(n,c)    (*(volatile unsigned char *)(n)=(c))
#define sfr_inw(n)      (*(volatile unsigned short *)(n))
#define sfr_outw(n,c)   (*(volatile unsigned short *)(n)=(c))
#define sfr_inl(n)      (*(volatile unsigned long *)(n))
#define sfr_outl(n,c)   (*(volatile unsigned long *)(n)=(c))
#define sfr_set(n,c)    (*(volatile unsigned char *)(n)|=(c))
#define sfr_clr(n,c)    (*(volatile unsigned char *)(n)&=~(c))
#define sfr_setw(n,c)   (*(volatile unsigned short *)(n)|=(c))
#define sfr_clrw(n,c)   (*(volatile unsigned short *)(n)&=~(c))
#define sfr_setl(n,c)   (*(volatile unsigned long *)(n)|=(c))
#define sfr_clrl(n,c)   (*(volatile unsigned long *)(n)&=~(c))

#endif

/* マルチチャネル対応の定義 */

#if (defined(DIF_NUM))
  #if (DIF_NUM == 0)
    #define disr_isrlan     disr_isrlan0
    #define disr_isrdma     disr_isrdma0
    #define lan_def_int     lan_def_int0
    #define lan_undef_int   lan_undef_int0
    #define lan_intr        lan_intr0
    #define lan_dma_ini     lan_dma_ini0
    #define lan_dma_sts     lan_dma_sts0
    #define lan_dma_clr     lan_dma_clr0
    #define lan_dma_sta     lan_dma_sta0
    #define lan_dma_int     lan_dma_int0
    #define lan_dma_intr    lan_dma_intr0
  #elif (DIF_NUM==1)
    #define disr_isrlan     disr_isrlan1
    #define disr_isrdma     disr_isrdma1
    #define lan_def_int     lan_def_int1
    #define lan_undef_int   lan_undef_int1
    #define lan_intr        lan_intr1
    #define lan_dma_ini     lan_dma_ini1
    #define lan_dma_sts     lan_dma_sts1
    #define lan_dma_clr     lan_dma_clr1
    #define lan_dma_sta     lan_dma_sta1
    #define lan_dma_int     lan_dma_int1
    #define lan_dma_intr    lan_dma_intr1
  #elif
    #error DIF_NUM is out of range! Customize it.
  #endif
#else
  #define DIF_NUM   0
#endif

/* コンフィグレーション */

#if (CH == 0)
#define INT_LANC        INT_IRL14    /* 割込み番号 */
#define IP              14          /* 割込み優先レベル (1～15、ただし IP ≦ KNL_LEVEL) */
#define DMA_CH          0           /* default use Channel-0 */
#define INT_REQ_LAN     (1<<28)     /* LANの割込み要因 */
#define INT_PRI_LAN     (IP<<16)    /* LANの優先度 */
#define INT_MSKCLR_LAN  (1<<28)     /* LANの割込みマスククリア */
#elif (CH == 1)
#define INT_LANC        INT_IRQ1    /* 割込み番号 */
#define IP              12          /* 割込み優先レベル (1～15、ただし IP ≦ KNL_LEVEL) */
#define DMA_CH          1           /* default use Channel-1 */
#define INT_REQ_LAN     (1<<30)     /* LANの割込み要因 */
#define INT_PRI_LAN     (IP<<24)    /* LANの優先度 */
#define INT_MSKCLR_LAN  (1<<30)     /* LANの割込みマスククリア */
#else
//#define INT_LANC        INT_IRQ3    /* 割込み番号 */
#define INT_LANC        INT_IRL14    /* 割込み番号 */											//Ver106-1
//#define IP              12          /* 割込み優先レベル (1～15、ただし IP ≦ KNL_LEVEL) */
#define IP              14          /* 割込み優先レベル (1～15、ただし IP ≦ KNL_LEVEL) */		//Ver106-1
#define INT_REQLAN      (1<<31)     /* 割込み要因 */
#define INT_PRILAN      (IP<<28)    /* LANの優先度 */
#define INT_MSKCLR_LAN  (1<<31)     /* LANの割込みマスククリア */
#endif

#ifndef DMA_CH
#define DMA_CH     0         /* default use Channel-0 */
#endif

#ifndef DMA_BURST
#define DMA_BURST  0         /* default is Cycle-steal */
#endif

/* DMAチャネル毎の再定義 */

#if DMA_CH == 0
#define INT_DMAC   INT_DMTE0  /* DMA0割込み番号 */
#define DMAC_SAR   DMAC_SAR0
#define DMAC_DAR   DMAC_DAR0
#define DMAC_TCR   DMAC_DMATCR0
#define DMAC_CHCR  DMAC_CHCR0

#elif DMA_CH == 1
#define INT_DMAC   INT_DMTE1  /* DMA1割込み番号 */
#define DMAC_SAR   DMAC_SAR1
#define DMAC_DAR   DMAC_DAR1
#define DMAC_TCR   DMAC_DMATCR1
#define DMAC_CHCR  DMAC_CHCR1

#elif DMA_CH == 2
#define INT_DMAC   INT_DMTE2  /* DMA2割込み番号 */
#define DMAC_SAR   DMAC_SAR2
#define DMAC_DAR   DMAC_DAR2
#define DMAC_TCR   DMAC_DMATCR2
#define DMAC_CHCR  DMAC_CHCR2

#else /* DMA_CH == 3 */
#define INT_DMAC   INT_DMTE3  /* DMA3割込み番号 */
#define DMAC_SAR   DMAC_SAR3
#define DMAC_DAR   DMAC_DAR3
#define DMAC_TCR   DMAC_DMATCR3
#define DMAC_CHCR  DMAC_CHCR3
#endif

#define CHCR_LCKN  0x40000000  /* Bus Lock Signal Disable */
#define CHCR_RPT0  (0x00<<25)  /* Repeat/Reload Mode (000):Nornal             */
#define CHCR_RPT1  (0x01<<25)  /* Repeat/Reload Mode (001):Repeat SAR/DAR/TCR */
#define CHCR_RPT2  (0x02<<25)  /* Repeat/Reload Mode (010):Repeat DAR/TCR     */
#define CHCR_RPT3  (0x03<<25)  /* Repeat/Reload Mode (011):Repeat SAR/TCR     */
#define CHCR_RPT4  (0x05<<25)  /* Repeat/Reload Mode (101):Reload SAR/DAR     */
#define CHCR_RPT5  (0x06<<25)  /* Repeat/Reload Mode (110):Reload DAR         */
#define CHCR_RPT6  (0x07<<25)  /* Repeat/Reload Mode (111):Reload SAR         */
#define CHCR_DO    0x00800000  /* Check DMA Overrun */
#define CHCR_RL    0x00400000  /* Request check Level bit */
#define CHCR_HE    0x00080000  /* Half End Flag */
#define CHCR_HIE   0x00040000  /* Half End Interrupt Enable */
#define CHCR_AM    0x00020000  /* Acknowledge Mode bit */
#define CHCR_AL    0x00010000  /* Acknowledge Level */
#define CHCR_DM0   (0x00<<14)  /* Destination address Mode(00) */
#define CHCR_DM1   (0x01<<14)  /* Destination address Mode(01) */
#define CHCR_DM2   (0x02<<14)  /* Destination address Mode(10) */
#define CHCR_SM0   (0x00<<12)  /* Source address Mode(00) */
#define CHCR_SM1   (0x01<<12)  /* Source address Mode(01) */
#define CHCR_SM2   (0x02<<12)  /* Source address Mode(10) */
#define CHCR_RS0   (0x00<<8)   /* Resource Select(0000):DREQ,Dual */
#define CHCR_RS2   (0x02<<8)   /* Resource Select(0010):DREQ,Single(MEM->DEV) */
#define CHCR_RS3   (0x03<<8)   /* Resource Select(0011):DREQ,Single(DEV->MEM) */
#define CHCR_RS4   (0x04<<8)   /* Resource Select(0100):Auto */
#define CHCR_DL    0x00000080  /* DREQ Select */
#define CHCR_DS    0x00000040  /* DREQ Select */
#define CHCR_TB    0x00000020  /* Transfer Bus Mode (0):Cycle Steal / (1):Burst */
#define CHCR_TS0   (0x00<<3)   /* Transmit Size(000):8bit   */
#define CHCR_TS1   (0x01<<3)   /* Transmit Size(001):16bit  */
#define CHCR_TS2   (0x02<<3)   /* Transmit Size(010):32bit  */
#define CHCR_TS3   (0x03<<3)   /* Transmit Size(011):16byte */
#define CHCR_TS4   0x00100000  /* Transmit Size(100):32byte */
#define CHCR_IE    0x00000004  /* Interrupt Enable */
#define CHCR_TE    0x00000002  /* Transfer End */
#define CHCR_DE    0x00000001  /* DMAC Enable */
#define CHCR_RS    CHCR_RS4    /* Auto-REQ */

#ifndef BUFF_ALIGN
#define BUFF_ALIGN 4
#endif

#if BUFF_ALIGN == 4
#define CHCR_TS    CHCR_TS2    /* 32bit witdh */
#define DMA_SZ     2           /* right-shift number (div 4) */

#elif BUFF_ALIGN == 16
#define CHCR_TS    CHCR_TS3    /* 16byte witdh */
#define DMA_SZ     4           /* right-shift number (div 16) */

#else
#error no support BUFF_ALIGN
#endif

/* 外部参照 */

void lan_intr(VP_INT exinf);
void lan_dma_intr(VP_INT exinf);

/* LAN & DMA interrupt service routine information */

static void lan_ints(VP_INT exinf);
const T_CISR disr_isrlan = { TA_HLNG, NULL, INT_LANC, lan_ints, IP };
const T_CISR disr_isrdma = { TA_HLNG, NULL, INT_DMAC, lan_dma_intr, IP };
static ID isrid_lan;
static ID isrid_dma;

/*******************************************************************************
* DMA初期化(共通)
*
*******************************************************************************/

ER lan_dma_ini(void)
{
    UH tmp;

  #if DMA_BURST != 0
    sfr_outl(DMAC_CHCR, CHCR_AL|CHCR_DM1|CHCR_RS|CHCR_TS|CHCR_IE|CHCR_TB);
  #else
    sfr_outl(DMAC_CHCR, CHCR_AL|CHCR_DM1|CHCR_RS|CHCR_TS|CHCR_IE);
  #endif
            /* Reference  : Direct                   (DI)
             * DACK-signal: read cycle, high-active  (AM,AL)
             * destination: increment                (DM)
             * source     : fix                      (SM)
             * resource   : single or dual           (RS)
             * DREQ       : Edge                     (DS)
             * bus-cycle  : burst or cycle           (TM)
             * Width      : 16byte or 32bit          (TS)
             * INT        : enable                   (IE)
             */

    if (!((tmp = sfr_inw(DMAC_DMATOR)) & 0x0001))
        sfr_outw(DMAC_DMATOR, tmp|0x0001);  /* DMAC master enable */

    return E_OK;
}

/*******************************************************************************
* DMA駆動状態
*
*******************************************************************************/

BOOL lan_dma_sts(void)
{
    return (BOOL)(sfr_inl(DMAC_CHCR) & CHCR_DE);
}

/*******************************************************************************
* DMA割込み要因クリア
*
*******************************************************************************/

ER lan_dma_clr(void)
{
    sfr_clrl(DMAC_CHCR, CHCR_TE|CHCR_DE);

    return E_OK;
}

/*******************************************************************************
* DMA駆動
*
*******************************************************************************/

ER lan_dma_sta(UW des, UW src, UW len)
{
    if (len > 0x1000000UL)
        return E_PAR;

    len >>= DMA_SZ;        /* count for transfer-width */

    sfr_outl(DMAC_SAR, src);                /* ソースアドレス */
    sfr_outl(DMAC_DAR, des);                /* デスティネーションアドレス */
    sfr_outl(DMAC_TCR, len & 0x00ffffffUL); /* 転送回数 */
    sfr_setl(DMAC_CHCR, CHCR_DE);           /* DMA開始 */

    return E_OK;
}

/*******************************************************************************
* LAN controller interrupt service routine definition function
*
*******************************************************************************/

ER lan_def_int(void)
{
    UINT    psw;
    ER      ercd;

    ercd = acre_isr(&disr_isrlan);       /* create LAN interrupt service routine */
    if (ercd <= 0)
        return ercd;
    isrid_lan = ercd;

    ercd = acre_isr(&disr_isrdma);       /* create DMA interrupt service routine */
    if (ercd <= 0)
        return ercd;
    isrid_dma = ercd;

    psw = vdis_psw();

    /* 割込みコントローラの設定 */
    fpga_clrw(INT_CRL, INT_CRL_LAN);
    fpga_setw(INT_CRL, INT_CRL_LAN);

    vset_psw(psw);

    return E_OK;
}

/*******************************************************************************
* Interrupt service routine for the LAN controller
*
*******************************************************************************/

static void lan_ints(VP_INT exinf)
{
    fpga_clrw(INT_CRL, INT_CRL_LAN);			/* LANの割込み要因をクリア */

    lan_intr(exinf);

    fpga_setw(INT_CRL, INT_CRL_LAN);			/* LANの割込みを許可 */
}

/******************************************************************************
* Delete LAN controller interrupt
*
******************************************************************************/

ER lan_undef_int(void)
{
    ER ercd;

    fpga_clrw(INT_CRL, INT_CRL_LAN);			/* LANの割込み要因をクリア */

    ercd = del_isr(isrid_lan);     /* delete LAN interrupt service routine */

    if (ercd != E_OK)
        return ercd;

    ercd = del_isr(isrid_dma);     /* delete DMA interrupt service routine */

    return ercd;
}

/* end */
