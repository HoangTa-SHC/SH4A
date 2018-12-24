//=============================================================================
/**
 *
 * VA-300プログラム
 * <<TSC2004関連モジュール>>
 *
 *	@brief TSC2004用ドライバ()
 *	
 *	@file drv_tsc2004.c
 *	
 *	@date	2012/09/12
 *	@version 1.00 新規作成
 *	@author	F.Saeki
 *
 *	Copyright (C) 2012, OYO Electric Corporation
 */
//=============================================================================
#define	_DRV_TSC2004_C_
#include "kernel.h"
#include "drv_tsc2004.h"

// 定義
#define	REG_POS_X	0						///< X座標レジスタアドレス
#define	REG_POS_Y	1						///< Y座標レジスタアドレス
#define	COM_TMOUT	1000					///< 通信タイムアウト時間

// コマンド定義
static const UB s_cCmdResetOn[]  = { 0x83 };	///< リセットON	
												//	Control Byte ID=1
												//	SWRST=1
												//	STS  =1
											
static const UB s_cCmdResetOff[] = { 0x80 };	///< リセットOFF
												//	Control Byte ID=1
												//	SWRST=0
												//	STS  =0
											
static const UB s_cCmdCFR0Set[]  = { 0x62,		// CFR0レジスタ指定, PND0=1
									0x8C,		// PSM=1,STS=0,RM=0,CL1=0, CL0=1,PV2=1,PV1=0,PV0=0,
									0x50 };		// PR2=0,PR1=1,PR0=0,SNS2=1, SNS1=0,SNS0=0,DTW=0,LSM=0
									
static const UB s_cCmdCFR1Set[]  = { 0x6A,		// CFR1レジスタ指定, PND0=1
									0x03,		// TBM3=0,TBM2=0,TBM1=1,TBM0=1
									0x07 };		// BTD2=1,BTD1=1,BTD0=1(10SSPS)
									
static const UB s_cCmdCFR2Set[]  = { 0x72,		// CFR2レジスタ指定, PND0=1
									0x24,		// PINTS1=0,PINTS0=0(PINTDAV#をPENIRQ#&&DAVに設定),M1=1,M0=0,W1=0,W0=1,TZ1=0,TZ0=0,
									0x18 };		// AZ1=0,AZ0=0,MX=1,MY=1,MZ=0,MA=0,MT=0

static const UB s_cCmdPosXGet[]  = { 0x03 };	// レジスタ0h(X)のデータ読出し指定
static const UB s_cCmdPosYGet[]  = { 0x0B };	// レジスタ1h(Y)のデータ読出し指定

// 変数定義
static ID s_idTsk;								///< 割込み時に起床するタスクID

// プロトタイプ宣言
static ER tsc2004Send(UB *p, int iSize);		///< タッチパネルコントローラへ送信
static ER tsc2004Read(UB ubAddr, short *pVal);	///< タッチパネルコントローラのレジスタ読出し

//=============================================================================
/**
 * タッチパネルコントローラ初期化
 *
 * @retval E_OK 成功
 */
//=============================================================================
ER Tsc2004Init(void)
{
	// リセットを送信
	tscSendResetOn();
	tscSendResetOff();
	
	// CFRレジスタに設定
	tscSetCFR0();
	tscSetCFR1();
	tscSetCFR2();

	s_idTsk = 0;

	return E_OK;
}

//=============================================================================
/**
 * タッチパネルの座標取得
 * 
 * @param piPosX X座標
 * @param piPosY Y座標
 * @retval E_OK 成功
 */
//=============================================================================
ER Tsc2004PosGet(int *piPosX, int *piPosY)
{
	ER	ercd;
	short	sVal;

	tscSendPosXGet();						// Xレジスタ読出しコマンド送信
	ercd = tsc2004Read(REG_POS_X, &sVal);	// X座標値読出し
	if (ercd == E_OK) {
		*piPosX = sVal;
	} else {
		return ercd;
	}
	
	tscSendPosYGet();						// Yレジスタ読出しコマンド送信
	ercd = tsc2004Read(REG_POS_Y, &sVal);	// Y座標値読出し
	if (ercd == E_OK) {
		*piPosY = sVal;
	}
	
	return ercd;
}

//=============================================================================
/**
 * タッチパネルコントローラへ送信
 *
 * @param p 送信データ
 * @param iSize 送信データ数
 * @param OSのエラーコード
 */
//=============================================================================
static ER tsc2004Send(UB *p, int iSize)
{
	ER ercd;
	UH uhVal;
	
	s_idTsk = vget_tid();				// タスクIDの取得
	vcan_wup();							// 起床要求のクリア

	// CONTROL BYTE設定
	tscSetCtrlByte( *p );
	
	if (iSize == 1) {					// Control Byteのみの送信
		tscSendCtrlByte();
	} else {							// Data Byteも送信
		p++;
		uhVal = (UH)*p << 8;
		p++;
		uhVal |= *p;
		tscSetDataByte( uhVal );
		tscSendDataByte();
	}
	
	ercd = tslp_tsk((COM_TMOUT / MSEC));
	
	s_idTsk = 0;
	
	return ercd;	
}

//=============================================================================
/**
 * タッチパネルコントローラから読出し
 *
 * @param p 送信データ
 * @param iSize 送信データ数
 */
//=============================================================================
static ER tsc2004Read(UB ubAddr, short *pVal)
{
	ER ercd;
	UB ubVal;
	
	ubVal = ((ubAddr & 0x0F) << 3) | 0x03;
	
	// CONTROL BYTE設定
	tscSetCtrlByte( ubVal );
	
	s_idTsk = vget_tid();				// タスクIDの取得
	vcan_wup();							// 起床要求のクリア
	
	tscReadReg();						// 読出しシーケンス起動

	ercd = tslp_tsk((COM_TMOUT / MSEC));
	if (ercd == E_OK) {
		*pVal = tscGetDataByte();
	}
	s_idTsk = 0;
	
	return ercd;
}

//=============================================================================
/**
 * タッチパネルコントローラ通信割込み処理
 */
//=============================================================================
void Tsc2004ComInt(void)
{
	disable_tplcom_int();				// タッチパネル通信割込みフラグ初期化
	if (s_idTsk) {
		iwup_tsk( s_idTsk );
	}
	enable_tplcom_int();				// タッチパネル通信割込み許可
}
