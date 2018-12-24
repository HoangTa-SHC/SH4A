//=============================================================================
//					�r�g-�S �b�o�t���W���[�� ���j�^�v���O����
//				<<< �������`�F�b�N�R�}���h�}���h���W���[�� >>>
//
// Version 1.00 1999.10.22 S.Nakano	�V�K
// Version 1.10 2001.12.07 S.Nakano	MCM(HJ945010BP)�Ή��ǉ�
//=============================================================================
#include "kernel.h"
#include "nonet.h"
#include "nonteln.h"
#include "mon.h"

static BYTE ParsParm(void);		// �R�}���h�p�����[�^�̉���

//=============================================================================
// [�@�\] �b�R�}���h�̎��s
// [����] �Ȃ�
// [�ߒl] �Ȃ�
//=============================================================================
void CmdCheck(void)
{
	BYTE	wCode,rCode;
	BYTE*	addr;

	Ctrl.Proc = ReadCmd;
	if (wCode = ParsParm())						// �R�}���h�p�����[�^����
	{
		SendMsg(wCode); return;					// �p�����[�^�G���[
	}

	for (addr=Ctrl.topAddr; addr<=Ctrl.endAddr; addr++)
	{
		*addr = (BYTE)(LONG)addr;				// �A�h���X���ʂP�o�C�g��������
	}
	for (addr=Ctrl.topAddr; addr<=Ctrl.endAddr; addr++)
	{
		wCode = (BYTE)(LONG)addr;				// �x���t�@�C
		rCode = *addr;
		if (wCode != rCode) { SendVfy(addr,wCode,rCode); return; }
	}
	for (addr=Ctrl.topAddr; addr<=Ctrl.endAddr; addr++)
	{
		*addr = ~(BYTE)(LONG)addr;				// �r�b�g���]�f�[�^��������
	}
	for (addr=Ctrl.topAddr; addr<=Ctrl.endAddr; addr++)
	{
		wCode = ~(BYTE)(LONG)addr;				// �x���t�@�C
		rCode = *addr;
		if (wCode != rCode) { SendVfy(addr,wCode,rCode); return; }
	}
	SendText("Complete!" CRLF);					// �������`�F�b�N����
}

//-----------------------------------------------------------------------------
// �R�}���h�p�����[�^�̉���
//-----------------------------------------------------------------------------
static BYTE ParsParm(void)
{
	char*	Parm;
	LONG	nByte;

	if (!(Parm = strtok(NULL," \x0D\t")))		return MSG_BADADDR;
	if (!Asc2Hex(Parm,(LONG*)&Ctrl.topAddr,0))	return MSG_BADADDR;
	if (!(Parm = strtok(NULL," \x0D\t")))		return MSG_BADPARM;
	if (Parm[0] == '@')
	{
		if (!Asc2Hex(Parm+1,&nByte,0))	return MSG_BADHEX;
		if (nByte == 0)					return MSG_BADPARM;
		Ctrl.endAddr = Ctrl.topAddr + nByte - 1;
	}
	else
	{
		if (!Asc2Hex(Parm,(LONG*)&Ctrl.endAddr,0))		return MSG_BADADDR;
		if ((LONG)Ctrl.topAddr > (LONG)Ctrl.endAddr)	return MSG_BADADDR;
	}
	return MSG_NONE;
}
