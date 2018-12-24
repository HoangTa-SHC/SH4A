/**
*	VA-300プログラム
*
*	@file tsk_mode.c
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/08/27
*	@brief  モード制御タスク(ELM-10から流用)
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
#include <stdio.h>

#include "kernel.h"
#include "id.h"
#include "err_ctrl.h"
#include "command.h"

#include "drv_7seg.h"
#include "drv_led.h"
#include "drv_dsw.h"
#include "drv_rtc.h"

#include "va300.h"

// 定義
enum MD_LAN_CMD_STATUS {
	MD_LAN_CMD_NONE,						///< コマンド未送信状態
	MD_LAN_CMD_ACK_WAIT,					///< ACK待ち
	MD_LAN_CMD_RES_WAIT,					///< 応答コマンド待ち状態

};

// 変数定義
static enum MODE_STATUS s_eMachineMode;		// 状態変数
static enum SUBMODE_STATUS s_eMainteSubMode;// メンテナンスモードのサブ状態
static enum SUBMODE_STATUS s_eNormalSubMode;// 通常モードのサブ状態
static enum MD_LAN_CMD_STATUS s_eLanCmdSubMode;// LANコマンドモードのサブ状態

// プロトタイプ宣言
	//モード処理
static void PowerOnMode(void);				// パワーオンモード
static void MainteMode(void);				// メンテナンスモード
static void NormalMode(void);				// 通常モード
static void PowerFailMode(void);			// 停電時動作モード
static void PanicOpenMode(void);			// 非常時開錠モード
static void SelfTestMode(void);				// セルフテスト状態
static void ErrorMode(void);				// エラー発生状態

	//モード処理（サブ）
static void MdPowerOnModeLanCmd(T_MDCMDMSG *cmdmsg);// パワーオンモードのLANコマンド処理
static void MdPowerOnModeMon(T_MDCMDMSG *cmdmsg);	// パワーオンモードのモニタコマンド処理
static void MdMainteModeLanCmd(T_MDCMDMSG *cmdmsg);	// メンテナンスモードのLANコマンド処理
static void MdMainteModeMon(T_MDCMDMSG *cmdmsg);	// メンテナンスモードのモニタコマンド処理
static void MdNormalModeLanCmd(T_MDCMDMSG *cmdmsg);	// 通常モードのLANコマンド処理
static void MdNormalModeIo(T_MDCMDMSG *cmdmsg);		// 通常モードのI/O処理
static void mdNormalModeMon(T_MDCMDMSG *cmdmsg);	// 通常モードのモニタコマンド処理
static void MdNormalRegMode(T_MDCMDMSG *pCmdMsg);	// 登録モード
static void MdNormalDelMode(T_MDCMDMSG *pCmdMsg);	// 削除モード
static void mdPowerFailModeLan(T_MDCMDMSG *cmdmsg);	// 停電時動作モードのLANコマンド処理
static void mdPowerFailModeIo(T_MDCMDMSG *cmdmsg);	// 停電時動作モードのI/O処理
static void mdPanicOpenModeLan(T_MDCMDMSG *cmdmsg);	// パニックオープンモードのLANコマンド処理
static void mdPanicOpenModeIo(T_MDCMDMSG *cmdmsg);	// パニックオープンモードのI/O処理
static void mdPanicOpenMon(T_MDCMDMSG *cmdmsg);		// パニックオープンモードのモニタコマンド処理

static void MdChangeStatus(enum MODE_STATUS nextMode);	//モード遷移

/*==========================================================================*/
/**
 *	マシン状態を返す（モード参照）
 *	@return マシンの状態
 */
/*==========================================================================*/
enum MODE_STATUS MdGetMachineMode(void)
{
	return s_eMachineMode;
}

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
/**
 * モード管理タスク
 *
 */
/*==========================================================================*/
TASK ModeTask(void)
{
	s_eMachineMode = MD_POWER_ON;
	
	for(;;) {
		switch(s_eMachineMode) {
		
		case MD_POWER_ON:			// 電源ON（初期値)
			PowerOnMode();
			break;
			
		case MD_MAINTENANCE:		// メンテナンスモード
			MainteMode();
			break;
			
		case MD_NORMAL:				// 通常モード
			NormalMode();
			break;
			
		case MD_POWER_FAIL:			// 停電時動作モード
			PowerFailMode();
			break;
			
		case MD_PANIC:				// 非常時開錠モード
			PanicOpenMode();
			break;
			
		case MD_SELFTEST:			// セルフテスト状態
			SelfTestMode();
			break;
			
		case MD_ERROR:				// エラー状態
			ErrorMode();
			break;
		
		default:					// ありえない
			PrgErrSet();
			break;
		}
	}
}

/*==========================================================================*/
/* パワーオンモード（自動通過)												*/
/*==========================================================================*/
static void PowerOnMode(void)
{
	ER			ercd;
	T_MDCMDMSG	*msg;

	// 内部初期化
	MdChangeStatus(MD_POWER_ON);
	
	
	
	// LCD初期表示
	
	
	
	while(s_eMachineMode == MD_POWER_ON) {
		ercd = rcv_mbx(MBX_MODE, &msg);
		if (ercd == E_OK) {
			switch(msg->idOrg) {
			case TSK_CMD_LAN:
				MdPowerOnModeLanCmd(msg);
				break;
				
			case TSK_IO:				// 間違ってI/O入力あったときの処理
				break;
				
			case TSK_CMD_MON:
				MdPowerOnModeMon(msg);
				break;
				
			default:					// ありえない
				PrgErrSet();
				break;
			}
			rel_mpf(MPF_COM, (VP)msg);
		}
	}
}

/*==========================================================================
	パワーオンモードのLANコマンド処理
============================================================================*/
static void MdPowerOnModeLanCmd(T_MDCMDMSG *pCmdMsg) 
{
	switch (pCmdMsg->uhCmd) {
	case CMD_MD_MNT_C:					// メンテナンスモードへ移行要求
		MdChangeStatus(MD_MAINTENANCE);	// メンテナンスモードへ移行
		break;
	
	case CMD_MD_BAT_C:					// 停電モードへ移行要求
		MdChangeStatus(MD_POWER_FAIL);	// 停電モードへ移行
		break;

	case MD_CMD_POL_REQ:				// 制御盤へのポーリング要求
		
		break;
		
	case MD_CMD_RCV:					// コマンド受信通知
		break;
		
	default:
		break;
	}//switch (msg->command) {
}

/*==========================================================================
	パワーオンモードのモニタコマンド処理
============================================================================*/
static void MdPowerOnModeMon(T_MDCMDMSG *pCmdMsg) 
{
	switch (pCmdMsg->uhCmd) {
	case CMD_MD_MNT_C:					// メンテナンスモードへ移行要求
		MdChangeStatus(MD_MAINTENANCE);	// メンテナンスモードへ移行
		break;
	
	case CMD_MD_BAT_C:					// 停電モードへ移行要求
		MdChangeStatus(MD_POWER_FAIL);	// 停電モードへ移行
		break;

	default:
		break;
	}//switch (pCmdMsg->uhCmd) {
}

/*==========================================================================*/
/* メンテナンスモード														*/
/*==========================================================================*/
static void MainteMode(void)
{
	ER			ercd;
	T_MDCMDMSG	*msg;

	s_eMainteSubMode = SUB_MD_SETTING;		

	while(s_eMachineMode == MD_MAINTENANCE) {
		ercd = rcv_mbx(MBX_MODE, &msg);
		if (ercd == E_OK) {
			switch(s_eMainteSubMode) {
			case SUB_MD_SETTING:		// 設定モード
				MdMainteSetMode(msg);
				break;
				
			case SUB_MD_BLK:			// ブロック転送モード
				MdMainteBlkMode(msg);
				break;
				
			case SUB_MD_INIT_REGIST:	// 初期登録モード
				MdMainteInitRegistMode(msg);
				break;
				
			default:					// ありえない
				PrgErrSet();
				break;
			}
			rel_mpf(MPF_COM, (VP)msg);
		} else {
			ErrCodeSet(ercd);
		}
	}
}

/*==========================================================================*/
/* 通常モード																*/
/*==========================================================================*/
/*===========================================================================
	認証開始指令があると、認証を開始する
=============================================================================*/
static void NormalMode(void)
{
	ER			ercd;
	T_MDCMDMSG	*msg;
	
	// サブモードを認証モードに
	s_eNormalSubMode = SUB_MD_AUTHENTICATE;
	
	while(s_eMachineMode == MD_NORMAL) {
		ercd = rcv_mbx(MBX_MODE, &msg);
		if (ercd == E_OK) {
			switch(s_eNormalSubMode) {
			case SUB_MD_AUTHENTICATE:			// 認証モード
				MdNormalAutMode(msg);
				break;
				
			case SUB_MD_REGIST:					// 登録モード
				MdNormalRegMode(msg);
				break;
				
			case SUB_MD_DELETE:					// 削除モード
				MdNormalDelMode(msg);
				break;
				
			default:
				
				break;
			}//switch (msg->idOrg)
			rel_mpf(MPF_COM, (VP)msg);
		} else {
			ErrCodeSet(ercd);
		}
	}
}

/**
 *	登録モード
 */
static void MdNormalRegMode(T_MDCMDMSG *pCmdMsg)
{
	
	
	
}

/**
 *	削除モード
 */
static void MdNormalDelMode(T_MDCMDMSG *pCmdMsg)
{
	
	
	
}

/*==========================================================================*/
/* 停電動作モード															*/
/*==========================================================================*/
static void PowerFailMode(void)
{
	ER			ercd;
	T_MDCMDMSG	*msg;
	
	while(s_eMachineMode == MD_POWER_FAIL) {
		ercd = rcv_mbx(MBX_MODE, &msg);
		if (ercd == E_OK) {
			switch(msg->idOrg) {
			case TSK_CMD_LAN:
				mdPowerFailModeLan(msg);
				break;
			case TSK_IO:
				mdPowerFailModeIo(msg);
				break;
				
			}
			rel_mpf(MPF_COM, (VP)msg);
		} else {
			ErrCodeSet(ercd);
		}
	}//while(iMachineMode == MD_RUN) {
}

/*==========================================================================*/
/*	停電時動作モードのLANコマンド処理										*/
/*==========================================================================*/
static void mdPowerFailModeLan(T_MDCMDMSG *pCmdMsg) 
{
	switch (pCmdMsg->uhCmd) {
	default:
		break;
	}//switch
}

/*==========================================================================*/
/*	停電時動作モードのI/O処理												*/
/*==========================================================================*/
static void mdPowerFailModeIo(T_MDCMDMSG *pCmdMsg) 
{
	switch (pCmdMsg->uhCmd) {
	default:
		break;
	}//switch
}

/*==========================================================================*/
/* 非常時開錠モード															*/
/*==========================================================================*/
static void PanicOpenMode(void)
{
	ER			ercd;
	T_MDCMDMSG	*msg;
	
	while(s_eMachineMode == MD_PANIC) {
		ercd = rcv_mbx(MBX_MODE, &msg);
		if (ercd == E_OK) {
			switch(msg->idOrg) {
			case TSK_CMD_LAN:
				mdPanicOpenModeLan(msg);
				break;
			case TSK_IO:
				break;
							
			case TSK_CMD_MON:
				mdPanicOpenModeMon(msg);
				break;
			
			}
			rel_mpf(MPF_COM, (VP)msg);
		} else {
			ErrCodeSet(ercd);
		}
	}
}

/*==========================================================================
	パニックオープンモードのLANコマンド処理
============================================================================*/
static void mdPanicOpenModeLan(T_MDCMDMSG *pCmdMsg) 
{
	switch (pCmdMsg->uhCmd) {

	default:
		break;
	}//switch
}

/*==========================================================================
	パニックオープンモードのI/O処理
============================================================================*/
static void mdPanicOpenModeIo(T_MDCMDMSG *pCmdMsg) 
{
	switch (pCmdMsg->uhCmd) {

	default:
		break;
	}//switch
}

/*==========================================================================
	パニックオープンモードのモニタコマンド処理
============================================================================*/
static void mdPanicOpenMon(T_MDCMDMSG *pCmdMsg) 
{
	switch (pCmdMsg->uhCmd) {

	default:
		break;
	}//switch (pCmdMsg->uhCmd) {
}

/*==========================================================================*/
/* セルフテストモード														*/
/*==========================================================================*/
static void SelfTestMode(void)
{
	ER			ercd;
	T_MDCMDMSG	*msg;
	
	while(s_eMachineMode == MD_SELFTEST) {
		ercd = rcv_mbx(MBX_MODE, &msg);
		if (ercd == E_OK) {
			switch(msg->idOrg) {
			case TSK_CMD_LAN:
				
				break;
			}
			rel_mpf(MPF_COM, (VP)msg);
		} else {
			ErrCodeSet(ercd);
		}
	}
}

/*==========================================================================*/
/* エラー発生モード													*/
/*==========================================================================*/
static void ErrorMode(void)
{
	ER			ercd;
	T_MDCMDMSG	*msg;
	
	while(s_eMachineMode == MD_ERROR) {
		ercd = rcv_mbx(MBX_MODE, &msg);
		if (ercd == E_OK) {
			switch(msg->idOrg) {
			case TSK_CMD_LAN:
				
				break;
			}
			rel_mpf(MPF_COM, (VP)msg);
		} else {
			ErrCodeSet(ercd);
		}
	}
}

/*==========================================================================
	LANコマンド処理
============================================================================*/
static void mdLanCmdProc(T_MDCMDMSG *pCmdMsg) 
{
	switch (s_eLanCmdSubMode) {
	case MD_LAN_CMD_NONE:
		
		break;
		
	case MD_LAN_CMD_ACK_WAIT:
		mdLanCmdProcAckWait(pCmdMsg);
		break;
		
	case MD_LAN_CMD_RES_WAIT:
	
		break;
	
	default:
		break;
	}//switch
}

/**
 * コマンドに対するACK受信待ち
 * 
 * @param @CmdMsg コマンド
 */
static void mdLanCmdProcAckWait(T_MDCMDMSG *pCmdMsg) 
{
	switch (pCmdMsg->uhCmd) {
	case MD_CMD_ACK_RCV:
		
		break;
	
	default:
		CmmSendNak();					// NAK送信
		break;
	}
}

/**
 * モードタスクへのメッセージ送信
 *
 * @param 送信元タスクID
 * @param コマンド
 * @return エラーコード
 */
ER MdSendMsg(ID idOrg, UH uhCmd, UH uhBlkno, B *p, int iCount)
{
	ER			ercd;
	T_MDCMDMSG	*msg;
	int			iSize;
	
	ercd = tget_mpf(MPF_COM, &msg, (100/MSEC));
	if (ercd != E_OK) {
		ErrCodeSet(ercd);		// エラー処理
		return ercd;
	}
	msg->idOrg   = idOrg;				// 送信元タスクID
	msg->uhCmd   = uhCmd;				// コマンド
	msg->uhBlkNo = uhBlkno;				// ブロック番号
	iSize = iCount;
	if (iSize > sizeof msg->cData) {
		iSize = sizeof msg->cData;
	}
	memcpy(&(msg->cData), p, iSize);
	msg->cnt     = iSize;				// データ数
	
	//メッセージ送信
	ercd = snd_mbx(MBX_MODE, (T_MSG*)msg);
	
	return ercd;
}
