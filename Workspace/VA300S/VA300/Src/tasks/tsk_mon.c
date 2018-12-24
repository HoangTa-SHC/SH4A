//=============================================================================
/**
 *	@brief	�r�g-�S �b�o�t���W���[�� ���j�^�v���O����
 *	
 *	@file tsk_mon.c
 *	
 *	@date		2011/4/7
 *	@version	1.20 OYO-105A�p�u�[�g�v���O�������痬�p
 *	@author		F.Saeki
 *
 *	Copyright (C) 2007, OYO Electric Corporation
 */
//=============================================================================
#include "itron.h"
#include "nonet.h"
#include "kernel.h"

#define	_MON_MAIN_
#include "mon.h"
#include "id.h"
#include "nosio.h"
#include "va300.h"
#include "drv_led.h"

// ��`
#define	LED_DCMD	LED_DBG1	// ���j�^�R�}���hLED

static const struct				//=== ���j�^�R�}���h�ꗗ ===
{
	char*	CmdName;			// �R�}���h��
	void	(*CmdProc)(void);	// �R�}���h�����֐�
} CmdList[]	=
{
	{"C"	,CmdCheck},			// Check RAM memory access
	{"D"	,CmdDump },			// Dump
	{"DA"	,CmdDumpA},			// Dump with Assembler instruction
	{"DB"	,CmdDumpB},			// Dump with Byte unit
	{"DW"	,CmdDumpW},			// Dump with 16bit Word unit
	{"DL"	,CmdDumpL},			// Dump with 32bit Longword unit
	{"G"	,CmdExec },			// Go to user program
	{"M"	,CmdEdit },			// Memory edit
	{"MB"	,CmdEdit1},			// Memory edit with Byte
	{"MW"	,CmdEdit2},			// Memory edit with Word
	{"ML"	,CmdEdit3},			// Memory edit with Longword
	{"VER"	,CmdVer },			// Version information
//	{"HWT"	,CmdHardTest},		// �n�[�h�E�F�A�e�X�g
//	{"SWT"	,CmdSoftTest},		// �\�t�g�E�F�A�e�X�g
	{"?"	,CmdHelp },			// Show Help Message
	{"MS"	,CmdMS   },			// MAC Address Set
	{"NS"	,CmdNS   },			// Network parameter Set
	{"CNS"	,CmdCNS  },			// Control box Network parameter Set
	{"IDS"	,CmdIDS  },			// ID number Set
	{"NR"	,CmdNR   },			// Network parameter Read
	{"MON"	,CmdMon  },			// ���j�^�R�}���h
};

static void InitMon(INT ch);			// �v���O�����̏�����

// �R�}���h�ǂ݂����V�[�P���X
static void ReadCmd01(void);
static void ReadCmd02(void);
static void PasswdCheck(void);			///< �p�X���[�h�`�F�b�N

static	int	nRead;

#if 1
extern void DebugOut(char*);			///< �f�o�b�O�p�o�͊֐�
#endif

const T_CTSK cMonTask  = { TA_HLNG, NULL, MonTask, 7, 2048, NULL, "MonTask" };
const T_CTSK cMonRcvTask  = { TA_HLNG, NULL, MonRcvTask, 3, 2048, NULL, "MonRcvTask" };
const T_CMBX cMonMbx   = { TA_TFIFO|TA_MFIFO, 0, NULL, (B *)"mon_mbx" };
const T_CMPF cMonMpf   = { TA_TFIFO, 4, sizeof (T_MONMSG), NULL, (B *)"mon_mpf" };

const unsigned long MonAddr = 0x80001000;	///< ���j�^���[�h���̃A�h���X

static const char cMonPasswd[] = "20011";	///< ���j�^�p�p�X���[�h(��)

//=============================================================================
/**
 * ���j�^�v���O�����N��
 *
 * @param ch	�`�����l���ԍ�
 *
 * @retval E_OK ���j�^�N������
 */
//=============================================================================
ER Mon_ini(INT ch)
{
	ER ercd;
	
//	ercd = cre_tsk(TSK_CMD_MON, &cMonTask);
	if (ercd != E_OK) {
		return ercd;
	}
	
//	ercd = cre_tsk(TSK_RCV_MON, &cMonRcvTask);
	if (ercd != E_OK) {
		return ercd;
	}
	
	ercd = cre_mbx(MBX_MON, &cMonMbx);
	if (ercd != E_OK) {
		return ercd;
	}
	
	ercd = cre_mpf(MPF_MON, &cMonMpf);
	if (ercd != E_OK) {
		return ercd;
	}

//	ercd = sta_tsk(TSK_CMD_MON, ch);
	if (ercd != E_OK) {
		return ercd;
	}

//	ercd = sta_tsk(TSK_RCV_MON, ch);

	return ercd;
}

//-----------------------------------------------------------------------------
// �v���O�����̏�����
//-----------------------------------------------------------------------------
static void InitMon(INT ch)
{
	
	memset(&Ctrl,0,sizeof Ctrl);				// ���s������N���A
	Ctrl.DumpUnit = 'B';						// Dump�P�ʏȗ��l
	Ctrl.EditUnit = 'B';						// Edit�P�ʏȗ��l
	Ctrl.Proc	  = PasswdCheck;				// �����V�[�P���X�Z�b�g
	Ctrl.DumpSize = 256;						// �o�C�g���ȗ��l�Z�b�g
	Ctrl.ComCh = ch;							// �ʐM�`�����l���̐ݒ�
}

//=============================================================================
/**
 * ���j�^�R�}���h��M
 */
//=============================================================================
void ReadCmd(void)
{
	char*	Cmd;
	int		i;

	SendText(">>");								// �v�����v�g���M
	ReadText();									// �R�}���h�e�L�X�g��M

	if (Cmd = strtok(Ctrl.RxText," \x0D\t"))
	{
		for (i=0; i<(sizeof CmdList / sizeof CmdList[0]); i++)
		{
			if (!strcmp(Cmd,CmdList[i].CmdName))
			{
				LedOut(LED_DCMD, LED_ON);
				CmdList[i].CmdProc();			// �R�}���h�����֐��Ɉڍs
				LedOut(LED_DCMD, LED_OFF);
				return;
			}
		}
		SendMsg(MSG_BADCMD);					// ����`�R�}���h
	}
}

//=============================================================================
/**
 * �f�R�}���h�̎��s
 */
//=============================================================================
void CmdExec(void)
{
	char*	Parm;
	LONG	Addr;

	Ctrl.Proc = ReadCmd;

	if (!(Parm = strtok(NULL," \x0D\t"))) 	{ SendMsg(MSG_BADADDR); return; }
	if (!Asc2Hex(Parm,&Addr,0))				{ SendMsg(MSG_BADADDR); return; }

	(**(void (**)(void))Addr)();			/* �v���O�����ɕ���			*/
}

//=============================================================================
/**
 * HELP�R�}���h�̎��s
 */
//=============================================================================
void CmdHelp(void)
{
	const char *HelpText[] = {
		"C :�������`�F�b�N             C <Addr> {<addr>|@<size>}",
		"D :�������_���v               {D|DB|DW|DL} [<addr> [<addr>|@<size>}]",
		"G :���[�U�v���O�������s       G <addr>",
		"M :�������f�[�^�ύX           M <addr> [{B|W|L}] [N]",
		"DA:�t�A�Z���u��               {D|DA} [<addr> [<addr>|@<size>}]",
		"MS:MAC�A�h���X�ύX            MS **-**-**-**-**-**",
		"NS:�l�b�g���[�N�p�����[�^�ύX NS <IP Addr> <Sub net Mask> <Port>",
		"NR:�l�b�g���[�N�p�����[�^�Q�� NR",
		"CNS:���M��ݒ�                CNS <IP Addr> <Port>",
		"IDS:ID�ԍ��ݒ�                IDS <ID>",
		"HWT:�n�[�h�E�F�A�e�X�g        HWT [<number>}",
		"SWT:�\�t�g�E�F�A�e�X�g        SWT [<number>}",
		"MON:���x���O���j�^�N��        MON",
		0,
	};

	int	i=0;

	while (HelpText[i])
	{
		SendText(HelpText[i++]);
		SendText(CRLF);
	}
	Ctrl.Proc = ReadCmd;						// �R�}���h���͂ɖ߂�
}

//=============================================================================
/**
 * MON�R�}���h�̎��s
 */
//=============================================================================
void CmdMon(void)
{
	Ctrl.Proc = ReadCmd;

	(*(void (*)(void))MonAddr)();			/* ���j�^�v���O�����ɕ���		*/
}

//=============================================================================
/**
 * ASCII��16�i�e�L�X�g����o�C�i���ւ̕ϊ�
 *
 * @param Text	ASCII��16�i�e�L�X�g
 * @param Bin	�o�C�i���ϊ����ʂ̊i�[��
 * @param Len	�ϊ��Ώۃe�L�X�g�̃o�C�g��
 *
 * @retval TRUE �ϊ�����
 * @retval FALSE Text���ɕϊ��ł��Ȃ�����������
 *
 * @note Len==0�̏ꍇ��, Text�̏I�[(NULL)�܂ł��ϊ��ΏۂɂȂ�
 */
//=============================================================================
BOOL Asc2Hex(char *Text, UW *Bin, UH Len)
{
	BYTE	ch;
	WORD	i;

	if (Len == 0) Len = strlen(Text);
	for (*Bin=0,i=0; i<Len; i++)
	{
		ch = (BYTE)toupper(Text[i]);
		if		(ch >= '0' && ch <= '9') ch -= '0';
		else if (ch >= 'A' && ch <= 'F') ch -= 'A' - 10;
		else return FALSE;
		*Bin = (*Bin << 4) | (LONG)ch;
	}
	return TRUE;
}

//=============================================================================
/**
 * ASCII��10�i�e�L�X�g����o�C�i���ւ̕ϊ�
 *
 * @param Text	ASCII��10�i�e�L�X�g
 * @param Bin	�o�C�i���ϊ����ʂ̊i�[��
 * @param Len	�ϊ��Ώۃe�L�X�g�̃o�C�g��
 *
 * @retval TRUE �ϊ�����
 * @retval FALSE Text���ɕϊ��ł��Ȃ�����������
 *
 * @note Len==0�̏ꍇ��, Text�̏I�[(NULL)�܂ł��ϊ��ΏۂɂȂ�
 */
//=============================================================================
BOOL Asc2Dec(const char *Text, UW *Bin, UH Len)
{
	BYTE	ch;
	WORD	i;
	LONG	v;
	
	if (Len == 0) Len = strlen(Text);
	for (v=0,i=0; i<Len; i++)
	{
		ch = (BYTE)Text[i];
		if (ch >= '0' && ch <= '9') ch -= '0';
		else return FALSE;
		v = (v * 10) + (LONG)ch;
	}
	*Bin = v;
	return TRUE;
}

//=============================================================================
/**
 * �����O�f�[�^��ASCII��16�i�e�L�X�g�ɕϊ�
 *
 * @param hex	�ϊ��Ώۂ̃����O�f�[�^
 * @param Text	�ϊ����ʂ̊i�[��
 *
 * @return �Ȃ�
 */
//=============================================================================
void LongAsc(LONG hex, char *Text)
{
	Text[7] = HexAsc[hex & 0x0000000F]; hex >>= 4;
	Text[6] = HexAsc[hex & 0x0000000F]; hex >>= 4;
	Text[5] = HexAsc[hex & 0x0000000F]; hex >>= 4;
	Text[4] = HexAsc[hex & 0x0000000F]; hex >>= 4;
	Text[3] = HexAsc[hex & 0x0000000F]; hex >>= 4;
	Text[2] = HexAsc[hex & 0x0000000F]; hex >>= 4;
	Text[1] = HexAsc[hex & 0x0000000F]; hex >>= 4;
	Text[0] = HexAsc[hex & 0x0000000F];
}

//=============================================================================
/**
 * ���[�h�f�[�^��ASCII��16�i�e�L�X�g�ɕϊ�
 *
 * @param hex	�ϊ��Ώۂ̃��[�h�f�[�^
 * @param Text	�ϊ����ʂ̊i�[��
 *
 * @return �Ȃ�
 */
//=============================================================================
void WordAsc(WORD hex, char *Text)
{
	Text[3] = HexAsc[hex & 0x000F];	hex >>= 4;
	Text[2] = HexAsc[hex & 0x000F];	hex >>= 4;
	Text[1] = HexAsc[hex & 0x000F];	hex >>= 4;
	Text[0] = HexAsc[hex & 0x000F];
}

//=============================================================================
/**
 * �o�C�g�f�[�^��ASCII��16�i�e�L�X�g�ɕϊ�
 *
 * @param hex	�ϊ��Ώۂ̃o�C�g�f�[�^
 * @param Text	�ϊ����ʂ̊i�[��
 *
 * @return �Ȃ�
 */
//=============================================================================
void ByteAsc(BYTE hex, char *Text)
{
	Text[1] = HexAsc[hex & 0x0F]; hex >>= 4;
	Text[0] = HexAsc[hex & 0x0F];
}

//=============================================================================
/**
 * �G���[���b�Z�[�W���M
 *
 * @param Code	���b�Z�[�W�R�[�h
 *
 * @return �Ȃ�
 */
//=============================================================================
void SendMsg(BYTE Code)
{
	const char *MsgTable[] =
	{
		"",
		"*** Invalid Command",
		"*** Hex Value Error",
		"*** Invalid Parameter",
		"*** Invalid Address",
		"*** Odd Address",
		"*** Data Verify Error",
	};
	SendText(MsgTable[Code]);
	SendText(CRLF);
}

//=============================================================================
/**
 * �������݃`�F�b�N�m�f�f�[�^���M
 * 
 * @param addr	�������ُ݈�����o�����������A�h���X
 * @param wd	�������ɏ������񂾃f�[�^�l
 * @param rd	���������瓾��ꂽ�f�[�^�l
 *
 * @return �Ȃ�
 */
//=============================================================================
void SendVfy(BYTE *addr, BYTE wd, BYTE rd)
{
	memset(Ctrl.TxText,' ',sizeof Ctrl.TxText);
	LongAsc((LONG)addr,Ctrl.TxText);
	Ctrl.TxText[ 9] = 'W'; Ctrl.TxText[10] = ':'; ByteAsc(wd,Ctrl.TxText+11);
	Ctrl.TxText[14] = 'R'; Ctrl.TxText[15] = ':'; ByteAsc(rd,Ctrl.TxText+16);
	strcpy(Ctrl.TxText+18,CRLF);
	SendText(Ctrl.TxText);
	SendMsg(MSG_VERIFY);
}

//=============================================================================
/**
 * �^�[�~�i���ւ̕�����f�[�^���M
 *
 * @param Text	���M������(ASCIIZ)
 *
 * @return �Ȃ�
 */
//=============================================================================
void SendText(char *Text)
{
	switch (Ctrl.ComCh) {
	case SCI_CH:DebugOut(Text);
				break;
	case LAN_CH:telnetd_send(Text);
				break;
	}
}

//=============================================================================
/**
 * �^�[�~�i������̕�����f�[�^��M
 *
 * @param �Ȃ�
 *
 * @return �Ȃ�
 */
//=============================================================================
void ReadText(void)
{
	ER			ercd;
	T_MONMSG	*msg;

	memset(Ctrl.RxText,0,sizeof Ctrl.RxText);	// ��M�f�[�^��N���A
	ercd = trcv_mbx( MBX_MON, (T_MSG**)&msg, TMO_FEVR);
	if( ercd == E_OK) {
		memcpy( &Ctrl.RxText, &msg->buf, msg->cnt);
		Ctrl.ComCh = msg->eCom;
		rel_mpf( MPF_MON, (VP)msg);
	}
}

//==========================================================================
// [�@�\] �R�}���h��M
// [����] �R�}���h
// [�ߒl] ����
// [�⑫] 
//==========================================================================
BOOL GetCmd(char *s)
{
	int			nRead = 0;
	char		c;
	ER			ercd;
	T_MONMSG	*msg;
	
	ercd = tget_mpf( MPF_MON, &msg, TMO_FEVR);
	if( ercd == E_OK) {
		memset( &msg->buf, 0, sizeof MON_BUF_SIZE);	// ��M�f�[�^��N���A
	
		for(;;)
		{
			c = *s;
			s++;
			if( c < 0x00) break;
			if (c == 0x08)							// BackSpace
			{
				if (nRead) msg->buf[--nRead] = 0;
				continue;
			}
			if( nRead < MON_BUF_SIZE) {
				msg->buf[ nRead ] = c;				// ��M�f�[�^�ۑ�
				nRead++;
			}
			if( c == 0x00) {
				if( strcmp((B*)msg->buf, "BYE")) {	// �ؒf�v���H
					msg->cnt = nRead;
					msg->eCom = LAN_CH;
					snd_mbx( MBX_MON, (T_MSG*)msg);
					break;
				}
				else {
					Ctrl.Proc = ReadCmd;			// �R�}���h���͂ɖ߂�
					return FALSE;
				}
			}
		}
	}
	return TRUE;
}

//==========================================================================
/**
 * �R�}���h��M
 * 
 * @param ch �`�����l���ԍ�
 * 
 * @return �Ȃ�
 */
//==========================================================================
TASK MonRcvTask(INT ch)
{
	T_MONMSG *msg;
	ER ercd;
	UINT i;
	UB c;

    /* initialize */

	ini_sio(ch, "115200 B8 PN S1 XON");
	ctl_sio(ch, TSIO_RXE|TSIO_TXE|TSIO_RTSON);

	for (;;) {
		/* Get memory block */

		ercd = tget_mpf(MPF_MON, &msg, TMO_FEVR);

		/* Received one line */

		for (i = 0;;) {
//			ercd = get_sio(ch, &c, 5000/MSEC);
			ercd = get_sio(ch, &c, TMO_FEVR);
			if (ercd == E_TMOUT) {
				if ( i ) {
					break;
				}
				continue;
			}
			if (c == 0x08) {
				if ( i ) {
					msg->buf[--i] = 0;
				}
				continue;
			}
			if (c == '\n') {
				continue;
			}
			msg->buf[i] = c;
			if ((++i >= MON_BUF_SIZE - 1) || (c == '\r')) {
				break;
			}
		}
		msg->buf[i] = '\0';
		msg->ch  = ch;					// �`�����l���ݒ�
		msg->eCom = SCI_CH;
		msg->cnt = i;

		/* Send message */

		snd_mbx(MBX_MON, msg);
	}
}

//=============================================================================
/**
 * ���C���^�X�N
 *
 * @param ch �`�����l���ԍ�
 * 
 * @return �Ȃ�
 */
//=============================================================================
TASK MonTask(INT ch)
{
	InitMon(ch);								// �v���O�����̏�����
	
	while(1) Ctrl.Proc();						// ����V�[�P���X�쓮
}

//=============================================================================
/**
 * �^�[�~�i������̕�����f�[�^��M
 *
 * @param �Ȃ�
 *
 * @return �Ȃ�
 */
//=============================================================================
static void PasswdCheck(void)
{
	ER			ercd;
	T_MONMSG	*msg;
	int iLen;

	ercd = trcv_mbx( MBX_MON, (T_MSG**)&msg, TMO_FEVR);
	if( ercd == E_OK) {
		iLen = strlen(cMonPasswd);
		if (!memcmp(&msg->buf, cMonPasswd, iLen)) {
			Ctrl.Proc = ReadCmd;
		}
		rel_mpf( MPF_MON, (VP)msg);
	}
}
