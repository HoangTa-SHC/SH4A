/*******************************************************************************
* LANコントローラドライバのボード依存部 (SMSC LAN9118)                         *
*                                                                              *
*  Copyright (c) 2005 -2010, MiSPO Co., Ltd.                                   *
*  All rights reserved.                                                        *
*                                                                              *
* 2005-04-19 Created                                                        OK *
* 2005-09-21 マルチチャネル対応                                             OK *
* 2005-09-21 Modified for SH7706 [I/O space is 0x0C000000]                  OK *
* 2006-05-22 Modified for SEM150                                            SZ *
* 2007-09-14 Modified for SH-2007                                           YM *
* 2010-03-12 Deleted LITTLE_ENDIAN macro automatically defined in itron.h   HE *
*******************************************************************************/

/* LANコントローラのチャネル番号 */

#ifdef DIF_NUM
#undef  CH
#define CH          DIF_NUM
#else
#ifndef CH
#define CH          0
#endif
#endif

/* LANコントローラのベースアドレス */

#if (CH == 0)
//#define LAN_BASE    0xba000000UL
#define LAN_BASE    0xa4000000UL		//Ver106-1
#elif (CH == 1)
//#define LAN_BASE    0xba000000UL
#define LAN_BASE    0xa4000000UL		//Ver106-1
#endif

/* 非キャッシュ空間オフセット (LANバッファ変数空間アドレスに加算します) */

//Ver106-1	#define LAN_BUF_BASE    0x20000000UL    /* +0x80000000(P1) = 0xA0000000(P2) */

/* バッファアライメント */

#ifdef NO_DMA
#if NO_DMA
#undef BUFF_ALIGN
#define BUFF_ALIGN  4
#endif
#endif

/* 各レジスタのアドレス */

#define LAN_ADDR(r) (LAN_BASE+(r))
#define LAN_ADDRD   LAN_ADDR


/* バス接続方法 */

#ifndef BUSSZ
//#define BUSSZ       32  /* バス幅 (16, 32) */
#define BUSSZ       16  /* バス幅 (16, 32) */		//Ver106-1
#endif
#ifndef BUS_SWAP
#define BUS_SWAP    0   /* BIG ENDIAN時のバス入れ替え (0:なし, 1:あり) */
#endif


/*****************************************************************************
* バイト並び入れ替え
*
******************************************************************************/
#if (BUS_SWAP)

static UW swap4(UW d)
{
    UW dd;

    dd  = (UB)d; dd <<= 8; d >>= 8;
    dd |= (UB)d; dd <<= 8; d >>= 8;
    dd |= (UB)d; dd <<= 8; d >>= 8;
    dd |= (UB)d;
    return dd;
}

#endif
/*****************************************************************************
* LANコントローラ入出力マクロ
*
******************************************************************************/

/* データレジスタの32ビット読み書き */

#define lan_ind(r)        (*(volatile UW *)LAN_ADDRD(r))
#define lan_outd(r,d)     (*(volatile UW *)LAN_ADDRD(r) = (d))

/* ワードの読み書き */

#define lan_in32(r)       (*(volatile UW *)LAN_ADDR(r))
#define lan_out32(r,d)    (*(volatile UW *)LAN_ADDR(r) = (d))

#if (BUS_SWAP)
#define lan_rreg(r)       swap4(lan_in32(r))
#define lan_wreg(r,d)     lan_out32(r, swap4(d))
#else
#define lan_rreg(r)       lan_in32(r)
#define lan_wreg(r,d)     lan_out32(r, d)
#endif

/*****************************************************************************
* DMA I/F
*
******************************************************************************/

ER lan_dma_ini(void);
BOOL lan_dma_sts(void);
ER lan_dma_clr(void);
ER lan_dma_sta(UW des, UW src, UW len);

/* end */
