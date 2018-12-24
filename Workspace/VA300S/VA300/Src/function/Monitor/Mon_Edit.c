//=============================================================================
/**
 *	@brief�r�g-�Q �b�o�t���W���[�� ���j�^�v���O����
 *	
 *	@file Mon_NS.c
 *	
 *	@date	2007/12/10
 *	@version 1.00 �������ҏW�R�}���h�}���h���W���[��
 *	@author	F.Saeki
 *
 *	@date	2011/4/7
 *	@version 1.20 OYO-105A�p�u�[�g�v���O�������痬�p
 *	@author	F.Saeki
 *
 *	Copyright (C) 2007, OYO Electric Corporation
 */
//=============================================================================
#include "kernel.h"
#include "nonet.h"
#include "nonteln.h"
#include "nonethw.h"
#include "mon.h"

static UB*	Addr;
static UB		Flag;

static UB ParsParm(void);		// �R�}���h�p�����[�^�̉���
static UB ParsParm0(void);		// �R�}���h�p�����[�^�̉���

//=============================================================================
/**
 * �l�R�}���h�̎��s
 */
//=============================================================================
void CmdEdit(void)
{
	char*	Resp;
	UB		bCode;
	UH		wCode;
	UW		lCode;
	BOOL	Verify;
	int		offs,unit;

	if (bCode = ParsParm())							// �p�����[�^����
	{
		SendMsg(bCode);								// �p�����[�^�G���[
		Ctrl.Proc = ReadCmd;						// �R�}���h���͂ɖ߂�
		return;
	}

	unit = 1;										// �o�C�g�P�ʂŏ�����
	
	switch (Ctrl.EditUnit)							// �A�N�Z�X�P�ʊm�F
	{
	case 'B': unit = 1; break;						// �o�C�g�P��
	case 'W': unit = 2; break;						// ���[�h�P��
	case 'L': unit = 4; break;						// �����O�P��
	}
	while (1)
	{
		memset(Ctrl.TxText,' ',sizeof Ctrl.TxText);
		LongAsc((UW)Ctrl.topAddr,Ctrl.TxText);	// �v�����v�g�ҏW
		offs = 9;
		if (Ctrl.Flag & FLAG_VERIFY) switch (Ctrl.EditUnit)
		{
		case 'B': ByteAsc(*(UB*)Ctrl.topAddr,Ctrl.TxText+9); offs = 12; break;
		case 'W': WordAsc(*(UH*)Ctrl.topAddr,Ctrl.TxText+9); offs = 14; break;
		case 'L': LongAsc(*(UW*)Ctrl.topAddr,Ctrl.TxText+9); offs = 18; break;
		}
		strcpy(Ctrl.TxText+offs,"? ");
		SendText(Ctrl.TxText);

		ReadText();									// �����e�L�X�g��M
		if (Resp = strtok(Ctrl.RxText," \x0D\t"))
		{
			if (Resp[0] == '.' && Resp[1] == 0) break;
			if (Resp[0] == '^' && Resp[1] == 0)
			{
				if (Ctrl.topAddr) Ctrl.topAddr -= unit;
				continue;
			}
			if (!Asc2Hex(Resp,&lCode,0))			// �X�V�f�[�^�l�擾
			{
				SendMsg(MSG_BADHEX); continue;
			}
			switch (Ctrl.EditUnit)					// �f�[�^��������
			{
			case 'B': *(UB*)Ctrl.topAddr = (bCode = lCode); break;
			case 'W': *(UH*)Ctrl.topAddr = (wCode = lCode); break;
			case 'L': *(UW*)Ctrl.topAddr = (lCode = lCode); break;
			}
			Verify = TRUE;							// �������݃`�F�b�N
			if (Ctrl.Flag & FLAG_VERIFY) switch (Ctrl.EditUnit)
			{
			case 'B': if (*(UB*)Ctrl.topAddr != bCode) Verify = FALSE; break;
			case 'W': if (*(UH*)Ctrl.topAddr != wCode) Verify = FALSE; break;
			case 'L': if (*(UW*)Ctrl.topAddr != lCode) Verify = FALSE; break;
			}
			if (!Verify) { SendMsg(MSG_VERIFY); continue; }
		}
		if (!(Ctrl.topAddr += unit)) break;			// �ΏۃA�h���X�X�V
	}
	Ctrl.Proc = ReadCmd;							// �R�}���h���͂ɖ߂�
}

//=============================================================================
/**
 * �l�a�R�}���h�̎��s
 */
//=============================================================================
void CmdEdit1(void)
{
	char*	Resp;
	UB		bCode;
	UW		lCode;
	BOOL	Verify;
	int		offs;

	Ctrl.EditUnit = 'B';							// �A�N�Z�X�P�ʊm�F

	if (bCode = ParsParm0())						// �p�����[�^����
	{
		SendMsg(bCode);								// �p�����[�^�G���[
		Ctrl.Proc = ReadCmd;						// �R�}���h���͂ɖ߂�
		return;
	}

	while (1)
	{
		memset(Ctrl.TxText,' ',sizeof Ctrl.TxText);
		LongAsc((UW)Addr,Ctrl.TxText);			// �v�����v�g�ҏW
		offs = 9;
		if (Flag != 'N')							// �����������e�Q��
		{
			ByteAsc(*(UB*)Addr,Ctrl.TxText+9);
			offs = 12;
		}
		strcpy((char*)(Ctrl.TxText+offs),"? ");
		SendText(Ctrl.TxText);

		ReadText();									// �����e�L�X�g��M
		if (Resp = strtok((char*)Ctrl.RxText," \x0D\t"))
		{
			if (Resp[0] == '.' && Resp[1] == 0)	break;	// �l�R�}���h�I��
			if (Resp[0] == '^' && Resp[1] == 0)
			{
				if (Addr) Addr --;					// �ΏۃA�h���X���
				continue;
			}
			if (!Asc2Hex(Resp,&lCode,0))			// �X�V�f�[�^�l�擾
			{
				SendMsg(MSG_BADHEX); continue;
			}
			*(UB*)Addr = (bCode = lCode);
			
			Verify = TRUE;							// �������݃`�F�b�N
			if (Flag != 'N')
			{
				if (*(UB*)Addr != bCode) Verify = FALSE;
			}
			if (!Verify) { SendMsg(MSG_VERIFY); continue; }
		}
		if (!(Addr ++)) break;						// �ΏۃA�h���X�X�V
	}
	Ctrl.Proc = ReadCmd;							// �R�}���h���͂ɖ߂�
}

//=============================================================================
/**
 * �l�v�R�}���h�̎��s
 */
//=============================================================================
void CmdEdit2(void)
{
	char*	Resp;
	UB		bCode;
	UH		wCode;
	UW		lCode;
	BOOL	Verify;
	int		offs,unit;

	Ctrl.EditUnit = 'W';							// �A�N�Z�X�P�ʊm�F

	if (bCode = ParsParm0())						// �p�����[�^����
	{
		SendMsg(bCode);								// �p�����[�^�G���[
		Ctrl.Proc = ReadCmd;						// �R�}���h���͂ɖ߂�
		return;
	}

	unit = 2;										// ���[�h�P��

	while (1)
	{
		memset(Ctrl.TxText,' ',sizeof Ctrl.TxText);
		LongAsc((UW)Addr,Ctrl.TxText);			// �v�����v�g�ҏW
		offs = 9;
		if (Flag != 'N')							// �����������e�Q��
		{
			WordAsc(*(UH*)Addr,Ctrl.TxText+9); offs = 14;
		}
		strcpy((char*)(Ctrl.TxText+offs),"? ");
		SendText(Ctrl.TxText);

		ReadText();									// �����e�L�X�g��M
		if (Resp = strtok((char*)Ctrl.RxText," \x0D\t"))
		{
			if (Resp[0] == '.' && Resp[1] == 0)	break;	// �l�R�}���h�I��
			if (Resp[0] == '^' && Resp[1] == 0)
			{
				if (Addr) Addr -= unit;				// �ΏۃA�h���X���
				continue;
			}
			if (!Asc2Hex(Resp,&lCode,0))			// �X�V�f�[�^�l�擾
			{
				SendMsg(MSG_BADHEX); continue;
			}
			*(UH*)Addr = (wCode = lCode);
			
			Verify = TRUE;							// �������݃`�F�b�N
			if (Flag != 'N')
			{
				if (*(UH*)Addr != wCode) Verify = FALSE;
			}
			if (!Verify) { SendMsg(MSG_VERIFY); continue; }
		}
		if (!(Addr += unit)) break;					// �ΏۃA�h���X�X�V
	}
	Ctrl.Proc = ReadCmd;							// �R�}���h���͂ɖ߂�
}

//=============================================================================
/**
 * �l�k�R�}���h�̎��s
 */
//=============================================================================
void CmdEdit3(void)
{
	char*	Resp;
	UB		bCode;
	UW		lCode;
	BOOL	Verify;
	int		offs,unit;

	if (bCode = ParsParm0())						// �p�����[�^����
	{
		SendMsg(bCode);								// �p�����[�^�G���[
		Ctrl.Proc = ReadCmd;						// �R�}���h���͂ɖ߂�
		return;
	}

	unit = 4;										// �����O�P��

	while (1)
	{
		memset(Ctrl.TxText,' ',sizeof Ctrl.TxText);
		LongAsc((UW)Addr,Ctrl.TxText);			// �v�����v�g�ҏW
		offs = 9;
		if (Flag != 'N')							// �����������e�Q��
		{
			LongAsc(*(UW*)Addr,Ctrl.TxText+9); offs = 18;
		}
		strcpy((char*)(Ctrl.TxText+offs),"? ");
		SendText(Ctrl.TxText);

		ReadText();									// �����e�L�X�g��M
		if (Resp = strtok((char*)Ctrl.RxText," \x0D\t"))
		{
			if (Resp[0] == '.' && Resp[1] == 0)	break;	// �l�R�}���h�I��
			if (Resp[0] == '^' && Resp[1] == 0)
			{
				if (Addr) Addr -= unit;				// �ΏۃA�h���X���
				continue;
			}
			if (!Asc2Hex(Resp,&lCode,0))			// �X�V�f�[�^�l�擾
			{
				SendMsg(MSG_BADHEX); continue;
			}
			*(UW*)Addr = (lCode = lCode);			// �f�[�^��������
			Verify = TRUE;							// �������݃`�F�b�N
			if (Flag != 'N')
			{
				if (*(UW*)Addr != lCode) Verify = FALSE;
			}
			if (!Verify) { SendMsg(MSG_VERIFY); continue; }
		}
		if (!(Addr += unit)) break;					// �ΏۃA�h���X�X�V
	}
	Ctrl.Proc = ReadCmd;							// �R�}���h���͂ɖ߂�
}

//-----------------------------------------------------------------------------
// �R�}���h�p�����[�^�̉���
//-----------------------------------------------------------------------------
static UB ParsParm(void)
{
	char*	Parm;
	UB	Unit;

	Unit = Ctrl.EditUnit;						// ���݂�Edit�P�ʎQ��
	Ctrl.Flag |= FLAG_VERIFY;					// �x���t�@�C����i���j

	if (!(Parm = strtok(NULL," \x0D\t")))		return MSG_BADADDR;
	if (!Asc2Hex(Parm,(UW*)&Ctrl.topAddr,0))	return MSG_BADADDR;

	while (Parm = strtok(NULL," \x0D\t"))		// �I�v�V�����p�����[�^�擾
	{
		if (Parm[1]) return MSG_BADPARM;		// �����ȃp�����[�^
		switch (Parm[0])
		{
		case 'B': Unit = 'B'; break;				// �o�C�g�A�N�Z�X
		case 'W': Unit = 'W'; break;				// ���[�h�A�N�Z�X
		case 'L': Unit = 'L'; break;				// �����O���[�h�A�N�Z�X
		case 'N': Ctrl.Flag &= ~FLAG_VERIFY; break;	// �x���t�@�C�Ȃ�
		default : return MSG_BADPARM;				// �����ȃp�����[�^
		}
	}
	if ((Unit == 'W') && ((UW)Ctrl.topAddr & 0x00000001)) return MSG_ODDADDR;
	if ((Unit == 'L') && ((UW)Ctrl.topAddr & 0x00000003)) return MSG_ODDADDR;

	Ctrl.EditUnit = Unit;						// �w��Edit�P�ʕۑ�
	return MSG_NONE;
}

//-----------------------------------------------------------------------------
// �R�}���h�p�����[�^�̉���
//-----------------------------------------------------------------------------
static UB ParsParm0(void)
{
	char*	Parm;
	UB	Unit;

	Unit = Ctrl.EditUnit;						// ���݂�Edit�P�ʎQ��
	Flag = 0x00;

	if (!(Parm = strtok(NULL," \x0D\t"))) return MSG_BADADDR;
	if (!Asc2Hex(Parm,(UW*)&Addr,0))	  return MSG_BADADDR;

	if ((Unit == 'W') && ((UW)Addr & 0x00000001)) return MSG_ODDADDR;
	if ((Unit == 'L') && ((UW)Addr & 0x00000003)) return MSG_ODDADDR;

	return MSG_NONE;
}
