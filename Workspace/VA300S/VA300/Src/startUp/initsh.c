/******************************************************************************
* 日立Ｃ初期設定ルーチンの例（SH）                                            *
*                                                                             *
*  File name : initsh.c                                                       *
*                                                                             *
* 24/May/1997 作成                                                            *
* 06/Dec/1999 -fpu=doubleオプションに対応                               MiSPO *
******************************************************************************/

#include "machine.h"

extern int *_D_ROM, *_D_BGN, *_D_END, *_B_BGN, *_B_END;
extern int *_PFL_BGN, *_XFL_BGN, *_CFL_BGN, *_YFL_BGN, *_XFL_END, *_YFL_END;
extern void _INITSCT(void);
extern void main(void);
extern void stop_wdt( void );
#pragma noregsave(_INIT)

void _INIT(void)
{
	stop_wdt();	// WDT stop Added T.N 2015.3.10
	
    _INITSCT();
#ifdef _FPD
    set_fpscr(((int)get_fpscr()&~0x00100000)|0x00080000);
#endif
    main();
    for (;;)
        ;
}

void _INITSCT(void)
{
    int *p, *q;

    for (p = _B_BGN; p < _B_END; p++)
        *p = 0;

    for (p = _D_BGN, q = _D_ROM; p < _D_END; p++, q++)
        *p = *q;

    for (p = _XFL_BGN, q = _PFL_BGN; p < _XFL_END; p++, q++)
        *p = *q;

    for (p = _YFL_BGN, q = _CFL_BGN; p < _YFL_END; p++, q++)
        *p = *q;

}

/* end */
