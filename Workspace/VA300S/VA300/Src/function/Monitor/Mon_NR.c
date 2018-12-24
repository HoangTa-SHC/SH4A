//=============================================================================
/**
 *	ＳＨ-４ ＣＰＵモジュール モニタプログラム
 *	
 *	@file Mon_NR.c
 *	@version 1.00
 *	
 *	@author	F.Saeki
 *	@date	2007/12/10
 *	@brief	MAC、IPアドレス、ポート番号、サブネットマスク読込みコマンドモジュール
 *
 *	Copyright (C) 2007, OYO Electric Corporation
 */
//=============================================================================
#include "kernel.h"
#include "nonet.h"
#include "nonteln.h"
#include "nonethw.h"
#include "mon.h"
#include "lan9118.h"
#include "drv_eep.h"

//=============================================================================
/**
 * ＮＲコマンドの実行
 */
//=============================================================================
void CmdNR(void)
{
	UB	mac_addr[6];
	UB	ip_addr[4];
	UB	mask[4];
	UH	portno;
	UH	idno;

	Ctrl.Proc = ReadCmd;						// コマンド入力に戻る
	
	lan_get_mac( mac_addr);
	lan_get_ip( ip_addr);
	lan_get_mask( mask);
	lan_get_port( &portno);
	lan_get_id( &idno );
	
	i_sprintf( Ctrl.TxText, "MAC ADDRESS  = %02X-%02X-%02X-%02X-%02X-%02X\r\n", 
					mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
	SendText( Ctrl.TxText);
	i_sprintf( Ctrl.TxText, "IP ADDRESS   = %d.%d.%d.%d\r\n", ip_addr[0], ip_addr[1], ip_addr[2], ip_addr[3]);
	SendText( Ctrl.TxText);
	i_sprintf( Ctrl.TxText, "SUB NET MASK = %d.%d.%d.%d\r\n", mask[0], mask[1], mask[2], mask[3]);
	SendText( Ctrl.TxText);
	i_sprintf( Ctrl.TxText, "PORT No.     = %d\r\n", portno);
	SendText( Ctrl.TxText);
	i_sprintf( Ctrl.TxText, "ID No.       = %d\r\n", idno);
	SendText( Ctrl.TxText);
	
	// 制御盤側の設定
	lan_get_ctrlip( ip_addr );
	lan_get_ctrlpt( &portno );
	
	SendText("<Control Box>\r\n");
	i_sprintf( Ctrl.TxText, "IP ADDRESS   = %d.%d.%d.%d\r\n", ip_addr[0], ip_addr[1], ip_addr[2], ip_addr[3]);
	SendText( Ctrl.TxText);
	i_sprintf( Ctrl.TxText, "PORT No.     = %d\r\n", portno);
	SendText( Ctrl.TxText);
}
