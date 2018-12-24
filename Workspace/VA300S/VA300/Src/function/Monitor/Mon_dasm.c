//=============================================================================
//				ＳＨ-４ ＣＰＵモジュール モニタプログラム
//					<<< 逆アセンブル処理モジュール >>>
//
// Version 1.00 1999.04.03 S.Nakano	新規
// Version 1.01 1999.10.26 F.Saeki	インクルードファイル変更
// Version 1.10 2001.12.07 S.Nakano	MCM(HJ945010BP)対応追加
//=============================================================================
#include "kernel.h"
#include "nonet.h"
#include "nonteln.h"
#include "mon.h"

static void EditInst_0	(int);	// 命令テキスト編集：０形式
static void EditInst_n	(int);	// 命令テキスト編集：ｎ形式
static void EditInst_m	(int);	// 命令テキスト編集：ｍ形式
static void EditInst_i	(int);	// 命令テキスト編集：ｉ形式
static void EditInst_d	(int);	// 命令テキスト編集：ｄ形式
static void EditInst_nm	(int);	// 命令テキスト編集：ｎｍ形式
static void EditInst_md	(int);	// 命令テキスト編集：ｍｄ形式
static void EditInst_ni	(int);	// 命令テキスト編集：ｎｉ形式
static void EditInst_nd4(int);	// 命令テキスト編集：ｎｄ４形式
static void EditInst_nd8(int);	// 命令テキスト編集：ｎｄ８形式
static void EditInst_nmd(int);	// 命令テキスト編集：ｎｍｄ形式
static void EditInst_d12(int);	// 命令テキスト編集：ｄ１２形式

#define	POS_ADDR		0
#define	POS_CODE		9
#define	POS_INST		14

#define	INST_TYPE_0		0xFFFF,EditInst_0
#define	INST_TYPE_n		0xF0FF,EditInst_n
#define	INST_TYPE_m		0xF0FF,EditInst_m
#define	INST_TYPE_i		0xFF00,EditInst_i
#define	INST_TYPE_d		0xFF00,EditInst_d
#define	INST_TYPE_nm	0xF00F,EditInst_nm
#define	INST_TYPE_md	0xFF00,EditInst_md
#define	INST_TYPE_ni	0xF000,EditInst_ni
#define	INST_TYPE_nd4	0xFF00,EditInst_nd4
#define	INST_TYPE_nd8	0xF000,EditInst_nd8
#define	INST_TYPE_nmd	0xF000,EditInst_nmd
#define	INST_TYPE_d12	0xF000,EditInst_d12
#define	INST_TYPE_ng	0x0000,EditInst_0

static const struct
{
	WORD	code;
	WORD	mask;
	void	(*proc)();
	BYTE	size;
	char*	form;
} InstList[] =
{
	{0x0002,INST_TYPE_n		,4,"STC     SR,R%u\n"			},
	{0x0003,INST_TYPE_m		,4,"BSRF    R%u\n"				},
	{0x0004,INST_TYPE_nm	,1,"MOV.B   R%u,@(R0,R%u)\n"	},
	{0x0005,INST_TYPE_nm	,2,"MOV.W   R%u,@(R0,R%u)\n"	},
	{0x0006,INST_TYPE_nm	,4,"MOV.L   R%u,@(R0,R%u)\n"	},
	{0x0007,INST_TYPE_nm	,4,"MUL.L   R%u,R%u\n"			},
	{0x0008,INST_TYPE_0		,0,"CLRT\n"						},
	{0x0009,INST_TYPE_0		,0,"NOP\n"						},
	{0x000A,INST_TYPE_n		,4,"STS     MACH,R%u\n"			},
	{0x000B,INST_TYPE_0		,0,"RTS\n"						},
	{0x000C,INST_TYPE_nm	,1,"MOV.B   @(R0,R%u),R%u\n"	},
	{0x000D,INST_TYPE_nm	,2,"MOV.W   @(R0,R%u),R%u\n"	},
	{0x000E,INST_TYPE_nm	,4,"MOV.L   @(R0,R%u),R%u\n"	},
	{0x000F,INST_TYPE_nm	,4,"MAC.L   @R%u+,@R%u+\n"		},
	{0x0012,INST_TYPE_n		,4,"STC     GBR,R%u\n"			},
	{0x0018,INST_TYPE_0		,0,"SETT\n"						},
	{0x0019,INST_TYPE_0		,0,"DIV0U\n"					},
	{0x001A,INST_TYPE_n		,4,"STS     MACL,R%u\n"			},
	{0x001B,INST_TYPE_0		,0,"SLEEP\n"					},
	{0x0022,INST_TYPE_n		,4,"STC     VBR,R%u\n"			},
	{0x0023,INST_TYPE_m		,4,"BRAF    R%u\n"				},
	{0x0028,INST_TYPE_0		,0,"CLRMAC\n"					},
	{0x0029,INST_TYPE_n		,4,"MOVT    R%u\n"				},
	{0x002A,INST_TYPE_n		,4,"STS     PR,R%u\n"			},
	{0x002B,INST_TYPE_0		,0,"RTE\n"						},
	{0x1000,INST_TYPE_nmd	,4,"MOV.L   R%u,@(H'%08X,R%u)\n"},
	{0x2000,INST_TYPE_nm	,1,"MOV.B   R%u,@R%u\n"			},
	{0x2001,INST_TYPE_nm	,2,"MOV.W   R%u,@R%u\n"			},
	{0x2002,INST_TYPE_nm	,4,"MOV.L   R%u,@R%u\n"			},
	{0x2004,INST_TYPE_nm	,1,"MOV.B   R%u,@-R%u\n"		},
	{0x2005,INST_TYPE_nm	,2,"MOV.W   R%u,@-R%u\n"		},
	{0x2006,INST_TYPE_nm	,4,"MOV.L   R%u,@-R%u\n"		},
	{0x2007,INST_TYPE_nm	,4,"DIV0S   R%u,R%u\n"			},
	{0x2008,INST_TYPE_nm	,4,"TST     R%u,R%u\n"			},
	{0x2009,INST_TYPE_nm	,4,"AND     R%u,R%u\n"			},
	{0x200A,INST_TYPE_nm	,4,"XOR     R%u,R%u\n"			},
	{0x200B,INST_TYPE_nm	,4,"OR      R%u,R%u\n"			},
	{0x200C,INST_TYPE_nm	,4,"CMP/STR R%u,R%u\n"			},
	{0x200D,INST_TYPE_nm	,4,"XTRCT   R%u,R%u\n"			},
	{0x200E,INST_TYPE_nm	,4,"MULU.W  R%u,R%u\n"			},
	{0x200F,INST_TYPE_nm	,4,"MULS.W  R%u,R%u\n"			},
	{0x3000,INST_TYPE_nm	,4,"CMP/EQ  R%u,R%u\n"			},
	{0x3002,INST_TYPE_nm	,4,"CMP/HS  R%u,R%u\n"			},
	{0x3003,INST_TYPE_nm	,4,"CMP/GE  R%u,R%u\n"			},
	{0x3004,INST_TYPE_nm	,4,"DIV1    R%u,R%u\n"			},
	{0x3005,INST_TYPE_nm	,4,"DMULU.L R%u,R%u\n"			},
	{0x3006,INST_TYPE_nm	,4,"CMP/HI  R%u,R%u\n"			},
	{0x3007,INST_TYPE_nm	,4,"CMP/GT  R%u,R%u\n"			},
	{0x3008,INST_TYPE_nm	,4,"SUB     R%u,R%u\n"			},
	{0x300A,INST_TYPE_nm	,4,"SUBC    R%u,R%u\n"			},
	{0x300B,INST_TYPE_nm	,4,"SUBV    R%u,R%u\n"			},
	{0x300C,INST_TYPE_nm	,4,"ADD     R%u,R%u\n"			},
	{0x300D,INST_TYPE_nm	,4,"DMULS.L R%u,R%u\n"			},
	{0x300E,INST_TYPE_nm	,4,"ADDC    R%u,R%u\n"			},
	{0x300F,INST_TYPE_nm	,4,"ADDV    R%u,R%u\n"			},
	{0x4000,INST_TYPE_n		,4,"SHLL    R%u\n"				},
	{0x4001,INST_TYPE_n		,4,"SHLR    R%u\n"				},
	{0x4002,INST_TYPE_n		,4,"STS.L   MACH,@-R%u\n"		},
	{0x4003,INST_TYPE_n		,4,"STC.L   SR,@-R%u\n"			},
	{0x4004,INST_TYPE_n		,4,"ROTL    R%u\n"				},
	{0x4005,INST_TYPE_n		,4,"ROTR    R%u\n"				},
	{0x4006,INST_TYPE_m		,4,"LDS.L   @R%u+,MACH\n"		},
	{0x4007,INST_TYPE_m		,4,"LDC.L   @R%u+,SR\n"			},
	{0x4008,INST_TYPE_n		,4,"SHLL2   R%u\n"				},
	{0x4009,INST_TYPE_n		,4,"SHLR2   R%u\n"				},
	{0x400A,INST_TYPE_m		,4,"LDS     R%u,MACH\n"			},
	{0x400B,INST_TYPE_m		,4,"JSR     @R%u\n"				},
	{0x400E,INST_TYPE_m		,4,"LDC     R%u,SR\n"			},
	{0x400F,INST_TYPE_nm	,2,"MAC.W   @R%u+,@R%u+\n"		},
	{0x4010,INST_TYPE_n		,4,"DT      R%u\n"				},
	{0x4011,INST_TYPE_n		,4,"CMP/PZ  R%u\n"				},
	{0x4012,INST_TYPE_n		,4,"STS.L   MACL,@-R%u\n"		},
	{0x4013,INST_TYPE_n		,4,"STC.L   GBR,@-R%u\n"		},
	{0x4015,INST_TYPE_n		,4,"CMP/PL  R%u\n"				},
	{0x4016,INST_TYPE_m		,4,"LDS.L   @R%u+,MACL\n"		},
	{0x4017,INST_TYPE_m		,4,"LDC.L   @R%u+,GBR\n"		},
	{0x4018,INST_TYPE_n		,4,"SHLL8   R%u\n"				},
	{0x4019,INST_TYPE_n		,4,"SHLR8   R%u\n"				},
	{0x401A,INST_TYPE_m		,4,"LDS     R%u,MACL\n"			},
	{0x401B,INST_TYPE_n		,1,"TAS.B   @R%u\n"				},
	{0x401E,INST_TYPE_m		,4,"LDC     R%u,GBR\n"			},
	{0x4020,INST_TYPE_n		,4,"SHAL    R%u\n"				},
	{0x4021,INST_TYPE_n		,4,"SHAR    R%u\n"				},
	{0x4022,INST_TYPE_n		,4,"STS.L   PR,@-R%u\n"			},
	{0x4023,INST_TYPE_n		,4,"STC.L   VBR,@-R%u\n"		},
	{0x4024,INST_TYPE_n		,4,"ROTCL   R%u\n"				},
	{0x4025,INST_TYPE_n		,4,"ROTCR   R%u\n"				},
	{0x4026,INST_TYPE_m		,4,"LDS.L   @R%u+,PR\n"			},
	{0x4027,INST_TYPE_m		,4,"LDC.L   @R%u+,VBR\n"		},
	{0x4028,INST_TYPE_n		,4,"SHLL16  R%u\n"				},
	{0x4029,INST_TYPE_n		,4,"SHLR16  R%u\n"				},
	{0x402A,INST_TYPE_m		,4,"LDS     R%u,PR\n"			},
	{0x402B,INST_TYPE_m		,4,"JMP     @R%u\n"				},
	{0x402E,INST_TYPE_m		,4,"LDC     R%u,VBR\n"			},
	{0x5000,INST_TYPE_nmd	,4,"MOV.L   @(H'%08X,R%u),R%u\n"},
	{0x6000,INST_TYPE_nm	,1,"MOV.B   @R%u,R%u\n"			},
	{0x6001,INST_TYPE_nm	,2,"MOV.W   @R%u,R%u\n"			},
	{0x6002,INST_TYPE_nm	,4,"MOV.L   @R%u,R%u\n"			},
	{0x6003,INST_TYPE_nm	,4,"MOV     R%u,R%u\n"			},
	{0x6004,INST_TYPE_nm	,1,"MOV.B   @R%u+,R%u\n"		},
	{0x6005,INST_TYPE_nm	,2,"MOV.W   @R%u+,R%u\n"		},
	{0x6006,INST_TYPE_nm	,4,"MOV.L   @R%u+,R%u\n"		},
	{0x6007,INST_TYPE_nm	,4,"NOT     R%u,R%u\n"			},
	{0x6008,INST_TYPE_nm	,1,"SWAP.B  R%u,R%u\n"			},
	{0x6009,INST_TYPE_nm	,2,"SWAP.W  R%u,R%u\n"			},
	{0x600A,INST_TYPE_nm	,4,"NEGC    R%u,R%u\n"			},
	{0x600B,INST_TYPE_nm	,4,"NEG     R%u,R%u\n"			},
	{0x600C,INST_TYPE_nm	,1,"EXTU.B  R%u,R%u\n"			},
	{0x600D,INST_TYPE_nm	,2,"EXTU.W  R%u,R%u\n"			},
	{0x600E,INST_TYPE_nm	,1,"EXTS.B  R%u,R%u\n"			},
	{0x600F,INST_TYPE_nm	,2,"EXTS.W  R%u,R%u\n"			},
	{0x7000,INST_TYPE_ni	,1,"ADD     #H'%08X,R%u\n"		},
	{0x8000,INST_TYPE_nd4	,1,"MOV.B   R0,@(H'%08X,R%u)\n"	},
	{0x8100,INST_TYPE_nd4	,2,"MOV.W   R0,@(H'%08X,R%u)\n"	},
	{0x8400,INST_TYPE_md	,1,"MOV.B   @(H'%08X,R%u),R0\n"	},
	{0x8500,INST_TYPE_md	,2,"MOV.W   @(H'%08X,R%u),R0\n"	},
	{0x8800,INST_TYPE_i		,1,"CMP/EQ  #H'%08X,R0\n"		},
	{0x8900,INST_TYPE_d		,0,"BT      H'%08X\n"			},
	{0x8B00,INST_TYPE_d		,0,"BF      H'%08X\n"			},
	{0x8D00,INST_TYPE_d		,0,"BT/S    H'%08X\n"			},
	{0x8F00,INST_TYPE_d		,0,"BF/S    H'%08X\n"			},
	{0x9000,INST_TYPE_nd8	,2,"MOV.W   @(H'%08X,PC),R%u\n"	},
	{0xA000,INST_TYPE_d12	,0,"BRA     H'%08X\n"			},
	{0xB000,INST_TYPE_d12	,0,"BSR     H'%08X\n"			},
	{0xC000,INST_TYPE_d		,1,"MOV.B   R0,@(H'%08X,GBR)\n"	},
	{0xC100,INST_TYPE_d		,2,"MOV.W   R0,@(H'%08X,GBR)\n"	},
	{0xC200,INST_TYPE_d		,4,"MOV.L   R0,@(H'&08X,GBR)\n"	},
	{0xC300,INST_TYPE_i		,4,"TRAPA   #'%08X\n"			},
	{0xC400,INST_TYPE_d		,1,"MOV.B   @(H'%08X,GBR),R0\n"	},
	{0xC500,INST_TYPE_d		,2,"MOV.W   @(H'%08X,GBR),R0\n"	},
	{0xC600,INST_TYPE_d		,4,"MOV.L   @(H'%08X,GBR),R0\n"	},
	{0xC700,INST_TYPE_d		,4,"MOVA    @(H'%08X,PC),R0\n"	},
	{0xC800,INST_TYPE_i		,0,"TST     #H'%08X,R0\n"		},
	{0xC900,INST_TYPE_i		,0,"AND     #H'%08X,R0\n"		},
	{0xCA00,INST_TYPE_i		,0,"XOR     #H'%08X,R0\n"		},
	{0xCB00,INST_TYPE_i		,0,"OR      #H'%08X,R0\n"		},
	{0xCC00,INST_TYPE_i		,0,"TST.B   #H'%08X,@(R0,GBR)\n"},
	{0xCD00,INST_TYPE_i		,0,"AND.B   #H'%08X,@(R0,GBR)\n"},
	{0xCE00,INST_TYPE_i		,0,"XOR.B   #H'%08X,@(R0,GBR)\n"},
	{0xCF00,INST_TYPE_i		,0,"OR.B    #H'%08X,@(R0,GBR)\n"},
	{0xD000,INST_TYPE_nd8	,4,"MOV.L   @(H'%08X,PC),R%u\n"	},
	{0xE000,INST_TYPE_ni	,1,"MOV     #H'%08X,R%u\n"		},
	{0x0000,INST_TYPE_ng	,0,"??????\n"					},
};

static WORD	Word;						// Wordデータ

static int i_vsprintf(char *, const char *, va_list);
static void ConvText(unsigned int,char**,int,const char*,int,int,int);

//=============================================================================
// [機能] 逆アセンブルダンプ表示
// [引数] なし
// [戻値] なし
//=============================================================================
void CmdDumpA(void)
{
	int		no;
	long	rest;

	Ctrl.Proc = ReadCmd;
	if (no = ParsParmD())						// コマンドパラメータ解釈
	{
		SendMsg(no);							// パラメータエラー
		return;
	}

	Ctrl.DumpAddr = (BYTE*)((LONG)Ctrl.DumpAddr & 0xFFFFFFFE);
	for (rest=Ctrl.DumpSize; rest>0; rest-=2)
	{
		memset(Ctrl.TxText,' ',sizeof Ctrl.TxText);

		Word = *(WORD*)Ctrl.DumpAddr;
		LongAsc((LONG)Ctrl.DumpAddr,Ctrl.TxText+POS_ADDR);
		WordAsc(Word			 ,Ctrl.TxText+POS_CODE);

		for (no=0;; no++)
		{
			if ((Word & InstList[no].mask) == InstList[no].code)
			{
				InstList[no].proc(no); break;
			}
		}
		strcat((char*)Ctrl.TxText,CRLF);
		SendText(Ctrl.TxText);
		Ctrl.DumpAddr += 2;
	}
	Ctrl.DumpUnit = 'A';						// 逆アセンブルダンプ保存
}

//-----------------------------------------------------------------------------
// 命令テキストの編集：０形式
//-----------------------------------------------------------------------------
static void EditInst_0(int no)
{
	strcpy((char *)Ctrl.TxText+POS_INST,InstList[no].form);
}
//-----------------------------------------------------------------------------
// 命令テキストの編集：ｎ形式
//-----------------------------------------------------------------------------
static void EditInst_n(int no)
{
	LONG	n;

	n = (Word & 0x0F00) >> 8;
	i_sprintf((char *)Ctrl.TxText+POS_INST,InstList[no].form,n);
}
//-----------------------------------------------------------------------------
// 命令テキストの編集：ｍ形式
//-----------------------------------------------------------------------------
static void EditInst_m(int no)
{
	LONG	n;

	n = (Word & 0x0F00) >> 8;
	i_sprintf((char *)Ctrl.TxText+POS_INST,InstList[no].form,n);
}
//-----------------------------------------------------------------------------
// 命令テキストの編集：ｉ形式
//-----------------------------------------------------------------------------
static void EditInst_i(int no)
{
	LONG	i;

	i = (Word & 0x00FF);
	switch (InstList[no].size)
	{
	case 0:										break;	// ゼロ拡張
	case 1: if (i & 0x0080) i |= 0xFFFFFF00;	break;	// 符号拡張
	case 4: i <<= 2;							break;	// ゼロ拡張後４倍
	}
	i_sprintf((char *)Ctrl.TxText+POS_INST,InstList[no].form,i);
}
//-----------------------------------------------------------------------------
// 命令テキストの編集：ｄ形式
//-----------------------------------------------------------------------------
static void EditInst_d(int no)
{
	LONG	d;

	d = (Word & 0x00FF);
	if (InstList[no].size)	d *= InstList[no].size;
	else if (d & 0x0080)	d  = (LONG)Ctrl.DumpAddr + 4 - ((d ^ 0x00FF + 1) << 1);
	else					d  = (LONG)Ctrl.DumpAddr + 4 + (d << 1);

	i_sprintf((char *)Ctrl.TxText+POS_INST,InstList[no].form,d);
}
//-----------------------------------------------------------------------------
// 命令テキストの編集：ｄ１２形式
//-----------------------------------------------------------------------------
static void EditInst_d12(int no)
{
	LONG	d;

	d = (Word & 0x0FFF);
	if (d & 0x0800)	d  = (LONG)Ctrl.DumpAddr + 4 - ((d ^ 0x0FFF + 1) << 1);
	else			d  = (LONG)Ctrl.DumpAddr + 4 + (d << 1);

	i_sprintf((char *)Ctrl.TxText+POS_INST,InstList[no].form,d);
}
//-----------------------------------------------------------------------------
// 命令テキストの編集：ｎｍ形式
//-----------------------------------------------------------------------------
static void EditInst_nm(int no)
{
	LONG	n,m;

	n = (Word & 0x0F00) >> 8;
	m = (Word & 0x00F0) >> 4;
	i_sprintf((char *)Ctrl.TxText+POS_INST,InstList[no].form,m,n);
}
//-----------------------------------------------------------------------------
// 命令テキストの編集：ｍｄ形式
//-----------------------------------------------------------------------------
static void EditInst_md(int no)
{
	LONG	m,d;

	m = (Word & 0x00F0) >> 4;
	d = (Word & 0x000F) * InstList[no].size;
	i_sprintf((char *)Ctrl.TxText+POS_INST,InstList[no].form,d,m);
}
//-----------------------------------------------------------------------------
// 命令テキストの編集：ｎｉ形式
//-----------------------------------------------------------------------------
static void EditInst_ni(int no)
{
	LONG	n,i;

	n = (Word & 0x0F00) >> 8;
	i = (Word & 0x00FF);
	if (i & 0x0080) i |= 0xFFFFFF00;			// 符号拡張

	i_sprintf((char *)Ctrl.TxText+POS_INST,InstList[no].form,i,n);
}
//-----------------------------------------------------------------------------
// 命令テキストの編集：ｎｄ４形式
//-----------------------------------------------------------------------------
static void EditInst_nd4(int no)
{
	LONG	n,d;

	n = (Word & 0x00F0) >> 4;
	d = (Word & 0x000F) * InstList[no].size;
	i_sprintf((char *)Ctrl.TxText+POS_INST,InstList[no].form,d,n);
}
//-----------------------------------------------------------------------------
// 命令テキストの編集：ｎｄ８形式
//-----------------------------------------------------------------------------
static void EditInst_nd8(int no)
{
	LONG	n,d;

	n = (Word & 0x0F00) >> 8;
	d = (Word & 0x00FF) * InstList[no].size;
	i_sprintf((char *)Ctrl.TxText+POS_INST,InstList[no].form,d,n);
}
//-----------------------------------------------------------------------------
// 命令テキストの編集：ｎｍｄ形式
//-----------------------------------------------------------------------------
static void EditInst_nmd(int no)
{
	LONG	n,m,d;

	n = (Word & 0x0F00) >> 8;
	m = (Word & 0x00F0) >> 4;
	d = (Word & 0x000F) * InstList[no].size;
	switch (Word & 0xF000)
	{
	case 0x1000: i_sprintf((char *)Ctrl.TxText+POS_INST,InstList[no].form,m,d,n);
				 break;
	case 0x5000: i_sprintf((char *)Ctrl.TxText+POS_INST,InstList[no].form,d,m,n);
				 break;
	}
}

//=============================================================================
// [機能] カーネル用簡易sprintf関数
// [引数] buff 	文字列を格納するバッファ
//		  form	フォーマット制御文字列
//		  ...	オプションの引数
// [戻値] buffに格納した文字数(終端のNULL文字は含まない)
//=============================================================================
int i_sprintf(char *buffer, const char *format, ...)
{
	va_list	ap;
	int		n;

	va_start(ap,format);
	n = i_vsprintf(buffer,format,ap);
	va_end(ap);

	return n;
}

//=============================================================================
// [機能] カーネル用簡易vsprintf関数
// [引数] buff 	文字列を格納するバッファ
//		  form	フォーマット制御文字列
//		  args	オプションの引数
// [戻値] buffに格納した文字数(終端のNULL文字は含まない)
//=============================================================================
static int i_vsprintf(char *buf, const char *format, va_list ap)
{
	char*	bufpt;
	char*	str;
	LONG	val;
	int		width,padzero,c;

	bufpt = buf;
	while (c = *(format++))
	{
		if (c != '%')
		{
			*(bufpt++) = c;
			continue;
		}
		width = padzero = 0;

		if ((c = *(format++)) == '0')
		{
			padzero = 1;
			c = *(format++);
		}
		while (('0' <= c) && (c <= '9'))
		{
			width = width * 10 + c - '0';
			c = *(format++);
		}

		switch (c)
		{
		case 'd':
			val = va_arg(ap,int);
			if (val >= 0) 
				ConvText( val,&bufpt,10,(char *)Hexasc,width,0,padzero);
			else
				ConvText(-val,&bufpt,10,(char *)Hexasc,width,1,padzero);
			break;

		case 'u':
			val = va_arg(ap,int);
			ConvText(val,&bufpt,10,(char *)Hexasc,width,0,padzero);
			break;

		case 'x':
			val = va_arg(ap,int);
			ConvText(val,&bufpt,16,(char *)Hexasc,width,0,padzero);
			break;

		case 'X':
			val = va_arg(ap,int);
			ConvText(val,&bufpt,16,(char *)HexAsc,width,0,padzero);
			break;

		case 'c':
			*(bufpt++) = va_arg(ap,int);
			break;

		case 's':
			str = va_arg(ap,char*);
			while (c = *(str++)) *(bufpt++) = c;
			break;

		case '%':
			*(bufpt++) = '%';
			break;

		case 0:
			format--;
			break;
		}
	}
	*bufpt = 0;
	return(bufpt - buf);
}

//-----------------------------------------------------------------------------
// 数値データのASCII編集
//-----------------------------------------------------------------------------
static void ConvText(unsigned int val, char **bufpt, int radix,
					const char *radchar, int width, int minus, int padzero)
{
	char	buf[24];
	int		i, j;

	i = 0;
	do
	{
		buf[i++] = radchar[val % radix];
		val		/= radix;
	} while (val);
	if (minus) buf[i++] = '-';

	for (j=i; j<width; j++) *((*bufpt)++) = padzero? '0': ' ';
	while (i>0) *((*bufpt)++) = buf[--i];
}
