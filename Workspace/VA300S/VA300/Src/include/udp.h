/**
*	UDP通信プログラム
*
*	@file udp.h
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2008/04/09
*
*	Copyright (C) 2008, OYO Electric Corporation
*/
#ifndef	_UDP_H_
#define	_UDP_H_
#include "kernel.h"

#if defined(EXTERN)
#undef	EXTERN
#endif
#if defined(_UDP_C_)
#define	EXTERN							///< プロトタイプ宣言
#else
#define	EXTERN	extern					///< 外部参照宣言
#endif

/* Configuration */
#define	UDP_TSK_LVL		5				///< UDPタスクレベル

/* Creation Information */
EXTERN TASK udprcv_tsk(ID);				///< UDPタスク
EXTERN void put_udp(UB*, UH);			///< UDP送信
EXTERN ER get_udp(UB*, TMO);			///< UDP受信
EXTERN ER udp_ini(ID,ID,UH);			///< UDPタスクの初期化
EXTERN void udp_ext(void);				///< UDP通信切断
#endif									/* end of _UDP_H_ */
/* end */
