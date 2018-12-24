/**
*	UDP�ʐM�v���O����
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
#define	EXTERN							///< �v���g�^�C�v�錾
#else
#define	EXTERN	extern					///< �O���Q�Ɛ錾
#endif

/* Configuration */
#define	UDP_TSK_LVL		5				///< UDP�^�X�N���x��

/* Creation Information */
EXTERN TASK udprcv_tsk(ID);				///< UDP�^�X�N
EXTERN void put_udp(UB*, UH);			///< UDP���M
EXTERN ER get_udp(UB*, TMO);			///< UDP��M
EXTERN ER udp_ini(ID,ID,UH);			///< UDP�^�X�N�̏�����
EXTERN void udp_ext(void);				///< UDP�ʐM�ؒf
#endif									/* end of _UDP_H_ */
/* end */
