/**
*	VA-300プログラム
*
*	@file drv_eep.h
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/09/10
*	@brief  EEPROM定義情報
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
#ifndef	_DRV_EEP_H_
#define	_DRV_EEP_H_

// マクロ定義
#define	EEP_IP_ADDR		0x40			///< IPアドレス
#define	EEP_NET_MASK	0x44			///< サブネットマスク
#define	EEP_PORT_NO		0x48			///< ポート番号
#define	EEP_TERM_NO		0x4A			///< 端末番号
#define	EEP_CTRL_ADDR	0x50			///< 制御盤IPアドレス
#define	EEP_CTRL_PORT	0x54			///< 制御盤ポート番号

#define	lan_get_ip(n)		lan_get_4byte(EEP_IP_ADDR,n)	///< IPアドレスの読込み
#define	lan_set_ip(n)		lan_set_4byte(EEP_IP_ADDR,n)	///< IPアドレスの書込み
#define	lan_get_mask(n)		lan_get_4byte(EEP_NET_MASK,n)	///< サブネットマスクの読込み
#define	lan_set_mask(n)		lan_set_4byte(EEP_NET_MASK,n)	///< サブネットマスクの書込み
#define	lan_get_ctrlip(n)	lan_get_4byte(EEP_CTRL_ADDR,n)	///< 制御盤IPアドレスの読込み
#define	lan_set_ctrlip(n)	lan_set_4byte(EEP_CTRL_ADDR,n)	///< 制御盤IPアドレスの書込み

#define	lan_get_port(n)		lan_get_word(EEP_PORT_NO,n)		///< ポート番号の読込み
#define	lan_set_port(n)		lan_set_word(EEP_PORT_NO,n)		///< ポート番号の書込み
#define	lan_get_ctrlpt(n)	lan_get_word(EEP_CTRL_PORT,n)	///< 制御盤ポート番号の読込み
#define	lan_set_ctrlpt(n)	lan_set_word(EEP_CTRL_PORT,n)	///< 制御盤ポート番号の書込み
#define	lan_get_id(n)		lan_get_word(EEP_TERM_NO,n)		///< 機器番号の読込み
#define	lan_set_id(n)		lan_set_word(EEP_TERM_NO,n)		///< 機器番号の書込み

// EXTERN宣言の定義
#ifdef EXTERN
#undef EXTERN
#endif

#if defined(_DRV_EEP_C_)
#define	EXTERN
#else
#define	EXTERN extern
#endif

// プロトタイプ宣言
EXTERN ER lan_get_4byte(UH addr, UB *pval);		///< 4バイトデータの読出し
EXTERN ER lan_set_4byte(UH addr, UB *pval);		///< 4バイトデータの書込み
EXTERN ER lan_get_word(UH addr, UH *pval);		///< 2バイトデータの読出し
EXTERN ER lan_set_word(UH addr, UH val);		///< 2バイトデータの書込み

#endif										/* end of _DRV_EEP_H_				*/
