//=============================================================================
/**
 *	ＳＨ-４ ＣＰＵモジュール モニタプログラム
 *	
 *	@file Mon_Test.c
 *	@version 1.00
 *	
 *	@author	F.Saeki
 *	@date	2001/12/07
 *	@brief	ハードウェア/ソフトウェアチェックコマンドモジュール
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

// 変数定義
static char* s_pcTestTarget;
static char* s_pcErrMsg;
static UB s_ubTestRW;							// リードライトテスト用
static UH s_uhTestRW;							// リードライトテスト用
static UW s_uwTestRW;							// リードライトテスト用

// プロトタイプ宣言
static void BufDisplay(UB *p);					// バッファ表示
static void FpgaRegDisplay(void);				// FPGAレジスタ表示
static void LedTest(void);						// LED表示テスト
static void SegTest(void);						// 7SEGテスト
static void TestError(void);					// テストエラー
static void SaveResult(void);					// 保存
static void AccTestRamReadInit(void);			// RAMから読出し続けるテスト初期化
static void AccTestRamReadTest(void);			// RAMから読出し続けるテスト実行
static void AccTestRamWriteInit(void);			// RAMへの書込みテスト初期化
static void AccTestRamWriteTest(void);			// RAMへの書込みテスト実行
static void AccTestFpgaReadInit(void);			// FPGAから読出し続けるテスト初期化
static void AccTestFpgaReadTest(void);			// FPGAから読出し続けるテスト実行
static void AccTestFpgaWriteInit(void);			// FPGAへの書込みテスト初期化
static void AccTestFpgaWriteTest(void);			// FPGAへの書込みテスト実行
static void IrLedTest(void);					// 赤外線LEDテスト
static void BuzTest(void);						// ブザーテスト
static void DswTest(void);						// DSWテスト
static void CmrCapTest(void);					// キャプチャーテスト
static void saveCapData(char *fname, volatile UB *p, UW uwSize);	// キャプチャデータの保存
static void histogramSend(void) ;				// ヒストグラム送信
//static void CmrCmdTest(void);					// カメラコマンドテスト
static void CmrPrmCmdTest(void);				// カメラパラメータコマンドテスト
static void CmrCapGetTest(void);				// カメラ取り込みテスト
static void FRomTest(void);						// フラッシュメモリのテスト
//static void AccTestLanReadInit(void);			// LANから読出し続けるテスト初期化
//static void AccTestLanReadTest(void);			// LANから読み出し続けるテスト実行
//static void AccTestLanWriteInit(void);			// LANへの書込みテスト初期化
//static void AccTestLanWriteTest(void);			// LANへの書込みテスト実行
static void dispCompResult(long lVal1, long lVal2);	// 比較結果を表示する
//static void AccTestLanWriteRam(void);			// LANへの書込みテスト２
static void LanCmdTest(void);					// LANコマンドテスト
static ER LanCmdTestSendCmd(char* p);			// LANのコマンドをコマンド処理タスクへ送信
static void MpfInfo(void);						// MPF状態表示

extern ER lan_tst_ram(void);

//=============================================================================
/**
 * HWTコマンドの実行
 */
//=============================================================================
void CmdHardTest(void)
{
	char* Parm;

	Ctrl.Proc = ReadCmd;						// コマンド入力に戻る
	SendText("<<Hardware Test>>\r\n");
	
	if (Parm = strtok(NULL," \x0D\t")) {		// パラメータ指定あり
		switch(*Parm) {
		case '0':Ctrl.Proc = LedTest;
				 break;
		case '1':Ctrl.Proc = SegTest;
				 break;		 
		case '2':Ctrl.Proc = AccTestRamReadInit;	// SDRAM読出しテスト
				 break;
		case '3':Ctrl.Proc = AccTestRamWriteInit;	// SDRAM書込みテスト
				 break;
		case '4':Ctrl.Proc = AccTestFpgaReadInit;	// FPGA読出しテスト
				 break;
		case '5':Ctrl.Proc = AccTestFpgaWriteInit;	// FPGA書込みテスト
				 break;
//		case '7':Ctrl.Proc = AccTestLanReadInit;	// LAN読出しテスト
//				 break;
//		case '8':Ctrl.Proc = AccTestLanWriteInit;	// LAN書込みテスト
//				 break;
//		case 'A':Ctrl.Proc = AccTestLanWriteRam;	// LAN書込みテスト2
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
		Ctrl.Proc = FpgaRegDisplay;				// FPGAレジスタの内容表示
	}
}

//=============================================================================
/**
 * SWTコマンドの実行
 */
//=============================================================================
void CmdSoftTest(void)
{
	char* Parm;

	SendText("<<Function Test>>\r\n");
	
	if (Parm = strtok(NULL," \x0D\t")) {		// パラメータ指定あり
		switch(*Parm) {
		case '1':Ctrl.Proc = BuzTest;			// ブザーテスト
				 break;
		case '2':Ctrl.Proc = IrLedTest;			// 赤外線LEDテスト
				 break;
		case '3':Ctrl.Proc = DswTest;			// DSWテスト
				 break;
		case '4':Ctrl.Proc = CmrCapTest;		// カメラのキャプチャーテスト
				 break;
		case '5':Ctrl.Proc = CmrCapGetTest;		// カメラのキャプチャーが切り出しテスト
				 break;
		case '6':Ctrl.Proc = CmrCmdTest;		// カメラのコマンドテスト
				 break;
		case '7':Ctrl.Proc = CmrPrmCmdTest;		// カメラのパラメータコマンドテスト
				 break;
		case '8':Ctrl.Proc = FRomTest;			// フラッシュメモリテスト
				 break;
		case 'L':Ctrl.Proc = LanCmdTest;		// LANコマンドテスト
				 break;
		case 'B':Ctrl.Proc = MpfInfo;			// MPF状態表示
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
		Ctrl.Proc = MpfInfo;					// メモリープール状態表示
	}
}

//-----------------------------------------------------------------------------
// FPGAレジスタ表示
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
			SendText( CRLF);				/* 改行 */
		}
		SendText(Ctrl.TxText);
		iLine++;
	}
	SendText(CRLF);									// 改行

	// Mode
//	_sprintf(&Ctrl.TxText, "<<Mode Number>>\nMode = %d\r\n", MdGetMachineMode());
//	SendText(Ctrl.TxText);

	Ctrl.Proc = ReadCmd;
}

//-----------------------------------------------------------------------------
// LED点灯テスト
//-----------------------------------------------------------------------------
static void LedTest(void)
{
	static const int iLed[] = {
		LED_POW,						///< 電源LED
		LED_OK,							///< OK LED
		LED_ERR,						///< ERR LED
		LED_DBG1,						///< デバッグ用LED1
		LED_DBG2,						///< デバッグ用LED2
		LED_DBG3,						///< デバッグ用LED3
		LED_DBG4,						///< デバッグ用LED4
	};
	int i;
	static const int nLed = sizeof iLed / sizeof iLed[ 0 ];
	
	SendText("<LED TEST>\r\n...");
	
	LedOut(LED_ALL,  LED_OFF);
	
	// LED点灯→ウェイト→消灯
	for(i = 0;i < nLed;i++) {
		LedOut(iLed[ i ], LED_ON);		// ONテスト
		dly_tsk((500/MSEC));
		LedOut(iLed[ i ], LED_OFF);		// OFFテスト
	}
	
	Ctrl.Proc = ReadCmd;
	
	SendText("END\r\n");
}

//-----------------------------------------------------------------------------
// 7Seg点灯テスト
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
	
		// 7SEG点灯→ウェイト→消灯
		while(ercd == E_OK) {
			ercd = Led7SegOut(eSeg[ i ], n);		// ONテスト
			dly_tsk((500/MSEC));
			n++;
		}
	}

	Ctrl.Proc = ReadCmd;
	
	SendText("END\r\n");
}

//-----------------------------------------------------------------------------
// エラー終了
//-----------------------------------------------------------------------------
static void TestError(void)
{
	SaveResult();								// 結果保存
	Ctrl.Proc = ReadCmd;						// コマンド入力に戻る
}

//-----------------------------------------------------------------------------
// ＲＡＭドライブのファイルに結果書込み
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
// SDRAMから読出し続けるテスト(初期化)
//-----------------------------------------------------------------------------
static void AccTestRamReadInit(void)
{
	SendText("SRAM Read Test...");
	
	Ctrl.Proc = AccTestRamReadTest;
}

//-----------------------------------------------------------------------------
// SDRAMから読出し続けるテスト(実行)
//-----------------------------------------------------------------------------
static void AccTestRamReadTest(void)
{
	volatile UW uwTemp;
	
	uwTemp = s_ubTestRW;
	uwTemp = s_uhTestRW;
	uwTemp = s_uwTestRW;
}

//-----------------------------------------------------------------------------
// SDRAMへ書込みし続けるテスト(初期化)
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
// SDRAMへ書込みし続けるテスト(実行)
//-----------------------------------------------------------------------------
static void AccTestRamWriteTest(void)
{
	s_ubTestRW++;
	s_uhTestRW++;
	s_uwTestRW++;
}

//-----------------------------------------------------------------------------
// FPGAから読出し続けるテスト(初期化)
//-----------------------------------------------------------------------------
static void AccTestFpgaReadInit(void)
{
	SendText("FPGA Read Test...");
	
	Ctrl.Proc = AccTestFpgaReadTest;
}

//-----------------------------------------------------------------------------
// FPGAから読出し続けるテスト(実行)
//-----------------------------------------------------------------------------
static void AccTestFpgaReadTest(void)
{
	volatile UH uwTemp;
	
	uwTemp = FPGA_VER;
}

//-----------------------------------------------------------------------------
// FPGAへ書込みし続けるテスト(初期化)
//-----------------------------------------------------------------------------
static void AccTestFpgaWriteInit(void)
{
	SendText("FPGA Write Test...");
	
	Ctrl.Proc = AccTestFpgaWriteTest;
}

//-----------------------------------------------------------------------------
// FPGAへ書込みし続けるテスト(実行)
//-----------------------------------------------------------------------------
static void AccTestFpgaWriteTest(void)
{
	fpga_outw(TPL_DBYTE, 0x1000);
}

//-----------------------------------------------------------------------------
// LANレジスタから読出し続けるテスト(初期化)
//-----------------------------------------------------------------------------
//static void AccTestLanReadInit(void)
//{
//	SendText("LAN Read Test...");
//	
//	Ctrl.Proc = AccTestLanReadTest;
//}

//-----------------------------------------------------------------------------
// LANから読出し続けるテスト(実行)
//-----------------------------------------------------------------------------
//static void AccTestLanReadTest(void)
//{
//	s_uhTestRW = lan_inw(LAN_CR);
//}

//-----------------------------------------------------------------------------
// LANへ書込みテスト(初期化)
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
// LANへ書込みテスト(実行)
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
// 比較結果を表示する
//-----------------------------------------------------------------------------
static void dispCompResult(long lVal1, long lVal2)
{
	_sprintf(&Ctrl.TxText, "A: %X != B: %X\r\n", lVal1, lVal2);
	SendText(Ctrl.TxText);
}

//-----------------------------------------------------------------------------
// コマンドテスト
//-----------------------------------------------------------------------------
static void LanCmdTest(void)
{
	const char *LanCmd1[] = {
		"R90",								// R90コマンドテスト
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
	Ctrl.Proc = ReadCmd;				// コマンド入力に戻る
}

//-----------------------------------------------------------------------------
// コマンドをメッセージで送信する
//-----------------------------------------------------------------------------
static ER LanCmdTestSendCmd(char* p)
{
	ER ercd;
	T_COMMSG *msg;
	
	ercd = tget_mpf( MPF_COM, (VP*)&msg, (1000/MSEC));	// メモリープール獲得
	if (ercd == E_OK) {
		strcpy(&msg->buf, p);
		msg->cnt = strlen( p );
		ercd = snd_mbx(MBX_CMD_LAN, (T_MSG*)msg);
	}
	return ercd;
}

//-----------------------------------------------------------------------------
// MPF状態取得コマンド
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
	
	Ctrl.Proc = ReadCmd;				// コマンド入力に戻る
}

//-----------------------------------------------------------------------------
// ブザーテスト
//-----------------------------------------------------------------------------
static void BuzTest(void)
{
	char *cParm;
	enum SOUND_CTRL eCtrl;
	
	SendText("<BUZZER TEST>\r\n...");

	eCtrl = SOUND_CTRL_EMG_OFF;
	
	// パラメータ指定あるときはコマンドの内容を変える
	if (cParm = strtok(NULL, " \x0D\t")) {		// パラメータ指定有
		switch(*cParm) {
		case '1':						// no break
		case '2':						// no break
		case '3':						// no break
		case '4':eCtrl = (enum SOUND_CTRL)(*cParm - '1');
				 break;
		}
	}
	SoundCtrl(eCtrl);					// サウンド制御

	Ctrl.Proc = ReadCmd;				// コマンド入力に戻る
}

//-----------------------------------------------------------------------------
// 赤外線LEDテスト
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
	
	IrLedCrlOn( IR_LED_1 );				// 赤外線LED1点灯
	dly_tsk( 1000/MSEC );
	IrLedCrlOn( IR_LED_2 );				// 赤外線LED2点灯
	IrLedCrlOff( IR_LED_1 );
	dly_tsk( 1000/MSEC );
	IrLedCrlOn( IR_LED_3 );				// 赤外線LED3点灯
	IrLedCrlOff( IR_LED_2 );
	dly_tsk( 1000/MSEC );
	IrLedCrlOn( IR_LED_4 );				// 赤外線LED4点灯
	IrLedCrlOff( IR_LED_3 );
	dly_tsk( 1000/MSEC );
	IrLedCrlOn( IR_LED_5 );				// 赤外線LED5点灯
	IrLedCrlOff( IR_LED_4 );
	dly_tsk( 1000/MSEC );
	IrLedCrlOff( IR_LED_5 );
	
	IrLedCrlOff( uhAllLed );			// 全赤外線LED消灯

	Ctrl.Proc = ReadCmd;				// コマンド入力に戻る
}

//-----------------------------------------------------------------------------
// DSWテスト
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

	Ctrl.Proc = ReadCmd;				// コマンド入力に戻る
}

//-----------------------------------------------------------------------------
// カメラキャプチャーテスト
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
	
	// パラメータ指定あるときはコマンドの内容を変える
	if (cParm = strtok(NULL, " \x0D\t")) {		// パラメータ指定有
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
	if (ercd == E_OK) {					// キャプチャー
//		ercd = CmrCapGet(ciX, ciY, GET_IMG_SIZE_X, GET_IMG_SIZE_Y, g_ubCapTest);
		saveCapData("A:\\RawData.raw", g_ubCapTest, (CAP_X_SIZE * CAP_Y_SIZE));
		histogramSend();
	} else {
		i_sprintf(&Ctrl.TxText, "エラー: %X\r\n", ercd);
		SendText(Ctrl.TxText);
	}
	Ctrl.Proc = ReadCmd;				// コマンド入力に戻る
}

//-----------------------------------------------------------------------------
// キャプチャーデータの保存
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
// ヒストグラム送信
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
// カメラコマンドテスト
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
	
	ercd = CmrPktSend( cCmd, sizeof cCmd, iRecvSize);	// コマンド送信
	if (ercd == E_OK) {
		ercd = CmrPktRecv( cBuf, iRecvSize, (5000 / MSEC));		// 応答受信
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
	
	Ctrl.Proc = ReadCmd;				// コマンド入力に戻る
}
*/

//-----------------------------------------------------------------------------
// カメラ画像取得テスト
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
	
	Ctrl.Proc = ReadCmd;				// コマンド入力に戻る
}

//-----------------------------------------------------------------------------
// カメラパラメータ設定コマンドテスト
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
	
	Ctrl.Proc = ReadCmd;				// コマンド入力に戻る
}

//-----------------------------------------------------------------------------
// フラッシュメモリのテスト
//-----------------------------------------------------------------------------
static void FRomTest(void)
{
	ER	ercd;

	
	SendText("<Flash ROM Test>\r\n");
	
	
	
	Ctrl.Proc = ReadCmd;				// コマンド入力に戻る
}
