//=============================================================================
/**
 *	@file Mon.h
 *
 *	@brief	�r�g-�S �b�o�t���W���[�� ���j�^�v���O����
 *	
 *	@date		1999/10/20
 *	@version	1.00 �V�K�쐬
 *	@author		S.Nakano
 *
 *	@date		2001/12/7
 *	@version	1.10 MCM(HJ945010BP)�Ή��ǉ�
 *	@author		S.Nalano
 *
 *	@date		2007/11/27
 *	@version	1.11 �T���v���p�ɕs�v�������폜�Akernel.h��include
 *	@author		F.Saeki
 *
 *	@date		2011/4/7
 *	@version	1.20 include�t�@�C����ύX
 *	@author		F.Saeki
 *
 *	Copyright (C) 2007, OYO Electric Corporation
 */
//=============================================================================

#include <ctype.h>
#include <machine.h>
#include <string.h>
#include <stdarg.h>
#if defined(NORTI_USE)
 #include "kernel.h"
 #define	BYTE	UB
 #define	WORD	UH
 #define	LONG	UW
#else
 #if defined(SH4)
//  #include "sh4def.h"
 #else
//  #include "sh2def.h"
 #endif
#endif

//=== ���X�|���X�R�[�h ===
#define	RX_NORM		("0000" CRLF)		///< ����
#define	RX_DATA		("0001" CRLF)		///< �f�[�^���M�v��
#define	RX_SAME		("0002" CRLF)		///< �f�[�^�đ��v��
#define	RX_ERAS		("0101" CRLF)		///< �q�n�l�C���[�Y���s
#define	RX_WERR		("0102" CRLF)		///< �q�n�l�������ݎ��s

//=== �G���[���b�Z�[�W�R�[�h ===
#define	MSG_NONE		0
#define	MSG_BADCMD		1
#define MSG_BADHEX		2
#define	MSG_BADPARM		3
#define	MSG_BADADDR		4
#define	MSG_ODDADDR		5
#define	MSG_VERIFY		6

//=== ����t���O ===
#define	FLAG_VERIFY		0x01

#define	CRLF			"\x0D\x0A"

#define	MON_BUF_SIZE		128
#define	SEC_SIZE			512
#define	SEC_MUL_CNT			3

#ifndef	TRUE
	#define	TRUE			1
#endif
#ifndef	FALSE
	#define	FALSE			0
#endif

//#define	reset()				(**(void (**)(void))0)()

enum COMM_CH {							///< �ʐM�`�����l����`
	SCI_CH,								///< �V���A��
	LAN_CH								///< LAN
};

//=== �f�o�b�O�p ===
#if defined( _DEBUG_)
	#define DPRINT	SendText
#else
	#define DPRINT
#endif

#if defined(EXTERN)
#undef EXTERN
#endif

#if defined(_MON_MAIN_)
	#define	EXTERN
	const BYTE Hexasc[ 16]	= "0123456789abcdef";
	const BYTE HexAsc[ 16]	= "0123456789ABCDEF";
	const BYTE HexChr[256]	=
		"................"	"................"
		" !\"#$%&'()*+,-./"	"0123456789:;<=>?"
		"@ABCDEFGHIJKLMNO"	"PQRSTUVWXYZ[\\]^_"
		"`abcdefghijklmno"	"pqrstuvwxyz{|}~."
		"................"	"................"
		"................"	"................"
		"................"	"................"
		"................"	"................"
	;
#else
	#define	EXTERN	extern
	extern const BYTE Hexasc[];
	extern const BYTE HexAsc[];
	extern const BYTE HexChr[];
#endif

typedef struct						//=== ���s������ ========================
{
	void	(*Proc)(void);			///< �V�[�P���X����|�C���^
	void	(*AppStart)(void);		///< �A�v���P�[�V�����|�C���^
	BYTE	Flag;					///< ����t���O
	BYTE*	DumpAddr;				///< Dump�擪�A�h���X
	LONG	DumpSize;				///< Dump�\���o�C�g��
	BYTE	DumpUnit;				///< Dump�P��	('B'/'W'/'L')
	BYTE	EditUnit;				///< Edit�P��	('B'/'W'/'L')
	BYTE*	topAddr;				///< �R�}���h�J�n�A�h���X
	BYTE*	endAddr;				///< �R�}���h�I���A�h���X
	WORD	Retry;					///< ���g���C�J�E���^
									//=== �^�[�~�i���ʐM��� ==================
	char	TxText[1024];			///< �V���A�����M�f�[�^
	char	RxText[128];			///< �V���A����M�f�[�^
	BYTE	RxSetP;					///< �V���A����M�|�C���^(�i�[)
	BYTE	RxGetP;					///< �V���A����M�|�C���^(��o)
	char	RxBuff[128];			///< �V���A����M�o�b�t�@
	
	BYTE	sel;		//0�FSCI�A1�Ftelnet 2003.01.10�ǉ�
	
	LONG	DlyTimer;				///< �f�B���C�p�^�C�}�[
	LONG	LedTimer;				///< LED�p�^�C�}�[
	LONG	SlpTimer;				///< �X���[�v�p�^�C�}�[
	int		ComCh;					///< �ʐM�`�����l��
} CTRL;

typedef struct t_monmsg{
	struct t_monmsg *next;			///< �擪4�޲Ă� OS �Ŏg�p(4)	*/
	INT			ch;					///< �`�����l���ԍ�
	enum COMM_CH eCom;				///< �ʐM�`�����l�����
	UH			cnt;				///< �f�[�^��
	BYTE		buf[ MON_BUF_SIZE ];
} T_MONMSG;


EXTERN	CTRL	Ctrl;				///< ���s������

//=== MON_MAIN.C ===
EXTERN ER Mon_ini(INT ch);				///< ���j�^�N��
EXTERN BOOL GetCmd(char*);				///< ���j�^�R�}���h��M
EXTERN void CmdExec(void);				///< �f  �R�}���h
EXTERN void CmdHelp(void);				///< HELP�R�}���h
EXTERN void CmdVer(void);				///< �u�����R�}���h
EXTERN void CmdMon(void);				///< Mon�R�}���h
EXTERN BOOL Asc2Hex(char*,UW*,UH);		///< ASCII��16�i�e�L�X�g����o�C�i���ւ̕ϊ�
EXTERN void LongAsc(LONG,char*);		///< �����O�f�[�^��ASCII��16�i�e�L�X�g�ɕϊ�
EXTERN void WordAsc(WORD,char*);		///< ���[�h�f�[�^��ASCII��16�i�e�L�X�g�ɕϊ�
EXTERN void ByteAsc(BYTE,char*);		///< �o�C�g�f�[�^��ASCII��16�i�e�L�X�g�ɕϊ�
EXTERN BOOL Asc2Dec(const char*,UW*,UH);///< ASCII��10�i�e�L�X�g����o�C�i���ւ̕ϊ�
EXTERN void SendMsg(BYTE);				///< �G���[���b�Z�[�W���M
EXTERN void SendVfy(BYTE*,BYTE,BYTE);	///< �������݃`�F�b�N�m�f�f�[�^���M
EXTERN void SendText(char*);			///< �^�[�~�i���ւ̕�����f�[�^���M
EXTERN void SendBin(long, const char*);	///< �^�[�~�i���ւ̃f�[�^���M
EXTERN void ReadText(void);				///< �^�[�~�i�����當����f�[�^��M
EXTERN void ReadCmd(void);				///< �R�}���h�̓Ǐo��
EXTERN void cyc_tim(VP_INT);			///< �����N���n���h��
EXTERN TASK MonRcvTask(INT);			///< ���j�^��M�^�X�N
EXTERN TASK MonTask(INT);				///< ���j�^�^�X�N

//=== MON_CHCK.C ===
void CmdCheck(void);				///< �b�R�}���h

//=== MON_DUMP.C ===
void CmdDump (void);				///< �c  �R�}���h
void CmdDumpB(void);				///< �c�a�R�}���h
void CmdDumpW(void);				///< �c�v�R�}���h
void CmdDumpL(void);				///< �c�k�R�}���h
BYTE ParsParmD(void);				///< �R�}���h�p�����[�^�̉���

//=== MON_EDIT.C ===
void CmdEdit (void);				///< �l�R�}���h
void CmdEdit1(void);				///< �l�a�R�}���h
void CmdEdit2(void);				///< �l�v�R�}���h
void CmdEdit3(void);				///< �l�k�R�}���h

//=== MON_DASM.C ===
void CmdDumpA(void);				///< Dump with Assembler instruction
int i_sprintf(char*,const char*,...);	///< �J�[�l���p�Ȉ�sprintf�֐�

//=== MON_MAC.C ===
void CmdMS(void);					///< MAC�A�h���X�ҏW

//=== MON_NS.C ===
void CmdNS(void);					///< �l�b�g���[�N�p�����[�^�ҏW

//=== MON_NR.C ===
void CmdNR(void);					///< �l�b�g���[�N�p�����[�^�Q��

//=== TIMER.C ===
INTHDR IntTMU1(void);				///< �����݃^�C�}�[�n���h��
INTHDR IntTMU2(void);				///< �����݃^�C�}�[�n���h��

//=== MON_TEST.C ===
void CmdHardTest(void);				///< �n�[�h�E�F�A�̃`�F�b�N���[�h
void CmdSoftTest(void);				///< �\�t�g�E�F�A�̃`�F�b�N���[�h

//=== MON_CNR.C ===
void CmdCNR(void);					///< ����Ղ̃l�b�g���[�N�p�����[�^�Q��

//=== MON_IDS.C ===
void CmdIDS(void);					///< �@��ԍ��ҏW

//=== MON_CNS.C ===
void CmdCNS(void);					///< ����Ղ̃l�b�g���[�N�p�����[�^�ݒ�

#if defined(NORTI_USE)
extern char* _strtok(char *s1, const char *s2);	///< NORTi�pstrtok
#endif