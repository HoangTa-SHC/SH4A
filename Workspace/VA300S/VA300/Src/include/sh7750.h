/******************************************************************************
* SH7750 制御レジスタ定義                                                     *
*                                                                             *
*  File name : sh7750.h                                                       *
*  Copyright (c)  1995-2000, MiSPO Co., Ltd.                                  *
*  All rights reserved.                                                       *
*                                                                             *
* 01/Feb/1999 created                                                         *
* 06/Dec/1999 割込み番号の不足分を追加                                        *
* 16/Apr/2000                                                                 *
* 16/Jan/2001 SCI_SCSPRT1 -> SCI_SCSPTR1                                      *
******************************************************************************/

#ifndef SH7750_H
#define SH7750_H

/* 制御レジスタのベースアドレス */

#if (!defined(M_I86)&&!defined(__TURBOC__))
#define IO_BASE ((volatile unsigned char *)0xff000000)
#endif

/* 制御レジスタ入出力マクロ */

unsigned char sfr_in(int n);                /* バイトデータの入力 */
void sfr_out(int n, unsigned char c);       /* バイトデータの出力 */
unsigned short sfr_inw(int n);              /* ワードデータの入力 */
void sfr_outw(int n, unsigned short c);     /* ワードデータの出力 */
unsigned long sfr_inl(int n);               /* ロングワードデータの入力 */
void sfr_outl(int n, unsigned long c);      /* ロングワードデータの出力 */
void sfr_set(int n, unsigned char c);       /* バイトデータのビットセット */
void sfr_clr(int n, unsigned char c);       /* バイトデータのビットクリア */
void sfr_setw(int n, unsigned short c);     /* ワードデータのビットセット */
void sfr_clrw(int n, unsigned short c);     /* ワードデータのビットクリア */

#ifdef IO_BASE
#define sfr_in(n)       (*(IO_BASE+(n)))
#define sfr_out(n,c)    (*(IO_BASE+(n))=(c))
#define sfr_inw(n)      (*(volatile unsigned short *)(IO_BASE+(n)))
#define sfr_outw(n,c)   (*(volatile unsigned short *)(IO_BASE+(n))=(c))
#define sfr_inl(n)      (*(volatile unsigned long *)(IO_BASE+(n)))
#define sfr_outl(n,c)   (*(volatile unsigned long *)(IO_BASE+(n))=(c))
#define sfr_set(n,c)    (*(IO_BASE+(n))|=(c))
#define sfr_clr(n,c)    (*(IO_BASE+(n))&=~(c))
#define sfr_setw(n,c)   (*(volatile unsigned short *)(IO_BASE+(n))|=(c))
#define sfr_clrw(n,c)   (*(volatile unsigned short *)(IO_BASE+(n))&=~(c))
/// 応用電機追加--->
#define sfr_clrl(n,c)   (*(volatile unsigned long *)(IO_BASE+(n))&=~(c))
#define sfr_setl(n,c)   (*(volatile unsigned long *)(IO_BASE+(n))|=(c))
/// <---応用電機追加
#endif

/* 制御レジスタのオフセットアドレス */

#define CCN_PTEH        0x000000    /* ページテーブルエントリ上位レジスタ */
#define CCN_PTEL        0x000004    /* ページテーブルエントリ下位レジスタ */
#define CCN_TTB         0x000008    /* 変換テーブルベースレジスタ */
#define CCN_TEA         0x00000c    /* TLB 例外アドレスレジスタ */
#define CCN_MMUCR       0x000010    /* MMU 制御レジスタ */
#define CCN_BASRA       0x000014    /* ブレーク ASID レジスタA */
#define CCN_BASRB       0x000018    /* ブレーク ASID レジスタB */
#define CCN_CCR         0x00001c    /* キャッシュ制御レジスタ */
#define CCN_TRA         0x000020    /* TRAPA 例外レジスタ */
#define CCN_EXPEVT      0x000024    /* 例外事象レジスタ */
#define CCN_INTEVT      0x000028    /* 割り込み事象レジスタ */
#define CCN_PTEA        0x000034    /* ページテーブルエントリアシスタンスレジスタ */
#define CCN_QACR0       0x000038    /* キューアドレス制御レジスタ0 */
#define CCN_QACR1       0x00003c    /* キューアドレス制御レジスタ1 */

#define UBC_BARA        0x200000    /* ブレークアドレスレジスタA */
#define UBC_BAMRA       0x200004    /* ブレークアドレスマスクレジスタA */
#define UBC_BBRA        0x200008    /* ブレークバスサイクルレジスタA */
#define UBC_BARB        0x20000c    /* ブレークアドレスレジスタB */
#define UBC_BAMRB       0x200010    /* ブレークアドレスマスクレジスタB */
#define UBC_BBRB        0x200014    /* ブレークバスサイクルレジスタB */
#define UBC_BDRB        0x200018    /* ブレークデータレジスタB */
#define UBC_BDMRB       0x20001c    /* ブレークデータマスクレジスタB */
#define UBC_BRCR        0x200020    /* ブレークコントロールレジスタ */

#define BSC_BCR1        0x800000    /* バスコントロールレジスタ1 */
#define BSC_BCR2        0x800004    /* バスコントロールレジスタ2 */
#define BSC_WCR1        0x800008    /* ウェイトステートコントロールレジスタ1 */
#define BSC_WCR2        0x80000c    /* ウェイトステートコントロールレジスタ2 */
#define BSC_WCR3        0x800010    /* ウェイトステートコントロールレジスタ3 */
#define BSC_MCR         0x800014    /* 個別メモリコントロールレジスタ */
#define BSC_PCR         0x800018    /* PCMCIA レジスタ */
#define BSC_RTCSR       0x80001c    /* リフレッシュタイマコントロール/ステータスレジスタ */
#define BSC_RTCNT       0x800020    /* リフレッシュタイマカウンタ */
#define BSC_RTCOR       0x800024    /* リフレッシュタイムコンスタントカウンタ */
#define BSC_RFCR        0x800028    /* リフレッシュカウントレジスタ */
#define BSC_PCTRA       0x80002c    /* ポートコントールレジスタA */
#define BSC_PDTRA       0x800030    /* ポートデータレジスタA */
#define BSC_PCTRB       0x800040    /* ポートコントールレジスタB */
#define BSC_PDTRB       0x800044    /* ポートデータレジスタB */
#define BSC_GPIOIC      0x800048    /* GPIO 割込みコントロールレジスタ */
#define BSC_SDMR2       0x900000    /* シンクロナスDRAM モードレジスタ2 */
#define BSC_SDMR3       0x940000    /* シンクロナスDRAM モードレジスタ3 */

#define DMAC_SAR0       0xa00000    /* DMA ソースアドレスレジスタ0 */
#define DMAC_DAR0       0xa00004    /* DMA デスティネーションアドレスレジスタ0 */
#define DMAC_DMATCR0    0xa00008    /* DMA トランスファカウントレジスタ0 */
#define DMAC_CHCR0      0xa0000c    /* DMA チャンネルコントロールレジスタ0 */
#define DMAC_SAR1       0xa00010    /* DMA ソースアドレスレジスタ1 */
#define DMAC_DAR1       0xa00014    /* DMA デスティネーションアドレスレジスタ1 */
#define DMAC_DMATCR1    0xa00018    /* DMA トランスファカウントレジスタ1 */
#define DMAC_CHCR1      0xa0001c    /* DMA チャンネルコントロールレジスタ1 */
#define DMAC_SAR2       0xa00020    /* DMA ソースアドレスレジスタ2 */
#define DMAC_DAR2       0xa00024    /* DMA デスティネーションアドレスレジスタ2 */
#define DMAC_DMATCR2    0xa00028    /* DMA トランスファカウントレジスタ2 */
#define DMAC_CHCR2      0xa0002c    /* DMA チャンネルコントロールレジスタ2 */
#define DMAC_SAR3       0xa00030    /* DMA ソースアドレスレジスタ3 */
#define DMAC_DAR3       0xa00034    /* DMA デスティネーションアドレスレジスタ3 */
#define DMAC_DMATCR3    0xa00038    /* DMA トランスファカウントレジスタ3 */
#define DMAC_CHCR3      0xa0003c    /* DMA チャンネルコントロールレジスタ3 */
#define DMAC_DMATOR     0xa00040    /* DMA オペレーションレジスタ */

#define CPG_FRQCR       0xc00000    /* 周波数制御レジスタ */
#define CPG_STBCR       0xc00004    /* スタンバイコントロールレジスタ */
#define CPG_WTCNT       0xc00008    /* ウォッチドッグタイマカウンタ */
#define CPG_WTCSR       0xc0000c    /* ウォッチドッグタイマコントロール/ステータスレジスタ */
#define CPG_STBCR2      0xc00010    /* スタンバイコントロールレジスタ2 */

#define RTC_R64CNT      0xc80000    /* 64Hz カウンタ */
#define RTC_RSECCNT     0xc80004    /* 秒カウンタ */
#define RTC_RMINCNT     0xc80008    /* 分カウンタ */
#define RTC_RHRCNT      0xc8000c    /* 時カウンタ */
#define RTC_RWKCNT      0xc80010    /* 曜日カウンタ */
#define RTC_RDAYCNT     0xc80014    /* 日カウンタ */
#define RTC_RMONCNT     0xc80018    /* 月カウンタ */
#define RTC_RYRCNT      0xc8001c    /* 年カウンタ */
#define RTC_RSECAR      0xc80020    /* 秒アラームレジスタ */
#define RTC_RMINAR      0xc80024    /* 分アラームレジスタ */
#define RTC_RHRAR       0xc80028    /* 時アラームレジスタ */
#define RTC_RWKAR       0xc8002c    /* 曜日アラームレジスタ */
#define RTC_RDAYAR      0xc80030    /* 日アラームレジスタ */
#define RTC_RMONAR      0xc80034    /* 月アラームレジスタ */
#define RTC_RCR1        0xc80038    /* RTC コントロールレジスタ1 */
#define RTC_RCR2        0xc8003c    /* RTC コントロールレジスタ2 */

#define INTC_ICR        0xd00000    /* 割り込みコントロールレジスタ */
#define INTC_IPRA       0xd00004    /* 割り込み優先レベル設定レジスタA */
#define INTC_IPRB       0xd00008    /* 割り込み優先レベル設定レジスタB */
#define INTC_IPRC       0xd0000c    /* 割り込み優先レベル設定レジスタC */
#define INTC_IPRD       0xd00010    /* 割り込み優先レベル設定レジスタD */

#define TMU_TOCR        0xd80000    /* タイマアウトプットコントロールレジスタ */
#define TMU_TSTR        0xd80004    /* タイマスタートレジスタ */
#define TMU_TCOR0       0xd80008    /* タイマコンスタントレジスタ0 */
#define TMU_TCNT0       0xd8000c    /* タイマカウンタ0 */
#define TMU_TCR0        0xd80010    /* タイマコントロールレジスタ0 */
#define TMU_TCOR1       0xd80014    /* タイマコンスタントレジスタ1 */
#define TMU_TCNT1       0xd80018    /* タイマカウンタ1 */
#define TMU_TCR1        0xd8001c    /* タイマコントロールレジスタ1 */
#define TMU_TCOR2       0xd80020    /* タイマコンスタントレジスタ2 */
#define TMU_TCNT2       0xd80024    /* タイマカウンタ2 */
#define TMU_TCR2        0xd80028    /* タイマコントロールレジスタ2 */
#define TMU_TCPR2       0xd8002c    /* インプットキャプチャレジスタ */

#define SCI_SCSMR1      0xe00000    /* シリアルモードレジスタ */
#define SCI_SCBRR1      0xe00004    /* ビットレートレジスタ */
#define SCI_SCSCR1      0xe00008    /* シリアルコントロールレジスタ */
#define SCI_SCTDR1      0xe0000c    /* トランスミットデータレジスタ */
#define SCI_SCSSR1      0xe00010    /* シリアルステータスレジスタ */
#define SCI_SCRDR1      0xe00014    /* レシーブデータレジスタ */
#define SCI_SCSCMR1     0xe00018    /* スマートカードモードレジスタ */
#define SCI_SCSPTR1     0xe0001c    /* シリアルポートレジスタ */

#define SCIF_SCSMR2     0xe80000    /* シリアルモードレジスタ */
#define SCIF_SCBRR2     0xe80004    /* ビットレートレジスタ */
#define SCIF_SCSCR2     0xe80008    /* シリアルコントロールレジスタ */
#define SCIF_SCFTDR2    0xe8000c    /* トランスミットFIFOデータレジスタ */
#define SCIF_SCFSR2     0xe80010    /* シリアルステータスレジスタ */
#define SCIF_SCFRDR2    0xe80014    /* レシーブFIFOデータレジスタ */
#define SCIF_SCFCR2     0xe80018    /* FIFOコントロールレジスタ */
#define SCIF_SCFDR2     0xe8001c    /* FIFOデータ数レジスタ */
#define SCIF_SCSPTR2    0xe80020    /* シリアルポートレジスタ */
#define SCIF_SCLSR2     0xe80024    /* ラインステータスレジスタ */

    /* 割り込み番号（例外コード÷32）*/

#define INT_NMI         14          /* ノンマスカブル割り込み */
#define INT_IRL15       16          /* レベル15外部ハードウェア割り込み */
#define INT_IRL14       17          /* レベル14外部ハードウェア割り込み */
#define INT_IRL13       18          /* レベル13外部ハードウェア割り込み */
#define INT_IRL12       19          /* レベル12外部ハードウェア割り込み */
#define INT_IRL11       20          /* レベル11外部ハードウェア割り込み */
#define INT_IRL10       21          /* レベル10外部ハードウェア割り込み */
#define INT_IRL9        22          /* レベル9 外部ハードウェア割り込み */
#define INT_IRL8        23          /* レベル8 外部ハードウェア割り込み */
#define INT_IRL7        24          /* レベル7 外部ハードウェア割り込み */
#define INT_IRL6        25          /* レベル6 外部ハードウェア割り込み */
#define INT_IRL5        26          /* レベル5 外部ハードウェア割り込み */
#define INT_IRL4        27          /* レベル4 外部ハードウェア割り込み */
#define INT_IRL3        28          /* レベル3 外部ハードウェア割り込み */
#define INT_IRL2        29          /* レベル2 外部ハードウェア割り込み */
#define INT_IRL1        30          /* レベル1 外部ハードウェア割り込み */
#define INT_TUNI0       32          /* TMU0 アンダーフロー割り込み */
#define INT_TUNI1       33          /* TMU1 アンダーフロー割り込み */
#define INT_TUNI2       34          /* TMU2 アンダーフロー割り込み */
#define INT_TICPI2      35          /* TMU2 インプットキャプチャ割り込み */
#define INT_ATI         36          /* RTC アラーム割り込み */
#define INT_PRI         37          /* RTC 周期割り込み */
#define INT_CUI         38          /* RTC 桁上げ割り込み */
#define INT_ERI         39          /* SCI 受信エラー割り込み */
#define INT_RXI         40          /* SCI 受信データフル割り込み */
#define INT_TXI         41          /* SCI 送信データフル割り込み */
#define INT_TEI         42          /* SCI 送信データエンプティ割り込み */
#define INT_ITI         43          /* WDT インターバルタイマ割り込み */
#define INT_RCMI        44          /* REF コンペアマッチ割り込み */
#define INT_ROVI        45          /* REF リフレッシュカウントオーバフロー割り込み */
#define INT_HUDI        48          /* HUDI 割り込み */
#define INT_DMTE0       50          /* DMA CH0 転送完了割り込み */
#define INT_DMTE1       51          /* DMA CH1 転送完了割り込み */
#define INT_DMTE2       52          /* DMA CH2 転送完了割り込み */
#define INT_DMTE3       53          /* DMA CH3 転送完了割り込み */
#define INT_DMAE        54          /* DMA アドレスエラー割り込み */
#define INT_FERI        56          /* SCIF 受信エラー割り込み */
#define INT_FRXI        57          /* SCIF 受信データフル割り込み */
#define INT_FBRI        58          /* SCIF ブレーク割り込み */
#define INT_FTXI        59          /* SCIF 送信データフル割り込み */

#endif /* SH7750_H */
