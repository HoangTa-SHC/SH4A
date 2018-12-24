/****************************************************************************
* インターバル割込みハンドラ（MS7750SE01/Computex SH7750 EVA BOD)           *
*                                                                           *
*  File name : n4i7750.c                                                    *
*  Copyright (c)  1995-2001, MiSPO Co., Ltd.                                *
*  All rights reserved.                                                     *
*                                                                           *
*  Compile : shc -cp=sh4 -op=1 -def=CLK=?,CH=? n4i7750.c        (Renesas C) *
*            gcc -c -m4 -O2 -DCLK=? -DCH=? n4i7750.c              (exeGCC)  *
*            ccsh -c -ansi -noasmwarn -cpu=sh4 -O                           *
*                                        -DCLK=? -DCH=? n4i7750.c (GHS)     *
*                                                                           *
* 24/Apr/2000                                                               *
* 12/Sep/2000 カウンタ値の計算を修正                                        *
* 05/Seb/2001 v4_def_inhで、パラメータがNULL, NADRの時の処理を修正          *
* 19/Sep/2001 制御系の判断を見直し                                          *
*             タイムスタンプ用のクロックの取り出しを追加                 TA *
* 27/Dec/2002 v4_def_inhで pk_dinh->imaskチェック修正間違いを修正        KA *
* 25/Feb/2003 INTVEC のデフォルトを 0xa0000000L から 0x80000000L に変更  KA *
* 15/Jan/2004 sig_tim()をisig_tim()に修正                                KA *
****************************************************************************/

/*

最適化コンパイル

          割り込みハンドラのコンパイルには、必ず、最適化オプションを付け、
          ent_int 呼び出し前に、不要な命令が生成されないよう注意してください。
          ルネサスCでは、割込みハンドラを #pragma interrupt で定義してください。

使用するハードウェア

          SH7750 内蔵 TMU を周期タイマとして使用しています。
          それ以外のタイマを用いる場合は、ユーザーで改造してください。

割込み周期の指定

          割込み周期は、MSEC マクロで決まります。デフォルトは 10 msec です。
          例えば、5 msec に変更する場合は、次の様にコンパイルしてください。

          (例) shc <options> -def=MSEC=5 n4i7750.c
                                  ~~~~~~
          (例) gcc <options> -DMSEC=5 n4i7750.c
                               ~~~~~~
クロックの指定

          タイマのソースクロック（周辺クロックPφ）デフォルトは 33.3 MHz です。
          変更する場合 は CLK マクロを定義してコンパイルしてください。
          値は [Hz] 単位の周波数です。

          (例) shc <options> -def=CLK=19660800 n4i7750.c ... 19.6608 MHz
                                  ~~~~~~~~~~~~
          (例) gcc <options> -DCLK=19660800 n4i7750.c
                               ~~~~~~~~~~~~
チャネル番号の指定

          CH マクロで、使用する TMU チャネルを指定できます。（CH=0～2）
          デフォルトは CH=0 です。

          (例) shc <options> -def=CH=1 n4i7750.c  .......... TMU CH1
                                  ~~~~
          (例) gcc <options> -DCH=2 n4i7750.c  ............. TMU CH2
                               ~~~~
割り込みハンドラへの分岐

          割り込みの入口から例外コードを判別して、本ファイルの割り込みハンド
          ラへ分岐（jmp）するまでのルーチンは、ユーザにてアセンブラで記述して
          ください。（例が vec7750?.asm にあります）
          このルーチンでは、レジスタを破壊しないで分岐してください。

割込み優先順位

          本ハンドラの、デフォルトの割込み優先順位は、7 です。
          これを変更する場合、IP マクロを定義してコンパイルしてください。
          ただし、カーネルより高優先にはできません。（IP ≦ KNL_LEVEL）
          本ハンドラより高優先度の割込みハンドラが有る場合、多重割込みが起こ
          ります。

          (例) shc <options> -def=IP=2 n4i7750.c ... 割込み優先順位 2
                                  ~~~~
          (例) gcc <options> -DIP=2 n4i7750.c
                               ~~~~
dis_int, ena_int

          dis_int, ena_int システムコールは実装されていません。
          個別の割込み禁止許可を行いたい場合は、各制御レジスタを直接アクセス
          してください。
*/

#ifndef SH4
#define SH4
#endif

#include "kernel.h"
#include "nosys4.h"
#include "sh7750.h"


/* 割込み優先順位の定義 */

#ifndef IP
#define IP      7               /* IP 未定義なら 7 とする */
#endif

/* チャネル番号の定義 */

#ifndef CH
#define CH      0               /* CH 未定義なら 0 とする */
#endif

/* 処理系の差異の調整 */

#if defined(__GNUC__)           /* exeGCC */
void set_vbr(void **base);
extern char _srdata[], _erdata[];
extern char _stext[], _etext[];

#elif defined(__ghs)            /* Green Hills */
void set_vbr(void **base);
extern char __ghsbegin_rodata[], __ghsend_rodata[];
extern char __ghsbegin_text[], __ghsend_text[];

#elif defined(__TID__)          /* IAR EW/ICC */
void set_vbr(void **base);
extern int *_C_BGN, *_C_END, *_P_BGN, *_P_END;

#elif defined(GAIO)             /* XASSV */
#include <ht1_inline.h>
extern UW CONST_TOP[], CONST_SIZE[];

#elif defined(__HITACHI__)      /* Renesas C */
#include <machine.h>
extern int *_C_BGN, *_C_END, *_P_BGN, *_P_END;
#endif

/* クロックの定義 */

#define MHz         1000000L
#ifndef CLK
#define CLK         (100*MHz/3) /* 周辺クロックPφ[Hz] */
#endif

/* 割込みベクタアドレス */

#ifndef INTVEC
#define INTVEC      0x80000000L
#endif

/* 割り込みハンドラテーブル */

UW intdef[INT_FTXI-INT_NMI+1][2];

extern int RST_VECT;

#ifdef HOOK
/* チック入力積算カウンタ */

UW TOTAL_TICKCK;
UW TICK_RATE;
#endif

extern  UB _KERNEL_pisrposi;
extern  UB _KERNEL_hdrlen;
extern  void _kernel_interr(void);


/*****************************************************************************
* 割込み初期化
*
* 形式  intini();
*
* 解説  カーネルの初期化ルーチン（sysini）から割込み禁止状態で呼び出されます。
******************************************************************************/

ER intini(void)
{
    INT i;
    int *p;

    /* 割込みベクタテーブルの初期化 */

    for (i = 0; i < sizeof (intdef) / 8; i++)
    {   intdef[i][0] = 0x500000f0;              /* SR MD=1, BL=1, IMASK=15 */
        intdef[i][1] = (UW)_kernel_interr;
    }

    /* ベクタベースレジスタを設定 */
    /* リモートデバッガ(リセットベクタが0番地でない場合)は除く */

    p = &RST_VECT;
    if (((int)p & 0x1fffffff) == 0x00000000)
        set_vbr((void **)INTVEC);
    else
    {
      #ifdef REBOOT
        set_vbr((void **)INTVEC);
      #endif
    }
  #ifdef HOOK
    TOTAL_TICKCK = 0;
  #endif

    return E_OK;
}

/*****************************************************************************
* 定数か否かをチェックする
*
* 形式  result = _kernel_inROM(const void *p);
*
*       VP p;         チェックする領域へのポインタ
*       BOOL result;  TRUE .... 定数 or コードセクション
*                     FALSE ... 上記以外
*
* 解説  パケット生成情報等が定数か変数かを判定します。
*       カーネルの各種システムコールから呼び出されます。
******************************************************************************/

BOOL _kernel_inROM(const void *p)
{
#if defined(__GNUC__)           /* exeGCC */
    return((((UW)p >= (UW)_stext) && ((UW)p <= (UW)_etext)) || (((UW)p >= (UW)_srdata) && ((UW)p <= (UW)_erdata)));

#elif defined(__ghs)            /* Green Hills */
    return((((UW)p >= (UW)__ghsbegin_text) && ((UW)p <= (UW)__ghsend_text)) || (((UW)p >= (UW)__ghsbegin_rodata) && ((UW)p <= (UW)__ghsend_rodata)));

#elif defined(GAIO)             /* XASSV */
    return(((UW)p >= (UW)CONST_TOP) && ((UW)p <= ((UW)CONST_TOP+(UW)CONST_SIZE)));

#elif defined(__HITACHI__)      /* Renesas C */
    return((((UW)p >= (UW)_P_BGN) && ((UW)p <= (UW)_P_END)) || (((UW)p >= (UW)_C_BGN) && ((UW)p <= (UW)_C_END)));

#elif defined(__TID__)           /* IAR EW/ICC */
    return((((UW)p >= (UW)_P_BGN) && ((UW)p <= (UW)_P_END)) || (((UW)p >= (UW)_C_BGN) && ((UW)p <= (UW)_C_END)));

#endif
}

/*****************************************************************************
* 割込みサービスルーチン定義情報のチェック
*
* 形式  ercd = _kernel_check_cisr(const T_CISR *pk_cisr);
*
*       T_CISR pk_cisr;  チェックする割込みサービスルーチン定義情報へのポインタ
*       ER ercd;         E_PAR ... 割込み番号とマスクレベルのいずれかが範囲外
*                        E_OK .... 割込み番号とマスクレベルは共に正常
*
* 解説  割込みサービスルーチン定義情報の割込み番号とマスクレベルの範囲を
*       チェックします。
*       cre_isr(), acre_isr() システムコールから呼び出されます。
******************************************************************************/

ER _kernel_check_cisr(const T_CISR *pk_cisr)
{
    UINT intno, imask;

    intno = pk_cisr->intno - INT_NMI;
    if (intno < 0 || intno >= sizeof (intdef) / 8)
        return E_PAR;
    imask = pk_cisr->imask;
    if (imask < 1 || imask > 15)
        return E_PAR;
    return E_OK;
}

/*****************************************************************************
* 割込みハンドラの書き込み処理
*
* 形式  _kernel_set_inthdr(FP hdr, const T_CISR *pk_cisr);
*
*       FP hdr;          割込みサービスルーチン使用時の割込みハンドラアドレス
*       T_CISR pk_cisr;  チェックする割込みサービスルーチン定義情報へのポインタ
*
* 解説  割込みサービスルーチン使用時の割込みハンドラのアドレスを割込みサービス
*       ルーチン定義情報の割込み番号より算出したアドレスに格納します。
*       cre_isr(), acre_isr() システムコールから呼び出されます。
******************************************************************************/

extern  _kernel_cash_purge(FP);

void _kernel_set_inthdr(FP hdr, T_CISR *pk_cisr)
{
    UINT intno;

    _kernel_cash_purge(hdr);
    intno = pk_cisr->intno - INT_NMI;
    intdef[intno][0] = 0x50000000 | (pk_cisr->imask << 4);
    intdef[intno][1] = (UW)hdr;
}

/*****************************************************************************
* 割込みハンドラの読み出し処理
*
* 形式  inthdr = _kernel_get_inthdr(const T_CISR *pk_cisr);
*
*       T_CISR pk_cisr;  チェックする割込みサービスルーチン定義情報へのポインタ
*       FP inthdr;       当該割込み番号の割込みハンドラアドレス
*
* 解説  割込みサービスルーチン定義情報の割込み番号より算出したアドレスから
*       割込みハンドラアドレスを読み出します。
*       cre_isr(), acre_isr() システムコールから呼び出されます。
******************************************************************************/

FP _kernel_get_inthdr(T_CISR *pk_cisr)
{
    return (FP)intdef[pk_cisr->intno - INT_NMI][1];
}

/*****************************************************************************
* 割込みハンドラを定義する
*
* 形式  ercd = def_inh(UINT inhno, const T_DINH *pk_dinh);
*
*       UINT inhno; 　    割込みベクタ番号
*       T_DINH *pk_dinh; 割込みハンドラ定義情報
*         ATR intatr;    割込みハンドラ属性（TA_HLNG のみ, 未参照）
*         FP inthdr;     割込みハンドラアドレス
*         UINT imask;    割込みマスクレベル
*       ER ercd;         エラーコード
*
* 解説  inhno に対応する割込みベクタテーブルに、inthdr を設定します。
******************************************************************************/

ER v4_def_inh(INHNO inhno, const V4_T_DINH *pk_dinh)
{
    UINT psw;
    UINT i, m;
    FP fp;

    i = inhno - INT_NMI;
    if (i < 0 || i >= sizeof (intdef) / 8)
        return E_PAR;

    /* 定義解除の場合 */
    /* 未定義割込みを設定 */

    if (pk_dinh == (T_DINH *)NADR) {
        fp = (FP)_kernel_interr;
        m = 15;
    }
    else {
        fp = pk_dinh->inthdr;
        m = pk_dinh->imask;
        if (m < 1 || m > 15)
            return E_PAR;
    }

    /* CPU割込み禁止 */

    psw = vdis_psw();

    /* 定義の場合 */
    /* 新しい割込みベクタ設定 */

    intdef[i][0] = 0x50000000 | (m << 4);   /* SR MD=1, BL=1, IMASK=imask */
    intdef[i][1] = (UW)fp;

    /* CPU割込み禁止戻す */
    vset_psw(psw);
    return E_OK;
}


/*****************************************************************************
* 周期タイマ起動（SH7750 内蔵 TMU）
*
******************************************************************************/

#if (CH==0)
#define TMU_TCR     TMU_TCR0
#define TMU_TCNT    TMU_TCNT0
#define TMU_TCOR    TMU_TCOR0
#define INT_TUNI    INT_TUNI0
#elif (CH==1)
#define TMU_TCR     TMU_TCR1
#define TMU_TCNT    TMU_TCNT1
#define TMU_TCOR    TMU_TCOR1
#define INT_TUNI    INT_TUNI1
#elif (CH==2)
#define TMU_TCR     TMU_TCR2
#define TMU_TCNT    TMU_TCNT2
#define TMU_TCOR    TMU_TCOR2
#define INT_TUNI    INT_TUNI2
#else
#error illegal CH number! (CH = 0 to 2)
#endif

void isritm(VP_INT exinf);
const T_CISR disr_isritm = { TA_HLNG, NULL, INT_TUNI, isritm, IP };

static UH tpsc;

ER intsta(void)
{
    UW tc;
    UW psw;

    /* 時定数を計算 */

  #if   (((CLK) / 256000) * MSEC >= 0x0fffffff)
    tc = ((CLK) / 256000) * MSEC;
    tpsc = 3;
  #elif (((CLK) /  64000) * MSEC >= 0x0fffffff)
    tc = ((CLK) /  64000) * MSEC;
    tpsc = 2;
  #elif (((CLK) /  16000) * MSEC >= 0x0fffffff)
    tc = ((CLK) /  16000) * MSEC;
    tpsc = 1;
  #else
    tc = ((CLK) /   4000) * MSEC;
    tpsc = 0;
  #endif

  #ifdef HOOK
    TICK_RATE = tc;
  #endif

    /* 割込み定義 */

    acre_isr(&disr_isritm);

    /* 割込み優先レベル設定 */

    psw = vdis_psw();
  #if (CH==0)
    sfr_outw(INTC_IPRA, sfr_inw(INTC_IPRA) | (IP << 12));
  #elif (CH==1)
    sfr_outw(INTC_IPRA, sfr_inw(INTC_IPRA) | (IP <<  8));
  #elif (CH==2)
    sfr_outw(INTC_IPRA, sfr_inw(INTC_IPRA) | (IP <<  4));
  #endif

    /* タイマユニット初期化 */

    sfr_outl(TMU_TCOR, (tc-1));     /* タイマコンスタント設定 */
    sfr_outl(TMU_TCNT, (tc-1));     /* タイマカウンタ初期値 */
    sfr_outw(TMU_TCR, tpsc          /* タイマプリスケーラ選択 */
                    | 0x20);        /* アンダーフロー割り込みを許可 */
    sfr_set(TMU_TSTR, 0x01 << CH);  /* カウント動作スタート */
    vset_psw(psw);

    return E_OK;
}

/*****************************************************************************
* 周期タイマ割込みサービスルーチン（内蔵 TMU）
*
******************************************************************************/

void isritm(VP_INT exinf)
{
#ifdef HOOK
    UINT psw;

    psw = vdis_psw();
    TOTAL_TICKCK += TICK_RATE;
#endif
    sfr_outw(TMU_TCR, tpsc | 0x20); /* アンダーフローフラグをクリア */
#ifdef HOOK
    vset_psw(psw);
#endif
    isig_tim();                     /* 周期タイマ処理 */
}

/*****************************************************************************
* チック入力積算値を算出（ダミー）
*
******************************************************************************/

#ifdef HOOK
#if HOOK==1
UW tickck_read(void)
#else
UW ctex_timer_read(void)
#endif
{
    UW clk;

    if (!(sfr_inw(TMU_TCR) & 0x0100))
    {   clk = TICK_RATE - sfr_inl(TMU_TCNT);
    }
    if (sfr_inw(TMU_TCR) & 0x0100)
    {   clk = TICK_RATE + TICK_RATE - sfr_inl(TMU_TCNT);
    }
    return (((clk + TOTAL_TICKCK)*(MSEC*1000.0))/(TICK_RATE));
}
#else
UW tickck_read(void)
{
    return 0L;
}
#endif

#if defined(__GNUC__)   /* exeGCC */
/*****************************************************************************
* ＶＢＲの設定／取得（exeGCC）
*
******************************************************************************/

asm("
        .align  2
        .global _set_vbr
_set_vbr:
        rts
        ldc     r4,vbr

        .align  2
        .global _get_vbr
_get_vbr:
        rts
        stc     vbr,r0

");

#endif
#if defined(__ghs)      /* Green Hills */
/*****************************************************************************
* ＧＢＲ／ＶＢＲの設定／取得（Green Hills）
*
******************************************************************************/

void set_vbr(void **base)
{
    asm("   ldc     r4,vbr  ");
}

void **get_vbr(void)
{
    asm("   stc     vbr,r0  ");
}

#endif

/* end */
