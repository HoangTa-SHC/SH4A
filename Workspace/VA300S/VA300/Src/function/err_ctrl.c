/**
*	VA-300プログラム
*
*	@file err_ctrl.c
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2010/01/22
*	@brief  エラー制御(他案件から流用)
*
*	Copyright (C) 2010, OYO Electric Corporation
*/
#define	_ERR_CTRL_C_
#include <stdio.h>
#include <string.h>
#include "kernel.h"
#include "id.h"
#include "err_ctrl.h"
#include "drv_led.h"
#include "va300.h"
#include "drv_fpga.h"
#include "nonfile.h"
#include "sh7750.h"

// 外部関数
extern void DebugOut(char*);
extern TASK MainTask(void);
extern TASK ModeTask(void);
extern TASK DispTask(void);
extern TASK RcvTask(INT);
extern TASK SndTask(INT);
extern TASK UdpRcvTask(int);
extern TASK UdpSndTask(INT);
extern TASK MonTask(INT ch);
extern TASK MonRcvTask(INT ch);
extern TASK IoTask(void);
extern TASK udprcv_tsk(ID);
extern TASK NinshouTask(void);
extern TASK LogTask(void);

// 変数定義
static ST_ERR	ErrStatus;
static const struct {
	FP		fpAddr;
	const char*	pTaskName;
} TaskName[] = {
	{MainTask,		"Main Task"},
	{DispTask,		"Display Task"},
	{RcvTask,		"Receive Task"},
	{SndTask,		"Send Task"},
#if ( VA300S == 0 || VA300S == 2 )
	{udprcv_tsk,	"udp receive task"},
	{UdpRcvTask,	"UDP Receive Task"},
	{UdpSndTask,	"UDP Send Task"},
#endif
	{MonTask,		"Monitor Task"},
	{MonRcvTask,	"Monitor Recieve Task"},
	{IoTask,		"I/O Task"},
};
static const int nTaskName = sizeof TaskName / sizeof (TaskName[0]);
static T_RTSK	s_rtsk;
static char s_cBuf[ 256 ];				// 処理用バッファ

static void ErrReport(void);

/*==========================================================================*/
/**
 * 測定開始SW割込み
 *
 */
/*==========================================================================*/
void ErrStatusInit(void)
{
	ErrStatus.task  = 0;
	ErrStatus.byTaskName = "";
	ErrStatus.ubErrType = MAIN_ERR_NONE;
	ErrStatus.ercd  = 0;
	LedOut(LED_ERR, LED_OFF);
}

/*==========================================================================*/
/**
 * エラーステータス設定
 *
 * @param ubErrType エラータイプ
 * @param iErcd エラーコード
 * @param pcFilename ファイルネーム
 * @param iLine 行番号
 * 
 * @retval E_OK 設定OK
 */
/*==========================================================================*/
ER ErrStatusSet(UB ubErrType, int iErcd, char* pcFilename, int iLine)
{
	static const struct {
		ER		ercd;
		const char*	pErrorCode;
	} ErrorCode[] = {
		{E_OK,		"E_OK"},			// 正常終了(0)
		{E_SYS,		"E_SYS"},
		{E_NOSPT,	"E_NOSPT"},
		{E_RSFN,	"E_RSFN"},
		{E_RSATR,	"E_RSATR"},
		{E_PAR,		"E_PAR"},
		{E_ID,		"E_ID"},
		{E_CTX,		"E_CTX"},
		{E_ILUSE,	"E_ILUSE"},
		{E_NOMEM,	"E_NOMEM"},
		{E_NOID,	"E_NOID"},
		{E_OBJ,		"E_OBJ"},
		{E_NOEXS,	"E_NOEXS"},
		{E_QOVR,	"E_QOVR"},
		{E_TMOUT,	"E_TMOUT"},
		{E_RLWAI,	"E_RLWAI"},
		{E_DLT,		"E_DLT"},
//		{EV_LANC,	"EV_LANC"},
//		{EV_LINK,	"EV_LINK"},
//		{EV_RXERR,	"EV_RXERR"},
//		{EV_TXERR,	"EV_TXERR"},
	};
	int nErrorCode = sizeof ErrorCode / sizeof (ErrorCode[0]);
	int	i;
	ER	sem_ercd;
	char cBuf[80];
	
	ref_tsk(TSK_SELF, &s_rtsk);			// タスクの状態取得
	// エラーステータスの設定
	sem_ercd = twai_sem(SEM_ERR, 100/MSEC);
	if (sem_ercd != E_OK) {
		return sem_ercd;
	}
	ErrStatus.task  = s_rtsk.task;
	ErrStatus.ubErrType = ubErrType;
	ErrStatus.ercd  = (ER)iErcd;

	// ログへ出力
	ErrLog("\r\nTASK: ");
	for(i = 0;i < nTaskName;i++) {
		if (s_rtsk.task == TaskName[ i ].fpAddr) {
			break;
		}
	}
	ErrStatus.byTaskName = "???";
	if (i < nTaskName) {
		ErrStatus.byTaskName = TaskName[ i ].pTaskName;
	}
	ErrLog(ErrStatus.byTaskName);

	sig_sem(SEM_ERR);

	if (ubErrType == MAIN_ERR_RTOS) {
		ErrLog("\r\nERCD: ");
		for(i = 0;i < nErrorCode;i++) {
			if ((ER)iErcd == ErrorCode[ i ].ercd) {
				break;
			}
		}
		if (i < nErrorCode) {
			ErrLog(ErrorCode[ i ].pErrorCode);
		} else {
			ErrLog("???");
		}
	} else {
		ErrLog("\r\nError Code: ");
		_sprintf(cBuf, "%04X", iErcd);
		ErrLog(cBuf);
	}
	ErrLog("\r\nFILE: ");
	ErrLog(strrchr(pcFilename, '\\'));
	ErrLog(" LINE: ");
	_sprintf(cBuf, "%d\x0d\x0a", iLine);
	ErrLog(cBuf);

	ErrReport();
	
	LedOut(LED_ERR, LED_ON);			// エラーLED ON
	
	return E_OK;
}

/*==========================================================================*/
/**
 * エラーステータスのエラーコード読込み
 *
 * @param pMain メインエラーコード
 * @param pSub サブエラーコード
 */
/*==========================================================================*/
void ErrStatusGet(UB* pMain, UB* pSub)
{
	*pMain = ErrStatus.ubErrType;
	*pSub  = (UB)ErrStatus.ercd;
}

/*==========================================================================*/
/**
 * エラーログ
 *
 * @param p エラーレポート
 */
/*==========================================================================*/
void ErrLog(char *p)
{
	FILE *pfile;
	
	DebugOut( p );
	pfile = fopen("A:\\error.log", "a");
	if (pfile == NULL) {
		return;
	}
	fputs(p, pfile);
	
	fclose(pfile);
}

/*==========================================================================*/
/**
 * エラーレポートの作成
 */
/*==========================================================================*/
static void ErrReport(void)
{
	FILE *pfile;
	int	 iSize, iCnt, iLine, iPcs, iNo;
	long lStart, lStop, lVal;
	
	pfile = fopen("A:\\e_report.txt", "w");
	if (pfile == NULL) {
		return;
	}
	iCnt  = 0;
	iLine = 0;
	fputs("<<FPGA REG>>\n", pfile);
	while(iSize = FpgaRegDisplayLine(iLine, &s_cBuf[ iCnt ])) {
		iCnt += iSize;
		if (iCnt >= 75) {
			s_cBuf[ iCnt ] = '\n';	iCnt++;
			s_cBuf[ iCnt ] = (char)NULL;
			fputs(s_cBuf, pfile);				/* 改行 */
			iCnt = 0;
		}
		iLine++;
	}
	if (iCnt) {
		fputs(s_cBuf, pfile);
		fputs("\n", pfile);						/* 改行 */
	}
	
	// Mode
//	_sprintf(s_cBuf, "<<Mode Number>>\nMode = %d\n\n", MdGetMachineMode());
	fputs(s_cBuf, pfile);
	
	// 設定
	fputs("<Parameters>\n", pfile);
	

	fputs("\n", pfile);					// 改行
	
	
	fclose(pfile);
}
