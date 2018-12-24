/**
*	VA-300プログラム
*
*	@file sub_set.c
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/08/29
*	@brief  メンテナンスモードのサブモード(設定)
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
#include <stdio.h>

#include "kernel.h"
#include "id.h"
#include "message.h"
#include "err_ctrl.h"
#include "command.h"

#include "drv_7seg.h"
#include "drv_led.h"
#include "drv_dsw.h"
#include "drv_rtc.h"

// 定義

// 変数定義
static enum MODE_STATUS s_eMachineMode;		// 状態変数
static enum SUBMODE_STATUS s_eNormalSubMode;// サブ状態

// プロトタイプ宣言
	//モード処理

	//モード処理（サブ）
static void MdMainteModeLanCmd(ST_COMMAND *cmdmsg);	// メンテナンスモードのLANコマンド処理
static void MdMainteModeMon(ST_COMMAND *cmdmsg);	// メンテナンスモードのモニタコマンド処理


/*==========================================================================*/
/**
 *	モード遷移
 *	@param eNextMode 次の状態
 */
/*==========================================================================*/
static void MdChangeStatus(enum MODE_STATUS eNextMode)
{
	//退場処理
	switch (s_eMachineMode) {
	case MD_POWER_ON:

		break;
	case MD_NORMAL:						//認証モード

		break;
	}//switch
	
	s_eMachineMode = eNextMode;			//次のモード設定
	
	//入場処理
	switch (s_eMachineMode) {
	case MD_INITIALIZE:					//初期化
		break;		

	case MD_MAINTENANCE:
		break;
		
	case MD_NORMAL:						//認証モード

		break;
	}
	
	//LED表示
	LedOut(LED_ERR,  LED_OFF);
	switch (s_eMachineMode) {
	case MD_PANIC:
		LedOut(LED_ERR,  LED_ON);
		break;
	}
}



/*==========================================================================*/
/* メンテナンスモード時の設定モード											*/
/*==========================================================================*/
void MdMainteSetMode(void)
{
	switch() {
	case :
	
	case :
	
	default:
	
		break;
	}
}

/*==========================================================================
	メンテナンスモードの処理
	・TST_CMDを処理する
============================================================================*/
static void MdMainteModeLanCmd(ST_COMMAND *cmdmsg) 
{
	switch (cmdmsg->command) {
	case CMD_READY:						// 
		break;

	case CMD_RESET:
		MdChangeStatus(MD_INITIALIZE);	//初期化モード
		CmmSendAck();
		break;	

	default:
		CmmSendNack(ERR_COM_COMMAND);	//コマンド異常
		break;
	}//switch (msg->command) {
}

/*==========================================================================
	メンテナンスモードの処理
	・モニタコマンドを処理する
============================================================================*/
static void MdMainteModeMon(ST_COMMAND *cmdmsg) 
{
	switch (cmdmsg->command) {
	case CMD_READY:						// 

		break;
	case CMD_RESET:
		MdChangeStatus(MD_INITIALIZE);	//初期化モード
		break;	
	default:
		PrgErrSet();
		break;
	}//switch (msg->command) {
}
