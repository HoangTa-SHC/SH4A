//=============================================================================
//					�r�g-�S �b�o�t���W���[�� ���j�^�v���O����
//					<<< �������_���v�R�}���h���W���[�� >>>
//
// Version 1.00 1999.10.21 S.Nakano	�V�K
// Version 1.10 2001.12.07 S.Nakano	MCM(HJ945010BP)�Ή��ǉ�
//=============================================================================
#include "kernel.h"
#include "nonet.h"
#include "nonteln.h"
#include "mon.h"

//=============================================================================
// [�@�\] �c�R�}���h�̎��s
// [����] �Ȃ�
// [�ߒl] �Ȃ�
//=============================================================================
void CmdDump(void)
{
	switch (Ctrl.DumpUnit)						// ���O�̃_���v�P�ʌ���
	{
	case 'B': Ctrl.Proc = CmdDumpB; break;		// �o�C�g�P�ʃ_���v
	case 'W': Ctrl.Proc = CmdDumpW; break;		// ���[�h�P�ʃ_���v
	case 'L': Ctrl.Proc = CmdDumpL; break;		// �����O�P�ʃ_���v
	case 'A': Ctrl.Proc = CmdDumpA; break;		// �A�Z���u���_���v
	}
}

//=============================================================================
// [�@�\] �c�a�R�}���h�̎��s
// [����] �Ȃ�
// [�ߒl] �Ȃ�
//=============================================================================
void CmdDumpB(void)
{
	BYTE	code;
	long	rest;
	int		i;

	if (code = ParsParmD())						// �R�}���h�p�����[�^����
	{
		SendMsg(code);							// �p�����[�^�G���[
		Ctrl.Proc = ReadCmd;					// �R�}���h���͂ɖ߂�
		return;
	}

	for (rest=Ctrl.DumpSize; rest>0;)
	{
		memset(Ctrl.TxText,' ',sizeof Ctrl.TxText);
		LongAsc((LONG)Ctrl.DumpAddr,Ctrl.TxText);
		for (i=0; i<16 && rest>0; i++,rest--)	// �P�s���̃e�L�X�g�ҏW
		{
			code = *(Ctrl.DumpAddr++);			// �������f�[�^�擾
			ByteAsc(code,Ctrl.TxText+9+3*i);	// 16�i�e�L�X�g�ɕϊ�
			Ctrl.TxText[58+i] = HexChr[code];	// �Ή�ASCII�����Z�b�g
		}
		strcpy(Ctrl.TxText+58+i,CRLF);
		SendText(Ctrl.TxText);
	}
	Ctrl.DumpUnit = 'B';						// �o�C�g�P�ʃ_���v�ۑ�
	Ctrl.Proc = ReadCmd;						// �R�}���h���͂ɖ߂�
}

//=============================================================================
// [�@�\] �c�v�R�}���h�̎��s
// [����] �Ȃ�
// [�ߒl] �Ȃ�
//=============================================================================
void CmdDumpW(void)
{
	WORD*	addr;
	WORD	data;
	BYTE	code;
	long	rest;
	int		i;

	if (code = ParsParmD())						// �R�}���h�p�����[�^����
	{
		SendMsg(code);							// �p�����[�^�G���[
		Ctrl.Proc = ReadCmd;					// �R�}���h���͂ɖ߂�
		return;
	}

	addr = (WORD*)((LONG)Ctrl.DumpAddr & 0xFFFFFFFE);
	for (rest=Ctrl.DumpSize; rest>0;)
	{
		memset(Ctrl.TxText,' ',sizeof Ctrl.TxText);
		LongAsc((LONG)addr,Ctrl.TxText);
		for (i=0; i<8 && rest>0; i++,rest-=2)	// �P�s���̃e�L�X�g�ҏW
		{
			data = *(addr++);					// �������f�[�^�擾
			WordAsc(data,Ctrl.TxText+9+5*i);	// 16�i�e�L�X�g�ɕϊ�
		}
		strcpy(Ctrl.TxText+9+5*i,CRLF);
		SendText(Ctrl.TxText);
	}
	Ctrl.DumpAddr = (BYTE*)addr;				// ���_���v�A�h���X�ۑ�
	Ctrl.DumpUnit = 'W';						// ���[�h�P�ʃ_���v�ۑ�
	Ctrl.Proc = ReadCmd;						// �R�}���h���͂ɖ߂�
}

//=============================================================================
// [�@�\] �c�k�R�}���h�̎��s
// [����] �Ȃ�
// [�ߒl] �Ȃ�
//=============================================================================
void CmdDumpL(void)
{
	LONG*	addr;
	LONG	data;
	BYTE	code;
	long	rest;
	int		i;

	if (code = ParsParmD())						// �R�}���h�p�����[�^����
	{
		SendMsg(code);							// �p�����[�^�G���[
		Ctrl.Proc = ReadCmd;					// �R�}���h���͂ɖ߂�
		return;
	}

	addr = (LONG*)((LONG)Ctrl.DumpAddr & 0xFFFFFFFC);
	for (rest=Ctrl.DumpSize; rest>0;)
	{
		memset(Ctrl.TxText,' ',sizeof Ctrl.TxText);
		LongAsc((LONG)addr,Ctrl.TxText);
		for (i=0; i<4 && rest>0; i++,rest-=4)	// �P�s���̃e�L�X�g�ҏW
		{
			data = *(addr++);					// �������f�[�^�擾
			LongAsc(data,Ctrl.TxText+9+9*i);	// 16�i�e�L�X�g�ɕϊ�
		}
		strcpy(Ctrl.TxText+9+9*i,CRLF);
		SendText(Ctrl.TxText);
	}
	Ctrl.DumpAddr = (BYTE*)addr;				// ���_���v�A�h���X�ۑ�
	Ctrl.DumpUnit = 'L';						// �����O�P�ʃ_���v�ۑ�
	Ctrl.Proc = ReadCmd;						// �R�}���h���͂ɖ߂�
}

//-----------------------------------------------------------------------------
// �R�}���h�p�����[�^�̉���
//-----------------------------------------------------------------------------
BYTE ParsParmD(void)
{
	char*	Parm;
	LONG	dTop,dEnd,dSize;
	BOOL	fTop,fEnd,fSize;

	Ctrl.DumpSize = 256;						// �o�C�g���ȗ��l�Z�b�g
	dTop = dEnd = dSize = 0;					// �p�����[�^������
	fTop = fEnd = fSize = FALSE;				// �p�����[�^�t���O�N���A

	if (Parm = strtok(NULL," \x0D\t"))			// �p�����[�^�w�肠��
	{
		if (!(fTop = Asc2Hex(Parm,&dTop,0))) return MSG_BADADDR;

		if (Parm = strtok(NULL," \x0D\t"))
		{
			if (Parm[0] == '@')
			{
				if (!(fSize = Asc2Hex(Parm+1,&dSize,0))) return MSG_BADHEX;
				if (dSize == 0)							 return MSG_BADPARM;
			}
			else if (!(fEnd = Asc2Hex(Parm,&dEnd,0)))	 return MSG_BADADDR;
			else if (dTop > dEnd)						 return MSG_BADADDR;
		}
	}
	if (fTop)	Ctrl.DumpAddr = (BYTE*)dTop;	// �擪�A�h���X�w�肠��
	if (fEnd)	Ctrl.DumpSize = dEnd-dTop+1;	// �I�[�A�h���X�w�肠��
	if (fSize)	Ctrl.DumpSize = dSize;			// �o�C�g���w�肠��
	return MSG_NONE;
}
