;****************************************************************************
;* SHCベクタ設定の例（Computex SH7750 EVA BOD)                              *
;*                                                                          *
;*  File name : vec7750h.asm                                                *
;*                                                                          *
;* 24/Apr/2000                                                              *
;* 15/Aug/2000 SH4キャッシュのパージを追加                                  *
;* 12/Sep/2000  キャッシュの初期化処理を追加                                *
;* 12/Sep/2001  キャッシュのパージを強制的にP2領域で実行するように修正      *
;*              VECTセクションをCODE属性に修正                           TA *
;* 23/Jan/2002 ローダによる ROM->RAMコピーに対応                         TA *
;* 08/Feb/2005 OYO105の設定                                                 *
;* 08/Nov/2011 POP-500の設定                                                *
;* 15/Jun/2012 IAS-220の設定                                                *
;****************************************************************************

;* ボードの選定 
BOARD:	.assignc	"VA300"

;* メモリマップに合わせ、★印のスタックポインタ初期値を変更してください。

        .import __INIT          ; 初期設定ルーチン
        .import _intdef         ; 割り込みハンドラテーブル
EXPEVT:     .equ   H'FF000024   ; 例外事象レジスタ
INTEVT:     .equ   H'FF000028   ; 割り込み事象レジスタ
INT_NMI:    .equ   14           ; ノンマスカブル割り込み番号

;* キャッシュの有効／無効を設定します。

CACHE_ON: .define "1"           ; =0:無効／=1:有効

;* キャッシュのライトスルー／ライトバックを設定します。
;* キャッシュが有効の場合のみ有効です。

CACHE_WT: .define "1"           ; =0:ライトバック／=1:ライトスルー

;* 一般例外発生でホルトした場合は、この変数の値を調べてください。

        .section B,DATA,ALIGN=4
_expspc: .res.l 1               ; 一般例外発生時の SPC
_expssr: .res.l 1               ; 一般例外発生時の SSR
_expevt: .res.l 1               ; 一般例外発生時の EXPEVT

;* リセット

        .section VECT,CODE


        .export _RST_VECT
_RST_VECT:
        nop
        bra     _RESET          ; CSIDE デバッグ時の割込みエントリは +10
        nop
        nop
        nop
        nop
        nop
        nop
        .export _RESET
_RESET:
        mov.l   INIT_STACK,r15   ; スタックポインタ設定

;* メモリ設定等、最初に必要な処理がある場合は、追加してください。

        bsr     _BOOT
        nop

; FLASH/ROM -> RAM コピー部
; SDRAM上で実行するプログラムイメージをそのままオフセットを付けてFLASHに書き
; 込んだとき FLASH -> RAM コピーをするコードです。
; オフセット書込みが確認されると元のアドレス(SDRAM)にプログラムをコピーし元々
; のアドレス(__INIT)にジャンプしあたかも最初からSDRAMで動いていたかのように動
; き続けます。
; ☆移動(オフセット)がない場合はコピーは行われません。

        bsr     copy_s
        nop
copy_s: sts     pr,r1
        mov.l   COPY_ORG,r3
        mov.l   COPY_BGN,r2
        mov.l   ADDR_MSK,r0
        and     r0,r1
        and     r0,r2
        add     r3,r1
        mov.l   COPY_SIZE,r0
        cmp/eq  r1,r2
        add     #3,r0
;        bt      copy_1
        bra     copy_1
        shlr2   r0
        mov.l   ADDR_P2,r3
        or      r3,r1
        or      r3,r2
copy_0: mov.l   @r1+,r3
        mov.l   r3,@r2
        add     #-1,r0
        cmp/eq  #0,r0
        bf/s    copy_0
        add     #4,r2
copy_1:

        mov     #0,r0
        mov.l   expevt_4,r3
        mov.l   r0,@-r3         ; expevt = 0;
        mov.l   r0,@-r3         ; expssr = 0;
        mov.l   r0,@-r3         ; expspc = 0;

        mov.l   INIT,r3         ; 初期設定ルーチンへ
        jmp     @r3
        nop

        .align  4
INIT_STACK: .data.l _INIT_STACK ; スタック領域終わりのアドレス★
INIT:       .data.l __INIT

expevt_4:   .data.l _expevt+4

ADDR_MSK:   .data.l H'1FFFFFFF
ADDR_P2:    .data.l H'A0000000
COPY_BGN:   .data.l (STARTOF VECT)
COPY_SIZE:  .data.l (STARTOF D)+(SIZEOF D)-(STARTOF VECT)
COPY_ORG:   .data.l -(copy_s-_RST_VECT)


;* 一般例外事象

        .org    H'00000100
        nop
        bra     vect110         ; CSIDE デバッグ時の割込みエントリは +10
        nop
        nop
        nop
        nop
        nop
        nop
vect110:
        mov.l   expevt_41,r1
        mov.l   expevt_42,r0
        mov.l   @r0,r0
        mov.l   r0,@-r1         ; expevt = EXPEVTレジスタの例外コード;
        stc     ssr,r0
        mov.l   r0,@-r1         ; expssr = SSR;
        stc     spc,r0
        mov.l   r0,@-r1         ; expspc = SPC;

        bra     $               ; 必要ならユーザーで記述してください。
        nop

        .align  4
expevt_41:  .data.l _expevt+4
expevt_42:  .data.l EXPEVT


;* 一般例外事象（TLBミス）

        .org    H'00000400
        mov.l   expevt_44,r1
        mov.l   expevt_45,r0
        mov.l   @r0,r0
        mov.l   r0,@-r1         ; expevt = EXPEVTレジスタの例外コード;
        stc     ssr,r0
        mov.l   r0,@-r1         ; expssr = SSR;
        stc     spc,r0
        mov.l   r0,@-r1         ; expspc = SPC;

        bra     $               ; 必要ならユーザーで記述してください。
        nop

        .align  4
expevt_44:  .data.l _expevt+4
expevt_45:  .data.l EXPEVT


;* 割り込み要求

        .org    H'00000600
        nop
        bra     vect610         ; CSIDE デバッグ時の割込みエントリは +10
        nop
        nop
        nop
        nop
        nop
        nop
vect610:                        ; 割込みでは BL=1,RB=1 へ切り替わる
        stc.l   r0_bank,@-r15   ; R0_BANK0 退避
        mov.l   r0,@-r15        ; R0_BANK1 退避
        mov.l   r1,@-r15        ; R1_BANK1 退避

        mov.l   intevt,r0
        mov.l   @r0,r1          ; R1_BANK1 = INTEVTレジスタの例外コード;
        tst     r1,r1           ; if (例外コード == 0)
        bt/s    vect610_rte     ;    割込みリターンへ
        shar    r1
        mov.l   intdef,r0       ; R0_BANK1 = def_inh の登録テーブル
        shar    r1
        add     r1,r0           ; R0_BANK0 = テーブル内の対応アドレス
        mov.l   @r15+,r1        ; R1_BANK1 復元
        ldc.l   @r0+,sr         ; SR = def_inh で登録された値
                                ; ここで、RB=0 に切り替わる
        stc     r0_bank,r0      ; R0_BANK0 = R0_BANK1
        ldc.l   @r15+,r0_bank   ; R0_BANK1 復元

        mov.l   @r0,r0          ; R0_BANK0 = def_inh で登録の inthdr アドレス
        jmp     @r0             ; inthdr へ
        mov.l   @r15+,r0        ; R0_BANK0 復元

vect610_rte:                    ; 例外コード0の場合
        mov.l   @r15+,r1        ; R1_BANK1 復元
        mov.l   @r15+,r0        ; R0_BANK1 復元
        ldc.l   @r15+,r0_bank   ; R0_BANK0 復元
        rte                     ; 割込みリターン
        nop

        .align  4
intdef:     .data.l _intdef-INT_NMI*8
intevt:     .data.l INTEVT


;* ハードウェアの初期化

        .align  4
_BOOT:
        mov.l   CCR,r0
        mov.l   CCR_D,r1
        mov.l   r1,@r0          ; CCR (OC&IC disable)

        ; バスコントローラの初期化

        mov.l   RTCSR,r1        ; 再初期化の禁止
        mov.w   @r1,r0
        cmp/eq  #0,r0
        bf      INI_SKIP

        mov.l   BCR1,r0         ; Computex SH7750 EVA BOD 用設定
        mov.l   BCR1_D,r1
        mov.l   r1,@r0          ; BCR1

        mov.l   BCR2,r0
        mov.w   BCR2_D,r1
        mov.w   r1,@r0          ; BCR2

        mov.l   WCR1,r0
        mov.l   WCR1_D,r1
        mov.l   r1,@r0          ; WCR1

        mov.l   WCR2,r0
        mov.l   WCR2_D,r1
        mov.l   r1,@r0          ; WCR2

        mov.l   WCR3,r0
        mov.l   WCR3_D,r1
        mov.l   r1,@r0          ; WCR3

        mov.l   MCR,r0
        mov.l   MCR_D,r1
        mov.l   r1,@r0          ; MCR

        mov.l   SDMR3,r0
        mov     #0,r1
        mov.b   r1,@r0          ; SDMR3

        mov.l   PCR,r0
        mov.w   PCR_D,r1
        mov.w   r1,@r0          ; PCR

        mov.l   RTCNT,r0
        mov.w   RTCNT_D,r1
        mov.w   r1,@r0          ; RTCNT

        mov.l   RTCOR,r0
        mov.w   RTCOR_D,r1
        mov.w   r1,@r0          ; RTCOR

        mov.l   RFCR,r0
        mov.w   RFCR_D,r1
        mov.w   r1,@r0          ; RFCR

        mov.l   RTCSR,r0
        mov.w   RTCSR_D,r1
        mov.w   r1,@r0          ; RTCSR

        mov.l   CCR,r0
        mov.l   CCR_D,r1
        mov.l   r1,@r0          ; CCR (OC&IC disable)

        nop
        nop
        nop
        nop

        mov.l   RFCR,r1
        mov     #8,r2
BOOT_1: mov.w   @r1,r0
        cmp/hs  r2,r0
        bf      BOOT_1

        mov.l   MCR,r0
        mov.l   MCR_D+4,r1
        mov.l   r1,@r0          ; MCR

        mov.l   SDMR3,r0
        mov     #0,r1
        mov.b   r1,@r0          ; SDMR3

        mov.l   RTCSR,r0
        mov.w   RTCSR_D+2,r1
        mov.w   r1,@r0          ; RTCSR
INI_SKIP:

    .aif CACHE_ON ne 0
        mov.l   CCR,r0
        mov.l   CCR_D+4,r1
        mov.l   r1,@r0          ; CCR (OC&IC enable, write through)
    .aendi

        rts
        nop

        .import __KERNEL_hdrlen
        .export __kernel_cash_purge
        .align  4

__kernel_cash_purge:
        mov.l   L1_0,r0
        mov.l   L1_1,r1
        mov.l   L1_2,r2
        and     r1,r0
        or      r2,r0
        jmp     @r0
        nop

        .align  4
L1_0:   .data.l kernel_cash_purge
L1_1:   .data.l H'1FFFFFFF
L1_2:   .data.l H'A0000000

kernel_cash_purge:
        mov.l   HDRLEN,r0
        mov     #32,r2
        mov.b   @r0,r1
        mov     r4,r3
        add     r1,r4
        add     #-4,r4
        clrt
L0_1:
        ocbp    @r3
        subc    r2,r1
        bf/s    L0_1
        add     r2,r3

        mov.l   CCR,r2
        mov.l   CCR_ICI,r3
        mov.l   @r2,r0
        ocbp    @r4
        or      r3,r0
        mov.l   r0,@r2
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        rts
        nop


        .align  4
BCR1:       .data.l H'FF800000
BCR2:       .data.l H'FF800004
WCR1:       .data.l H'FF800008
WCR2:       .data.l H'FF80000C
WCR3:       .data.l H'FF800010
MCR:        .data.l H'FF800014
PCR:        .data.l H'FF800018
RTCSR:      .data.l H'FF80001C
RTCNT:      .data.l H'FF800020
RTCOR:      .data.l H'FF800024
RFCR:       .data.l H'FF800028

	.aif "\&BOARD" eq "OYO105A"	;●ボードによる変更
SDMR3:      .data.l H'FF94008C
	.aelif "\&BOARD" eq "VA300"	;●ボードによる変更
SDMR3:      .data.l H'FF9400CC
	.aelse
SDMR3:      .data.l H'FF940110	;●H'FF94008C
	.aendi
CCR:        .data.l H'FF00001C

	.aif "\&BOARD" eq "OYO105A";●ボードによる変更
BCR1_D:     .data.l H'00000008
WCR1_D:     .data.l H'77777372	;
WCR2_D:     .data.l H'FFFF4DFF	;
WCR3_D:     .data.l H'07777777	;
MCR_D:       .data.l H'10192194	;
             .data.l H'50192194	;(D30=1)
CCR_D:      .data.l H'00000808
    .aif CACHE_WT eq 0
            .data.l H'00000105
    .aelse
            .data.l H'00000103
    .aendi
CCR_ICI:    .data.l H'00000800

HDRLEN:     .data.l __KERNEL_hdrlen

;BCR2_D:     .data.w H'95D5	;●CS1=8bit,CS2=8bit,CS4=8bit,CS5=8bit
BCR2_D:     .data.w H'AEE9	;●CS1=16bit,CS2=8bit,CS4=16bit,CS5=32bit
PCR_D:      .data.w H'0000
RTCNT_D:    .data.w H'A500

RTCOR_D:    .data.w H'A50F	;
;RTCOR_D:    .data.w H'A585	;
RTCSR_D:    .data.w H'A510	;?
            .data.w H'A510	;
RFCR_D:     .data.w H'A482	;
	.aelif "\&BOARD" eq "VA300";●ボードによる変更
BCR1_D:     .data.l H'00080008
WCR1_D:     .data.l H'77777777	;
;;WCR2_D:     .data.l H'FE784B2F	;
;WCR2_D:     .data.l H'9E784B2F	;
WCR2_D:     .data.l H'FFFF6FFF	;
;WCR2_D:     .data.l H'FFFEEFFF	;
WCR3_D:     .data.l H'07777777;
;WCR3_D:     .data.l H'00030000	;CS4=hold
MCR_D:       .data.l H'10192D94	;
             .data.l H'50192D94	;(D30=1)
CCR_D:      .data.l H'00000808
    .aif CACHE_WT eq 0
            .data.l H'00000105
    .aelse
            .data.l H'00000123
    .aendi
CCR_ICI:    .data.l H'00000800

HDRLEN:     .data.l __KERNEL_hdrlen

;;BCR2_D:     .data.w H'AAE5	;●CS1=8bit,CS2=8bit,CS4=8bit,CS5=8bit
;BCR2_D:     .data.w H'9AE9	;●CS3=32bit,CS6=8bit
BCR2_D:     .data.w H'AEE9	;●CS1=16bit,CS2=16bit,CS3=32bit,CS4=16bit,CS5=32bit
PCR_D:      .data.w H'0000
RTCNT_D:    .data.w H'A500

RTCOR_D:    .data.w H'A540	;
;RTCOR_D:    .data.w H'A585	;
RTCSR_D:    .data.w H'A510	;?
            .data.w H'A510	;
RFCR_D:     .data.w H'A482	;
	.aelse
BCR1_D:     .data.l H'00000008
WCR1_D:     .data.l H'77777777	;●H'72322277	;アイドル
WCR2_D:     .data.l H'FFFF4FFF	;●H'FDF64F27
WCR3_D:     .data.l H'07770777	;●H'07770755
MCR_D:       .data.l H'08092014	;●H'10192194	; 
             .data.l H'48092014	;●H'50192194	;(D30=1)
CCR_D:      .data.l H'00000808
    .aif CACHE_WT eq 0
            .data.l H'00000105
    .aelse
            .data.l H'00000103
    .aendi
CCR_ICI:    .data.l H'00000800

HDRLEN:     .data.l __KERNEL_hdrlen

BCR2_D:     .data.w H'D514	;●CS1=8bit,CS2=8bit,CS4=8bit,CS5=8bit
PCR_D:      .data.w H'0000
RTCNT_D:    .data.w H'A500

RTCOR_D:    .data.w H'A503	;●H'A50F
RTCSR_D:    .data.w H'A511	;?
            .data.w H'A511	;●H'A510
RFCR_D:     .data.w H'A400	;●H'A482
	.aendi

;* 各セクションのアドレス

        .section P,CODE,ALIGN=4         ; プログラム領域（ROM）
        .section D,DATA,ALIGN=4         ; 初期化データ領域（ROM）
        .section R,DATA,ALIGN=4         ; 初期化データ領域（RAM）
        .section B,DATA,ALIGN=4         ; 未初期化データ領域（RAM）
        .section STACK,DATA,ALIGN=4     ; スタック領域（RAM）
        .section PFL,CODE,ALIGN=4       ; フラッシュメモリ書換えプログラム領域
        .section CFL,DATA,ALIGN=4       ; フラッシュメモリ書換えプログラム定数領域
        .section XFL,DATA,ALIGN=4       ; フラッシュメモリ書換えプログラム実行領域
        .section YFL,DATA,ALIGN=4       ; フラッシュメモリ書換えプログラムRAM領域
        .section C,DATA,ALIGN=4         ; 定数領域（ROM）
__P_BGN .data.l (STARTOF P)
__P_END .data.l (STARTOF P)+(SIZEOF P)
__C_BGN .data.l (STARTOF C)
__C_END .data.l (STARTOF C)+(SIZEOF C)
__D_ROM .data.l (STARTOF D)
__D_BGN .data.l (STARTOF R)
__D_END .data.l (STARTOF R)+(SIZEOF R)
__B_BGN .data.l (STARTOF B)
__B_END .data.l (STARTOF B)+(SIZEOF B)
__PFL_BGN .data.l (STARTOF PFL)
__CFL_BGN .data.l (STARTOF CFL)
__XFL_BGN .data.l (STARTOF XFL)
__XFL_END .data.l (STARTOF XFL)+(SIZEOF XFL)
__YFL_BGN .data.l (STARTOF YFL)
__YFL_END .data.l (STARTOF YFL)+(SIZEOF YFL)
        .export __P_BGN
        .export __P_END
        .export __C_BGN
        .export __C_END
        .export __D_ROM
        .export __D_BGN
        .export __D_END
        .export __B_BGN
        .export __B_END
        .export __PFL_BGN
        .export __CFL_BGN
        .export __XFL_BGN
        .export __XFL_END
        .export __YFL_BGN
        .export __YFL_END

; ROM先頭 +--------------------+
;         | 割込みベクタ(VECT) |
;         +--------------------+
;         |   プログラム領域   |
;         |        (P)         |
;         +--------------------+
;         |      定数領域      |
;         |        (C)         |
;         +--------------------+
;         |  初期化データ領域  |---+
;         |        (D)         |   |
;         +--------------------+   |_INITSCT() でコピー
;         ～                  ～   |
; RAM先頭 +--------------------+   |
;         |  初期化データ領域  |<--+
;         |        (R)         |
;         +--------------------+
;         | 未初期化データ領域 |
;         |        (B)         |
;         +--------------------+
;         |    スタック領域    |
;         |      (STACK)       |
;         ～                  ～
; SP初期値+--------------------+

        .END
