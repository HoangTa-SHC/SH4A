/**
*	VA-300プログラム
*
*	@file dbg_out.c
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2010/01/29
*	@brief  デバッグ用出力モジュール(他案件から流用)
*
*	Copyright (C) 2010, OYO Electric Corporation
*/
#define	_DBG_OUT_C_
#include "kernel.h"
#include "id.h"
//#include "drv_dsw.h"
#include "va300.h"

// デバッグ用出力関数
void DebugOut(char *pStr);

/*==========================================================================*/
/**
 * デバッグ用ポートから文字列を出力する
 *
 * @param pStr
 */
/*==========================================================================*/
void DebugOut(char *pStr)
{
	T_COMMSG *msg;
	int		cnt;
	ER		ercd;
	
	// DSW8がOFFのときは出力しない
//	if (!(DswGet() & DSW1_8)) {
//		return;
//	}
	
	ercd = tget_mpf(MPF_COM, (VP*)&msg, (10/MSEC));
	if( ercd == E_OK) {
		memset( &(msg->buf), 0, sizeof(msg->buf));
	
		cnt = strlen(pStr);
		
		if (cnt > sizeof(msg->buf)) {
			cnt = sizeof(msg->buf);
		}
		
		msg->cnt    = cnt;
		msg->msgpri = 1;
		memcpy( &(msg->buf), pStr, cnt);
	
		snd_mbx( MBX_SND_SIO, (T_MSG*)msg);
	}
}
