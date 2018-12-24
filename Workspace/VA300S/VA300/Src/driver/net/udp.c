/**
*	UDP通信プログラム
*
*	@file udp.c
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2008/04/09
*	@brief  NORTiのnonecho.cから変更
*	@brief  FIFO_SZの指定でバッファサイズ指定
*
*	Copyright (C) 2008, OYO Electric Corporation
*/
#include <stdio.h>
#include <string.h>
#include "kernel.h"
#include "nonet.h"
#include "udp.h"
#include "drv_fpga.h"
#include "drv_dsw.h"
#include "va300.h"

#define	_UDP_C_

#ifndef FIFO_SZ						// FIFOのサイズ設定
#define FIFO_SZ		(8192 * 2)		// 2^nで指定
//#define FIFO_SZ		2048			// 2^nで指定
#endif
#ifndef BUFSZ
#define BUFSZ		8192			// 一時バッファサイズ設定
//#define BUFSZ		1024			// 一時バッファサイズ設定
#endif

#define UDP_REMOTE_IP_ADDR		0xc0a8019f	///< 相手先IPアドレス（192.168.1.159）
#define UDP_REMOTE_PORT		50000	///< 相手先受信ポート番号

const T_CTSK c_udprcv_tsk = { TA_HLNG, NULL, udprcv_tsk, UDP_TSK_LVL, 512, NULL, "udprcv_tsk" };

// Local Data
static UH udprcv_buf[BUFSZ/2];		///< 一時バッファ(UH and /2 for Word Aglinement)
static ID rcv_tskid;				///< 受信タスクのID
static ID udp_cepid;				///< 通信端点ID
static ID rxtid;					///< 受信待ちタスクID
static T_IPEP udp_dstaddr;			///< UDP送信先アドレス
static int udprcv_len;				///< 受信データ数

typedef struct t_buf{				///< FIFO構造体
	UH setp;						///< 格納ポインタ
	UH getp;						///< 取得ポインタ
	UH cnt;							///< 格納データ数
	UB buf[FIFO_SZ];				///< バッファ
}T_BUF;

static T_BUF udprcv;				///< 受信バッファ

extern UB default_ipaddr[];			// IPアドレス				

// プロトタイプ宣言
static void init_buf(void);			// 受信バッファの初期化
static ER udprcv_cbk(ID, FN, VP);	// 受信コールバック関数
static void put_rxbuf(void);		// 受信バッファへの書込み
static int get_rxbuf(UB*);			// 受信バッファからの取得

/*==========================================================================*/
/* UDP受信バッファクリア                                                    */
/*==========================================================================*/

static void init_buf(void)
{
	memset( &udprcv, 0, sizeof udprcv);
}

/*==========================================================================*/
/* UDP Server Callback                                                      */
/*==========================================================================*/

static ER udprcv_cbk(ID cepid, FN fncd, VP parblk)
{
	switch(fncd)
	{
	case TFN_UDP_RCV_DAT:
		break;
	case TEV_UDP_RCV_DAT:
		udprcv_len = udp_rcv_dat(cepid, &udp_dstaddr, udprcv_buf, sizeof udprcv_buf, TMO_POL);
		put_rxbuf();					// 割込み中で処理するように変更
		wup_tsk(rcv_tskid);
		break;
	}
	return E_OK;
}

/*==========================================================================*/
/**
 * UDP受信タスク
 * @param cepid 通信端点ID
 */
/*==========================================================================*/

TASK udprcv_tsk(ID cepid)
{
	ID id;
	
	rcv_tskid = vget_tid();
	udp_cepid = cepid;

	for (;;)
	{
		/* Waiting for receive udp echo packet */
		/* It weke uped from udpecho_cbk */
		memset(udprcv_buf, 0, sizeof(udprcv_buf));
		slp_tsk();
		
		// 受信待ち解除
		if((id = rxtid) != 0) {
			rxtid = 0;
			wup_tsk((ID)id);
		}
	}
}

/*==========================================================================*/
/* 受信バッファへ受信文字/ステータス格納（内部関数）                        */
/*==========================================================================*/

static void put_rxbuf(void)
{
	UB *p;
	int ln;

	p  = (UB*)udprcv_buf;	
	ln = udprcv_len;
	
	while( ln) {
		// バッファ満杯チェック
		if (udprcv.cnt == FIFO_SZ)
			return;
		
		// バッファへ格納
		udprcv.buf[udprcv.setp] = *p;		// バッファへ格納
		udprcv.cnt++;						// バッファデータ数＋１
		p++;								// ポインタインクリメント
		
		// 格納ポインタを１つ進める
		udprcv.setp = (++udprcv.setp) & (FIFO_SZ - 1);
		
		ln--;
	}
}

/*==========================================================================*/
/* 受信バッファから１文字取得（内部関数）                                   */
/*                                                                          */
/* バッファ空で取得できなかった場合は、-1 を返す。                          */
/*==========================================================================*/

static int get_rxbuf(UB *c)
{
	int cnt;

	// 受信バッファ空チェック
	cnt = udprcv.cnt;
	if (--cnt == -1)
		return cnt;

	udprcv.cnt = (UH)cnt;				// 受信バッファ内データ数-1
	*c = udprcv.buf[ udprcv.getp ];		// 受信バッファから取得

	// 取得ポインタを１つ進める
	udprcv.getp = (++udprcv.getp) & (FIFO_SZ - 1);

	return 0;
}

/*==========================================================================*/
/**
 * UDPでの送信
 * @param *s 送信データ
 * @param n 送信データ数
 */
/*==========================================================================*/

void put_udp(UB *s, UH n)
{
	ER ercd;
	
	udp_dstaddr.ipaddr = UDP_REMOTE_IP_ADDR;		// 相手先IPアドレス
	udp_dstaddr.portno = UDP_REMOTE_PORT;			// 相手先ポート番号

	ercd = udp_snd_dat(udp_cepid, &udp_dstaddr, s, n, TMO_FEVR);
}

/*==========================================================================*/
/**
 * UDPでの受信
 * @param *c 受信データ格納ポインタ
 * @param tmout 受信タイムアウト
 * @retval E_OK 受信OK
 * @retval E_TMOUT タイムアウト
 */
/*==========================================================================*/

ER get_udp(UB *c, TMO tmout)
{
	ER ercd;
	int sts;

	for (;;)
	{
		// 受信バッファから１文字得る
		sts = get_rxbuf(c);
		if (sts != -1) {			// 受信データあった場合
			ercd = E_OK;
			return ercd;
		}

		// 受信割込み待ち
		rxtid = vget_tid();
		ercd = tslp_tsk(tmout);
		rxtid = 0;
		vcan_wup();					// 複数回 wup_tsk された場合の対策
		if (ercd)
			return ercd;			// タイムアウト終了
	}
}

/*==========================================================================*/
/* UDP 受信タスク Uninitialize                                              */
/*==========================================================================*/

void udp_ext(void)
{
	udp_can_cep(udp_cepid, TFN_TCP_ALL);
	udp_del_cep(udp_cepid);
	ter_tsk(rcv_tskid);
	del_tsk(rcv_tskid);
	rcv_tskid = udp_cepid = 0;
}

/*==========================================================================*/
/**
 * UDP通信初期化
 * @param tskid タスクID(0のときはID自動割当て)
 * @param cepid 通信端点ID(0のときはID自動割当て)
 * @param portno ポート番号
 * @retval E_OK 初期化成功
 */
/*==========================================================================*/

ER udp_ini(ID tskid, ID cepid, UH portno)
{
	ER ercd;
	static T_UDP_CCEP c_udp_cep;

	// UDP通信設定
	c_udp_cep.cepatr = 0;						// UDP通信端点属性(0)
	c_udp_cep.myaddr.ipaddr = IPV4_ADDRANY;		// 自分側のIPアドレス
	c_udp_cep.myaddr.portno = portno;			// UDPのポート番号
	c_udp_cep.callback = (FP)udprcv_cbk;		// コールバック関数の登録

	// 受信バッファ初期化
	init_buf();

	// Create UDP Recieve Task
	if(tskid == 0){/* ID auto (Add by Y.Y) */
		ercd = acre_tsk(&c_udprcv_tsk);
		if (ercd < 0)
			return ercd;
		tskid = ercd;
	}
	else{
		ercd = cre_tsk(tskid, &c_udprcv_tsk);
		if (ercd != E_OK)
			return ercd;
	}

	// Create UDP Commnunication End Point
	if(cepid == 0)
	{   ercd = cepid = udp_vcre_cep(&c_udp_cep);
		if(cepid <= 0)
			goto ERR;
	}
	else
	{   ercd = udp_cre_cep(cepid, &c_udp_cep);
		if (ercd != E_OK)
			goto ERR;
	}

	ercd = udp_set_opt(cepid, SET_UDP_RPKT_OPT, (VP)TRUE, sizeof(BOOL));
	if (ercd != E_OK) {
		goto ERR;
	}

	// Start Receive Server Task
	ercd = sta_tsk(tskid, cepid);
	if (ercd != E_OK)
		goto ERR;

	return E_OK;

ERR:
	udp_del_cep(cepid);
	del_tsk(tskid);
	return ercd;

}

/* end */
