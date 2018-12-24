/**
*	VA-300�v���O����
*
*	@file dbg_out.c
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2010/01/29
*	@brief  �f�o�b�O�p�o�̓��W���[��(���Č����痬�p)
*
*	Copyright (C) 2010, OYO Electric Corporation
*/
#define	_DBG_OUT_C_
#include "kernel.h"
#include "id.h"
//#include "drv_dsw.h"
#include "va300.h"

// �f�o�b�O�p�o�͊֐�
void DebugOut(char *pStr);

/*==========================================================================*/
/**
 * �f�o�b�O�p�|�[�g���當������o�͂���
 *
 * @param pStr
 */
/*==========================================================================*/
void DebugOut(char *pStr)
{
	T_COMMSG *msg;
	int		cnt;
	ER		ercd;
	
	// DSW8��OFF�̂Ƃ��͏o�͂��Ȃ�
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
