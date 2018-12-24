//=============================================================================
/**
 *	�r�g-�S �b�o�t���W���[�� ���j�^�v���O����
 *	
 *	@file Mon_Test.c
 *	@version 1.00
 *	
 *	@author	F.Saeki
 *	@date	2001/12/07
 *	@brief	�n�[�h�E�F�A/�\�t�g�E�F�A�`�F�b�N�R�}���h���W���[��
 *
 *	Copyright (C) 2011, OYO Electric Corporation
 */
//=============================================================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kernel.h"
#include "nonet.h"
#include "nonteln.h"
#include "nonfile.h"
#include "mon.h"
#include "va300.h"
#include "nonethw.h"
#include "lan9118.h"
#include "drv_fpga.h"
#include "drv_led.h"
#include "drv_irled.h"
#include "drv_7seg.h"
#include "id.h"
#include "drv_tim.h"
#include "drv_cmr.h"
#include "command.h"

// �ϐ���`
static char* s_pcTestTarget;
static char* s_pcErrMsg;
static UB s_ubTestRW;							// ���[�h���C�g�e�X�g�p
static UH s_uhTestRW;							// ���[�h���C�g�e�X�g�p
static UW s_uwTestRW;							// ���[�h���C�g�e�X�g�p

// �v���g�^�C�v�錾
static void BufDisplay(UB *p);					// �o�b�t�@�\��
static void FpgaRegDisplay(void);				// FPGA���W�X�^�\��
static void LedTest(void);						// LED�\���e�X�g
static void SegTest(void);						// 7SEG�e�X�g
static void TestError(void);					// �e�X�g�G���[
static void SaveResult(void);					// �ۑ�
static void AccTestRamReadInit(void);			// RAM����Ǐo��������e�X�g������
static void AccTestRamReadTest(void);			// RAM����Ǐo��������e�X�g���s
static void AccTestRamWriteInit(void);			// RAM�ւ̏����݃e�X�g������
static void AccTestRamWriteTest(void);			// RAM�ւ̏����݃e�X�g���s
static void AccTestFpgaReadInit(void);			// FPGA����Ǐo��������e�X�g������
static void AccTestFpgaReadTest(void);			// FPGA����Ǐo��������e�X�g���s
static void AccTestFpgaWriteInit(void);			// FPGA�ւ̏����݃e�X�g������
static void AccTestFpgaWriteTest(void);			// FPGA�ւ̏����݃e�X�g���s
static void IrLedTest(void);					// �ԊO��LED�e�X�g
static void BuzTest(void);						// �u�U�[�e�X�g
static void DswTest(void);						// DSW�e�X�g
static void CmrCapTest(void);					// �L���v�`���[�e�X�g
static void saveCapData(char *fname, volatile UB *p, UW uwSize);	// �L���v�`���f�[�^�̕ۑ�
static void histogramSend(void) ;				// �q�X�g�O�������M
//static void CmrCmdTest(void);					// �J�����R�}���h�e�X�g
static void CmrPrmCmdTest(void);				// �J�����p�����[�^�R�}���h�e�X�g
static void CmrCapGetTest(void);				// �J������荞�݃e�X�g
static void FRomTest(void);						// �t���b�V���������̃e�X�g
//static void AccTestLanReadInit(void);			// LAN����Ǐo��������e�X�g������
//static void AccTestLanReadTest(void);			// LAN����ǂݏo��������e�X�g���s
//static void AccTestLanWriteInit(void);			// LAN�ւ̏����݃e�X�g������
//static void AccTestLanWriteTest(void);			// LAN�ւ̏����݃e�X�g���s
static void dispCompResult(long lVal1, long lVal2);	// ��r���ʂ�\������
//static void AccTestLanWriteRam(void);			// LAN�ւ̏����݃e�X�g�Q
static void LanCmdTest(void);					// LAN�R�}���h�e�X�g
static ER LanCmdTestSendCmd(char* p);			// LAN�̃R�}���h���R�}���h�����^�X�N�֑��M
static void MpfInfo(void);						// MPF��ԕ\��

extern ER lan_tst_ram(void);

//=============================================================================
/**
 * HWT�R�}���h�̎��s
 */
//=============================================================================
void CmdHardTest(void)
{
	char* Parm;

	Ctrl.Proc = ReadCmd;						// �R�}���h���͂ɖ߂�
	SendText("<<Hardware Test>>\r\n");
	
	if (Parm = strtok(NULL," \x0D\t")) {		// �p�����[�^�w�肠��
		switch(*Parm) {
		case '0':Ctrl.Proc = LedTest;
				 break;
		case '1':Ctrl.Proc = SegTest;
				 break;		 
		case '2':Ctrl.Proc = AccTestRamReadInit;	// SDRAM�Ǐo���e�X�g
				 break;
		case '3':Ctrl.Proc = AccTestRamWriteInit;	// SDRAM�����݃e�X�g
				 break;
		case '4':Ctrl.Proc = AccTestFpgaReadInit;	// FPGA�Ǐo���e�X�g
				 break;
		case '5':Ctrl.Proc = AccTestFpgaWriteInit;	// FPGA�����݃e�X�g
				 break;
//		case '7':Ctrl.Proc = AccTestLanReadInit;	// LAN�Ǐo���e�X�g
//				 break;
//		case '8':Ctrl.Proc = AccTestLanWriteInit;	// LAN�����݃e�X�g
//				 break;
//		case 'A':Ctrl.Proc = AccTestLanWriteRam;	// LAN�����݃e�X�g2
//				 break;
		default:SendText("HWT Option Error!\r\n");
				SendText("0:LED Test 1:7Seg Test\r\n");
				SendText("2:SRAM Read Test 3:SRAM Write Test \r\n");
				SendText("4:FPGA Read Test 5:FPGA Write Test\r\n");
//				SendText("7:LAN Read Test 8:LAN Write Test\r\n");
//				SendText("A:LAN RAM Write Test\r\n");
				Ctrl.Proc = ReadCmd;
				break;
		}
	} else {
		Ctrl.Proc = FpgaRegDisplay;				// FPGA���W�X�^�̓��e�\��
	}
}

//=============================================================================
/**
 * SWT�R�}���h�̎��s
 */
//=============================================================================
void CmdSoftTest(void)
{
	char* Parm;

	SendText("<<Function Test>>\r\n");
	
	if (Parm = strtok(NULL," \x0D\t")) {		// �p�����[�^�w�肠��
		switch(*Parm) {
		case '1':Ctrl.Proc = BuzTest;			// �u�U�[�e�X�g
				 break;
		case '2':Ctrl.Proc = IrLedTest;			// �ԊO��LED�e�X�g
				 break;
		case '3':Ctrl.Proc = DswTest;			// DSW�e�X�g
				 break;
		case '4':Ctrl.Proc = CmrCapTest;		// �J�����̃L���v�`���[�e�X�g
				 break;
		case '5':Ctrl.Proc = CmrCapGetTest;		// �J�����̃L���v�`���[���؂�o���e�X�g
				 break;
		case '6':Ctrl.Proc = CmrCmdTest;		// �J�����̃R�}���h�e�X�g
				 break;
		case '7':Ctrl.Proc = CmrPrmCmdTest;		// �J�����̃p�����[�^�R�}���h�e�X�g
				 break;
		case '8':Ctrl.Proc = FRomTest;			// �t���b�V���������e�X�g
				 break;
		case 'L':Ctrl.Proc = LanCmdTest;		// LAN�R�}���h�e�X�g
				 break;
		case 'B':Ctrl.Proc = MpfInfo;			// MPF��ԕ\��
				 break;
		default:SendText("SWT Option Error!\r\n");
				SendText("1:Buzzer Test <Option> 1-4\r\n");
				SendText("2:IR LED Test\r\n");
				SendText("3:Dsw Test\r\n");
				SendText("4:Capture Test <Option> 0-4\r\n");
				SendText("5:Camera Trim Test 6:Camera Command Test\r\n");
				SendText("7:Camera Parameter Command Test\r\n");
				SendText("8:Flash ROM Test\r\n");
				SendText("B:Memory Pool Status\r\n");
				SendText("L:LAN Command Test(Option 1 or 2)\r\n");
				Ctrl.Proc = ReadCmd;
				break;
		}
	} else {
		Ctrl.Proc = MpfInfo;					// �������[�v�[����ԕ\��
	}
}

//-----------------------------------------------------------------------------
// FPGA���W�X�^�\��
//-----------------------------------------------------------------------------
static void FpgaRegDisplay(void)
{
	int	 iSize, iCnt, iLine;
	
	iCnt  = 0;
	iLine = 0;
	SendText( "<<FPGA REG>>\r\n");
	while(iSize = FpgaRegDisplayLine(iLine, &Ctrl.TxText)) {
		iCnt += iSize;
		if (iCnt >= 80) {
			iCnt = 0;
			SendText( CRLF);				/* ���s */
		}
		SendText(Ctrl.TxText);
		iLine++;
	}
	SendText(CRLF);									// ���s

	// Mode
//	_sprintf(&Ctrl.TxText, "<<Mode Number>>\nMode = %d\r\n", MdGetMachineMode());
//	SendText(Ctrl.TxText);

	Ctrl.Proc = ReadCmd;
}

//-----------------------------------------------------------------------------
// LED�_���e�X�g
//-----------------------------------------------------------------------------
static void LedTest(void)
{
	static const int iLed[] = {
		LED_POW,						///< �d��LED
		LED_OK,							///< OK LED
		LED_ERR,						///< ERR LED
		LED_DBG1,						///< �f�o�b�O�pLED1
		LED_DBG2,						///< �f�o�b�O�pLED2
		LED_DBG3,						///< �f�o�b�O�pLED3
		LED_DBG4,						///< �f�o�b�O�pLED4
	};
	int i;
	static const int nLed = sizeof iLed / sizeof iLed[ 0 ];
	
	SendText("<LED TEST>\r\n...");
	
	LedOut(LED_ALL,  LED_OFF);
	
	// LED�_�����E�F�C�g������
	for(i = 0;i < nLed;i++) {
		LedOut(iLed[ i ], LED_ON);		// ON�e�X�g
		dly_tsk((500/MSEC));
		LedOut(iLed[ i ], LED_OFF);		// OFF�e�X�g
	}
	
	Ctrl.Proc = ReadCmd;
	
	SendText("END\r\n");
}

//-----------------------------------------------------------------------------
// 7Seg�_���e�X�g
//-----------------------------------------------------------------------------
static void SegTest(void)
{
	ER ercd;
	int n, i;
	enum LED_7SEG_SEL eSeg[] = {LED_7SEG_1, LED_7SEG_2, LED_7SEG_3, LED_7SEG_4,
								LED_7SEG_5, LED_7SEG_6, LED_7SEG_7, LED_7SEG_8};
	
	SendText("<7SEG TEST>\r\n...");
	
	for (i = 0;i < (sizeof eSeg / sizeof eSeg[0]);i++) {
		ercd = E_OK;
		n = 0;
	
		// 7SEG�_�����E�F�C�g������
		while(ercd == E_OK) {
			ercd = Led7SegOut(eSeg[ i ], n);		// ON�e�X�g
			dly_tsk((500/MSEC));
			n++;
		}
	}

	Ctrl.Proc = ReadCmd;
	
	SendText("END\r\n");
}

//-----------------------------------------------------------------------------
// �G���[�I��
//-----------------------------------------------------------------------------
static void TestError(void)
{
	SaveResult();								// ���ʕۑ�
	Ctrl.Proc = ReadCmd;						// �R�}���h���͂ɖ߂�
}

//-----------------------------------------------------------------------------
// �q�`�l�h���C�u�̃t�@�C���Ɍ��ʏ�����
//-----------------------------------------------------------------------------
static void SaveResult(void)
{
	FILE*	pfile;

	pfile = fopen("A:\\HardTest.csv", "w");
	if (pfile == NULL) {
		return;
	}
	fputs(s_pcTestTarget, pfile);
	fputs(s_pcErrMsg, pfile);
	
	fclose(pfile);
	
	SendText(s_pcErrMsg);
}

//-----------------------------------------------------------------------------
// SDRAM����Ǐo��������e�X�g(������)
//-----------------------------------------------------------------------------
static void AccTestRamReadInit(void)
{
	SendText("SRAM Read Test...");
	
	Ctrl.Proc = AccTestRamReadTest;
}

//-----------------------------------------------------------------------------
// SDRAM����Ǐo��������e�X�g(���s)
//-----------------------------------------------------------------------------
static void AccTestRamReadTest(void)
{
	volatile UW uwTemp;
	
	uwTemp = s_ubTestRW;
	uwTemp = s_uhTestRW;
	uwTemp = s_uwTestRW;
}

//-----------------------------------------------------------------------------
// SDRAM�֏����݂�������e�X�g(������)
//-----------------------------------------------------------------------------
static void AccTestRamWriteInit(void)
{
	SendText("SRAM Write Test...");

	s_ubTestRW = 0;
	s_uhTestRW = 0;
	s_uwTestRW = 0;	

	Ctrl.Proc = AccTestRamWriteTest;
}

//-----------------------------------------------------------------------------
// SDRAM�֏����݂�������e�X�g(���s)
//-----------------------------------------------------------------------------
static void AccTestRamWriteTest(void)
{
	s_ubTestRW++;
	s_uhTestRW++;
	s_uwTestRW++;
}

//-----------------------------------------------------------------------------
// FPGA����Ǐo��������e�X�g(������)
//-----------------------------------------------------------------------------
static void AccTestFpgaReadInit(void)
{
	SendText("FPGA Read Test...");
	
	Ctrl.Proc = AccTestFpgaReadTest;
}

//-----------------------------------------------------------------------------
// FPGA����Ǐo��������e�X�g(���s)
//-----------------------------------------------------------------------------
static void AccTestFpgaReadTest(void)
{
	volatile UH uwTemp;
	
	uwTemp = FPGA_VER;
}

//-----------------------------------------------------------------------------
// FPGA�֏����݂�������e�X�g(������)
//-----------------------------------------------------------------------------
static void AccTestFpgaWriteInit(void)
{
	SendText("FPGA Write Test...");
	
	Ctrl.Proc = AccTestFpgaWriteTest;
}

//-----------------------------------------------------------------------------
// FPGA�֏����݂�������e�X�g(���s)
//-----------------------------------------------------------------------------
static void AccTestFpgaWriteTest(void)
{
	fpga_outw(TPL_DBYTE, 0x1000);
}

//-----------------------------------------------------------------------------
// LAN���W�X�^����Ǐo��������e�X�g(������)
//-----------------------------------------------------------------------------
//static void AccTestLanReadInit(void)
//{
//	SendText("LAN Read Test...");
//	
//	Ctrl.Proc = AccTestLanReadTest;
//}

//-----------------------------------------------------------------------------
// LAN����Ǐo��������e�X�g(���s)
//-----------------------------------------------------------------------------
//static void AccTestLanReadTest(void)
//{
//	s_uhTestRW = lan_inw(LAN_CR);
//}

//-----------------------------------------------------------------------------
// LAN�֏����݃e�X�g(������)
//-----------------------------------------------------------------------------
//static void AccTestLanWriteInit(void)
//{
//	SendText("LAN Write Test...");
//	
//	s_uhTestRW = 0;
//	lan_outl(LAN_CR, 0x63);
//	Ctrl.Proc = AccTestLanWriteTest;
//}

//-----------------------------------------------------------------------------
// LAN�֏����݃e�X�g(���s)
//-----------------------------------------------------------------------------
//static void AccTestLanWriteTest(void)
//{
//	Ctrl.Proc = ReadCmd;
//	s_uhTestRW++;
//	lan_outh(LAN_PAR0, 0x55);
//	lan_outl(LAN_PAR1, 0xaa);
//	lan_outh(LAN_PAR2, 0x5a);
//	lan_outl(LAN_PAR3, 0xa5);
//	lan_outh(LAN_PAR4, 0x00);
//	lan_outl(LAN_PAR5, 0xff);
//	if (lan_inh(LAN_PAR0)!= 0x55) {
//		dispCompResult(lan_inh(LAN_PAR0), 0x55);
//		return;
//	}
//	if (lan_inl(LAN_PAR1)!= 0xaa) {
//		dispCompResult(lan_inh(LAN_PAR1), 0xaa);
//		return;
//	}
//	if (lan_inh(LAN_PAR2)!= 0x5a) {
//		dispCompResult(lan_inh(LAN_PAR2), 0x5a);
//		return;
//	}
//	if (lan_inl(LAN_PAR3)!= 0xa5) {
//		dispCompResult(lan_inh(LAN_PAR3), 0xa5);
//		return;
//	}
//	if (lan_inh(LAN_PAR4)!= 0x00) {
//		dispCompResult(lan_inh(LAN_PAR4), 0x00);
//		return;
//	}
//	if (lan_inl(LAN_PAR5)!= 0xff) {
//		dispCompResult(lan_inh(LAN_PAR5), 0xff);
//		return;
//	}
//	_sprintf(&Ctrl.TxText, "[ %d ] :OK  ", s_uhTestRW);
//	SendText(Ctrl.TxText);
//
#if 0
	if (lan_tst_ram() != E_OK) {
		SendText("lan_tst_ram() error.");
		return;
	}
	SendText("lan_tst_ram() ok.");
#endif
//}

//-----------------------------------------------------------------------------
// ��r���ʂ�\������
//-----------------------------------------------------------------------------
static void dispCompResult(long lVal1, long lVal2)
{
	_sprintf(&Ctrl.TxText, "A: %X != B: %X\r\n", lVal1, lVal2);
	SendText(Ctrl.TxText);
}

//-----------------------------------------------------------------------------
// �R�}���h�e�X�g
//-----------------------------------------------------------------------------
static void LanCmdTest(void)
{
	const char *LanCmd1[] = {
		"R90",								// R90�R�}���h�e�X�g
		0
	};
	int i;
	char *cParm;
	char **LanCmd;
	
	i = 0;
	LanCmd = LanCmd1;
	
	while(LanCmd[ i ]) {
		if (LanCmdTestSendCmd(LanCmd[ i ]) == E_OK) {
			dly_tsk(10/MSEC);
			i++;
		} else {
			SendText("Error.\r\n");
			dly_tsk(1000/MSEC);
//			break;
		}
	}
	Ctrl.Proc = ReadCmd;				// �R�}���h���͂ɖ߂�
}

//-----------------------------------------------------------------------------
// �R�}���h�����b�Z�[�W�ő��M����
//-----------------------------------------------------------------------------
static ER LanCmdTestSendCmd(char* p)
{
	ER ercd;
	T_COMMSG *msg;
	
	ercd = tget_mpf( MPF_COM, (VP*)&msg, (1000/MSEC));	// �������[�v�[���l��
	if (ercd == E_OK) {
		strcpy(&msg->buf, p);
		msg->cnt = strlen( p );
		ercd = snd_mbx(MBX_CMD_LAN, (T_MSG*)msg);
	}
	return ercd;
}

//-----------------------------------------------------------------------------
// MPF��Ԏ擾�R�}���h
//-----------------------------------------------------------------------------
static void MpfInfo(void)
{
	int i;
	
	i = 1;
	
	SendText("<MPF Status>\r\n");
	for(i = 1;i < MPF_MAX;i++) {
		if (MpfUseDisplayLine(i, &Ctrl.TxText) == TRUE) {
			SendText(Ctrl.TxText);
		}
	}
	
	Ctrl.Proc = ReadCmd;				// �R�}���h���͂ɖ߂�
}

//-----------------------------------------------------------------------------
// �u�U�[�e�X�g
//-----------------------------------------------------------------------------
static void BuzTest(void)
{
	char *cParm;
	enum SOUND_CTRL eCtrl;
	
	SendText("<BUZZER TEST>\r\n...");

	eCtrl = SOUND_CTRL_EMG_OFF;
	
	// �p�����[�^�w�肠��Ƃ��̓R�}���h�̓��e��ς���
	if (cParm = strtok(NULL, " \x0D\t")) {		// �p�����[�^�w��L
		switch(*cParm) {
		case '1':						// no break
		case '2':						// no break
		case '3':						// no break
		case '4':eCtrl = (enum SOUND_CTRL)(*cParm - '1');
				 break;
		}
	}
	SoundCtrl(eCtrl);					// �T�E���h����

	Ctrl.Proc = ReadCmd;				// �R�}���h���͂ɖ߂�
}

//-----------------------------------------------------------------------------
// �ԊO��LED�e�X�g
//-----------------------------------------------------------------------------
static void IrLedTest(void)
{
	const UH uhAllLed = IR_LED_1 | IR_LED_2 | IR_LED_3 | IR_LED_4 | IR_LED_5;
	
	SendText("<IR LED TEST>\r\n...");
	IrLedCrlOff( uhAllLed );
	IrLedSet(IR_LED_1, 50);
	IrLedSet(IR_LED_2, 75);
	IrLedSet(IR_LED_3, 100);
	IrLedSet((IR_LED_4 | IR_LED_5), 125);
	
	dly_tsk( 1000/MSEC );
	
	IrLedCrlOn( IR_LED_1 );				// �ԊO��LED1�_��
	dly_tsk( 1000/MSEC );
	IrLedCrlOn( IR_LED_2 );				// �ԊO��LED2�_��
	IrLedCrlOff( IR_LED_1 );
	dly_tsk( 1000/MSEC );
	IrLedCrlOn( IR_LED_3 );				// �ԊO��LED3�_��
	IrLedCrlOff( IR_LED_2 );
	dly_tsk( 1000/MSEC );
	IrLedCrlOn( IR_LED_4 );				// �ԊO��LED4�_��
	IrLedCrlOff( IR_LED_3 );
	dly_tsk( 1000/MSEC );
	IrLedCrlOn( IR_LED_5 );				// �ԊO��LED5�_��
	IrLedCrlOff( IR_LED_4 );
	dly_tsk( 1000/MSEC );
	IrLedCrlOff( IR_LED_5 );
	
	IrLedCrlOff( uhAllLed );			// �S�ԊO��LED����

	Ctrl.Proc = ReadCmd;				// �R�}���h���͂ɖ߂�
}

//-----------------------------------------------------------------------------
// DSW�e�X�g
//-----------------------------------------------------------------------------
static void DswTest(void)
{
	UH uhDsw, uhDswOld;
	UH uhCount;
	const UH uhCountInit = 500;
	
	uhDswOld = DswGet();
	uhCount  = uhCountInit;
	
	SendText("< DSW Test >\r\n");
	
	while( uhCount ) {
		uhDsw = DswGet();
		if (uhDswOld == uhDsw) {
			dly_tsk( 10 / MSEC );
			uhCount--;
		} else {
			i_sprintf(&Ctrl.TxText, "DSW = %04X\r\n", uhDsw);
			SendText(Ctrl.TxText);
			uhDswOld = uhDsw;
			uhCount = uhCountInit;
		}
	}

	Ctrl.Proc = ReadCmd;				// �R�}���h���͂ɖ߂�
}

//-----------------------------------------------------------------------------
// �J�����L���v�`���[�e�X�g
//-----------------------------------------------------------------------------
static void CmrCapTest(void)
{
	const int ciX = 320;
	const int ciY = 180;
	char *cParm;
	enum CAP_MODE eMode;
	ER ercd;
	
	SendText("<Capture Test>\r\n...");

	eMode = CAP_MODE_0;
	
	// �p�����[�^�w�肠��Ƃ��̓R�}���h�̓��e��ς���
	if (cParm = strtok(NULL, " \x0D\t")) {		// �p�����[�^�w��L
		switch(*cParm) {
		case '0':						// no break;
		case '1':						// no break
		case '2':						// no break
		case '3':						// no break
		case '4':eMode = (enum CAP_MODE)(cParm - '0');
				 break;
		}
	}
//	ercd = CmrCapture(eMode, (1000 / MSEC));
	if (ercd == E_OK) {					// �L���v�`���[
//		ercd = CmrCapGet(ciX, ciY, GET_IMG_SIZE_X, GET_IMG_SIZE_Y, g_ubCapTest);
		saveCapData("A:\\RawData.raw", g_ubCapTest, (CAP_X_SIZE * CAP_Y_SIZE));
		histogramSend();
	} else {
		i_sprintf(&Ctrl.TxText, "�G���[: %X\r\n", ercd);
		SendText(Ctrl.TxText);
	}
	Ctrl.Proc = ReadCmd;				// �R�}���h���͂ɖ߂�
}

//-----------------------------------------------------------------------------
// �L���v�`���[�f�[�^�̕ۑ�
//-----------------------------------------------------------------------------
static void saveCapData(char *fname, volatile UB *p, UW uwSize)
{
	FILE*	pfile;

	pfile = fopen(fname, "w");
	if (pfile == NULL) {
		return;
	}
	fwrite(p, 1, uwSize, pfile);
	
	fclose(pfile);
	
	i_sprintf(&Ctrl.TxText, "Save Capture data.(%s)\r\n", fname);
	SendText(Ctrl.TxText);
}

//-----------------------------------------------------------------------------
// �q�X�g�O�������M
//-----------------------------------------------------------------------------
static void histogramSend(void) 
{
	int i;
	
	SendText("(Histogram Send)");
	
	for (i = 0;i < 256;i++) {
		if ((i % 4) == 0) {
			SendText("\r\n");
		}
//		i_sprintf(&Ctrl.TxText, "[%03d] %08X ", i, CmrHistogramGet( i ));
		SendText(Ctrl.TxText);
	}
}

//-----------------------------------------------------------------------------
// �J�����R�}���h�e�X�g
//-----------------------------------------------------------------------------
/*static void CmrCmdTest(void)
{
	char *cParm;
	const UB cCmd[] = { 0x00, 0x00, 0x10, 0x00 };
	const int iRecvSize = 5;
	UB	cBuf[ 10 ];
	UB	*p;
	ER	ercd;
	
	SendText("<Camera Command Test>\r\n...");
	
	ercd = CmrPktSend( cCmd, sizeof cCmd, iRecvSize);	// �R�}���h���M
	if (ercd == E_OK) {
		ercd = CmrPktRecv( cBuf, iRecvSize, (5000 / MSEC));		// ������M
		if (ercd == E_OK) {
			if ((cBuf[ 0 ] & 0x80) && (cBuf[ 2 ] == 0x10) && (cBuf[ 3 ] == 0x01)
				&& (cBuf[ 4 ] == 0x00)) {
				SendText("Response OK!\r\n");
			} else {
				SendText("Response NG!\r\n");
				i_sprintf(&Ctrl.TxText, 
					"[0]: %02X [1]: %02X [2]: %02X [3] %02X [4] %02X\r\n",
					 cBuf[ 0 ], cBuf[ 1 ], cBuf[ 2 ], cBuf[ 3 ], cBuf[ 4 ]);
				SendText(Ctrl.TxText);
			}
		} else {
			SendText("CmrPktRecv Error.\r\n");
		}
	} else {
		SendText("CmrPktSend Error.\r\n");
	}
	
	Ctrl.Proc = ReadCmd;				// �R�}���h���͂ɖ߂�
}
*/

//-----------------------------------------------------------------------------
// �J�����摜�擾�e�X�g
//-----------------------------------------------------------------------------
static void CmrCapGetTest(void)
{
	const int ciX = 320;
	const int ciY = 180;
	ER	ercd;
	
	SendText("<Camera Capture Get Test>\r\n");
//	ercd = CmrCapGet(ciX, ciY, GET_IMG_SIZE_X, GET_IMG_SIZE_Y, g_ubCapTest);
	if (ercd == E_OK) {
		saveCapData("A:\\PartData.raw", g_ubCapTest, 
			((GET_IMG_SIZE_X - ciX) * (GET_IMG_SIZE_Y - ciY)));
	} else {
		SendText("Error.\r\n");
	}
	
	Ctrl.Proc = ReadCmd;				// �R�}���h���͂ɖ߂�
}

//-----------------------------------------------------------------------------
// �J�����p�����[�^�ݒ�R�}���h�e�X�g
//-----------------------------------------------------------------------------
static void CmrPrmCmdTest(void)
{
	ER	ercd;
	const UH cuhPrmAes = 0x12;
	const UH cuhPrmShutter = 0x34;
	const UH cuhPrmWaitCtrl = 0x56;
	const UH cuhBau = 0x05;
	UH uhBauOrg;
	
	SendText("<Camera Parameter Command Test>\r\n");
	
	CmrPrmAesSet(cuhPrmAes);
	if (fpga_inw(CMR_PRM_AES) != cuhPrmAes) {
		SendText("AES Set Error!\r\n");
	}
	
	CmrPrmShutterSet(cuhPrmShutter);
	if (fpga_inw(CMR_PRM_SHT) != cuhPrmShutter) {
		SendText("Fix Shutter Control Set Error!\r\n");
	}
	
	CmrWaitCrl(cuhPrmWaitCtrl);
	if (fpga_inw(CMR_MOD_WAIT) != cuhPrmWaitCtrl) {
		SendText("MODE4 Wait Control Set Error!\r\n");
	}
	
	uhBauOrg = fpga_inw(CMR_BAU);
	CmrBaudrateSet(cuhBau);
	if (fpga_inw(CMR_BAU) != cuhBau) {
		SendText("Baudrate Set Error!\r\n");
	}
	CmrBaudrateSet(uhBauOrg);
	
	Ctrl.Proc = ReadCmd;				// �R�}���h���͂ɖ߂�
}

//-----------------------------------------------------------------------------
// �t���b�V���������̃e�X�g
//-----------------------------------------------------------------------------
static void FRomTest(void)
{
	ER	ercd;

	
	SendText("<Flash ROM Test>\r\n");
	
	
	
	Ctrl.Proc = ReadCmd;				// �R�}���h���͂ɖ߂�
}
