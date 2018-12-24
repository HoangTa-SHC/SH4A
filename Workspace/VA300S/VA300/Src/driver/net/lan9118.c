/*******************************************************************************
* SMSC LAN9118                                                                 *
*                                                                              *
*  Copyright (c) 2005-2010, MiSPO Co., Ltd.                                    *
*  All rights reserved.                                                        *
*                                                                              *
* 2005-04-19 Created                                                        OK *
* 2005-06-10 Corrected for 16bit-Bus configuration (ENDIAN)                 OK *
* 2005-06-28 デバイスBug(10Base-T極性反転)対策                              OK *
* 2005-06-28 受信バッファの処理が遅延されるBug修正(DMAモード時)             OK *
* 2005-07-11 不要テストコード削除(実コードに変更なし)                       OK *
* 2005-09-21 LANバッファに非キャッシュ空間を割り当て可能にした              OK *
* 2006-02-05 割り込み線出力のバッファタイプをマクロ指定可能にした           HM *
* 2006-05-31 BUFF_ALIGN=16 or 32の場合のチップBug回避コードBug fix          OK *
* 2006-06-09 マルチチャネル対応の再定義漏れ修正(lan_dma_xxx?)               OK *
* 2006-10-31 PIO転送でリンク状態遷移により受信異常が起こる問題を修正        HM *
*            CSR_CMD=0に初期化する(HW-RESETなしに再スタート考慮 for debug)     *
*            ハンドラ登録をイベントクリア後に移動                              *
* 2006-11-10 リンクダウンのときの送信タスク起床削除                         HM *
* 2006-11-27 DMA使用モードでエラー受信あると通信が停止するbugの対策         OK *
*            リンク監視の方法の再見直し(Upは送受信独立。Downは優先処理)        *
*            送信側の待ち要因を「lan_wai_snd()」1ケ所に集約する                *
* 2006-12-13 BigEndian,BUS=16,BUS-SWAP=0でENDIAN_CTL書込み前にRead追加      OK *
*            (RESET復帰の初回にReadが必用なため)                               *
* 2006-12-22 半2重のとき自己パケットがループバックする機能を無効にした      HM *
* 2007-01-11 送信FIFO枯渇と同時の受信用DMA割込みで受信INT許可の遅れ対策     OK *
* 2007-09-14 BigEndian,DMA転送でのデータ並び替え対応                        YM *
* 2007-10-12 仮割り当てMACアドレスを変更 0050C2-040FFF → 123456-789ABC     SZ *
* 2007-10-16 GHS2000 compiler warning corrections.                          SZ *
* 2009-02-27 リンクダウン時にコントローラに出力せずに破棄(下記関数)      IS,OK *
*            lan_wai_snd、lan_put_end、lan_link                                *
* 2010-03-12 Removed unused local variable "delay" in lan_wai_snd           HE *
*            Replaced NORTi3 header include with NORTi4                        *
*            Replaced interrupt handler with interrupt service routine         *
*******************************************************************************/

/*

DMA使用の有無

          NO_DMA   = 0: DMA有効                  ..... default
                     1: DMA無効

          (例) shc <option> -def=NO_DMA=1 lan9118.c
                            ~~~~~~~~~~~~~

強制ループバックの設定

          LBTYPE   = 0: ループバック禁止(標準)   ..... default
                     1: 内部ループバック(MAC)
                     2: 外部ループバック(PHY)

          (例) shc <option> -def=LBTYPE=2 lan9118.c
                            ~~~~~~~~~~~~~

          注意! ドライバ単体テスト用です「NORTi TCP/IPプロトコルスタック」
                を使用した上位アプリケーションの実行目的には使用できません。


通信モードの設定

          FULLDPLX = 0: AutoNegotiation          ..... default
                     1:  10M/HALF Duplex mode
                     2:  10M/FULL Duplex mode
                     3: 100M/HALF Duplex mode
                     4: 100M/FULL Duplex mode

          (例) shc <option> -def=FULLDPLX=1 lan9118.c
                            ~~~~~~~~~~~~~~~

バッファアライメント

          DMA転送が有効な場合でLANバッファメモリにバースト転送モードを持つ
          デバイス(SDRAMなど)をご使用の場合にその転送サイズ(16 or 32byte)
          に合わせてください。バースト転送をもたないメモリデバイスや、バー
          スト転送を行わない場合は、デフォルトでご使用ください。

          BUFF_ALIGN =  4:  4byte alignment      ..... default
                       16: 16byte alignment
                       32: 32byte alignment

          (例) shc <option> -def=BUFF_ALIGN=16 lan9118.c
                            ~~~~~~~~~~~~~~~~~~

          注意! 現バージョンでは、BUFF_ALIGN=32の動作は未確認です。


LINK UP監視時間の設定

          LINK DOWN(未接続)の状態からLINK UP(接続)を監視する周期時間を設定
          します。設定値は、システムコールに指定のタイムアウト値相当です。
          デフォルトは、1秒です。LINK UP直後の自局送信が最大でこの設定時間
          遅れます。受信があれば送受信ともに即時復帰します。

          LINK_TIME =
                     0: 禁止
                     1: 最小
             1000/MSEC: 1秒                      ..... default

          ※MSECマクロについては、NORTi ユーザーズガイドを参照ください。


マルチキャストフレームの受信

          ALMUL    = 0: 受信しない               ..... default
                     1: 受信する


プロミスキャス受信(到着フレーム全受信)

          PRMS     = 0: 無効                     ..... default
                     1: 有効

割り込み線出力のバッファタイプ

          IRQ_BUFF = 0: オープンドレイン         ..... default
                     1: プッシュプル

*/

#include <string.h>
#ifndef NULL
#include <stdio.h>
#endif
#ifdef GAIO
#include <memory.h>
#endif
#include "kernel.h"
#include "nosys4.h"
#include "nonet.h"
#include "nonethw.h"
#include "lan9118.h"

/*
 * ドライバ機能定義
 */

#ifndef BUG_10BASE_POL
#define BUG_10BASE_POL  1  /* be workaround */
#endif

/* DMA使用 */

#ifndef NO_DMA
#define NO_DMA      0   /* default need DMA (0:DMA / 1:no DMA) */
//#define NO_DMA      1   /* default need DMA (0:DMA / 1:no DMA) */		//Ver106-1
#endif

#if NO_DMA
#else
#define lan_indrep(a,b)  lan_dma_sta((UW)a,(UW)&lan_ind(RX_DATA),(UW)b)
#endif

/* ループバック */

#ifndef LBTYPE
#define LBTYPE      0   /* ループバック禁止(標準モード) */
#endif

/* 半二重/全二重 */

#if LBTYPE != 0
#undef FULLDPLX
#define FULLDPLX    4   /* 100M/FullDuplex if loopback enable */
#else
#ifndef FULLDPLX
#define FULLDPLX    0   /* Auto-Negotiation available */
#endif
#endif

/* バッファアライメント(this must be 4,16,32) */

#if NO_DMA
#undef  BUFF_ALIGN      /* FIFO PIOアクセスは4に固定 */
#endif
#ifndef BUFF_ALIGN
#define BUFF_ALIGN  4
#endif

/* 非キャッシュ空間オフセット (LANバッファ用変数アドレスに加算します) */

#ifndef LAN_BUF_BASE
#define LAN_BUF_BASE    0        /* no cacheable */
#endif

/* LINK UP監視時間 */

#ifndef LINK_TIME
#define LINK_TIME   (1000/MSEC)  /* default is 1[sec] period */
#endif

/* マルチキャストフレームの受信 */

#ifndef ALMUL
#define ALMUL       0     /* 0 = 受信しない, 1 = 全て受信  */
#endif

/* プロミスキャス受信(到着フレーム全受信) */

#ifndef PRMS
#define PRMS        0     /* 0 = 無効, 1 = 有効 */
#endif

/* PHYデバイス局アドレス */

#ifndef PHYAD
#define PHYAD       0x01  /* 内蔵は0x01に固定 */
#endif

/* 割り込み線出力のバッファタイプ */

#ifndef IRQ_BUFF
#define IRQ_BUFF    0     /* 0 = オープンドレイン, 1 = プッシュプル */
#endif

/*
 * Buffer Descripter definition
 */

typedef struct t_bd {
    union{
        struct{
            UH ctl;
            UH len;
            UW stat;
        } rx;           /* Rx CSR */
        struct {
            UW cmd_a;
            UW cmd_b;
        } tx;           /* Tx CSR */
    } csr;              /* Control & Status registration(CSR) */
    UB *buf;
    struct t_bd *next;
} T_BD;

#define BD_SIZE         16          /* BD size number */

/* CSR bit field definition */

#define RX_BD_RDY       0x8000      /* Rx-BD Ready bit */

/* Buffer configuration */

#ifndef NUM_RXBDS
#define NUM_RXBDS       8           /* Rx buffer number */
#endif
#ifndef NUM_TXBDS
#define NUM_TXBDS       1           /* Tx buffer number */
#endif

#if BUFF_ALIGN == 4
#define RECV_BUFF_SIZE  1520        /* >= 1514+4(CRC) && 4-byte align */
#define SEND_BUFF_SIZE  1516        /* >= 1514        && 4-byte align */
#define TX_CMDA_EA      TX_CMDA_EA4 /* TX COMMAND 'A' field */
#define RCFG_EA         RCFG_EA4    /* RX_CFG register field */

#elif BUFF_ALIGN == 16
#define RECV_BUFF_SIZE  1520        /* >= 1514+4(CRC) && 16-byte align */
#define SEND_BUFF_SIZE  1520        /* >= 1514        && 16-byte align */
#define TX_CMDA_EA      TX_CMDA_EA16/* TX COMMAND 'A' field */
#define RCFG_EA         RCFG_EA16   /* RX_CFG register field */

#elif BUFF_ALIGN == 32
#define RECV_BUFF_SIZE  1536        /* >= 1514+4(CRC) && 32-byte align */
#define SEND_BUFF_SIZE  1536        /* >= 1514        && 32-byte align */
#define TX_CMDA_EA      TX_CMDA_EA32/* TX COMMAND 'A' field */
#define RCFG_EA         RCFG_EA32   /* RX_CFG register field */

#else
#error "other size is bad."
#endif

#define ETHMINLEN       64          /* Ethernetパケット最小長(FCS含む) */
#define ETHMAXLEN       1518        /* Ethernetパケット最大長(FCS含む) */

static T_BD *cRxBD;    /* Rx BD background address for DMA */
static T_BD *pRxBD;    /* Rx BD current    address for handling */
static T_BD *pTxBD;    /* Tx BD current    address for handling */

/* Buffer Memory */

static T_BD RxBD[NUM_RXBDS];  /* Rx BD area */
static T_BD TxBD[NUM_TXBDS];  /* Tx BD area */

static UB  RxBuf[NUM_RXBDS * RECV_BUFF_SIZE +BUFF_ALIGN-1];  /* Rx buffer */
static UB  TxBuf[NUM_TXBDS * SEND_BUFF_SIZE +BUFF_ALIGN-1];  /* Tx buffer */


/* LANコントローラドライバのコールバック関数の型 */

typedef ER (*LAN_CALLBACK)(int, ER, const char*);
                                /* 第1引数  チャネル番号
                                   第2引数  リンクステータス
                                             0: LAN_LNK_OFF  リンク消失
                                             1: LAN_LNK_10H  リンク復旧(10M半二重)
                                             2: LAN_LNK_10F  リンク復旧(10M全二重)
                                             3: LAN_LNK_100H リンク復旧(100M半二重)
                                             4: LAN_LNK_100F リンク復旧(100M全二重)
                                   第3引数  エラーメッセージ */

/* LANコントローラ制御ブロックの型 */

typedef struct t_lan {
    volatile UH txtid;
    volatile UH rxtid;
    FP callback;        /* コールバック関数のアドレス */
    UH txtag;           /* Tx COMMAND 'B' Tag number */
    volatile UB lnk_chg;/* LINK change indication flag */
    UB pad[1];
    ER sts_new;         /* LINK状態(current) */
    ER sts_old;         /* LINK状態(previous) */
    ER sts_mod;         /* LINK状態(mode before current) */
    volatile UW mask;   /* same as INT_EN register */
    UW mac_cr;          /* same as MAC_CR register */
    T_BD *rxbd;         /* Rx処理BD */
    T_BD *txbd;         /* Tx処理BD */
    UB *rxbuf;          /* Rx buffer position */
    UB *txbuf;          /* Tx buffer position */

/* for debug */
#ifdef RX_MONITOR
#define RMON(a) lan.err_count[a]++;

#define ERR_BDOV      0  /* RX BD overrun */
#define ERR_DROP      1  /* RX Drop frame */
#define ERR_RXCNT     2  /* RX frame count */
#define ERR_ERSTAT    3  /* RX error frame count */
#define ERR_TOOLG     4  /* RX too long frame length */
#define ERR_RXINT     5  /* RX INT count */
#define ERR_LOG_MAX   6

    UW err_count[ERR_LOG_MAX];
#else
#define RMON(a) do{}while(0)
#endif
} T_LAN;

/* 内部変数 */

static T_LAN lan;
static FP callbackini = 0;

static UH PHY_mode;
static UH PHY_auto;

/* マルチチャネル使用時の関数名リネーム */

#if (defined(DIF_NUM))
  #if (DIF_NUM == 0)
    #define lan_ini       lan_ini0
    #define lan_ini_dev   lan_ini_dev0
    #define lan_ext_dev   lan_ext_dev0
    #define lan_wai_rcv   lan_wai_rcv0
    #define lan_wai_snd   lan_wai_snd0
    #define lan_get_len   lan_get_len0
    #define lan_get_pkt   lan_get_pkt0
    #define lan_skp_pkt   lan_skp_pkt0
    #define lan_get_end   lan_get_end0
    #define lan_set_len   lan_set_len0
    #define lan_put_pkt   lan_put_pkt0
    #define lan_put_dmy   lan_put_dmy0
    #define lan_put_end   lan_put_end0
    #define lan_def_cbk   lan_def_cbk0
    #define lan_intr      lan_intr0
    #define lan_dma_intr  lan_dma_intr0
    #define lan_dma_ini   lan_dma_ini0
    #define lan_dma_sts   lan_dma_sts0
    #define lan_dma_clr   lan_dma_clr0
    #define lan_dma_sta   lan_dma_sta0
    #define lan_def_int   lan_def_int0
    #define lan_undef_int lan_undef_int0
    #define lan_def_cbk   lan_def_cbk0
    #define lan_dev_err   lan_dev_err0
    #define lan_get_eep   lan_get_eep0
    #define lan_set_eep   lan_set_eep0
    #define lan_get_mac   lan_get_mac0
    #define lan_set_mac   lan_set_mac0
    #define lan_tst_snd   lan_tst_snd0
    #define lan_tst_rcv   lan_tst_rcv0
  #elif (DIF_NUM==1)
    #define lan_ini       lan_ini1
    #define lan_ini_dev   lan_ini_dev1
    #define lan_ext_dev   lan_ext_dev1
    #define lan_wai_rcv   lan_wai_rcv1
    #define lan_wai_snd   lan_wai_snd1
    #define lan_get_len   lan_get_len1
    #define lan_get_pkt   lan_get_pkt1
    #define lan_skp_pkt   lan_skp_pkt1
    #define lan_get_end   lan_get_end1
    #define lan_set_len   lan_set_len1
    #define lan_put_pkt   lan_put_pkt1
    #define lan_put_dmy   lan_put_dmy1
    #define lan_put_end   lan_put_end1
    #define lan_def_cbk   lan_def_cbk1
    #define lan_intr      lan_intr1
    #define lan_dma_intr  lan_dma_intr1
    #define lan_dma_ini   lan_dma_ini1
    #define lan_dma_sts   lan_dma_sts1
    #define lan_dma_clr   lan_dma_clr1
    #define lan_dma_sta   lan_dma_sta1
    #define lan_def_int   lan_def_int1
    #define lan_undef_int lan_undef_int1
    #define lan_def_cbk   lan_def_cbk1
    #define lan_dev_err   lan_dev_err1
    #define lan_get_eep   lan_get_eep1
    #define lan_set_eep   lan_set_eep1
    #define lan_get_mac   lan_get_mac1
    #define lan_set_mac   lan_set_mac1
    #define lan_tst_snd   lan_tst_snd1
    #define lan_tst_rcv   lan_tst_rcv1
  #elif (DIF_NUM==2)
    #define lan_ini       lan_ini2
    #define lan_ini_dev   lan_ini_dev2
    #define lan_ext_dev   lan_ext_dev2
    #define lan_wai_rcv   lan_wai_rcv2
    #define lan_wai_snd   lan_wai_snd2
    #define lan_get_len   lan_get_len2
    #define lan_get_pkt   lan_get_pkt2
    #define lan_skp_pkt   lan_skp_pkt2
    #define lan_get_end   lan_get_end2
    #define lan_set_len   lan_set_len2
    #define lan_put_pkt   lan_put_pkt2
    #define lan_put_dmy   lan_put_dmy2
    #define lan_put_end   lan_put_end2
    #define lan_def_cbk   lan_def_cbk2
    #define lan_intr      lan_intr2
    #define lan_dma_intr  lan_dma_intr2
    #define lan_dma_ini   lan_dma_ini2
    #define lan_dma_sts   lan_dma_sts2
    #define lan_dma_clr   lan_dma_clr2
    #define lan_dma_sta   lan_dma_sta2
    #define lan_dma_int   lan_dma_int2
    #define lan_def_int   lan_def_int2
    #define lan_undef_int lan_undef_int2
    #define lan_def_cbk   lan_def_cbk2
    #define lan_dev_err   lan_dev_err2
    #define lan_get_eep   lan_get_eep2
    #define lan_set_eep   lan_set_eep2
    #define lan_get_mac   lan_get_mac2
    #define lan_set_mac   lan_set_mac2
    #define lan_tst_snd   lan_tst_snd2
    #define lan_tst_rcv   lan_tst_rcv2
  #else
    #error DNIF_NUM is out of range! Customize it.
  #endif
#else
  #define DIF_NUM   0
#endif

/* 外部参照 */

extern ER lan_def_int(void);
extern ER lan_dma_ini(void);
extern ER lan_undef_int(void);

/* 割込み関連の定義 */

#define DEF_RXINT (INT_RSFL)    /* 受信割込みとするメンバーのマスク */
#define DEF_TXINT (INT_TDFA)    /* 送信割込みとするメンバーのマスク */

#define DI_RXINT()  /* Rx割込み禁止 */\
do{\
    lan_wreg(INT_EN, 0);\
    dly_tick(1);\
    lan.mask &= ~DEF_RXINT;\
    lan_wreg(INT_EN, lan.mask);\
}while(0)

#define EI_RXINT()  /* Rx割込み許可 */\
do{\
    lan_wreg(INT_EN, 0);\
    dly_tick(1);\
    lan.mask |= DEF_RXINT;\
    lan_wreg(INT_EN, lan.mask);\
}while(0)

#if NO_DMA
#define DI_ALLINT_START()  do{lan_wreg(INT_EN, 0);dly_tick(1)
#define DI_ALLINT_END()                                     } while(0)
#define RX_RESTART()
#else
#define DI_ALLINT_START()  do{UINT psw; psw = vdis_psw()
#define DI_ALLINT_END()                       vset_psw(psw);} while(0)
#define RX_RESTART() lan_dma_restart()
#endif

#define DI_TXINT()  /* Tx割込み禁止 */\
do{\
    DI_ALLINT_START();\
    lan.mask &= ~DEF_TXINT;\
    lan_wreg(INT_EN, lan.mask);\
    DI_ALLINT_END();\
}while(0)

#define EI_TXINT()  /* Tx割込み許可 */\
do{\
    DI_ALLINT_START();\
    lan.mask |= DEF_TXINT;\
    lan_wreg(INT_EN, lan.mask);\
    DI_ALLINT_END();\
}while(0)

#define EI_PHYINT() /* PHY割込み許可 */\
do{\
    DI_ALLINT_START();\
    lan.mask |= INT_PHY;\
    lan_wreg(INT_EN, lan.mask);\
    DI_ALLINT_END();\
}while(0)

/*****************************************************************************
* 状態変化時のコールバック関数の呼び出し
*
******************************************************************************/

static void lan_callback(ER ercd)
{
    LAN_CALLBACK func;

    if ((func = (LAN_CALLBACK)lan.callback) != NULL)
        func(DIF_NUM, ercd, "");
}

/*****************************************************************************
* 致命的エラー
*
******************************************************************************/

ER lan_dev_err(ER ercd, const char *s)
{
    LAN_CALLBACK func;

    if ((func = (LAN_CALLBACK)lan.callback) == NULL)
        return ercd;
    return func(DIF_NUM, ercd, s);
}

/*****************************************************************************
* Read after Write timing delay
*
******************************************************************************/

static void dly_tick(unsigned n)
{
    do{
        lan_in32(BYTE_TEST);  /* dummy read */
    } while (--n);
}

/*****************************************************************************
* Write MAC Control and Status Register
*
******************************************************************************/

static void mac_wreg(UB index, UW data)
{
    while (lan_rreg(MAC_CSR_CMD) & CSR_BUSY);  /* wait until READY */
    lan_wreg(MAC_CSR_DATA, data);
    lan_wreg(MAC_CSR_CMD, CSR_BUSY | index);
    dly_tick(1);  /* wait after "MAC_CSR_CMD read" write cycle */
}

/*****************************************************************************
* Read MAC Control and Status Register
*
******************************************************************************/

static UW mac_rreg(UB index)
{
    while (lan_rreg(MAC_CSR_CMD) & CSR_BUSY);  /* wait until READY */
    lan_wreg(MAC_CSR_CMD, CSR_BUSY | CSR_RD | index);
    dly_tick(1);
    while (lan_rreg(MAC_CSR_CMD) & CSR_BUSY);  /* wait until READY */

    return lan_rreg(MAC_CSR_DATA);
}

/*****************************************************************************
* PHYレジスタ書き込み
*
******************************************************************************/

static void PHY_write(int reg, int dat)
{
    while (mac_rreg(MAC_MII_ACC) & MII_BUSY);  /* wait until READY */
    mac_wreg(MAC_MII_DATA, dat);
    mac_wreg(MAC_MII_ACC, (PHYAD << MII_PHYAD_SHFT)
                         |(reg   << MII_INDEX_SHFT)
                         | MII_WR | MII_BUSY);
}

/*****************************************************************************
* PHYレジスタ読み出し
*
******************************************************************************/

static UH PHY_read(int reg)
{
    while (mac_rreg(MAC_MII_ACC) & MII_BUSY);  /* wait until READY */
    mac_wreg(MAC_MII_ACC, (PHYAD << MII_PHYAD_SHFT)
                         |(reg   << MII_INDEX_SHFT)
                         | MII_BUSY);
    while (mac_rreg(MAC_MII_ACC) & MII_BUSY);  /* wait until READY */

    return (UH)mac_rreg(MAC_MII_DATA);
}

/*****************************************************************************
* PHYのソフトリセット
*
******************************************************************************/

static void PHY_reset(void)
{
    int i;

    PHY_write(PHY_CR, PHY_CR_RST);      /* PHY software reset */

    /* リセット解除まち(実際は即時終了。念のためMax.500msec監視) */
    for (i=0; i<50; i++){
        if (!(PHY_read(PHY_CR) & PHY_CR_RST))
            break;
        dly_tsk(10/MSEC);
    }
}

/*****************************************************************************
* Get LINK status
*
******************************************************************************/

static ER PHY_link(void)
{
    UH mode;

    PHY_read(PHY_SR);            /* clear */
    mode = PHY_read(PHY_SR);
    if (!(mode & PHY_SR_LINK))
        return LAN_LNK_OFF;      /* LINK down */
    mode = PHY_mode;

    /* FIX mode */

    if (PHY_auto != TRUE){
        if (mode == FD100)
            return LAN_LNK_100F;
        if (mode == HD100)
            return LAN_LNK_100H;
        if (mode == FD10)
            return LAN_LNK_10F;
        else
            return LAN_LNK_10H;
    }

    /* AutoNegotiation */

    while(!(PHY_read(PHY_SR) & PHY_SR_AN_OK))
        dly_tsk(100/MSEC);

    mode &= PHY_read(PHY_ANLPA);
    if (mode & AFD100)
        return LAN_LNK_100F;
    else
    if (mode & AHD100)
        return LAN_LNK_100H;
    else
    if (mode & AFD10)
        return LAN_LNK_10F;
    else
        return LAN_LNK_10H;
}

/*****************************************************************************
* PHY acknowledge interrupt
*
******************************************************************************/

static void PHY_int_ack(void)
{
    PHY_read(PHY_ISR);  /* clear */
}

/*****************************************************************************
* PHY initialize interrupt
*
******************************************************************************/

static void PHY_init_int(void)
{
    PHY_read(PHY_ISR);                /* clear */
    PHY_write(PHY_IMR, PHY_INT_LINK); /* enable interrupt */
}

/*****************************************************************************
* PHY初期化
*
* nego  TRUE = Auto-Negotiation Enable, FALSE = Auto-Negotiation Disable
* mode  HD10   =10BASE-T  /半2重
*       FD10   =10BASE-T  /全2重
*       HD100  =100BASE-TX/半2重
*       FD100  =100BASE-TX/全2重
*       AHD10  =10BASE-T  /半2重(Auto-Negotiation)
*       AFD10  =10BASE-T  /全2重(Auto-Negotiation)
*       AHD100 =100BASE-TX/半2重(Auto-Negotiation)
*       AFD100 =100BASE-TX/全2重(Auto-Negotiation)
*
* 注1) AXXXXは(AHD10|AHD100)と書いて10M半2重と100M半2重の接続を許すという
*      意味になる。
* 注2) AXXXXでない方は複数のORは書けません。
******************************************************************************/

static ER PHY_init(BOOL nego, int mode, int lbtype)
{
    int i;
    ER rtcd;

    PHY_auto = nego;
    PHY_mode = mode;

    if (nego) {                     /* Auto Negotiationモードにする */
        PHY_write(PHY_ANA, mode | PHY_AN_802_3);
        mode = PHY_CR_ANEG_EN|PHY_CR_ANEG_RST;
        PHY_write(PHY_CR, (UH)mode);  /* Re-Start Auto-Negotiation */
    }
    else {                          /* Manual mode settings */
        if (lbtype != 0)
            mode |= PHY_CR_LPBK|PHY_CR_DPLX; /* 強制ループバックの設定 */
        PHY_write(PHY_CR, (UH)mode);    /* Set Speed and Duplex mode */
    }

    /* LINK状態監視(起動直後のUPまでMax.3sec監視) */
    /* 10Base-TのLOOPBACKモードではLINK UPにならないため除く */
    if (mode != (FD10|PHY_CR_LPBK)){
        for (i=0; i<30; i++){
            if (PHY_read(PHY_SR) & PHY_SR_LINK)
                break;
            dly_tsk(100/MSEC);
        }
        rtcd = PHY_link();
    } else {
        rtcd = LAN_LNK_10F;  /* 無条件に */
    }

    PHY_init_int();
    return rtcd;
}

#ifndef lan_outdrep
/*****************************************************************************
* LANコントローラへパケットデータ書き込み
*
******************************************************************************/

static void lan_outdrep(UW *b, int len)
{
    int i;
  #if (defined(BIG_ENDIAN) && !BUS_SWAP)
    UB *p, *q, pq;
  #endif

    /* データ並び不一致なら、並び替え */
  #if (defined(BIG_ENDIAN) && !BUS_SWAP)
    p = (UB *)b;
    q = p - 2;
    for (i = len; i > 0; i -= 4) {
        q += 5;
        pq = *q;
        *q-- = *p;
        *p++ = pq;
        pq = *q;
        *q = *p;
        *p = pq;
        p += 3;
    }
  #endif

    /* LANコントローラへ書き込み */
    for (i = len; i >= 60; i -= 60) {
        lan_outd(TX_DATA, *b++);
        lan_outd(TX_DATA, *b++);
        lan_outd(TX_DATA, *b++);
        lan_outd(TX_DATA, *b++);
        lan_outd(TX_DATA, *b++);
        lan_outd(TX_DATA, *b++);
        lan_outd(TX_DATA, *b++);
        lan_outd(TX_DATA, *b++);
        lan_outd(TX_DATA, *b++);
        lan_outd(TX_DATA, *b++);
        lan_outd(TX_DATA, *b++);
        lan_outd(TX_DATA, *b++);
        lan_outd(TX_DATA, *b++);
        lan_outd(TX_DATA, *b++);
        lan_outd(TX_DATA, *b++);
    }
    for (; i > 0; i -= 4)
        lan_outd(TX_DATA, *b++);
}

#endif
#ifndef lan_indrep
/*****************************************************************************
* LANコントローラからパケットデータ読み出し
*
******************************************************************************/

static void lan_indrep(UW *b, int len)
{
    int i;
  #if (defined(BIG_ENDIAN) && !BUS_SWAP)
    UB *p, *q, pq;
  #endif

  #if (defined(BIG_ENDIAN) && !BUS_SWAP)
    p = (UB *)b;
  #endif

    for (i = len; i >= 60; i -= 60) {
        *b++ = lan_ind(RX_DATA);
        *b++ = lan_ind(RX_DATA);
        *b++ = lan_ind(RX_DATA);
        *b++ = lan_ind(RX_DATA);
        *b++ = lan_ind(RX_DATA);
        *b++ = lan_ind(RX_DATA);
        *b++ = lan_ind(RX_DATA);
        *b++ = lan_ind(RX_DATA);
        *b++ = lan_ind(RX_DATA);
        *b++ = lan_ind(RX_DATA);
        *b++ = lan_ind(RX_DATA);
        *b++ = lan_ind(RX_DATA);
        *b++ = lan_ind(RX_DATA);
        *b++ = lan_ind(RX_DATA);
        *b++ = lan_ind(RX_DATA);
    }
    for (; i > 0; i -= 4)
        *b++ = lan_ind(RX_DATA);

    /* データ並び不一致なら、並び替え */
  #if (defined(BIG_ENDIAN) && !BUS_SWAP)
    q = p - 2;
    for (i = len; i > 0; i -= 4) {
        q += 5;
        pq = *q;
        *q-- = *p;
        *p++ = pq;
        pq = *q;
        *q = *p;
        *p = pq;
        p += 3;
    }
  #endif
}

#endif
/*****************************************************************************
* MACアドレスのチェック
*
******************************************************************************/

static int check_macaddr(UB *macaddr)
{
    int i, sum;

    for (sum = i = 0; i < 6; i++)
        sum += macaddr[i];
    if (sum == 0)
        return 0;               /* All 0 */
    else if (sum == 0xff * 6)
        return -1;              /* All F */
    else
        return 1;               /* other */

}

/*****************************************************************************
* バッファディスクリプタの初期化
*
******************************************************************************/
static BOOL bd_ini(void)
{
    int i;
    T_BD *rbd;
    T_BD *tbd;
    UB *p;

    pTxBD  = TxBD;
    pRxBD  = cRxBD = RxBD;

  #ifndef LAN_BUF_ALLOC
    p = (UB *)(ALIGN(TxBuf, BUFF_ALIGN));
    p += LAN_BUF_BASE;
  #else
    p = (UB *)(ALIGN(lan_alloc(NUM_TXBDS * SEND_BUFF_SIZE +BUFF_ALIGN-1), BUFF_ALIGN));
    if (!p)
        return FALSE;
  #endif
    tbd = pTxBD;
    for (i = 0; i < NUM_TXBDS; i++, tbd++){
        tbd->csr.tx.cmd_a = 0;
        tbd->csr.tx.cmd_b = 0;
        tbd->buf = p;
        p += SEND_BUFF_SIZE;
        tbd->next = tbd + 1;  /* LINK to next BD */
    }
    tbd--;
    tbd->next = pTxBD;    /* LINK to 1st BD */

  #ifndef LAN_BUF_ALLOC
    p = (UB *)(ALIGN(RxBuf, BUFF_ALIGN));
    p += LAN_BUF_BASE;
  #else
    p = (UB *)(ALIGN(lan_alloc(NUM_RXBDS * RECV_BUFF_SIZE +BUFF_ALIGN-1), BUFF_ALIGN));
    if (!p)
        return FALSE;
  #endif
    rbd = pRxBD;
    for (i = 0; i < NUM_RXBDS; i++, rbd++){
        rbd->csr.rx.ctl = RX_BD_RDY;
        rbd->buf = p;
        p += RECV_BUFF_SIZE;
        rbd->next = rbd + 1;  /* LINK to next BD */
    }
    rbd--;
    rbd->next = pRxBD;    /* LINK to 1st BD */

    return TRUE;
}

/*****************************************************************************
* LANコントローラ初期化(ループバックのタイプ指定付き)
*
* macaddr   MACアドレス(6バイトの配列)へのポインタ
*
* lbtype    ループバックのタイプ(0～2)
*           0 = ループバック禁止(標準)
*           1 = 内部ループバック(MAC)
*           2 = 外部ループバック(PHY)
******************************************************************************/

ER lan_ini_dev(UB *macaddr, int lbtype)
{
    ER lan_tst_snd(void);
    ER lan_tst_rcv(void);

    int i,j;
    BOOL eep;
    ER ercd;
    UW tmp;
    UW mac_addrl, mac_addrh;
    ER mode;

    /* 内部変数を初期化 */
    memset(&lan, 0, sizeof (T_LAN));

    if (callbackini)
        lan.callback = callbackini;

    dly_tsk(30/MSEC);   /* >=25ms delay after power-on reset (On the safer side) */

  #if defined(BIG_ENDIAN) && BUSSZ == 16 && BUS_SWAP == 0
    lan_wreg(ENDIAN_CTL, lan_rreg(ENDIAN_CTL)|0xffffffff);
  #endif

    /* Testing register R/W */

    if ((tmp = lan_rreg(BYTE_TEST)) != BYTE_TEST_VAL)
        return lan_dev_err(-1, "BYTE_TEST error");
    tmp ^= lan_rreg(BYTE_TEST);
    if (tmp != 0)
        return lan_dev_err(-1, "read error");

    while(lan_rreg(MAC_CSR_CMD) & CSR_BUSY);
    lan_wreg(MAC_CSR_CMD, 0);                     /* R/nW bit low */

    for (i = 0; i < 3; i++){
        lan_wreg(MAC_CSR_DATA, BYTE_TEST_VAL);
        tmp  = lan_rreg(BYTE_TEST);  /* for wait read after write */
        tmp ^= lan_rreg(MAC_CSR_DATA);

        if (tmp != 0)
            return lan_dev_err(-1, "write error");
    }

    /* Reset */

    lan_wreg(HW_CFG, HWCFG_SRST);
    dly_tick(1);
    while (lan_rreg(HW_CFG) & HWCFG_SRST);

    PHY_reset();

    /* Setting */

    lan_wreg(HW_CFG, HWCFG_SF|HWCFG_TFSZ3);  /* TX Data FIFO size = 2560 */
    lan_wreg(GPIO_CFG, GPIO_LED3|GPIO_LED2|GPIO_LED1);  /* Set the LED output */
    lan_wreg(FIFO_INT, 0x18000000);  /* TFDA > 1536, RSFL > 0, other don't care */

    /* MACアドレスを何処から取得するか？ */
    if (check_macaddr(macaddr) <= 0)
        eep = TRUE;                    /* 000000かFFFFFFならEEPROMの値 */
    else
        eep = FALSE;                   /* それ以外ならmacaddr引数を使う */

    /* EEPROMの値をMACアドレスとする */
    if (eep) {
        if (!(lan_rreg(E2P_CMD) & E2P_MACLO)){  /* Auto-load after reset fail */
            mac_addrl = 0xffffffff;
            mac_addrh = 0xffffffff;
        }else{
            mac_addrl = mac_rreg(MAC_ADDRL);
            mac_addrh = mac_rreg(MAC_ADDRH);
        }
        macaddr[0] = (UB)(mac_addrl);
        macaddr[1] = (UB)(mac_addrl >> 8);
        macaddr[2] = (UB)(mac_addrl >> 16);
        macaddr[3] = (UB)(mac_addrl >> 24);
        macaddr[4] = (UB)(mac_addrh);
        macaddr[5] = (UB)(mac_addrh >> 8);

        /* 不正なMACアドレスならローカルなMACアドレスを仮割り当て */
        if (check_macaddr(macaddr) <= 0) {
            macaddr[0] = 0x12; macaddr[1] = 0x34;
            macaddr[2] = 0x56; macaddr[3] = 0x78;
            macaddr[4] = 0x9A; macaddr[5] = 0xBC;
            eep = FALSE;
        }
    }

    /* MACアドレスをコントローラにセット */
    if (!eep) {
        mac_wreg(MAC_ADDRL, (UW)macaddr[0] | ((UW)macaddr[1] << 8)
                     |((UW)macaddr[2]<<16) | ((UW)macaddr[3] << 24));
        mac_wreg(MAC_ADDRH, (UW)macaddr[4] | ((UW)macaddr[5] << 8));
    }

    if (bd_ini() == FALSE)        /* バッファディスクプリタの初期化 */
        return E_PAR;

    /* TX & RX enable */

    lan_wreg(RX_CFG, RCFG_EA);    /* End Align = 4 or 16 or 32 */
    lan_wreg(TX_CFG, TCFG_SDUMP|TCFG_DDUMP|TCFG_TXSAO|TCFG_TXON);

#if BUG_10BASE_POL
    lan.mac_cr = MAC_CR_FDPX|MAC_CR_PRMS;  /* for PHY LOOPBACK TEST */
    mac_wreg(MAC_CR, lan.mac_cr |= MAC_CR_TXEN|MAC_CR_RXEN);

    /* workaround for 10Base-T device bug */

    PHY_init(FALSE,FD10, 2);    /* Fixed 10M/Full */
    for (i = 0; i < 501; i++){  /* 任意回数 */
        for (j = 0; j < 20; j++){
            ercd = lan_tst_snd();
            if (ercd != E_OK)
                break;
            ercd = lan_tst_rcv();
            if (ercd != E_OK)
                break;
        }
        if (j == 20)  /* !!OK!! */
            break;
        else{         /* !!NG!! */
            PHY_reset();
            PHY_init(FALSE,FD10, 2);
            ercd = -1;
        }
    }
    if (ercd != E_OK)
        return ercd;
#endif

    /* PHY設定 */

#if FULLDPLX == 0
    mode = PHY_init(TRUE, AFD10|AFD100|AHD10|AHD100, lbtype);  /* AutoNego */
    lan.sts_new = mode;
    lan.sts_mod = mode = (mode == LAN_LNK_OFF) ? LAN_LNK_100F : mode;
#elif FULLDPLX == 2
    mode = PHY_init(FALSE,FD10,lbtype);   /* Fixed 10M/Full */
    lan.sts_new = mode;
    lan.sts_mod = mode = LAN_LNK_10F;
#elif FULLDPLX == 4
    mode = PHY_init(FALSE,FD100,lbtype);  /* Fixed 100M/Full */
    lan.sts_new = mode;
    lan.sts_mod = mode = LAN_LNK_100F;
#elif FULLDPLX == 1
    mode = PHY_init(FALSE,HD10,lbtype);   /* Fixed 10M/Half */
    lan.sts_new = mode;
    lan.sts_mod = mode = LAN_LNK_10H;
#elif FULLDPLX == 3
    mode = PHY_init(FALSE,HD100,lbtype);  /* Fixed 100M/Half */
    lan.sts_new = mode;
    lan.sts_mod = mode = LAN_LNK_100H;
#endif

    lan.lnk_chg = 0;
    lan.sts_old = LAN_LNK_OFF;

    lan_callback(lan.sts_new);            /* リンク状態通知コールバック */

    /* MAC_CR 再設定 */

    lan.mac_cr = 0;
    if (mode == LAN_LNK_100F || mode == LAN_LNK_10F)
        lan.mac_cr |= MAC_CR_FDPX;
    else
        lan.mac_cr |= MAC_CR_RCVOWN;
  #if ALMUL
    lan.mac_cr |= MAC_CR_MCPAS;
  #endif
  #if PRMS
    lan.mac_cr |= MAC_CR_PRMS;
  #endif
  #if LBTYPE == 1
    lan.mac_cr |= MAC_CR_LOOPBK;
  #endif
    mac_wreg(MAC_CR, lan.mac_cr |= MAC_CR_TXEN|MAC_CR_RXEN);

    /* 割込みマスクの初期設定 */

    lan_wreg(INT_STS, 0xffffffff);        /* clear all event */
    lan_wreg(IRQ_CFG, IRQ_EN|IRQ_BUFF);   /* IRQ-pin enable, IRQ buffer type */

    /* LANコントローラの割込みの登録 */
    ercd = lan_def_int();
    if (ercd != E_OK)
        return ercd;

    /* DMA初期化 */

  #if NO_DMA
  #else
    ercd = lan_dma_ini();
    if (ercd != E_OK)
        return ercd;
  #endif

#ifndef RX_MONITOR
  #if NO_DMA
    lan_wreg(INT_EN, lan.mask = INT_PHY);
  #else
    lan_wreg(INT_EN, lan.mask = INT_PHY|DEF_RXINT);
  #endif
#else
  #if NO_DMA
    lan_wreg(INT_EN, lan.mask = INT_PHY|INT_RXDF);
  #else
    lan_wreg(INT_EN, lan.mask = INT_PHY|INT_RXDF|DEF_RXINT);
  #endif
#endif
    return ercd;
}

/*****************************************************************************
* LANドライバの終了
*
******************************************************************************/

ER lan_ext_dev(void)
{
    lan_wreg(INT_EN, lan.mask = 0);
    lan_wreg(TX_CFG, lan_rreg(TX_CFG)|TCFG_STOP);  /* Stop TX */
    dly_tick(1);
    while (lan_rreg(TX_CFG) & TCFG_STOP) /* wait until Tx stop */
        dly_tsk(1);

    mac_wreg(MAC_CR, 0);  /* stop the controller */

    lan.rxtid = 0;
    lan.txtid = 0;

    return lan_undef_int();
}

/*****************************************************************************
* LAN コントローラ再初期化
*
******************************************************************************/

static void lan_restart(ER mode)
{
    if (mode == LAN_LNK_100F || mode == LAN_LNK_10F) {
        lan.mac_cr |= MAC_CR_FDPX;
        lan.mac_cr &= ~MAC_CR_RCVOWN;
    } else {
        lan.mac_cr &= ~MAC_CR_FDPX;
        lan.mac_cr |= MAC_CR_RCVOWN;
    }

    mac_wreg(MAC_CR, lan.mac_cr);
}

/*****************************************************************************
* LAN コントローラ初期化
*
* macaddr   MACアドレス(6バイトの配列)へのポインタ
******************************************************************************/

ER lan_ini(UB *macaddr)
{
    return lan_ini_dev(macaddr, LBTYPE);
}

/*****************************************************************************
* LINK状態変化時の処理
*
******************************************************************************/

static void lan_link(void)
{
    lan.sts_old = lan.sts_new;
    if (lan.lnk_chg) {
        lan.sts_new = LAN_LNK_OFF;  /* 処理遅延考慮で無条件にOFFに移行 */

        PHY_int_ack();  /* clear event */
        lan.lnk_chg = 0;
    }else{
        lan.sts_new = PHY_link();
    }

    if (!(lan.sts_new ^ lan.sts_old)){ /* same */
        return;
    }
    if (lan.sts_new == LAN_LNK_OFF){   /* LINK down */
        lan_callback(lan.sts_new);
        return;
    }

    switch(lan.sts_new){
    case LAN_LNK_100F:
    case LAN_LNK_10F:  /* FullDuplex */
        if (lan.sts_mod == LAN_LNK_100F || lan.sts_mod == LAN_LNK_10F){ /* no change duplex */
            lan.sts_mod = lan.sts_new;
            goto exit_wakeup;
        }
        break;
    case LAN_LNK_100H:
    case LAN_LNK_10H:  /* HalfDuplex */
        if (lan.sts_mod == LAN_LNK_100H || lan.sts_mod == LAN_LNK_10H){ /* no change duplex */
            lan.sts_mod = lan.sts_new;
            goto exit_wakeup;
        }
        break;
    }
    lan.sts_mod = lan.sts_new;

    /* stop sequence issue */
    /* re-start sequence issue */

    lan_restart(lan.sts_new);            /* re-start */
exit_wakeup:
    lan_callback(lan.sts_new);           /* リンク状態通知コールバック */
    EI_PHYINT();
}

#if NO_DMA
void lan_dma_intr(void)
{
    /* dummy function */
}
#else
/*****************************************************************************
* DMA転送再開(受信あれば)
*
******************************************************************************/

static void lan_dma_restart(void)
{
    UINT psw;
    UW len;

    psw = vdis_psw();
    DI_RXINT();         /* Rx-INT 禁止 */
    if (lan_dma_sts()){ /* DMA駆動中 */
        vset_psw(psw);  /* Restore INT-Level */
        return;
    }
    vset_psw(psw);

    /* FIFO available ? */

    while (lan_rreg(RX_FIFO_INF) & RINF_SUSED_MASK){
        if (cRxBD->csr.rx.ctl & RX_BD_RDY){
            dly_tick(3);
            len = cRxBD->csr.rx.stat = lan_rreg(RX_STAT);
            lan_wreg(INT_STS, DEF_RXINT);  /* clear event */
            len = cRxBD->csr.rx.len = (len & RX_STS_LEN_MASK) >> RX_STS_LEN_SHFT;
            if (ETHMINLEN <= len && len <= ETHMAXLEN){
                RMON(ERR_RXCNT);
                lan_indrep(cRxBD->buf,ALIGN(len, BUFF_ALIGN)); /* re-start */
                return;
            }

            RMON(ERR_TOOLG);
            if (len < (4 * sizeof(long))){
                len = ALIGN(len, 4);
                for (; len != 0; len-=4)
                    lan_rreg(RX_DATA);  /* dummy read for discard */
            }else{
                lan_wreg(RX_DP_CTL, RXDP_FFWD);  /* skip ON */
                dly_tick(1);
                while (lan_rreg(RX_DP_CTL) & RXDP_FFWD);  /* wait until complete(about 20[usec]) */
            }
        }
        else{  /* BD空きなし */
            RMON(ERR_BDOV);
            return;
        }
    }
    EI_RXINT();
}

/*****************************************************************************
* DMA interrupt service routine
*
******************************************************************************/

void lan_dma_intr(VP_INT exinf)
{
    UW len;
    ID id;

  #if (defined(BIG_ENDIAN) && !BUS_SWAP)
    int i;
    UB *p, *q, pq;
  #endif

    cRxBD->csr.rx.ctl &= ~RX_BD_RDY;  /* Data is available */

  #if (defined(BIG_ENDIAN) && !BUS_SWAP)
    p = (UB *)cRxBD->buf;
  #endif

    /* データ並び不一致なら、並び替え */
  #if (defined(BIG_ENDIAN) && !BUS_SWAP)
    q = p - 2;
    for (i = ALIGN(cRxBD->csr.rx.len, BUFF_ALIGN); i > 0; i -= 4) {
        q += 5;
        pq = *q;
        *q-- = *p;
        *p++ = pq;
        pq = *q;
        *q = *p;
        *p = pq;
        p += 3;
    }
  #endif

    cRxBD = cRxBD->next;

    if ((id = lan.rxtid) != FALSE) {  /* 受信割込み待ち中のタスクあるなら */
        lan.rxtid = FALSE;
        wup_tsk(id);                  /* その(IP受信)タスクを起床 */
    }
    lan_dma_clr();  /* clear INT status */

    /* FIFO available ? */

    while (lan_rreg(RX_FIFO_INF) & RINF_SUSED_MASK){
        if (cRxBD->csr.rx.ctl & RX_BD_RDY){
            dly_tick(3);
            len = cRxBD->csr.rx.stat = lan_rreg(RX_STAT);
            lan_wreg(INT_STS, DEF_RXINT);  /* clear event */
            len = cRxBD->csr.rx.len = (len & RX_STS_LEN_MASK) >> RX_STS_LEN_SHFT;
            if (ETHMINLEN <= len && len <= ETHMAXLEN){
                RMON(ERR_RXCNT);
                lan_indrep(cRxBD->buf,ALIGN(len, BUFF_ALIGN)); /* re-start */
                return;
            }

            RMON(ERR_TOOLG);
            if (len < (4 * sizeof(long))){
                len = ALIGN(len, 4);
                for (; len != 0; len-=4)
                    lan_rreg(RX_DATA);  /* dummy read for discard */
            }else{
                lan_wreg(RX_DP_CTL, RXDP_FFWD);  /* skip ON */
                dly_tick(1);
                while (lan_rreg(RX_DP_CTL) & RXDP_FFWD);
            }
        }
        else{  /* BD空きなし */
            RMON(ERR_BDOV);
            return;
        }
    }
    EI_RXINT();
}
#endif

/*****************************************************************************
* 送受信割込みサービスルーチン
*
******************************************************************************/

void lan_intr(VP_INT exinf)
{
    UW len;
    UW ist, mask;
    ID id;

    dly_tick(1);               /* read after INT_EN write delay(高速プロセッサ考慮) */
    mask = lan_rreg(INT_EN);
    if (!mask)                 /* 割込み禁止レイテンシ対策(valid after write delay) */
        return;

    ist = lan_rreg(INT_STS);   /* get interrupt status */
    ist &= mask;

  #ifdef RX_MONITOR
    if (ist & INT_RXDF){
        lan.err_count[ERR_DROP] += lan_rreg(RX_DROP);
    }
  #endif

    if (ist & DEF_RXINT) {
  #if NO_DMA
        if ((id = lan.rxtid) != FALSE) {  /* 受信割込み待ち中のタスクあるなら */
            lan.rxtid = FALSE;
            wup_tsk(id);                  /* その(IP受信)タスクを起床 */
        }
        mask &= ~DEF_RXINT;
  #else
        if (lan_rreg(RX_FIFO_INF) & RINF_SUSED_MASK){
            RMON(ERR_RXINT);

            if (cRxBD->csr.rx.ctl & RX_BD_RDY){   /* BD有効 */
                dly_tick(3);
                len = cRxBD->csr.rx.stat = lan_rreg(RX_STAT);
                len = cRxBD->csr.rx.len = (len & RX_STS_LEN_MASK) >> RX_STS_LEN_SHFT;
                if (ETHMINLEN <= len && len <= ETHMAXLEN){
                    RMON(ERR_RXCNT);
                    mask &= ~DEF_RXINT;
                    lan_indrep(cRxBD->buf,ALIGN(len, BUFF_ALIGN)); /* start DMA */
                }
                else{
                    RMON(ERR_TOOLG);
                    if (len < (4 * sizeof(long))){
                        len = ALIGN(len, 4);
                        for (; len != 0; len-=4)
                            lan_rreg(RX_DATA);  /* dummy read for discard */
                    }else{
                        lan_wreg(RX_DP_CTL, RXDP_FFWD);  /* skip ON */
                        dly_tick(1);
                        while (lan_rreg(RX_DP_CTL) & RXDP_FFWD);
                    }
                }
            }
            else{  /* BD空きなし */
                RMON(ERR_BDOV);
                if ((id = lan.rxtid) != FALSE) {  /* 受信割込み待ち中のタスクあるなら */
                    lan.rxtid = FALSE;
                    wup_tsk(id);                  /* その(IP受信)タスクを起床 */
                }
                mask &= ~DEF_RXINT;
            }
        }
  #endif
    }

    /* 送信割込み */

    if (ist & DEF_TXINT) {                /* transmit interrupt */
        if ((id = lan.txtid) != FALSE) {  /* 送信割込み待ち中のタスクあるなら */
            lan.txtid = FALSE;
            wup_tsk(id);                  /* その(IP送信)タスクを起床 */
        }
        mask &= ~DEF_TXINT;
    }

    /* PHY events (there use only LINK status) */

    if (ist & INT_PHY) {
        lan.lnk_chg = 1;
        if ((id = lan.rxtid) != FALSE) {  /* 受信割込み待ち中のタスクあるなら */
            lan.rxtid = FALSE;
            wup_tsk(id);                  /* その(IP受信)タスクを起床 */
        }
        mask &= ~INT_PHY;
    }
    lan_wreg(INT_EN, lan.mask = mask);
    lan_wreg(INT_STS, ist);  /* clear event */
}

#if NO_DMA
/*****************************************************************************
* Get FIFO data
*
******************************************************************************/

static void get_rfifo(void)
{
    UW len;

    /* FIFO available ? */

    while (lan_rreg(RX_FIFO_INF) & RINF_SUSED_MASK){
        if (cRxBD->csr.rx.ctl & RX_BD_RDY){
            dly_tick(3);
            len = cRxBD->csr.rx.stat = lan_rreg(RX_STAT);
            lan_wreg(INT_STS, DEF_RXINT);  /* clear event */
            len = cRxBD->csr.rx.len = (len & RX_STS_LEN_MASK) >> RX_STS_LEN_SHFT;
            if (ETHMINLEN <= len && len <= ETHMAXLEN){
                RMON(ERR_RXCNT);
                lan_indrep((UW *)cRxBD->buf, ALIGN(len, BUFF_ALIGN));
                cRxBD->csr.rx.ctl &= ~RX_BD_RDY;  /* Data is available */
                cRxBD = cRxBD->next;
            }
            else{
                RMON(ERR_TOOLG);
                if (len < (4 * sizeof(long))){
                    len = ALIGN(len, 4);
                    for (; len != 0; len-=4)
                        lan_rreg(RX_DATA);  /* dummy read for discard */
                }else{
                    lan_wreg(RX_DP_CTL, RXDP_FFWD);  /* skip ON */
                    dly_tick(1);
                    while (lan_rreg(RX_DP_CTL) & RXDP_FFWD);
                }
            }
        }
        else{  /* BD空きなし */
            RMON(ERR_BDOV);
            break;
        }
    }
}
#endif

/*****************************************************************************
* 受信割込み待ち
*
******************************************************************************/

ER lan_wai_rcv(TMO tmout)
{
    ER ercd = E_OK;
    TMO delay;

    for (;;){
  #if NO_DMA
        get_rfifo();     /* RX-FIFO読出し */
  #endif

        if (!(pRxBD->csr.rx.ctl & RX_BD_RDY) && !lan.lnk_chg){
                                              /* LINK Down検出ならそれを優先する */
            if (!(pRxBD->csr.rx.stat & RX_STS_ES)){
                if (pRxBD->csr.rx.len >= 4)
                    pRxBD->csr.rx.len -= 4;  /* CRC分引く */

                lan.rxbd  = pRxBD;       /* set current RxBD for working */
                lan.rxbuf = pRxBD->buf;  /* set current buffer for working */
                pRxBD = pRxBD->next;     /* set next BD to current */
                return E_OK;
            }
            RMON(ERR_ERSTAT);

            pRxBD->csr.rx.ctl = RX_BD_RDY;
            pRxBD = pRxBD->next;
            RX_RESTART();
            continue;
        }

        lan.rxtid = (UH)vget_tid();

        if (lan.lnk_chg || lan.sts_new == LAN_LNK_OFF)
            lan_link();

        /* LINK UP はポーリングで監視 */
        if (lan.sts_new == LAN_LNK_OFF){
            if (tmout == TMO_FEVR){
                delay = LINK_TIME;
            }else{
                if (!tmout)
                    return E_TMOUT;
                delay = (tmout < LINK_TIME) ? tmout : LINK_TIME;
                tmout -= delay;
            }
        }else{
            delay = tmout;
        }

        /* 受信待ち */

  #if NO_DMA
        EI_RXINT();                     /* 受信割込み許可 */
  #else
        if (!(pRxBD->csr.rx.ctl & RX_BD_RDY)){ /* rxtid取得前に受信INTあれば */
            lan.rxtid = FALSE;
            continue;                          /* やり直し */
        }
  #endif
        ercd = tslp_tsk(delay);         /* 受信割込みからの起床待ち */
  #if NO_DMA
        DI_RXINT();                     /* 受信割込み禁止 */
  #endif
        lan.rxtid = FALSE;
        vcan_wup();

        if (!(ercd == E_OK || ercd == E_TMOUT))
            break;
    }
    return ercd;
}

/*****************************************************************************
* 送信割込み待ち
*
******************************************************************************/

ER lan_wai_snd(TMO tmout)
{
    ER ercd = E_OK;

    for (;;){
        if ((lan_rreg(TX_FIFO_INF) & TINF_DFREE_MASK) >= (ETHMAXLEN+8)){  /* FIFOに空きあり */
            /* 待ち要因ある場合は今後のTCP/IP仕様考慮でlan_wai_snd()に集約する。
             * でも、ここでは、length分からないので、最大長でチェックする。
             * 2006.11.27 [OK]
             */
            if (!(pTxBD->csr.tx.cmd_b & TX_CMDB_TAG_MASK)){  /* Tx OK */
                lan.txtag++;
                if (!lan.txtag)
                    lan.txtag++;
                pTxBD->csr.tx.cmd_b = ((UW)lan.txtag) <<16;  /* BD is not empty */
                lan.txbd  = pTxBD;
                lan.txbuf = pTxBD->buf;
                pTxBD = pTxBD->next;
                return E_OK;
            }
        }

        lan.txtid = (UH)vget_tid();
        EI_TXINT();
        ercd = tslp_tsk(tmout);
        DI_TXINT();
        lan.txtid = FALSE;
        vcan_wup();
        if (ercd != E_OK)
            break;
    }
    return ercd;
}

/*****************************************************************************
* 受信パケット長を得る
*
******************************************************************************/

ER lan_get_len(UH *len)
{
    *len = lan.rxbd->csr.rx.len;
    return E_OK;
}

/*****************************************************************************
* 受信パケット読み出し
*
* buf   読み出すためのバッファ
* len   読み出すバイト数
******************************************************************************/

ER lan_get_pkt(void *buf, int len)
{
    memcpy(buf, lan.rxbuf, len);
    lan.rxbuf += len;
    return E_OK;
}

/*****************************************************************************
* 受信パケット読み出し終了
*
******************************************************************************/

ER lan_get_end(void)
{
    lan.rxbd->csr.rx.ctl = RX_BD_RDY;
    RX_RESTART();
    return E_OK;
}

/*****************************************************************************
* 受信パケット読み飛ばし
*
* len   受信パケットの未読バイト数
******************************************************************************/

ER lan_skp_pkt(int len)
{
    return lan_get_end();
}

/*****************************************************************************
* 送信パケット長を設定
*
******************************************************************************/

ER lan_set_len(int len)
{
    /* Transfer Command-A */
    lan.txbd->csr.tx.cmd_a = len |(TX_CMDA_IOC|TX_CMDA_EA|TX_CMDA_F|TX_CMDA_L);

    /* Transfer Command-B */
    lan.txbd->csr.tx.cmd_b |= len;

    return E_OK;
}

/*****************************************************************************
* 送信パケット書き込み
*
* data      書き込むデータ
* len       データのバイト数（> 0）
******************************************************************************/

ER lan_put_pkt(const void *data, int len)
{
    memcpy(lan.txbuf, data, len);
    lan.txbuf += len;
    return E_OK;
}

/*****************************************************************************
* 送信パケットが60バイト未満の場合のダミー書き込み
*
* len       ダミーデータのバイト数（> 0）
******************************************************************************/

ER lan_put_dmy(int len)
{
    memset(lan.txbuf, 0, len);
    lan.txbuf += len;

    return E_OK;
}

/*****************************************************************************
* 送信パケット書き込み終了
*
******************************************************************************/

ER lan_put_end(void)
{
    UW len;

    if (lan.sts_new == LAN_LNK_OFF){  /* Discarded if LAN is LINK Down */
        lan.txbd->csr.tx.cmd_b &= ~TX_CMDB_TAG_MASK; /* BD ready */
        return E_OK;
    }

    len = lan.txbd->csr.tx.cmd_b & TX_CMDB_LEN_MASK;

    /* Set from buffer to TX-FIFO */

    lan_wreg(TX_DATA, lan.txbd->csr.tx.cmd_a);
    lan_wreg(TX_DATA, lan.txbd->csr.tx.cmd_b);
    lan_outdrep((UW *)lan.txbd->buf, ALIGN(len, BUFF_ALIGN));  /* Transfer data */

    lan.txbd->csr.tx.cmd_b &= ~TX_CMDB_TAG_MASK; /* BD ready */
    return E_OK;
}

/*****************************************************************************
* コールバック関数登録
*
******************************************************************************/

ER lan_def_cbk(FP func)
{
    lan.callback = func;
    callbackini = func;
    return E_OK;
}

#if BUG_10BASE_POL
/*****************************************************************************
* テストフレーム送信 for LOOPBACK
*
******************************************************************************/

ER lan_tst_snd(void)
{
    UW i,d;
    UW len = 1512;  /* maximum by align 4 */

    /* Transfer Command-A */
    lan_wreg(TX_DATA, len |(TX_CMDA_EA|TX_CMDA_F|TX_CMDA_L));

    /* Transfer Command-B */
    lan_wreg(TX_DATA, len);

    /* Transfer Data */
    d = 0x00010203;
    len = ALIGN(len, BUFF_ALIGN);
    for (i = 0; i < len/4; i++, d += 0x04040404)
        lan_outd(TX_DATA, d);

    return E_OK;
}

/*****************************************************************************
* テストフレーム受信 for LOOPBACK
*
******************************************************************************/

ER lan_tst_rcv(void)
{
    UW i,d;
    UW len;
    UW len2;
    ER ercd = E_OK;

    while (!(lan_rreg(RX_FIFO_INF) & RINF_SUSED_MASK));
    dly_tick(3);

    len = lan_rreg(RX_STAT);
    len = (len & RX_STS_LEN_MASK) >> RX_STS_LEN_SHFT;
    if (len != 1512+4){
        if (len < (4 * sizeof(long))){
            len = ALIGN(len, 4);
            for (; len != 0; len-=4)
                lan_ind(RX_DATA);  /* dummy read for discard */
        }else{
            lan_wreg(RX_DP_CTL, RXDP_FFWD);  /* skip ON */
            dly_tick(1);
            while (lan_rreg(RX_DP_CTL) & RXDP_FFWD);
        }
        return -1;
    }

    len2 = ALIGN(len, BUFF_ALIGN) - len;
    len -=4;
    d = 0x00010203;
    for (i = 0; i < len/4; i++, d += 0x04040404)
        if (lan_ind(RX_DATA) != d)
            ercd = -1;

    lan_ind(RX_DATA);  /* discard FCS */

    len = ALIGN(len2, 4);
    for (; len != 0; len-=4)
        lan_rreg(RX_DATA);  /* dummy read for discard */

    return ercd;
}
#endif

/*****************************************************************************
* シリアルEEPROM 1バイト読み出し
*
* シリアルEEPROMから1バイトのデータを読み出す。
* 読み出し成功でE_OK、失敗でE_TMOUTを返す。
******************************************************************************/

ER lan_get_eep(UB addr,       /* EEPROM WORD ADDRESS */
               UB *data)      /* 読み出したデータ */
{
    UW sts;

    while (lan_rreg(E2P_CMD) & E2P_BUSY);
    lan_wreg(E2P_CMD, E2P_BUSY|E2P_READ|(UW)addr);
//    dly_tick(1);
    dly_tick(10);
    while ((sts = lan_rreg(E2P_CMD)) & E2P_BUSY);
    if (sts & E2P_TMO)
        return E_TMOUT;
    else
        *data = (UB)lan_rreg(E2P_DATA);

    return E_OK;
}

/*****************************************************************************
* シリアルEEPROM 1バイト書き込み
*
* シリアルEEPROMに1バイトのデータを書き込む。
* 書き込み成功でE_OK、失敗でE_TMOUTを返す。
******************************************************************************/

ER lan_set_eep(UB addr,  /* EEPROM WORD ADDRESS */
               UB data)  /* 書き込むデータ */
{
    ER ercd = E_OK;
    UW sts;

    while (lan_rreg(E2P_CMD) & E2P_BUSY);
    lan_wreg(E2P_CMD, E2P_BUSY|E2P_EWEN);  /* write enable */
//    dly_tick(1);
    dly_tick(10);
    while (lan_rreg(E2P_CMD) & E2P_BUSY);

    lan_wreg(E2P_DATA, (UW)data);
    lan_wreg(E2P_CMD, E2P_BUSY|E2P_WRITE|(UW)addr);
//    dly_tick(1);
    dly_tick(10);
    while ((sts = lan_rreg(E2P_CMD)) & E2P_BUSY);
    if (sts & E2P_TMO)
        ercd = E_TMOUT;

    lan_wreg(E2P_CMD, E2P_BUSY|E2P_EWDS);  /* write disable */
    return ercd;
}

/*****************************************************************************
* MACアドレス読み出し
*
* シリアルEEPROMからMACアドレスを読み出す。
* 読み出し成功でE_OK、失敗で(-1)を返す。
******************************************************************************/

ER lan_get_mac(UB *macaddr)
{
    UB d[7];

    if ((lan_get_eep(0x00, &d[0]) != E_OK)
     || (lan_get_eep(0x01, &d[1]) != E_OK)
     || (lan_get_eep(0x02, &d[2]) != E_OK)
     || (lan_get_eep(0x03, &d[3]) != E_OK)
     || (lan_get_eep(0x04, &d[4]) != E_OK)
     || (lan_get_eep(0x05, &d[5]) != E_OK)
     || (lan_get_eep(0x06, &d[6]) != E_OK))
        return -1;

    if (d[0] != 0xa5)  /* not save */
        return -1;

    macaddr[0] = d[1];
    macaddr[1] = d[2];
    macaddr[2] = d[3];
    macaddr[3] = d[4];
    macaddr[4] = d[5];
    macaddr[5] = d[6];
    return E_OK;
}

/*****************************************************************************
* MACアドレス書き込み
*
* シリアルEEPROMにMACアドレスを書き込む。
* 書き込み成功でE_OK、失敗で(-1)を返す。
******************************************************************************/

ER lan_set_mac(UB *macaddr)
{
    UB *d = macaddr;

    if ((lan_set_eep(0x00, 0xa5) != E_OK)
     || (lan_set_eep(0x01, d[0]) != E_OK)
     || (lan_set_eep(0x02, d[1]) != E_OK)
     || (lan_set_eep(0x03, d[2]) != E_OK)
     || (lan_set_eep(0x04, d[3]) != E_OK)
     || (lan_set_eep(0x05, d[4]) != E_OK)
     || (lan_set_eep(0x06, d[5]) != E_OK))
        return -1;
    return E_OK;
}

/* end */
