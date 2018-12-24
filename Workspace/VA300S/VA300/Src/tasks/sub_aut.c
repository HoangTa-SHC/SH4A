/**
*	VA-300プログラム
*
*	@file sub_aut.c
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/08/27
*	@brief  認証処理
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
enum MD_AUT_STATUS {						///< 認証ステータス
	MD_AUT_INP_NUMBER,						///< 共有部のときの番号入力待ち
	MD_AUT_INP_WAIT,						///< 認証の入力待ち
	MD_AUT_SND_DAT_ACK_WAIT,				///< 指データ送信ACK受信待ち
	MD_AUT_JDG_WAIT,						///< 判定受信待ち
	MD_AUT_PRM_WAIT,						///< パラメータの変更要求待ち
	MD_AUT_CMR_RTRY,						///< 再撮影要求待ち
	MD_AUT_EOT_WAIT,						///< EOT受信待ち
	
	MD_AUT_RES_WAIT,						///< レスポンス待ち
};

// 変数定義
static enum MD_AUT_STATUS s_eAutMode;		// 認証モード状態
static int s_iBlkNo;						// ブロック番号
static int s_iRetry;						// 共有部の場合のリトライ回数

// プロトタイプ宣言
static void mdAutChangeStatus(enum MD_AUT_STATUS eNextMode);	// 認証モードの状態遷移

	//モード処理（サブ）
static void mdNormalAutModeInputWaitLanCmd(T_MDCMDMSG *pCmdMsg);// 入力待ち(LANコマンド)
static void mdNormalAutModeInputWaitIo(T_MDCMDMSG *pCmdMsg);	// 入力待ち(I/O)
static void mdNormalAutModeInputWaitMon(T_MDCMDMSG *pCmdMsg);	// 入力待ち(モニタ)
static void mdNormalAutModeSndDatAckWait(T_MDCMDMSG *pCmdMsg);	// ACK待ち状態
static void mdNormalAutModeResWait(T_MDCMDMSG *pCmdMsg);		// 応答コマンド待ち状態
static void mdNormalAutModeSndBlkAckWait(T_MDCMDMSG *pCmdMsg);	// ブロック転送ACK待ち状態
static void mdNormalAutModeJdgWait(T_MDCMDMSG *pCmdMsg);		// 判定待ち状態
static void mdNormalAutModeParamWait(T_MDCMDMSG *pCmdMsg);		// パラメータ変更待ち状態
static void mdNormalAutModeRetryWait(T_MDCMDMSG *pCmdMsg);		// 再撮影要求待ち状態
static void mdNormalAutModeEotWait(T_MDCMDMSG *pCmdMsg);		// EOT受信待ち状態

/*==========================================================================*/
/**
 *	認証モードの状態初期化
 */
/*==========================================================================*/
void MdNormalAutInit(void)
{
	if (IsTypeCommon() == TRUE) {		// 共有部向けのとき
		s_iRetry = ;
		mdAutChangeStatus(MD_AUT_INP_NUMBER);
	} else {							// 専有タイプ
		s_iRetry = 1;
		mdAutChangeStatus(MD_AUT_INP_WAIT);
	}
}

/*==========================================================================*/
/**
 *	認証モードの状態遷移
 *	@param eNextMode 次の状態
 */
/*==========================================================================*/
static void mdAutChangeStatus(enum MD_AUT_STATUS eNextMode)
{
	//退場処理
	switch (s_eAutMode) {
	case MD_AUT_INP_WAIT:

		break;
	case MD_AUT_RES_WAIT:				//

		break;
	case MD_AUT_SND_DAT_ACK_WAIT:
	
		break;
	}//switch
	
	s_eAutMode = eNextMode;				//次のモード設定
	
	//入場処理
	switch (s_eAutMode) {
	case MD_AUT_INP_WAIT:				//
		break;		

	case MD_AUT_RES_WAIT:
		break;
		
	case MD_AUT_SND_DAT_ACK_WAIT:		//

		break;
	}
}

/*==========================================================================*/
/* 通常モード時のサブモード													*/
/*==========================================================================*/
/**
 *	認証モード
 */
void MdNormalAutMode(T_MDCMDMSG *pCmdMsg)
{
	switch( s_eAutMode ) {
	case MD_AUT_INP_NUMBER:						// キー入力待ち状態
		mdNormalAutModeInputNumber(pCmdMsg);
		break;
	
	case MD_AUT_INP_WAIT:						// 入力待ち状態
		mdNormalAutModeInputWait(pCmdMsg);
		break;
	
	case MD_AUT_SND_DAT_ACK_WAIT:				// 指データのACK受信待ち
		mdNormalAutModeSndDatAckWait(pCmdMsg);
		break;
	
	case MD_AUT_RES_WAIT:						// 応答コマンド待ち
		mdNormalAutModeResWait(pCmdMsg);
		break;
	
	case MD_CHK_SND_BLK_ACK_WAIT:				// 指データのACK受信待ち
		mdNormalAutModeSndBlkAckWait(pCmdMsg);
		break;
		
	case MD_AUT_JDG_WAIT:						// 指データの判定受信待ち状態
		mdNormalAutModeJdgWait(pCmdMsg);
		break;
		
	case MD_AUT_PRM_WAIT:						// パラメータ変更待ち状態
		mdNormalAutModeParamWait(pCmdMsg);
		break;
		
	case MD_AUT_CMR_RTRY:						// 再撮影要求待ち
		mdNormalAutModeRetryWait(pCmdMsg);
		break;
		
	case MD_AUT_EOT_WAIT:						// EOT待ち
		mdNormalAutModeEotWait(pCmdMsg);
		break;
		
	default:									// 実装エラー
	
		break;
	}
}

/**
 *	認証モードのID入力待ち状態
 */
static void mdNormalAutModeInputNumber(T_MDCMDMSG *pCmdMsg)
{
	switch(pCmdMsg->idOrg) {
	case TSK_CMD_LAN:
		mdNormalAutModeInputWaitLanCmd(pCmdMsg);
		break;
	
	case TSK_IO:
		mdNormalAutModeInputNumberIo(pCmdMsg);
		break;
		
	case TSK_CMD_MON:
		mdNormalAutModeMon(pCmdMsg);
		break;
	
	}//switch (pCmdMsg->idOrg)
}

/*==========================================================================
	入力待ちの処理
	・I/Oタスクからの要求を処理する
============================================================================*/
static void mdNormalAutModeInputNumberIo(T_MDCMDMSG *pCmdMsg) 
{
	switch (pCmdMsg->uhCmd) {
	case MD_CMD_KEY_ON:
	
		break;
		
	default:
		break;
	}//switch (pCmdMsg->uhCmd) {
}

/**
 *	認証モードの待ち状態
 */
static void mdNormalAutModeInputWait(T_MDCMDMSG *pCmdMsg)
{
	switch(pCmdMsg->idOrg) {
	case TSK_CMD_LAN:
		mdNormalAutModeInputWaitLanCmd(pCmdMsg);
		break;
	
	case TSK_IO:
		mdNormalAutModeInputWaitIo(pCmdMsg);
		break;
		
	case TSK_CMD_MON:
		mdNormalAutModeInputWaitMon(pCmdMsg);
		break;
	
	}//switch (pCmdMsg->idOrg)
}

/*==========================================================================
	入力待ちの処理
	・LANコマンドからの要求を処理する
============================================================================*/
static void mdNormalAutModeInputWaitLanCmd(T_MDCMDMSG *pCmdMsg) 
{
	switch (pCmdMsg->uhCmd) {
	case MD_CMD_POL_REQ:
		LanReqSend();					// 問い合わせコマンド発行
		break;
		
	default:
		CmmSendNak();					//コマンド異常
		break;
	}//switch (pCmdMsg->uhCmd) {
}

/*==========================================================================
	入力待ちの処理
	・I/Oタスクからの要求を処理する
============================================================================*/
static void mdNormalAutModeInputWaitIo(T_MDCMDMSG *pCmdMsg) 
{
	switch (pCmdMsg->uhCmd) {
	case MD_CMD_FIN_SET:
			// 撮影処理
		LanFingerDataSend();							// 指画像データの送信コマンド
		mdAutChangeStatus(MD_AUT_SND_DAT_ACK_WAIT);		// 指データのACK受信待ちへ
		break;
		
	case MD_CMD_KEY_ON:
	
		break;
		
	default:
		break;
	}//switch (pCmdMsg->uhCmd) {
}

/*==========================================================================
	入力待ちの処理
	・モニタコマンドからの要求を処理する
============================================================================*/
static void mdNormalAutModeInputWaitMon(T_MDCMDMSG *pCmdMsg) 
{
	switch (pCmdMsg->uhCmd) {

		break;	

	default:
		break;
	}//switch (pCmdMsg->uhCmd) {
}

/**
 *	認証モードのAck待ち状態
 */
static void mdNormalAutModeSndDatAckWait(T_MDCMDMSG *pCmdMsg)
{
	switch(pCmdMsg->idOrg) {
	case TSK_CMD_LAN:
		switch (pCmdMsg->uhCmd) {
		case MD_CMD_ACK_RCV:
			mdAutChangeStatus(MD_AUT_RES_WAIT);		// 応答待ち状態へ
			break;
		
		case MD_CMD_SND_FAIL:
			MdNormalAutInit();						// 入力待ち状態へ
			break;
		}								// switch (pCmdMsg->uhCmd)
		break;
	
	case TSK_IO:
		break;
		
	case TSK_CMD_MON:
		break;
	
	}//switch (pCmdMsg->idOrg)
}

/**
 *	認証モードの応答コマンド待ち状態
 */
static void mdNormalAutModeResWait(T_MDCMDMSG *pCmdMsg)
{
	switch(pCmdMsg->idOrg) {
	case TSK_CMD_LAN:
		switch (pCmdMsg->uhCmd) {
		case CMD_RES_C:
			if (!memcmp(pCmdMsg->cData, CMD_RES_OK, sizeof CMD_RES_OK)) {		// OKのとき
				s_iBlkNo = ;		// ブロック番号を設定
				CmmSendFinData( s_iBlkNo );				// 画像データを送信
				mdAutChangeStatus(MD_AUT_RES_WAIT);		// 応答待ち状態へ
			} else if (!memcmp(pCmdMsg->cData, CMD_RES_NG, sizeof CMD_RES_NG)) {	// NGのとき
				// LCDに認証できない表示
				
				MdNormalAutInit();						// 入力待ち状態へ
			} else {					// 異常データ
				CmmSendNak();			// 
			}
			break;
			
		case MD_CMD_EOT_RCV:
			MdNormalAutInit();							// 入力待ち状態へ
			break;

		default:						// ありえないはず
			break;
		}								// switch (pCmdMsg->uhCmd)
		break;
	
	case TSK_IO:
		break;
		
	case TSK_CMD_MON:
		break;
	
	}//switch (pCmdMsg->idOrg)
}

/**
 *	認証モードのAck待ち状態
 */
static void mdNormalAutModeSndBlkAckWait(T_MDCMDMSG *pCmdMsg)
{
	switch(pCmdMsg->idOrg) {
	case TSK_CMD_LAN:
		switch (pCmdMsg->uhCmd) {
		case MD_CMD_ACK_RCV:
			if ( s_iBlkNo ) {						// 送信データあるなら
				CmmSendFinData( s_iBlkNo );			// 画像データを送信
			} else {								// 最終ブロック送信完了したら
				mdAutChangeStatus(MD_AUT_JDG_WAIT);	// 指データの判定受信待ち状態へ
			}
			break;
		
		case MD_CMD_SND_FAIL:
			MdNormalAutInit();						// 入力待ち状態へ
			break;
			
		case MD_CMD_EOT_RCV:
			MdNormalAutInit();						// 入力待ち状態へ
			break;
		}								// switch (pCmdMsg->uhCmd)
		break;
	
	case TSK_IO:
		break;
		
	case TSK_CMD_MON:
		break;
	
	}//switch (pCmdMsg->idOrg)
}

/**
 *	認証モードの判定待ち状態
 */
static void mdNormalAutModeJdgWait(T_MDCMDMSG *pCmdMsg)
{
	switch(pCmdMsg->idOrg) {
	case TSK_CMD_LAN:
		switch (pCmdMsg->uhCmd) {
		case CMD_JDG_SND:
			if (!memcmp(pCmdMsg->cData, CMD_JDG_SND_OK, sizeof CMD_JDG_SND_OK)) {	// OKのとき
				CmmAckSend();									// ACK送信
				();// 認証OK表示
				mdAutChangeStatus(MD_AUT_EOT_WAIT);				// EOT受信待ち
			} else if (!memcmp(pCmdMsg->cData, CMD_JDG_SND_NG, sizeof CMD_JDG_SND_NG)) {	// NGのとき
				CmmAckSend();									// ACK送信
				mdAutChangeStatus(MD_AUT_PRM_WAIT);				// EOT受信待ち
			} else {
				CmmNakSend();									// NAK送信
			}
			break;
		
		case MD_CMD_SND_FAIL:
			MdNormalAutInit();									// 入力待ち状態へ
			break;
		}								// switch (pCmdMsg->uhCmd)
		break;
	
	case TSK_IO:
		break;
		
	case TSK_CMD_MON:
		break;
	
	}//switch (pCmdMsg->idOrg)
}

/**
 *	認証モードのパラメータ変更待ち状態
 */
static void mdNormalAutModeParamWait(T_MDCMDMSG *pCmdMsg)
{
	UH uhVal;

	switch(pCmdMsg->idOrg) {
	case TSK_CMD_LAN:
		switch (pCmdMsg->uhCmd) {
		case CMD_CMR_SET:							// カメラ・パラメータの変更要求
					// カメラパラメータの変更指令
			CmmAckSend();							// ACK送信
			LanResSend(PR_TYPE_D, CMD_RES_OK);		// 応答コマンド送信
			mdAutChangeStatus(MD_AUT_CMR_RTRY);		// 再撮影要求待ちへ
			break;
		
		case CMD_LGT_SET:							// LED光量変更要求
			uhVal = pCmdMsg->cData[ 0 ];
			uhVal <<= 8;
			uhVal |= (UH)pCmdMsg->cData[ 1 ];
			if (uhVal <= 255) {
								// LED光量変更
				CmmAckSend();						// ACK送信
				LanResSend(PR_TYPE_D, CMD_RES_OK);	// 応答コマンド送信
				mdAutChangeStatus(MD_AUT_CMR_RTRY);	// 再撮影要求待ちへ
			} else {
				CmmNakSend();						// NAK送信
			}
			break;
		
		case MD_CMD_SND_FAIL:
			MdNormalAutInit();						// 入力待ち状態へ
			break;
			
		case MD_CMD_EOT_RCV:
			mMdNormalAutInit();						// 入力待ち状態へ
			break;
		}								// switch (pCmdMsg->uhCmd)
		break;
	
	case TSK_IO:
		break;
		
	case TSK_CMD_MON:
		break;
	
	}//switch (pCmdMsg->idOrg)
}

/**
 *	認証モードの再撮影要求待ち
 */
static void mdNormalAutModeRetryWait(T_MDCMDMSG *pCmdMsg)
{
	switch(pCmdMsg->idOrg) {
	case TSK_CMD_LAN:
		switch (pCmdMsg->uhCmd) {
		case CMD_CMR_RTRY:							// 指データの再撮影要求
					// 再撮影指令
			CmmAckSend();							// ACK送信
			mdAutChangeStatus(MD_AUT_EOT_WAIT);		// EOT受信待ちへ
			break;
		
		case MD_CMD_SND_FAIL:
			MdNormalAutInit();						// 入力待ち状態へ
			break;

		case MD_CMD_EOT_RCV:
			MdNormalAutInit();						// 入力待ち状態へ
			break;
		}								// switch (pCmdMsg->uhCmd)
		break;
	
	case TSK_IO:
		break;
		
	case TSK_CMD_MON:
		break;
	
	}//switch (pCmdMsg->idOrg)
}

/**
 *	認証モードのEOT待ち状態
 */
static void mdNormalAutModeEotWait(T_MDCMDMSG *pCmdMsg)
{
	switch(pCmdMsg->idOrg) {
	case TSK_CMD_LAN:
		switch (pCmdMsg->uhCmd) {
		case MD_CMD_EOT_RCV:
			MdNormalAutInit();						// 入力待ち状態へ
			break;
		
		case MD_CMD_SND_FAIL:
			MdNormalAutInit();						// 入力待ち状態へ
			break;
		}								// switch (pCmdMsg->uhCmd)
		break;
	
	case TSK_IO:
		break;
		
	case TSK_CMD_MON:
		break;
	
	}//switch (pCmdMsg->idOrg)
}

