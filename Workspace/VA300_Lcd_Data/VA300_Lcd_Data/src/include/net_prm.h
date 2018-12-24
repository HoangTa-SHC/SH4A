/**
*	VA-300プログラム
*
*	@file net_prm.h
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/08/01
*	@brief  ネットワークパラメータ定義情報(他案件から流用)
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
#ifndef	_NET_PRM_H_
#define	_NET_PRM_H_

// ネットワークパラメータ
// 補足：MACアドレスが書き込まれていないときにここで設定されているMACアドレスの値が
//       使用されます。通常での使用はNGです。
//       IPアドレスなどはデフォルト設定モードのときに使用される値です。
static const UB ini_mac_addr[] = { 0x00, 0x0C, 0x67, 0x00, 0xB0, 0x02};	///< 非常用のMACアドレス
																		// MACアドレスは
																		// 00-0C-67-00-B0-00～
																		// 00-0C-67-00-B0-0Fまでは開発用
static const UB ini_ipaddr[]   = {  192,  168,   /*50*/ 1,  158};				///< IPアドレス
static const UB ini_mask[]     = {  255,  255,  255,    0};				///< サブネットマスク

static const UH default_udp_port = 50000;								///< UDPのデフォルトポート
static const UH default_telnet_port = 23;								///< TELNETのデフォルトポート

// ログインパスワードの設定
#define	LOGIN_ID	"VA300"			///< ログインID
#define	LOGIN_PASS	"VA300"			///< パスワード

#endif								// end of _NET_PRM_H_