/******************************************************************************
* ＮＯＲＴｉ対応 シルアル入出力関数                                           *
*                                                                             *
*  File name : nosio.c                                                        *
*  Copyright (c) 1993-2005, MiSPO Co., Ltd.                                   *
*                                                                             *
*  Compile : cl /c /Ox /DCHS=? nosio.c                   (MS-C/C++)           *
*            tcc -c -DCHS=? nosio.c                      (TC/C++)             *
*            bcc -c -O2 -DCHS=? nosio.c                  (BC/C++)             *
*            lcc80 -c -r -z80 -DCHS=? nosio.c            (LSIC-80)            *
*            iccz80 -v0 -ml -s9 -P -DCHS=? nosio.c       (ICC Z80)            *
*            icch83 -v? -mL -1i -s9 -P -DCHS=? nosio.c   (ICC H8/300H)        *
*            CHS=? : チャネル数（1～8）                                       *
*                                                                             *
* 93-04-13 作成                                                               *
* 93-05-16 Ver1.13β                                                          *
* 94-01-05 Ver1.14                                                            *
* 95-06-15 Ver1.20                                                            *
* 95-08-14 #include <stddef.h> 不要のため削除, #if (CHS>?) をネストさせない   *
*          T_SIOFP の定義を nosio.h から移す                                  *
* 96-05-29 Ver1.30 SIOFP 内に { } 追加                                        *
* 97-01-30 Ver1.40 T_SIOFP SIOFP[CHS] -> const T_SIOFP SIOFP[CHS]             *
* 98-09-06 V3 を定義してコンパイルで NORTi3 対応                              *
* 03/Oct/2000 NORTi4専用(旧バージョン互換やめる)                          OK  *
* 05-02-18 parameter pointer type "const B" -> "const char"               TA  *
******************************************************************************/

#include "kernel.h"
#include "nosio.h"

/* 処理系の差異の調整 */

#ifdef  M_I86
#pragma check_stack(off)        /* MS-C スタックチェックなし */
#endif

/* チャネル数の定義 */

#ifndef CHS
#define CHS         1           /* チャネル数 */
#endif

/* シリアル入出力制御ブロック */

T_SIO SIO[CHS];

/* シリアル入出力関数 */

ER cdecl ini_sio0(const char *);
void cdecl ext_sio0(void);
ER cdecl get_sio0(UB *, TMO);
ER cdecl put_sio0(UB, TMO);
ER cdecl ctl_sio0(UH);
ER cdecl ref_sio0(T_SIOS *);
ER cdecl fls_sio0(TMO);

#if (CHS>1)
ER cdecl ini_sio1(const char *);
void cdecl ext_sio1(void);
ER cdecl get_sio1(UB *, TMO);
ER cdecl put_sio1(UB, TMO);
ER cdecl ctl_sio1(UH);
ER cdecl ref_sio1(T_SIOS *);
ER cdecl fls_sio1(TMO);
#endif

#if (CHS>2)
ER cdecl ini_sio2(const char *);
void cdecl ext_sio2(void);
ER cdecl get_sio2(UB *, TMO);
ER cdecl put_sio2(UB, TMO);
ER cdecl ctl_sio2(UH);
ER cdecl ref_sio2(T_SIOS *);
ER cdecl fls_sio2(TMO);
#endif

#if (CHS>3)
ER cdecl ini_sio3(const char *);
void cdecl ext_sio3(void);
ER cdecl get_sio3(UB *, TMO);
ER cdecl put_sio3(UB, TMO);
ER cdecl ctl_sio3(UH);
ER cdecl ref_sio3(T_SIOS *);
ER cdecl fls_sio3(TMO);
#endif

#if (CHS>4)
ER cdecl ini_sio4(const char *);
void cdecl ext_sio4(void);
ER cdecl get_sio4(UB *, TMO);
ER cdecl put_sio4(UB, TMO);
ER cdecl ctl_sio4(UH);
ER cdecl ref_sio4(T_SIOS *);
ER cdecl fls_sio4(TMO);
#endif

#if (CHS>5)
ER cdecl ini_sio5(const char *);
void cdecl ext_sio5(void);
ER cdecl get_sio5(UB *, TMO);
ER cdecl put_sio5(UB, TMO);
ER cdecl ctl_sio5(UH);
ER cdecl ref_sio5(T_SIOS *);
ER cdecl fls_sio5(TMO);
#endif

#if (CHS>6)
ER cdecl ini_sio6(const char *);
void cdecl ext_sio6(void);
ER cdecl get_sio6(UB *, TMO);
ER cdecl put_sio6(UB, TMO);
ER cdecl ctl_sio6(UH);
ER cdecl ref_sio6(T_SIOS *);
ER cdecl fls_sio6(TMO);
#endif

#if (CHS>7)
ER cdecl ini_sio7(const char *);
void cdecl ext_sio7(void);
ER cdecl get_sio7(UB *, TMO);
ER cdecl put_sio7(UB, TMO);
ER cdecl ctl_sio7(UH);
ER cdecl ref_sio7(T_SIOS *);
ER cdecl fls_sio7(TMO);
#endif

#if (CHS>8)
#error  CHS is out of range!
#endif

/* シリアル入出力関数定義テーブル */

typedef struct t_siofp
{
    ER (cdecl *ini_sio)(const char *);  /* シリアル入出力初期化関数 */
    void (cdecl *ext_sio)(void);        /* シリアル入出力終了関数 */
    ER (cdecl *get_sio)(UB *, TMO);     /* シリアル１文字入力関数 */
    ER (cdecl *put_sio)(UB, TMO);       /* シリアル１文字出力関数 */
    ER (cdecl *ctl_sio)(UH);            /* シリアル入出力制御関数 */
    ER (cdecl *ref_sio)(T_SIOS *);      /* シリアル入出力状態参照関数 */
    ER (cdecl *fls_sio)(TMO);           /* シリアル送信バッファ･フラッシュ関数 */

} T_SIOFP;

const T_SIOFP SIOFP[CHS] =
{
  { ini_sio0, ext_sio0, get_sio0, put_sio0, ctl_sio0, ref_sio0, fls_sio0 },
  #if (CHS>1)
  { ini_sio1, ext_sio1, get_sio1, put_sio1, ctl_sio1, ref_sio1, fls_sio1 },
  #endif
  #if (CHS>2)
  { ini_sio2, ext_sio2, get_sio2, put_sio2, ctl_sio2, ref_sio2, fls_sio2 },
  #endif
  #if (CHS>3)
  { ini_sio3, ext_sio3, get_sio3, put_sio3, ctl_sio3, ref_sio3, fls_sio3 },
  #endif
  #if (CHS>4)
  { ini_sio4, ext_sio4, get_sio4, put_sio4, ctl_sio4, ref_sio4, fls_sio4 },
  #endif
  #if (CHS>5)
  { ini_sio5, ext_sio5, get_sio5, put_sio5, ctl_sio5, ref_sio5, fls_sio5 },
  #endif
  #if (CHS>6)
  { ini_sio6, ext_sio6, get_sio6, put_sio6, ctl_sio6, ref_sio6, fls_sio6 },
  #endif
  #if (CHS>7)
  { ini_sio7, ext_sio7, get_sio7, put_sio7, ctl_sio7, ref_sio7, fls_sio7 },
  #endif
};

/*****************************************************************************
* シリアル入出力初期化
*
******************************************************************************/

ER pascal ini_sio(INT ch, const char *param)
{
    if ((UINT)ch >= CHS)
        return E_PAR;
    return (*SIOFP[ch].ini_sio)(param);
}

/*****************************************************************************
* シリアル入出力終了
*
******************************************************************************/

void cdecl ext_sio(void)
{
    INT ch;

    for (ch = 0; ch < CHS; ch++)
        (*SIOFP[ch].ext_sio)();
}

/*****************************************************************************
* シリアル１文字入力
*
******************************************************************************/

ER pascal get_sio(INT ch, UB *p_c, TMO tmout)
{
    if ((UINT)ch >= CHS)
        return E_PAR;
    return (*SIOFP[ch].get_sio)(p_c, tmout);
}

/*****************************************************************************
* シリアル１文字出力
*
******************************************************************************/

ER pascal put_sio(INT ch, UB c, TMO tmout)
{
    if ((UINT)ch >= CHS)
        return E_PAR;
    return (*SIOFP[ch].put_sio)(c, tmout);
}

/*****************************************************************************
* シリアル入出力制御
*
******************************************************************************/

ER pascal ctl_sio(INT ch, UH fncd)
{
    if ((UINT)ch >= CHS)
        return E_PAR;
    return (*SIOFP[ch].ctl_sio)(fncd);
}

/*****************************************************************************
* シリアル入出力状態参照
*
******************************************************************************/

ER pascal ref_sio(INT ch, T_SIOS *pk_sios)
{
    if ((UINT)ch >= CHS)
        return E_PAR;
    return (*SIOFP[ch].ref_sio)(pk_sios);
}

/*****************************************************************************
* シリアル送信バッファ･フラッシュ
*
******************************************************************************/

ER pascal fls_sio(INT ch, TMO tmout)
{
    if ((UINT)ch >= CHS)
        return E_PAR;
    return (*SIOFP[ch].fls_sio)(tmout);
}

/* end */
